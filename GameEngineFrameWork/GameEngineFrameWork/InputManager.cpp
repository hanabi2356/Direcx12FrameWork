#include "InputManager.h"


InputManager::InputManager()
	:m_mouseX(0), m_mouseY(0), m_mouseWheelDelta(0), m_screenWidth(0), 
	m_screenHeight(0)
{
}

InputManager::~InputManager()
{
}

void InputManager::Initialize(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}
void InputManager::Update()
{
    // 휠은 프레임마다 리셋
    m_mouseWheelDelta = 0;
    // Down -> Pressed, Up -> None
    for (auto& pair : m_keyStates)
    {
        if (pair.second == KeyState::Down)
        {
            pair.second = KeyState::Pressed;
        }
        else if (pair.second == KeyState::Up)
        {
            pair.second = KeyState::None;
        }
    }
}

void InputManager::OnKeyDown(WPARAM wParam)
{
    // 이미 Pressed면 Down으로 다시 안 바꿈 (한 번만 Down)
    if (m_keyStates[wParam] == KeyState::None || m_keyStates[wParam] == KeyState::Up)
    {
        m_keyStates[wParam] = KeyState::Down;
    }
}

void InputManager::OnKeyUp(WPARAM wParam)
{
    m_keyStates[wParam] = KeyState::Up;
}

void InputManager::OnMouseMove(int x, int y)
{
    m_mouseX = x;
    m_mouseY = y;
}

void InputManager::OnMouseDown(WPARAM wParam, int x, int y)
{
    // VK_LBUTTON / VK_RBUTTON / VK_MBUTTON도 키맵에 같이 저장
    OnKeyDown(wParam);
    m_mouseX = x;
    m_mouseY = y;
}

void InputManager::OnMouseUp(WPARAM wParam, int x, int y)
{
    OnKeyUp(wParam);
    m_mouseX = x;
    m_mouseY = y;
}

void InputManager::OnMouseWheel(WPARAM wParam)
{
    m_mouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
}

bool InputManager::IsKeyDown(int key)
{
    return m_keyStates[key] == KeyState::Down;
}

bool InputManager::IsKeyUp(int key)
{
    return m_keyStates[key] == KeyState::Up;
}

bool InputManager::IsKeyPressed(int key)
{
    return m_keyStates[key] == KeyState::Pressed
        || m_keyStates[key] == KeyState::Down;
}
