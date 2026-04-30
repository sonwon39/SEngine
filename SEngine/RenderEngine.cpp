#pragma warning(disable : 4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

#include <fp16.h>
#include <pix3.h>

#include "RenderEngine.h"
#include "GraphicsCommon.h"

#include "RootSignature.h"
#include "PipelineState.h"
#include "GeometryGenerator.h"

#include "StaticMesh.h"

using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;

RenderEngine::RenderEngine(ID3D12Device5* device)
	:m_device(device)
{
}

RenderEngine::~RenderEngine()
{
	/*ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();*/
}

bool RenderEngine::Initialize(int width, int height, int guiWidth, IDXGIFactory7* factory, HWND wnd)
{
	m_guiWidth = guiWidth;
	m_width = width;
	m_height = height;
	mainWnd = wnd;

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);
	m_hdrViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(m_width), (FLOAT)m_height);

	rtvClearColor = { 0.53F, 0.81F, 0.92F, 1.0F };
	blackClearColor = { 0.F, 0.F, 0.F, 1.0F };

	CreateCommandObjects();

	utility = std::make_shared<GraphicsUtils::Utility>(m_device, m_commandList.Get());
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_createBufferfence.GetAddressOf()));

	// Descriptor Handle offset 구하기
	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	CreateDescriptorHeaps();
	CreateSwapChain(factory, wnd);

	// Create SwapChain RTVs
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_swapChainResources[i].Get(), nullptr, handle);

		handle.Offset(1, m_rtvDescriptorSize);
	}
		
	CreateTextureBuffers();
	CreateDepthBuffers();

	InitScene();

	return true;
}

bool RenderEngine::InitScene()
{
	// 모델 준비
	{
		using DirectX::SimpleMath::Vector3;

		m_commandAllocator->Reset();
		ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

		Mesh<SimpleVertex, uint16_t> rect = GeometryGenerator::MakeSimpleCube(0.5f, 0.5f, 0.5f);
		m_mesh = std::make_unique<StaticMesh>();
		m_mesh->Initialize(m_device, m_commandList.Get(), rect);

		m_commandList->Close();

		ID3D12CommandList* commands[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);
		FlushResourceCommands();

		// camera settings
		localConstant.model = DirectX::SimpleMath::Matrix();
		localConstant.view = DirectX::XMMatrixLookToLH(Vector3(0, 0, -3),
			Vector3(0, 0, 1), Vector3(0, 1, 0));
		float degrees = 70.f;
		float radians = DirectX::XMConvertToRadians(degrees);
		float aspectRatio = (float)m_width / m_height;
		localConstant.projection = DirectX::XMMatrixPerspectiveFovLH(radians,
			aspectRatio, 0.1f, 100.f);

		localConstant.model = localConstant.model.Transpose();
		localConstant.view = localConstant.view.Transpose();
		localConstant.projection = localConstant.projection.Transpose();

		utility->CreateConstantBuffer(sizeof(localConstant), m_localCB, &pLocalCB);
		memcpy(pLocalCB, &localConstant, sizeof(localConstant));
	}

	return true;
}

bool RenderEngine::InitGUI(HWND wnd)
{
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();

	//ImGuiIO& io = ImGui::GetIO();
	//io.IniFilename = nullptr;
	////io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//// io.Fonts->TexID = (ImTextureID)m_guiFont->GetSpriteSheet().ptr;

	//ImGui::StyleColorsLight();
	//const char* fontPath = "Fonts/Hack-Regular.ttf";
	//float fontSize = 15.0f;
	//// 폰트 로드 
	//io.Fonts->AddFontFromFileTTF(fontPath, fontSize);

	//D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//heapDesc.NumDescriptors = 1;
	//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_guiFontHeap));

	//// Setup Platform/Renderer backends
	//ImGui_ImplWin32_Init(wnd);

	//ImGui_ImplDX12_Init(m_device, m_swapChainBufferCount, Renderer::backBufferFormat,
	//	m_guiFontHeap.Get(),
	//	m_guiFontHeap->GetCPUDescriptorHandleForHeapStart(),
	//	m_guiFontHeap->GetGPUDescriptorHandleForHeapStart());

	return true;
}

// BOOKMARK
void RenderEngine::OnResize()
{
	if (m_swapChain == nullptr) return;

	// swapchain 버퍼 리셋
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChainResources[i].Reset();
	}

	// swapchain 버퍼 크기 조정
	m_swapChain->ResizeBuffers(m_swapChainBufferCount,
		m_width,
		m_height,
		DXGI_FORMAT_UNKNOWN,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	// 버퍼에 대한 RTV 재생성
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_swapChainResources[i].Get(), nullptr, handle);

		handle.Offset(1, m_rtvDescriptorSize);
	}

	// DepthBuffer 재생성
	CreateMainDepthBuffer();

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);
	m_hdrViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(m_width), (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);

}

void RenderEngine::CreateCommandObjects()
{
	ThrowIfFailed(
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_commandAllocator)
		));

	ThrowIfFailed(
		m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_commandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&m_commandList)
		));

	m_computeCommandAllocators.resize(computeCommandCount);
	m_computeCommandLists.resize(computeCommandCount);

	for (UINT i = 0; i < computeCommandCount; i++)
	{
		ThrowIfFailed(
			m_device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_computeCommandAllocators[i])
			));

		ThrowIfFailed(
			m_device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				m_computeCommandAllocators[i].Get(),
				nullptr,
				IID_PPV_ARGS(&m_computeCommandLists[i])
			));
		m_computeCommandLists[i]->Close();
	}
	D3D12_COMMAND_QUEUE_DESC queueDesc;
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(
		m_device->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&m_commandQueue))
	);

	m_commandList->Close();
}

void RenderEngine::CreateSwapChain(IDXGIFactory7* factory, HWND wnd)
{
	ComPtr<IDXGISwapChain1> swapChain;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_swapChainBufferCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = Renderer::backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		wnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	ThrowIfFailed(swapChain.As(&m_swapChain));

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void RenderEngine::CreateMainDepthBuffer()
{
	utility->CreateTextureBuffer(m_depthStencilBuffer, m_width, m_height,
		Renderer::dsBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, 0, L"mainDepthBuffer");

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	utility->CreateResourceView(m_depthStencilBuffer, Renderer::dsBufferFormat, false, handle, DescriptorType::DSV);

}

void RenderEngine::CreateDepthBuffers()
{
	CreateMainDepthBuffer();
}

void RenderEngine::CreateDescriptorHeaps()
{
	// DescriptorHeap 생성
	utility->CreateDescriptorHeap(m_swapChainBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_swapChainRTVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DSVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_hdrRTVHeap);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_hdrUAVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_hdrSRVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

void RenderEngine::CreateTextureBuffers()
{
	D3D12_RESOURCE_FLAGS hdrFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	utility->CreateTextureBuffer(m_hdrBuffer, m_width, m_height, Renderer::hdrFormat, hdrFlags, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"hdrBuffer");
	utility->CreateResourceView(m_hdrBuffer, Renderer::hdrFormat, false, m_hdrRTVHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::RTV);
	utility->CreateResourceView(m_hdrBuffer, Renderer::hdrFormat, false, m_hdrUAVHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::UAV);
	utility->CreateResourceView(m_hdrBuffer, Renderer::hdrFormat, false, m_hdrSRVHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::SRV);
}

// BOOKMARK
void RenderEngine::UpdateGUI()
{
	ImGui::SetWindowSize(ImVec2((float)m_guiWidth, (float)m_height));
	ImGui::SetWindowPos(ImVec2(0.f, 0.f), ImGuiCond_FirstUseEver);
}

// BOOKMARK
void RenderEngine::Tick(float deltaTime)
{
	{
		angle += rotateSpeed * deltaTime;
		float radians = DirectX::XMConvertToRadians(angle);
		localConstant.model = DirectX::XMMatrixRotationY(radians);
		localConstant.model = localConstant.model.Transpose();
		memcpy(pLocalCB, &localConstant, sizeof(localConstant));
	}

	Draw();
}

void RenderEngine::RenderMeshes(const std::string& psoName, ID3D12GraphicsCommandList* commandList)
{
	m_mesh->Render(commandList);
}

void RenderEngine::Compute(const std::string& psoName, int idx)
{
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 255, 0), psoName.c_str());

	using namespace Renderer;

	ComputePSO pso;
	if (m_CPSOs.find(psoName) != m_CPSOs.end())
	{
		pso = m_CPSOs[psoName];
	}
	else
	{
		pso = m_CPSOs["defaultCPSO"];
	}
	ID3D12CommandAllocator* alloc = m_computeCommandAllocators[idx].Get();
	ID3D12GraphicsCommandList* cmdList = m_computeCommandLists[idx].Get();
	alloc->Reset();
	ThrowIfFailed(cmdList->Reset(alloc, pso.GetPSO()));

	cmdList->SetPipelineState(pso.GetPSO());
	cmdList->SetComputeRootSignature(pso.GetRootSignature()->GetSignature());

	ID3D12DescriptorHeap* heaps[] = {
		m_hdrUAVHeap.Get()
	};

	cmdList->SetDescriptorHeaps(1, heaps);
	cmdList->SetComputeRootDescriptorTable(0, m_hdrUAVHeap->GetGPUDescriptorHandleForHeapStart());

	UINT numThreadsX = 4;
	UINT numThreadsY = 1024/ numThreadsX;
	UINT threadXCount = (m_width + numThreadsX - 1) / numThreadsX;
	UINT threadYCount = (m_height + numThreadsY - 1) / numThreadsY;
	cmdList->Dispatch(threadXCount, threadYCount, 1);
	cmdList->Close();

	ID3D12CommandList* commands[] = { cmdList };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	PIXEndEvent(m_commandQueue.Get());
}

void RenderEngine::Render(const std::string& psoName, bool clear)
{
	PIXBeginEvent(m_commandQueue.Get(), PIX_COLOR(255, 0, 0), psoName.c_str());

	using namespace Renderer;

	GraphicsPSO pso;
	if (m_PSOs.find(psoName) != m_PSOs.end())
	{
		pso = m_PSOs[psoName];
	}
	else
	{
		pso = m_PSOs["defaultPSO"];
	}
	m_commandAllocator->Reset();
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pso.GetPSO()));

	m_commandList->SetPipelineState(pso.GetPSO());
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());


	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);


	m_commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));
	if (clear)
	{
		m_commandList->ClearRenderTargetView(GetCurrentRtvCpuHandle(), rtvClearColor.data(), 0, nullptr);
		m_commandList->ClearDepthStencilView(GetDSVCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
	}
	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), TRUE, &GetDSVCpuHandle());
	ID3D12DescriptorHeap* heaps[] = {
		m_hdrSRVHeap.Get()
	};

	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_hdrSRVHeap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->SetGraphicsRootConstantBufferView(1, m_localCB->GetGPUVirtualAddress());

	RenderMeshes(psoName, m_commandList.Get());

	m_commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			GetCurrentSwapChainResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

	m_commandList->Close();

	ID3D12CommandList* commands[] = { m_commandList.Get()};
	{
		m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
		// text 업데이트를 위해 graphcics memory 사용 시 commit 해줘야 Graphics 메모리를 재사용한다
		ThrowIfFailed(m_swapChain->Present(1, 0));
		m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % m_swapChainBufferCount;

		FlushResourceCommands();
	}
	PIXEndEvent(m_commandQueue.Get());
}

void RenderEngine::RenderGUI(bool isFinal)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags flags = 0;

	ImGui::Begin("GUI", nullptr, flags);
	UpdateGUI();

	ImGui::End();
	ImGui::Render();

}

// BOOKMARK
void RenderEngine::Draw()
{
	Compute("defaultCPSO", 0);
	Render("", true/*clear RT*/);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetCurrentRtvCpuHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_swapChainRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_currentBackBufferIndex, m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetDSVCpuHandle() const
{
	return m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* RenderEngine::GetCurrentSwapChainResource() const
{
	return m_swapChainResources[m_currentBackBufferIndex].Get();
}

void RenderEngine::FlushCommands()
{
	m_currentBufferFence++;

	ThrowIfFailed(m_commandQueue->Signal(m_createBufferfence.Get(), m_currentBufferFence));

	if (m_createBufferfence->GetCompletedValue() < m_currentBufferFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		m_createBufferfence->SetEventOnCompletion(m_currentBufferFence, eventHandle);

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void RenderEngine::FlushResourceCommands()
{
	m_currentFence++;

	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
	if (m_fence->GetCompletedValue() < m_currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		m_fence->SetEventOnCompletion(m_currentFence, eventHandle);

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}


void RenderEngine::Quit()
{
	FlushResourceCommands();

}

void RenderEngine::GenerateMips(ID3D12Resource* tex)
{
	DirectX::ResourceUploadBatch upload(m_device);
	upload.Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

	upload.GenerateMips(tex);

	auto finish = upload.End(m_commandQueue.Get());
	finish.wait();
}
