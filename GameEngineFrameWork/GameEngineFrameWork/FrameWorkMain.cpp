#include"Common.h"
#include"Window.h"
#include"TimeManager.h"
#include"InputManager.h"
#include"EngineGraphicsCore.h"


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

	while (window.ProcessMessages())
	{
		time->Update();

		input->Update();
	}
	graphics->ShutDown();
	return 0;
}