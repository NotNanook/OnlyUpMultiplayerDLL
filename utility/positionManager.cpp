#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <chrono>
#include <future>

#include "positionManager.h"

#pragma comment(lib,"ws2_32.lib") 

#define PORT 50001

WSADATA ws;
sockaddr_in server;
int clientSocket;

void PositionManager::init() {
	PositionManager::baseModule = GetModuleHandle("OnlyUP-Win64-Shipping.exe");

}

void PositionManager::networkLoop() {
	static const std::chrono::nanoseconds targetDuration(50000000);
	int numPacketsSent = 0;

	struct timeval timeout;
	fd_set fds;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;

	FD_ZERO(&fds);
	FD_SET(clientSocket, &fds);

	while (true) {
		auto startTime = std::chrono::steady_clock::now();

		char buffer[sizeof(PositionPacket)] = {};
		PositionPacket packet = PositionPacket();
		packet.num = numPacketsSent;
		packet.packetType = PositionUpdate;
		memcpy(&packet.playerName, PositionManager::username, sizeof(PositionManager::username));

		// Find player position
		uintptr_t xCordPointer = util::FindDMAAddy(((uintptr_t)PositionManager::baseModule) + 0x074685C0, {0x48, 0x70, 0x260});
		if (xCordPointer == NULL) { continue; }
		double xCord = *(double*)xCordPointer;
		double zCord = *(double*)(xCordPointer + 0x8);
		double yCord = *(double*)(xCordPointer + 0x10);

		PositionManager::localPlayerPos.x = xCord;
		PositionManager::localPlayerPos.y = yCord;
		PositionManager::localPlayerPos.z = zCord;

		packet.position = PositionManager::localPlayerPos;

		memcpy(&buffer, &packet, sizeof(PositionPacket));
		if (sendto(clientSocket, buffer, sizeof(PositionPacket), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code: %d\n", WSAGetLastError());
			break;
		}

		numPacketsSent++;
		PositionManager::connected = true;
		PositionManager::lobbyFull = false;

		sockaddr_in clientAddr;
		int clientAddrLen = sizeof(clientAddr);
		ZeroMemory(&buffer, sizeof(PositionPacket));

		int result = select(0, &fds, 0, 0, &timeout);
		if (result == SOCKET_ERROR || result == 0) {
			continue;
		}

		int bytesRead = recvfrom(clientSocket, buffer, sizeof(PositionPacket), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if (bytesRead == SOCKET_ERROR) {
			printf("Failed to receive response!\n");
			break;
		}

		PositionPacket recvPacket = {};
		memcpy(&recvPacket, &buffer, sizeof(PositionPacket));

		if (strlen(recvPacket.playerName) == 0) {
			PositionManager::lobbyFull = true;
			break;
		}

		PositionManager::m.lock();
		memcpy(&PositionManager::mirrorPlayer.playerName, &recvPacket.playerName, strlen(recvPacket.playerName));
		PositionManager::mirrorPlayer.lastPosition = recvPacket.position;
		printf("X: %f\n", PositionManager::mirrorPlayer.lastPosition.x);
		PositionManager::m.unlock();

		auto elapsedTime = std::chrono::steady_clock::now() - startTime;
		auto remainingTime = targetDuration - elapsedTime;
		if (remainingTime > std::chrono::nanoseconds::zero()) {
			std::this_thread::sleep_for(remainingTime);
		}
	}

	PositionManager::disconnect();
}

int PositionManager::setupNetwork() {
	printf("[i] Initialising Winsock...\n");
	if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
	{
		printf("[!] Failed. Error Code: %d\n", WSAGetLastError());
		return 1;
	}
	printf("[+] Initialised.\n");

	if ((clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("[!] socket() failed with error code: %d\n", WSAGetLastError());
		return 2;
	}

	ZeroMemory(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	int result = inet_pton(AF_INET, PositionManager::ipAddress, &server.sin_addr);
	if (result == 0 || result == -1) {
		printf("[!] IP not valid: %d\n", WSAGetLastError());
		return 3;
	}
	printf("[+] IP set\n");

	std::thread t1(PositionManager::networkLoop);
	t1.detach();

	return 0;
}

void PositionManager::disconnect() {
	closesocket(clientSocket);
	WSACleanup();
	PositionManager::connected = false;
}