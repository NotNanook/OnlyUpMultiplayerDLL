#pragma once
#define WIN32_LEAN_AND_MEAN
#include "Minhook/include/MinHook.h"
#include <vector>
#include <iostream>

enum PacketType {
	PositionUpdate
};

struct Vec3 {
	double x;
	double y;
	double z;
};

struct PositionPacket {
	PacketType packetType;
	uint32_t num;
	char playerName[32];
	Vec3 position;
};

struct MirrorPlayer {
	char playerName[32];
	Vec3 lastPosition;
};

class util {
public:
	static bool CreateHook(uint16_t Index, void** Original, void* Function);
	static void DisableHook(uint16_t Index);
	static void DisableAll();
	static void createDebugConsole();
	static uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);
};