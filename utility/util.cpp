#include "util.h"
#include "DirectX.h"

bool util::CreateHook(uint16_t Index, void** Original, void* Function) {
	assert(_index >= 0 && _original != NULL && _function != NULL);
	void* target = (void*)DirectX::MethodsTable[Index];
	if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK) {
		return false;
	}
	return true;
}

void util::DisableHook(uint16_t Index) {
	assert(Index >= 0);
	MH_DisableHook((void*)DirectX::MethodsTable[Index]);
}

void util::DisableAll() {
	MH_DisableHook(MH_ALL_HOOKS);
	free(DirectX::MethodsTable);
	DirectX::MethodsTable = NULL;
}
void util::createDebugConsole() {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
}

bool isPtrInvalid(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	if (::VirtualQuery(p, &mbi, sizeof(mbi)))
	{
		DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
		bool b = !(mbi.Protect & mask);
		// check the page is not a guard page
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

		return b;
	}
	return true;
}

uintptr_t util::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	if (addr == 0) { return 0; }

	for (unsigned int i = 0; i < offsets.size(); ++i) {
		// Check if the current address is valid
		if (isPtrInvalid((void*)addr)) {
			return 0;
		}

		addr = *(uintptr_t*)addr;
		if (addr == 0) { return 0; }
		addr += offsets[i];
	}
	return addr;
}