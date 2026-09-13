#pragma once
#include<Windows.h>
#include"Singleton.h"
#include<unordered_map>
enum class KeyState
{
	None,
	Down,
	Pressed,
	Up
};

class InputManager : public Singleton<InputManager>
{
	friend class Singleton<InputManager>;

public:
    void Initialize(int screenWidth, int screenHeight);
	void Update();

    // Window::WndProc에서 호출
    void OnKeyDown(WPARAM wParam);
    void OnKeyUp(WPARAM wParam);
    void OnMouseMove(int x, int y);
    void OnMouseDown(WPARAM wParam, int x, int y);
    void OnMouseUp(WPARAM wParam, int x, int y);
    void OnMouseWheel(WPARAM wParam);
    // 조회
    bool IsKeyDown(int key);     // 이번 프레임에 막 누름
    bool IsKeyUp(int key);       // 이번 프레임에 막 뗌
    bool IsKeyPressed(int key);  // 누르고 있음 (Down | Pressed)

    int GetMouseX() const { return m_mouseX; }
    int GetMouseY() const { return m_mouseY; }
    int GetMouseWheelDelta() const { return m_mouseWheelDelta; }
    int GetScreenWidth() const { return m_screenWidth; }
    int GetScreenHeight() const { return m_screenHeight; }

private:
    InputManager();
    ~InputManager() override;
    std::unordered_map<int, KeyState> m_keyStates;
    int m_mouseX;
    int m_mouseY;
    int m_mouseWheelDelta;
    int m_screenWidth;
    int m_screenHeight;
};

