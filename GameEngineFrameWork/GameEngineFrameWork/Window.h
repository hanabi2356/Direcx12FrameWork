#pragma once
#include<Windows.h>
#include<string>
using namespace std;
class Window
{
public:
	Window(HWND hWnd, int width, int height);
	~Window();

	bool Create();
	bool ProcessMessages();
	HWND GetHWND() const { return m_hWnd; }
private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	int m_widht;
	int m_height;
	
};

