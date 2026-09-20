#pragma once
#include"Common.h"
#include"Singleton.h"
#include"Camera.h"

using namespace DirectX;



struct FrameConstant
{
	XMMATRIX view;
	XMMATRIX projection;
};

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

	/// <summary>
	/// Frame CB(Upload Heap)
	/// </summary>
	ComPtr<ID3D12Resource> m_frameCB;
	BYTE* m_frameCBMapped = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS m_frameCBGroupAddress = 0;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pso;
	ComPtr<ID3DBlob> m_vsBlob;
	ComPtr<ID3DBlob> m_psBlob;

	int m_clientWidth = 0;
	int m_clientHeight = 0;
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

	void BeginFrame(Camera* camera);
	void Clear(Camera::ClearFlags flag, const XMFLOAT4& color, float depth = 1.0f, UINT8 stencil = 0);
	void SetViewport(const D3D12_VIEWPORT& viewport);
	void EndFrame();

	int GetClientWidth() const { return m_clientWidth; }
	int GetClientHeight() const { return m_clientHeight; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	bool CreateFrameConstantBuffer();

	bool BuildRootSignature();

	/// <summary>
	/// HLSL 셰이더 파일을 런타임에서 DXIL(DirectX intermediate Language) 바이너리로 컴파일하는 함수
	/// </summary>
	/// <param name="file"> hlsl 경로</param>
	/// <param name="entry"> 진입점 함수 이름(VSMain, PSMain)...</param>
	/// <param name="target"> 셰이더 프로필 및 버전</param>
	/// <param name="outBlob">최종 컴파일된 바이너리가 자장될 ID3DBlob 스마트 포인터</param>
	/// <returns></returns>
	bool CompileShader(const wchar_t* file, const char* entry, const char* target, ComPtr<ID3DBlob>& outBlob);

	bool BuildPipelineState();
	
	
};

