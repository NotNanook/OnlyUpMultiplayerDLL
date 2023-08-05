#pragma once
#define WIN32_LEAN_AND_MEAN
#include "Minhook/include/MinHook.h"

#include <vector>
#include <iostream>
#include <mutex>
#include <cmath>
#include <DirectXMath.h>

enum PacketType {
	PositionUpdate
};

struct Vec2 {
	double x;
	double y;

	Vec2() : x(0), y(0) {}
	Vec2(double X, double Y) : x(X), y(Y) {}
};

struct Vec3 {
	double x;
	double y;
	double z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(double X, double Y, double Z): x(X), y(Y), z(Z) {}
};

struct Vec4 {
	double x;
	double y;
	double z;
	double w;

	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(double X, double Y, double Z, double W) : x(X), y(Y), z(Z), w(W) {}
};

struct PositionPacket {
	PacketType packetType;
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
	static DirectX::XMFLOAT2 WorldToScreen(const DirectX::XMVECTOR& worldPos, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix, const UINT viewportWidth, const UINT viewportHeight);
	static float convertHorizontalToVerticalFOV(float horizontalFOV, float aspectRatio);
	static Vec3 vectorSubtract(Vec3 vec1, Vec3 vec2);
	static Vec3 crossProduct(const Vec3 a, const Vec3 b);
	static Vec3 getSafeNormal(const Vec3 vec, float tolerance = 1e-8);
	static float dotProduct(const Vec3 vec1, const Vec3 vec2);
};