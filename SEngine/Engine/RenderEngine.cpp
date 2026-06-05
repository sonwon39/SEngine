#pragma warning(disable : 4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "Directxtk12/DDSTextureLoader.h"
#include "directxtk12/ResourceUploadBatch.h"

#include <fp16.h>
#include <pix3.h>
#include <random>

#include "RenderEngine.h"
#include "GraphicsCommon.h"
#include "World.h"

#include "RootSignature.h"
#include "PipelineState.h"
#include "GeometryGenerator.h"

#include "StaticMesh.h"
#include "AssetManager/TextureLoader.h"
#include "AssetManager/ModelLoader.h"

using Microsoft::WRL::ComPtr;
using namespace GraphicsUtils;
using namespace Graphics;
using namespace Renderer;

static const float PI = 3.141592f;

RenderEngine::RenderEngine(ID3D12Device5* device)
	:m_device(device)
{
}

RenderEngine::~RenderEngine()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

bool RenderEngine::Initialize(int width, int height, int guiWidth, IDXGIFactory7* factory)
{
	m_guiWidth = guiWidth;
	m_width = width;
	m_height = height;

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);
	m_hdrViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(m_width), (FLOAT)m_height);

	//rtvClearColor = { 0.53F, 0.81F, 0.92F, 1.0F };
	rtvClearColor = { 0.F, 0.F, 0.F, 1.0F };
	blackClearColor = { 0.F, 0.F, 0.F, 1.0F };

	CreateCommandObjects();

	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_createBufferfence.GetAddressOf()));

	// Descriptor Handle offset 구하기
	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	CreateDescriptorHeaps();
	CreateSwapChain(factory);

	// Create SwapChain RTVs
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_swapChainRTVHeap.CreateResourceView(m_swapChainResources[i].Get(), DescriptorType::RTV);
		m_swapChainResources[i].SetResourceStates(D3D12_RESOURCE_STATE_PRESENT);
	}

	CreateDepthBuffers();

	InitScene();

	m_camera = std::make_shared<Camera>(width, height);
	m_camera->Initialize();

	return true;
}

bool RenderEngine::InitScene()
{
	m_resourceCommandAllocator->Reset();
	ThrowIfFailed(m_resourceCommandList->Reset(m_resourceCommandAllocator.Get(), nullptr));

	// 모델 준비
	auto simpleModelLoader = m_world->GetSimpleModelLoader();
	simpleModelLoader->InitializeGPU(m_resourceCommandList.Get());
	auto modelLoader = m_world->GetModelLoader();
	modelLoader->InitializeGPU(m_resourceCommandList.Get());

	std::shared_ptr<TextureLoader> texLoader;
	// texture 준비
	{
		if (m_world)
		{
			texLoader = m_world->GetTextureLoader();
			texLoader->LoadIdx();
			texLoader->InitHeap(100);
			texLoader->LoadTextures(m_resourceCommandList.Get());
		}
	}

	// sph 초기화
	{
		m_sph = std::make_shared<SPH>();
		m_sph->Initialize(m_device, m_resourceCommandAllocator.Get(), m_resourceCommandList.Get());
	}
	// stable fluids 초기화
	{
		m_stableFluids = std::make_shared<StableFluids>();
		m_stableFluids->Initialize(m_width, m_height);
	}

	ID3D12CommandList* commands[] = { m_resourceCommandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);
	FlushCommands();

	// 텍스쳐, 메쉬 GPU RESOURCE 생성 후 blob 제거
	texLoader->ClearBlobs();
	m_world->ClearMeshBlobs();

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

	return true;
}

bool RenderEngine::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	ImGui::StyleColorsLight();
	const char* fontPath = "Assets/Fonts/Hack-Regular.ttf";
	float fontSize = 15.0f;
	// 폰트 로드 
	io.Fonts->AddFontFromFileTTF(fontPath, fontSize);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_guiFontHeap));

	// Setup Platform/Renderer backends
	if (m_world)
		ImGui_ImplWin32_Init(m_world->m_mainWnd);

	ImGui_ImplDX12_Init(m_device, m_swapChainBufferCount, Renderer::backBufferFormat,
		m_guiFontHeap.Get(),
		m_guiFontHeap->GetCPUDescriptorHandleForHeapStart(),
		m_guiFontHeap->GetGPUDescriptorHandleForHeapStart());

	return true;
}

// BOOKMARK
void RenderEngine::OnResize(int width, int height)
{
	if (m_swapChain == nullptr) return;

	m_width = width;
	m_height = height;

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
	m_swapChainRTVHeap.ResetIndex();
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_swapChainResources[i].ReleaseAndGetAddressOf()));
		m_swapChainRTVHeap.CreateResourceView(m_swapChainResources[i].Get(), DescriptorType::RTV);
		m_swapChainResources[i].SetResourceStates(D3D12_RESOURCE_STATE_PRESENT);
	}

	// DepthBuffer 재생성
	CreateMainDepthBuffer();

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	m_viewport = CD3DX12_VIEWPORT((FLOAT)m_guiWidth, 0.F, (FLOAT)(m_width - m_guiWidth), (FLOAT)m_height);
	m_hdrViewport = CD3DX12_VIEWPORT((FLOAT)0.F, 0.F, (FLOAT)(m_width), (FLOAT)m_height);
	m_scissorRect = CD3DX12_RECT((LONG)0, 0, (LONG)(m_width), (LONG)m_height);

	if (m_camera)
	{
		m_camera->OnResize(m_width, m_height);
	}
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

	ThrowIfFailed(
		m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_resourceCommandAllocator)
		));

	ThrowIfFailed(
		m_device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_resourceCommandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&m_resourceCommandList)
		));


	D3D12_COMMAND_QUEUE_DESC queueDesc;
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(
		m_device->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&m_commandQueue))
	);

	m_commandList->Close();
	m_resourceCommandList->Close();
}

void RenderEngine::CreateSwapChain(IDXGIFactory7* factory)
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

	if (m_world) {
		ThrowIfFailed(factory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),
			m_world->m_mainWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));
	}


	ThrowIfFailed(swapChain.As(&m_swapChain));

	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void RenderEngine::CreateMainDepthBuffer()
{
	utility->CreateTextureBuffer(m_depthStencilBuffer, m_width, m_height,
		Renderer::dsBufferFormat, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, 0, L"mainDepthBuffer");

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_DSVHeap->GetCPUDescriptorHandleForHeapStart());

	utility->CreateResourceView(m_depthStencilBuffer.Get(), Renderer::dsBufferFormat, false, handle, DescriptorType::DSV);

}

void RenderEngine::CreateDepthBuffers()
{
	CreateMainDepthBuffer();
}

void RenderEngine::CreateDescriptorHeaps()
{
	// DescriptorHeap 생성
	m_swapChainRTVHeap.Initialize(m_swapChainBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DSVHeap);
}

// BOOKMARK
void RenderEngine::UpdateGUI()
{
	//ImGui::SetWindowSize(ImVec2((float)m_guiWidth, (float)m_height));
	ImGui::SetWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
}

// BOOKMARK
void RenderEngine::Tick(float deltaTime)
{
	//SPHTick(deltaTime);
	//StableFluidsTick(deltaTime);

	for (auto& m : meshBatchs)
	{
		m->SyncCB();
	}

	RenderMeshes("defaultPSO");
	RenderGUI();
	Execute();
}

void RenderEngine::SPHTick(float deltaTime)
{
	m_sph->Tick(deltaTime);
	m_sph->Execute(m_commandQueue.Get());

	RenderSPH("particleRenderPSO", true/*clear RT*/);

}

void RenderEngine::StableFluidsTick(float deltaTime)
{
	m_stableFluids->Tick(deltaTime);
	m_stableFluids->Execute(m_commandQueue.Get());
}

void RenderEngine::RenderMeshes(const std::string& psoName)
{
	using namespace Renderer;

	InitGraphicsCommand(psoName);

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_swapChainResources[m_currentBackBufferIndex].Transition(D3D12_RESOURCE_STATE_RENDER_TARGET, barrier)) barriers.push_back(barrier);
	if (!barriers.empty())
		m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	m_commandList->ClearRenderTargetView(GetCurrentRtvCpuHandle(), rtvClearColor.data(), 0, nullptr);
	m_commandList->ClearDepthStencilView(GetDSVCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), TRUE, &GetDSVCpuHandle());

	if (m_camera)
	{
		m_commandList->SetGraphicsRootConstantBufferView(1, m_camera->GetGlboalConstant());
	}

	BindMainHeap();

	for (auto& m : meshBatchs)
	{
		m->Render(m_commandList.Get());
	}
}

void RenderEngine::RenderSPH(const std::string& psoName, bool clear)
{
	using namespace Renderer;

	InitGraphicsCommand(psoName);

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;
	if (m_swapChainResources[m_currentBackBufferIndex].Transition(D3D12_RESOURCE_STATE_RENDER_TARGET, barrier)) barriers.push_back(barrier);
	if (!barriers.empty())
		m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	if (clear)
	{
		m_commandList->ClearRenderTargetView(GetCurrentRtvCpuHandle(), rtvClearColor.data(), 0, nullptr);
		m_commandList->ClearDepthStencilView(GetDSVCpuHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
	}
	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), TRUE, &GetDSVCpuHandle());

	if (m_camera)
	{
		m_commandList->SetGraphicsRootConstantBufferView(1, m_camera->GetGlboalConstant());
	}

	m_sph->Render(m_commandList.Get());
}

void RenderEngine::RenderGUI()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags flags = 0;

	ImGui::Begin("GUI", nullptr, flags);
	UpdateGUI();

	ImGui::End();
	ImGui::Render();

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;
	if (m_swapChainResources[m_currentBackBufferIndex].Transition(D3D12_RESOURCE_STATE_RENDER_TARGET, barrier)) barriers.push_back(barrier);
	if (!barriers.empty())
		m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	m_commandList->OMSetRenderTargets(1, &GetCurrentRtvCpuHandle(), false, nullptr);
	ID3D12DescriptorHeap* pHeaps[] = { m_guiFontHeap.Get() };
	m_commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(pHeaps)), pHeaps);

	ImDrawData* dd = ImGui::GetDrawData();
	if (dd->Valid)
		ImGui_ImplDX12_RenderDrawData(dd, m_commandList.Get());
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetCurrentRtvCpuHandle() const
{
	return m_swapChainRTVHeap.GetCPUHandle(m_currentBackBufferIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderEngine::GetDSVCpuHandle() const
{
	return m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* RenderEngine::GetCurrentSwapChainResource() const
{
	return m_swapChainResources[m_currentBackBufferIndex].Get();
}

void RenderEngine::InitGraphicsCommand(const std::string& psoName)
{
	GraphicsPSO pso = GetGraphicsPSO(psoName);

	m_commandAllocator->Reset();
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
	m_commandList->SetPipelineState(pso.GetPSO());
	m_commandList->SetGraphicsRootSignature(pso.GetRootSignature()->GetSignature());

	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	m_commandList->RSSetViewports(1, &m_viewport);
}

void RenderEngine::BindMainHeap()
{
	ID3D12DescriptorHeap* heaps[] = { m_world->GetMainHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
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

void RenderEngine::Execute()
{
	std::vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;
	if (m_swapChainResources[m_currentBackBufferIndex].Transition(D3D12_RESOURCE_STATE_PRESENT, barrier)) barriers.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	m_commandList->Close();
	ID3D12CommandList* commands[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);

	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
	ThrowIfFailed(m_swapChain->Present(1, 0));
	m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	FlushResourceCommands();
}

void RenderEngine::RegistMeshBatch(std::shared_ptr<MeshBatch> meshBatch)
{
	meshBatchs.push_back(meshBatch);
	// Material이 nullptr이면 MeshBatch::Render에서 early-return. fallback은 호출측이 책임진다.
}
