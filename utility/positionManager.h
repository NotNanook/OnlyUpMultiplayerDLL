#pragma once
#include "util.h"

class PositionManager {
public:
	inline static uintptr_t baseAddress;
	inline static Vec3 localPlayerPos;
	inline static MirrorPlayer mirrorPlayer;
	inline static char username[32] = "";
	inline static char ipAddress[15] = "";
	inline static bool connected = false;
	inline static bool lobbyFull = false;

	static void init();
	static int setupNetwork();
	static void networkLoop();
	static void disconnect();
};