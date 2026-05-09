#pragma once

#include <array>


#include "d3d12.h"
#include "directxtk12/SpriteBatch.h"
#include "directxtk12/SpriteFont.h"
#include "directxtk12/GraphicsMemory.h"
#include <vector>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "Renderer.h"

#include "directxtk12/SimpleMath.h"
#include "DefaultHLSLCompat.h"

#include "Camera.h"
#include "SPH.h"

class StaticMesh;

enum RenderType {
	RT_TEXT,
	RT_Default,
	RT_PointCloud,
	RT_CubeMap,
	RT_Dot /*렌더타겟에 점하나 넣고 texture 그리기 용도*/
};

enum RenderPassType {
	RPT_Default,
	RPT_CubeMapPass,
	RPT_DepthOnlyPass
};

class RenderEngine {
public:
	RenderEngine(ID3D12Device5* device = nullptr);
	virtual ~RenderEngine();


public:
	bool Initialize(int width, int height, int guiWidth, IDXGIFactory7* factory, HWND wnd);
	bool InitScene();
	bool InitGUI(HWND wnd);

	void OnResize(int width, int height);


protected:
	void CreateCommandObjects();
	void CreateSwapChain(IDXGIFactory7* factory, HWND wnd);
	void CreateMainDepthBuffer();
	void CreateDepthBuffers();
	void CreateDescriptorHeaps();
	void CreateTextureBuffers();
	void UpdateGUI();


	void Update(float deltaTime);
	void RenderMeshes(const std::string& psoName, ID3D12GraphicsCommandList* commandList);
	void ComputeSPH(const std::string& psoName, int idx);
	void RenderSPH(const std::string& psoName, bool clear, bool isFinal);
	

	void SPHSimulation();

	//void Render(const std::string& psoName, int idx, RenderType renderType, bool isFinal, bool clear);
	void RenderGUI(bool isFinal);

public:
	void Tick(float deltaTime);
	void Quit();


private:
	void GenerateMips(ID3D12Resource* tex);

protected:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvCpuHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle() const;
	ID3D12Resource* GetCurrentSwapChainResource() const;

private:
	void FlushCommands();
	void FlushResourceCommands();

	//fence
private:

	UINT64 m_currentBufferFence = 0;
	UINT64 m_currentFence = 0;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_createBufferfence;

private:
	ID3D12Device5* m_device;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> gui_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> gui_commandList;

	// 버퍼 생성용
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_resourceCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_resourceCommandList;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

	UINT computeCommandCount = 4;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_computeCommandAllocators;
	std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> m_computeCommandLists;

private:
	D3D12_VIEWPORT m_viewport;
	D3D12_VIEWPORT m_hdrViewport;
	D3D12_RECT m_scissorRect;
	int m_width;
	int m_height;
	int m_guiWidth;
	HWND mainWnd;

private:
	int m_currentBackBufferIndex = 0;
	static const UINT m_swapChainBufferCount = 2;
	static const UINT m_dsBufferCount = 2;
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	std::array<float, 4> rtvClearColor;
	std::array<float, 4> blackClearColor;

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_swapChainRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainResources[m_swapChainBufferCount];

	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	// hdr 버퍼
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_hdrRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_hdrUAVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_hdrSRVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_hdrBuffer;

private:
	POINT currMousPt = { 0,0 };
	POINT prevMousePt = { 0,0 };

	std::unordered_map<uint32_t, std::string> r_idToName;
	std::unordered_map<std::string, uint32_t> r_nameToId;
	uint32_t r_idMax = 0;
	int r_selecteId = 0;

	// 임시 scene
private:
	std::unique_ptr<StaticMesh> m_mesh;
	DefaultLocalConstant localConstant;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_localCB;
	void* pLocalCB;

	float angle = 0.f;
	float rotateSpeed = 90.f;

	//sph
private:
	std::shared_ptr<SPH> m_sph;


	//camera
private:
	std::shared_ptr<Camera> m_camera;

	//font
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;
	bool resetFlag = false;
};
