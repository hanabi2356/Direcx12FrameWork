#include"Common.h"
#include"Window.h"
#include"TimeManager.h"
#include"InputManager.h"
#include"EngineGraphicsCore.h"
#include "Camera.h"

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
	camera.SetBackgroundColor({ 0.0f, 0.2f, 0.4f, 1.0f });

	while (window.ProcessMessages())
	{
		time->Update();
		camera.Update();

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

		graphics->EndFrame();
		input->Update();

	}
	graphics->ShutDown();
	EngineGraphicsCore::DestroyManager();
	return 0;
}