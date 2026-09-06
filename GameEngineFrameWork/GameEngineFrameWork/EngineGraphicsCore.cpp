#include "EngineGraphicsCore.h"

EngineGraphicsCore::EngineGraphicsCore()
	:m_clientWidth(1270), m_clientHeight(720)
{
}

EngineGraphicsCore::~EngineGraphicsCore()
{
}

bool EngineGraphicsCore::Initialize(HWND hWnd, int width, int height)
{
	return false;
}

void EngineGraphicsCore::ShutDown()
{
}

void EngineGraphicsCore::OnResize()
{
}

void EngineGraphicsCore::FlushCommandQueue()
{
}
