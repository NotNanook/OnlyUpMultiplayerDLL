#include "DirectX.h"
#include "Gui.h"
#include "positionManager.h"
#include <Psapi.h>

bool DirectX::isWindowFocused() {
	DWORD ForegroundWindowProcessID;
	GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
	return GetCurrentProcessId() == ForegroundWindowProcessID;
}

void DirectX::getWindowInformation() {
	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetCurrentProcessId() == ForegroundWindowProcessID) {

			DirectX::ID = GetCurrentProcessId();
			DirectX::Handle = GetCurrentProcess();
			DirectX::Hwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(DirectX::Hwnd, &TempRect);
			DirectX::WindowWidth = TempRect.right - TempRect.left;
			DirectX::WindowHeight = TempRect.bottom - TempRect.top;

			char TempTitle[MAX_PATH];
			GetWindowText(DirectX::Hwnd, TempTitle, sizeof(TempTitle));
			DirectX::Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(DirectX::Hwnd, TempClassName, sizeof(TempClassName));
			DirectX::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(DirectX::Handle, NULL, TempPath, sizeof(TempPath));
			DirectX::Path = TempPath;

			WindowFocus = true;
		}
	}
}

void DirectX::hookDirectX() {
	bool InitHook = false;
	while (InitHook == false) {
		if (DirectX::Init() == true) {
			util::CreateHook(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists);
			util::CreateHook(140, (void**)&oPresent, hkPresent);
			util::CreateHook(84, (void**)&oDrawInstanced, hkDrawInstanced);
			util::CreateHook(85, (void**)&oDrawIndexedInstanced, hkDrawIndexedInstanced);
			InitHook = true;
		}
	}
}

bool DirectX::Init() {
	if (InitWindow() == false) {
		return false;
	}

	HMODULE D3D12Module = GetModuleHandle("d3d12.dll");
	HMODULE DXGIModule = GetModuleHandle("dxgi.dll");
	if (D3D12Module == NULL || DXGIModule == NULL) {
		DeleteWindow();
		return false;
	}

	void* CreateDXGIFactory = GetProcAddress(DXGIModule, "CreateDXGIFactory");
	if (CreateDXGIFactory == NULL) {
		DeleteWindow();
		return false;
	}

	IDXGIFactory* Factory;
	if (((long(__stdcall*)(const IID&, void**))(CreateDXGIFactory))(__uuidof(IDXGIFactory), (void**)&Factory) < 0) {
		DeleteWindow();
		return false;
	}

	IDXGIAdapter* Adapter;
	if (Factory->EnumAdapters(0, &Adapter) == DXGI_ERROR_NOT_FOUND) {
		DeleteWindow();
		return false;
	}

	void* D3D12CreateDevice = GetProcAddress(D3D12Module, "D3D12CreateDevice");
	if (D3D12CreateDevice == NULL) {
		DeleteWindow();
		return false;
	}

	ID3D12Device* Device;
	if (((long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**))(D3D12CreateDevice))(Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&Device) < 0) {
		DeleteWindow();
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC QueueDesc;
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Priority = 0;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	QueueDesc.NodeMask = 0;

	ID3D12CommandQueue* CommandQueue;
	if (Device->CreateCommandQueue(&QueueDesc, __uuidof(ID3D12CommandQueue), (void**)&CommandQueue) < 0) {
		DeleteWindow();
		return false;
	}

	ID3D12CommandAllocator* CommandAllocator;
	if (Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&CommandAllocator) < 0) {
		DeleteWindow();
		return false;
	}

	ID3D12GraphicsCommandList* CommandList;
	if (Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&CommandList) < 0) {
		DeleteWindow();
		return false;
	}

	DXGI_RATIONAL RefreshRate;
	RefreshRate.Numerator = 60;
	RefreshRate.Denominator = 1;

	DXGI_MODE_DESC BufferDesc;
	BufferDesc.Width = 100;
	BufferDesc.Height = 100;
	BufferDesc.RefreshRate = RefreshRate;
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SAMPLE_DESC SampleDesc;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	SwapChainDesc.BufferDesc = BufferDesc;
	SwapChainDesc.SampleDesc = SampleDesc;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.OutputWindow = WindowHwnd;
	SwapChainDesc.Windowed = 1;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain* SwapChain;
	if (Factory->CreateSwapChain(CommandQueue, &SwapChainDesc, &SwapChain) < 0) {
		DeleteWindow();
		return false;
	}

	MethodsTable = (uint64_t*)::calloc(150, sizeof(uint64_t));
	memcpy(MethodsTable, *(uint64_t**)Device, 44 * sizeof(uint64_t));
	memcpy(MethodsTable + 44, *(uint64_t**)CommandQueue, 19 * sizeof(uint64_t));
	memcpy(MethodsTable + 44 + 19, *(uint64_t**)CommandAllocator, 9 * sizeof(uint64_t));
	memcpy(MethodsTable + 44 + 19 + 9, *(uint64_t**)CommandList, 60 * sizeof(uint64_t));
	memcpy(MethodsTable + 44 + 19 + 9 + 60, *(uint64_t**)SwapChain, 18 * sizeof(uint64_t));

	MH_Initialize();
	Device->Release();
	Device = NULL;
	CommandQueue->Release();
	CommandQueue = NULL;
	CommandAllocator->Release();
	CommandAllocator = NULL;
	CommandList->Release();
	CommandList = NULL;
	SwapChain->Release();
	SwapChain = NULL;
	DeleteWindow();
	return true;
}

bool DirectX::InitWindow() {
	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = DefWindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.hIcon = NULL;
	WindowClass.hCursor = NULL;
	WindowClass.hbrBackground = NULL;
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = "MJ";
	WindowClass.hIconSm = NULL;
	RegisterClassEx(&WindowClass);
	WindowHwnd = CreateWindow(WindowClass.lpszClassName, "DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, WindowClass.hInstance, NULL);
	if (WindowHwnd == NULL) {
		return false;
	}
	return true;
}

bool DirectX::DeleteWindow() {
	DestroyWindow(WindowHwnd);
	UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
	if (WindowHwnd != NULL) {
		return false;
	}
	return true;
}

LRESULT APIENTRY DirectX::WndProcFunc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (DirectX::ShowMenu) {
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
		return true;
	}
	return CallWindowProc(DirectX::WndProc, hwnd, uMsg, wParam, lParam);
}

HRESULT APIENTRY DirectX::hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
	if (!DirectX::ImGui_Initialised) {
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&DirectX::Device))) {
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO(); (void)io;
			ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			DXGI_SWAP_CHAIN_DESC Desc;
			pSwapChain->GetDesc(&Desc);
			Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			Desc.OutputWindow = DirectX::Hwnd;
			Desc.Windowed = ((GetWindowLongPtr(DirectX::Hwnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

			DirectX::BuffersCounts = Desc.BufferCount;
			DirectX::FrameContext = new _FrameContext[DirectX::BuffersCounts];

			D3D12_DESCRIPTOR_HEAP_DESC DescriptorImGuiRender = {};
			DescriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			DescriptorImGuiRender.NumDescriptors = DirectX::BuffersCounts;
			DescriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			if (DirectX::Device->CreateDescriptorHeap(&DescriptorImGuiRender, IID_PPV_ARGS(&DirectX::DescriptorHeapImGuiRender)) != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			ID3D12CommandAllocator* Allocator;
			if (DirectX::Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)) != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			for (size_t i = 0; i < DirectX::BuffersCounts; i++) {
				DirectX::FrameContext[i].CommandAllocator = Allocator;
			}

			if (DirectX::Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator, NULL, IID_PPV_ARGS(&DirectX::CommandList)) != S_OK ||
				DirectX::CommandList->Close() != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			D3D12_DESCRIPTOR_HEAP_DESC DescriptorBackBuffers;
			DescriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			DescriptorBackBuffers.NumDescriptors = DirectX::BuffersCounts;
			DescriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			DescriptorBackBuffers.NodeMask = 1;

			if (DirectX::Device->CreateDescriptorHeap(&DescriptorBackBuffers, IID_PPV_ARGS(&DirectX::DescriptorHeapBackBuffers)) != S_OK)
				return oPresent(pSwapChain, SyncInterval, Flags);

			const auto RTVDescriptorSize = DirectX::Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = DirectX::DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

			for (size_t i = 0; i < DirectX::BuffersCounts; i++) {
				ID3D12Resource* pBackBuffer = nullptr;
				DirectX::FrameContext[i].DescriptorHandle = RTVHandle;
				pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
				DirectX::Device->CreateRenderTargetView(pBackBuffer, nullptr, RTVHandle);
				DirectX::FrameContext[i].Resource = pBackBuffer;
				RTVHandle.ptr += RTVDescriptorSize;
			}

			ImGui_ImplWin32_Init(DirectX::Hwnd);
			ImGui_ImplDX12_Init(DirectX::Device, DirectX::BuffersCounts, DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::DescriptorHeapImGuiRender, DirectX::DescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(), DirectX::DescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
			ImGui_ImplDX12_CreateDeviceObjects();
			ImGui::GetIO().ImeWindowHandle = DirectX::Hwnd;
			DirectX::WndProc = (WNDPROC)SetWindowLongPtr(DirectX::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProcFunc);
			Gui::setStyle();
		}
		ImGui_Initialised = true;
	}

	if (DirectX::CommandQueue == nullptr)
		return oPresent(pSwapChain, SyncInterval, Flags);


	if (GetAsyncKeyState(VK_RSHIFT) & 1) ShowMenu = !ShowMenu;
	if ((GetAsyncKeyState(VK_ESCAPE) & 1) && ShowMenu) ShowMenu = !ShowMenu;
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::GetIO().MouseDrawCursor = ShowMenu;
	if (ShowMenu == true) {
		Gui::drawGui();
	}
	Gui::drawUi();
	ImGui::EndFrame();

	_FrameContext& CurrentFrameContext = DirectX::FrameContext[pSwapChain->GetCurrentBackBufferIndex()];
	CurrentFrameContext.CommandAllocator->Reset();

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.pResource = CurrentFrameContext.Resource;
	Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	DirectX::CommandList->Reset(CurrentFrameContext.CommandAllocator, nullptr);
	DirectX::CommandList->ResourceBarrier(1, &Barrier);
	DirectX::CommandList->OMSetRenderTargets(1, &CurrentFrameContext.DescriptorHandle, FALSE, nullptr);
	DirectX::CommandList->SetDescriptorHeaps(1, &DirectX::DescriptorHeapImGuiRender);

	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DirectX::CommandList);
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	DirectX::CommandList->ResourceBarrier(1, &Barrier);
	DirectX::CommandList->Close();
	DirectX::CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&DirectX::CommandList));
	return oPresent(pSwapChain, SyncInterval, Flags);
}



void DirectX::hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists) {
	if (!DirectX::CommandQueue)
		DirectX::CommandQueue = queue;

	DirectX::oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
}

void APIENTRY DirectX::hkDrawInstanced(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {

	return DirectX::oDrawInstanced(dCommandList, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void APIENTRY DirectX::hkDrawIndexedInstanced(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
	return DirectX::oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}