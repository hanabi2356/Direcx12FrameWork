#include "Window.h"
#include"InputManager.h"
#include<windowsx.h>
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	InputManager* input = InputManager::GetInstance();

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256) input->OnKeyDown(wParam);
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256) input->OnKeyUp(wParam);
		break;
	case WM_MOUSEMOVE:
		input->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONDOWN:
		input->OnMouseDown(VK_LBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONUP:
		input->OnMouseUp(VK_LBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONDOWN:
		input->OnMouseDown(VK_RBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONUP:
		input->OnMouseUp(VK_RBUTTON, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
Window::Window(HINSTANCE hInstance, int width, int height)
	:m_hInstance(hInstance), m_hWnd(nullptr), m_widht(width), m_height(height)
{

}
Window::~Window()
{
	if (m_hWnd) DestroyWindow(m_hWnd);
}

bool Window::Create()
{
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Window::WndProc;
	wc.hInstance = m_hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"GameEngineFrameWork";

	if (!RegisterClassExW(&wc)) return false;
	RECT wr = { 0, 0, m_widht, m_height };

	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	m_hWnd = CreateWindowExW(
		0, L"GameEngineFrameWork", L"DX12 Game Framework",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, m_hInstance, nullptr);

	if (!m_hWnd) return false;
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
	
	return true;

}
bool Window::ProcessMessages()
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT) return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}
