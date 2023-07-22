#include "DirectX.h"
#include "positionManager.h"

DWORD WINAPI MainThread(LPVOID lpParameter) {
	DirectX::getWindowInformation();
	DirectX::hookDirectX();
	util::createDebugConsole();
	PositionManager::init();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DirectX::Module = hModule;
		CreateThread(0, 0, MainThread, 0, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		FreeLibraryAndExitThread(hModule, TRUE);
		util::DisableAll();
		break;
	}
	return TRUE;
}