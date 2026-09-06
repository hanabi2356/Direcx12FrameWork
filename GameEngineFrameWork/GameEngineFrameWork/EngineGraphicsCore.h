#pragma once
#include"Common.h"
#include"Singleton.h"

class EngineGraphicsCore : public Singleton<EngineGraphicsCore>
{
private:
	/// <summary>
	/// 디바이스, 팩토리
	/// </summary>
	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGIFactory6> m_factory;

	/// <summary>
	/// 명령 제출 관련
	/// </summary>
	ComPtr<ID3D12CommandAllocator> m_cmdAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_cmdList;
	ComPtr<ID3D12CommandQueue> m_cmdQueue;

	/// <summary>
	/// 서술자 힙
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	/// <summary>
	/// 서술자 크기
	/// </summary>
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	UINT m_cbvSrvUavDescriptorSize = 0;

	/// <summary>
	/// 스왑 체인 및 버퍼
	/// </summary>
	static const int SwapChainBufferCount = 2;
	int m_currentBackBuffer;
	ComPtr<IDXGISwapChain4> m_swapChain;
	ComPtr<ID3D12Resource> m_swapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;

	/// <summary>
	/// 동기화 객체
	/// </summary>
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_currentFence;
	HANDLE m_fenceEvent;

	int m_clientWidth = 1270;
	int m_clientHeight = 720;
	D3D12_VIEWPORT m_screenViewport = {};
	D3D12_RECT m_scissorRect = {};

	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
public:
	EngineGraphicsCore();
	~EngineGraphicsCore();

	bool Initialize(HWND hWnd, int width, int height);
	void ShutDown();
	void OnResize();
	void FlushCommandQueue();
};

