#pragma once
#include "util.h"

class PositionManager {
public:
	inline static HMODULE baseModule;
	inline static Vec3 localPlayerPos;
	inline static MirrorPlayer mirrorPlayer;
	inline static char username[32] = "";
	inline static char ipAddress[16] = "";
	inline static bool connected = false;
	inline static bool lobbyFull = false;
	inline static std::mutex m;

	static void init();
	static int setupNetwork();
	static void networkLoop();
	static void disconnect();
};