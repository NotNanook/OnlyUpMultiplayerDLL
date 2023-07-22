#pragma once
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <cstdint>
#pragma comment(lib, "d3d12.lib")

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/imgui_impl_win32.h"

struct _FrameContext {
	ID3D12CommandAllocator* CommandAllocator;
	ID3D12Resource* Resource;
	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
};

class DirectX
{
public:
	static bool isWindowFocused();
	static void getWindowInformation();
	static void hookDirectX();
	static void DisableHooks();

	static HRESULT APIENTRY hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
	static LRESULT APIENTRY WndProcFunc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	typedef HRESULT(APIENTRY* Present12) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	inline static Present12 oPresent;

	typedef void(APIENTRY* DrawInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
	inline static DrawInstanced oDrawInstanced;

	typedef void(APIENTRY* DrawIndexedInstanced)(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
	inline static DrawIndexedInstanced oDrawIndexedInstanced;

	typedef void(APIENTRY* ExecuteCommandLists)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);
	inline static ExecuteCommandLists oExecuteCommandLists;

	static void hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);
	static void hkDrawInstanced(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
	static void hkDrawIndexedInstanced(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);

	inline static bool ShowMenu = false;
	inline static bool ImGui_Initialised = false;

	inline static WNDCLASSEX WindowClass;
	inline static HWND WindowHwnd;

	inline static uint64_t* MethodsTable = NULL;

	inline static DWORD ID;
	inline static HANDLE Handle;
	inline static HWND Hwnd;
	inline static HMODULE Module;
	inline static WNDPROC WndProc;
	inline static int WindowWidth;
	inline static int WindowHeight;
	inline static LPCSTR Title;
	inline static LPCSTR ClassName;
	inline static LPCSTR Path;

	inline static ID3D12Device* Device = nullptr;
	inline static ID3D12DescriptorHeap* DescriptorHeapBackBuffers;
	inline static ID3D12DescriptorHeap* DescriptorHeapImGuiRender;
	inline static ID3D12GraphicsCommandList* CommandList;
	inline static ID3D12CommandQueue* CommandQueue;

	inline static uint64_t BuffersCounts = -1;
	inline static _FrameContext* FrameContext;

private:
	static bool Init();
	static bool InitWindow();
	static bool DeleteWindow();
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);