#include "EngineGraphicsCore.h"

EngineGraphicsCore::EngineGraphicsCore()
	:m_clientWidth(1270), m_clientHeight(720), m_currentBackBuffer(0),
	m_currentFence(0), m_fenceEvent(nullptr)
{
}

EngineGraphicsCore::~EngineGraphicsCore()
{
	ShutDown();
}

bool EngineGraphicsCore::Initialize(HWND hWnd, int width, int height)
{
	m_clientWidth = width;
	m_clientHeight = height;

	UINT dxgiFactoryFlag = 0;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	//Factory 생성
	if (FAILED(CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(&m_factory)))) return false;
	
	//GPU 어댑터 선택 및 Device 생성
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	for (UINT idx = 0; SUCCEEDED(m_factory->EnumAdapterByGpuPreference(idx, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&hardwareAdapter))); idx++)
	{
		DXGI_ADAPTER_DESC1 desc;
		hardwareAdapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

		if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) break;

	}

	//어댑터 탐색 실패 시 기본 디바이스 생성
	if (m_device == nullptr)
	{
		if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) return false;
	}

	//Fence 및 이벤트 동기화 객체 생성
	if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))) return false;
	
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr) return false;

	//서술자 크기 캐싱
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//queue, allocator, list 생성
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cmdQueue))))
		return false;
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator))))
		return false;
	if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&m_cmdList))))
		return false;

	m_cmdList->Close();

	//SwapChain 생성
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = m_clientWidth;
	swapChainDesc.Height = m_clientHeight;
	swapChainDesc.Format = m_backBufferFormat;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SwapChainBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	ComPtr<IDXGISwapChain1> swapChain;

	if (FAILED(m_factory->CreateSwapChainForHwnd(m_cmdQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain)))
		return false;

	if (FAILED(swapChain.As(&m_swapChain))) return false;

	//DescriptorHeap 생성
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap))))
		return false;

	if (CreateFrameConstantBuffer() == false) return false;
	if (BuildRootSignature() == false) return false;
	if (BuildPipelineState() == false) return false;

	OnResize();
	return true;
}

void EngineGraphicsCore::ShutDown()
{
	if (m_device == nullptr) return;
	
	if (m_cmdQueue && m_fence)
	{
		FlushCommandQueue();
	}

	if (m_frameCB && m_frameCBMapped)
	{
		m_frameCB->Unmap(0, nullptr);
		m_frameCBMapped = nullptr;
	}
	m_frameCB.Reset();
	m_frameCBGroupAddress = 0;

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		m_swapChainBuffer[i].Reset();
	}
	m_depthStencilBuffer.Reset();
	
	m_rtvHeap.Reset();
	m_dsvHeap.Reset();
	m_swapChain.Reset();

	m_fence.Reset();
	if (m_fenceEvent)
	{
		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	m_pso.Reset();
	m_rootSignature.Reset();
	m_vsBlob.Reset();
	m_psBlob.Reset();

	m_device.Reset();
	m_factory.Reset();
}

void EngineGraphicsCore::OnResize()
{
	if (m_device == nullptr || m_swapChain == nullptr || m_cmdAllocator == nullptr)  return;

	FlushCommandQueue();

	m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		m_swapChainBuffer[i].Reset();
	}
	m_depthStencilBuffer.Reset();

	m_swapChain->ResizeBuffers(SwapChainBufferCount, m_clientWidth, m_clientHeight, m_backBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	m_currentBackBuffer = 0;

	// 3. RTV 생성
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; ++i)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i]));
		m_device->CreateRenderTargetView(m_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.ptr += m_rtvDescriptorSize;
	}

	// 4. Depth-Stencil Buffer 및 DSV 생성
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_clientWidth;
	depthStencilDesc.Height = m_clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = m_depthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = m_depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &optClear, IID_PPV_ARGS(&m_depthStencilBuffer));

	m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, 
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// 5. Command List 닫기 및 실행
	m_cmdList->Close();
	ID3D12CommandList* cmdsWith[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdsWith), cmdsWith);

	// 실행 완료 대기
	FlushCommandQueue();

	// 6. Viewport 및 Scissor Rect 업데이트
	m_screenViewport.TopLeftX = 0.0f;
	m_screenViewport.TopLeftY = 0.0f;
	m_screenViewport.Width = static_cast<float>(m_clientWidth);
	m_screenViewport.Height = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, static_cast<LONG>(m_clientWidth), static_cast<LONG>(m_clientHeight) };
}

void EngineGraphicsCore::FlushCommandQueue()
{
	m_currentFence++;

	if(FAILED(m_cmdQueue->Signal(m_fence.Get(), m_currentFence))) return;

	if (m_fence->GetCompletedValue() < m_currentFence)
	{
		m_fence->SetEventOnCompletion(m_currentFence, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
		
	}
}

void EngineGraphicsCore::BeginFrame(Camera* camera)
{
	if (camera == nullptr) return;

	FlushCommandQueue();
	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_swapChainBuffer[m_currentBackBuffer].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_cmdList->ResourceBarrier(1, &barrier);

	auto rtv = CurrentBackBufferView();
	auto dsv = DepthStencilView();
	m_cmdList->OMSetRenderTargets(1, &rtv, TRUE, &dsv);
	m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_cmdList->SetPipelineState(m_pso.Get());

	if (m_frameCBMapped)
	{
		FrameConstant fc = {};
		fc.view = XMMatrixTranspose(camera->GetViewMatrix());
		fc.projection = XMMatrixTranspose(camera->GetProjectionMatrix());
		memcpy(m_frameCBMapped, &fc, sizeof(FrameConstant));
	}

}

void EngineGraphicsCore::Clear(Camera::ClearFlags flag, const XMFLOAT4& color, float depth, UINT8 stencil)
{
	auto rtv = CurrentBackBufferView();
	auto dsv = DepthStencilView();
	const float c[] = { color.x, color.y, color.z, color.w };

	if (flag == Camera::ClearFlags::SolidColor)
	{
		m_cmdList->ClearRenderTargetView(rtv, c, 0, nullptr);
		m_cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
	}
	else if (flag == Camera::ClearFlags::DepthOnly)
	{
		m_cmdList->ClearDepthStencilView(dsv,
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
	}

}

void EngineGraphicsCore::SetViewport(const D3D12_VIEWPORT& viewport)
{
	m_cmdList->RSSetViewports(1, &viewport);
	D3D12_RECT scissor = {
		(LONG)viewport.TopLeftX,
		(LONG)viewport.TopLeftY,
		(LONG)(viewport.TopLeftX + viewport.Width),
		(LONG)(viewport.TopLeftY + viewport.Height)
	};

	m_cmdList->RSSetScissorRects(1, &scissor);
}

void EngineGraphicsCore::EndFrame()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_swapChainBuffer[m_currentBackBuffer].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_cmdList->ResourceBarrier(1, &barrier);

	m_cmdList->Close();
	ID3D12CommandList* lists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(1, lists);

	m_swapChain->Present(1, 0);
	m_currentBackBuffer = static_cast<int>(m_swapChain->GetCurrentBackBufferIndex());
}

D3D12_CPU_DESCRIPTOR_HANDLE EngineGraphicsCore::CurrentBackBufferView() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<SIZE_T>(m_currentBackBuffer) * m_rtvDescriptorSize;
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE EngineGraphicsCore::DepthStencilView() const
{
	return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

bool EngineGraphicsCore::CreateFrameConstantBuffer()
{
	const UINT size = (sizeof(FrameConstant) + 255) & ~255;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	if (FAILED(m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_frameCB)))) return false;

	m_frameCB->Map(0, nullptr, reinterpret_cast<void**>(&m_frameCBMapped));
	m_frameCBGroupAddress = m_frameCB->GetGPUVirtualAddress();

	return true;
}

bool EngineGraphicsCore::BuildRootSignature()
{
	D3D12_ROOT_PARAMETER param = {};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	param.Descriptor.ShaderRegister = 0;
	param.Descriptor.RegisterSpace = 0;
	param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = 1;
	desc.pParameters = &param;
	desc.NumStaticSamplers = 0;
	desc.pStaticSamplers = nullptr;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> serialized, error;
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
	if (FAILED(hr)) return false;

	hr = m_device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	return SUCCEEDED(hr);
}

bool EngineGraphicsCore::CompileShader(const wchar_t* file, const char* entry, const char* target, ComPtr<ID3DBlob>& outBlob)
{
	ComPtr<ID3DBlob> error;

	HRESULT hr = D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, 0, 0, &outBlob, &error);

	return SUCCEEDED(hr);
}

bool EngineGraphicsCore::BuildPipelineState()
{
	if (CompileShader(L"Shader\\Basic.hlsl", "VSMain", "vs_5_1", m_vsBlob) == false) return false;
	if (CompileShader(L"Shader\\Basic.hlsl", "PSMain", "ps_5_1", m_psBlob) == false) return false;

	D3D12_INPUT_ELEMENT_DESC inputLayout [] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
	pso.pRootSignature = m_rootSignature.Get();
	pso.VS = { m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize() };
	pso.PS = { m_psBlob->GetBufferPointer(), m_psBlob->GetBufferSize() };
	pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	pso.SampleMask = UINT_MAX;
	pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pso.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	pso.RasterizerState.DepthClipEnable = TRUE;
	pso.DepthStencilState.DepthEnable = TRUE;
	pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pso.InputLayout = { inputLayout, _countof(inputLayout) };
	pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso.NumRenderTargets = 1;
	pso.RTVFormats[0] = m_backBufferFormat;
	pso.DSVFormat = m_depthStencilFormat;
	pso.SampleDesc.Count = 1;

	return SUCCEEDED(m_device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&m_pso)));
}
