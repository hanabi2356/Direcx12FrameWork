#include"Common.h"
#include"Window.h"
#include"TimeManager.h"
#include"InputManager.h"
#include"EngineGraphicsCore.h"
#include"ComponentHeader.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
	
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	const int width = 1280;
	const int height = 720;

	Window window(hInstance, width, height);
	if (window.Create() == false) return -1;

	auto* graphics = EngineGraphicsCore::GetInstance();
	if (graphics->Initialize(window.GetHWND(), width, height) == false)return -1;

	auto* input = InputManager::GetInstance();
	input->Initialize(width, height);

	auto* time = TimeManager::GetInstance();
	time->Initialize();

	Camera camera;
	camera.SetAspectRatio(static_cast<float>(width) / height);
	camera.SetPosition(XMVectorSet(0.0f, 1.0f, -5.0f, 1.0f));
	camera.SetLookAt(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));

	// ---- ImGui Test ----
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window.GetHWND());

	ID3D12DescriptorHeap* srvHeap = graphics->GetImGuiSrvHeap(); // 직접 만든 SRV 힙

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = graphics->GetDevice();
	initInfo.CommandQueue = graphics->GetCommandQueue(); // 필수
	initInfo.NumFramesInFlight = 2;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	initInfo.SrvDescriptorHeap = srvHeap;
	initInfo.LegacySingleSrvCpuDescriptor = srvHeap->GetCPUDescriptorHandleForHeapStart();
	initInfo.LegacySingleSrvGpuDescriptor = srvHeap->GetGPUDescriptorHandleForHeapStart();
	ImGui_ImplDX12_Init(&initInfo);

	XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	camera.SetBackgroundColor(color);

	

	while (window.ProcessMessages())
	{
		time->Update();
		camera.Update();

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(); // ← 테스트 핵심
		ImGui::Render();

		graphics->BeginFrame(&camera);

		const auto& rect = camera.GetViewportRect();
		D3D12_VIEWPORT vp = {};
		vp.TopLeftX = rect.x * width;
		vp.TopLeftY = rect.y * height;
		vp.Width = rect.z * width;
		vp.Height = rect.w * height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		graphics->SetViewport(vp);

		graphics->Clear(camera.GetClearFlags(), camera.GetBackgroundColor());

		ID3D12DescriptorHeap* heaps[] = { srvHeap };
		graphics->GetCommandList()->SetDescriptorHeaps(1, heaps);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), graphics->GetCommandList());

		graphics->EndFrame();
		input->Update();

	}

	// GPU가 ImGui 리소스(버퍼 / 텍스처) 사용을 끝낼 때까지 대기 후 정리
	graphics->FlushCommandQueue();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	graphics->ShutDown();
	EngineGraphicsCore::DestroyManager();
	return 0;
}

