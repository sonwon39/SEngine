#pragma once

#include <array>
#include < memory>

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

#include "SPH/SPH.h"
#include "StableFluids/StableFluids.h"
#include "Noise/Noise.h"
#include "ConstantBuffer.h"
#include "GPUBUffer.h"
#include "DescriptorHeap.h"
#include "MeshBatch.h"
#include "StructuredBuffer.h"
#include "ImageInfo.h"

class StaticMesh;
class CameraComponent;



enum RenderType
{
    RT_TEXT,
    RT_Default,
    RT_PointCloud,
    RT_CubeMap,
    RT_Dot /*렌더타겟에 점하나 넣고 texture 그리기 용도*/
};

enum RenderPassType
{
    RPT_Default,
    RPT_CubeMapPass,
    RPT_DepthOnlyPass
};

class RenderEngine
{
  public:
    RenderEngine(ID3D12Device5* device = nullptr);
    virtual ~RenderEngine();

  public:
    bool Initialize(int width, int height, int guiWidth, IDXGIFactory7* factory);
	// Scene에서 사용할 GPU버퍼 생성
    bool InitScene();

    //  mesh buffer 초기화 (vertex, index 버퍼)
    void InitMeshBuffer();
    void InitShaderResources();
    bool InitGUI();

    void OnResize(int width, int height);

  protected:
    void CreateCommandObjects();
    void CreateSwapChain(IDXGIFactory7* factory);
    void CreateMainDepthBuffer();
    void CreateDepthBuffers();
    void CreateDescriptorHeaps();

  protected:
    void UpdateGUI();
    void Update(float deltaTime);

    void SPHTick(float deltaTime);
    void StableFluidsTick(float deltaTime);
    void RenderTick(float deltaTime);
    void NoiseSimulationTick(float deltaTime);
    void GenerateNoise();


	// RSSet, barrier, Clear, OMSet
    void InitRenderPipeline(bool clear = true);

    void RenderMeshes();
    void RenderSPH(const std::string& psoName, bool clear);

    void RenderGUI();

  public:
    void Tick(float deltaTime);
    void Quit();

  private:
    void GenerateMips(ID3D12Resource* tex);

  protected:
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRtvCpuHandle() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCpuHandle(int idx) const;
    ID3D12Resource* GetCurrentSwapChainResource() const;

    void ResetCommand();

  private:
    void BindMainHeap();
    void FlushCommands();
    void FlushRenderCommands();
    void Execute();

  private:
    // 현재 백버퍼를 readback 버퍼로 복사하고 ImageInfo를 반환 (저장 트리거는 호출측이 결정)
    ImageInfo ReadbackBackbuffer();
	void SaveTextureGPU(const std::string& resultPath);

    // PNG 없이 바로 mp4로 인코딩: ffmpeg 자식 프로세스 stdin 파이프
    // inFps: 입력(캡처) 프레임레이트 = 실시간 재생 속도. outFps: 저장 mp4 fps(0이면 inFps와 동일).
    bool OpenVideoPipe(int width, int height, int inFps, int outFps, const std::string& outPath);
    void WriteVideoFrame(const std::vector<uint8_t>& frame);
    void CloseVideoPipe();

  public:
    void RegistMeshBatch(std::shared_ptr<MeshBatch> meshBatch);
    void RegistCamera(CameraComponent* camera);

  public:
    std::string GetCurrPSOName() const
    {
        return m_currPSOName;
    }
    void SetCurrPSOName(std::string newPSOName)
    {
        m_currPSOName = newPSOName;
    }

    // fence
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
    // 버퍼 생성용
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_resourceCommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_resourceCommandList;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

  private:
    D3D12_VIEWPORT m_viewport;
    D3D12_VIEWPORT m_hdrViewport;
    D3D12_RECT m_scissorRect;
    int m_width;
    int m_height;
    int m_guiWidth;

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
    DescriptorHeap m_swapChainRTVHeap;
    GPUBuffer m_swapChainResources[m_swapChainBufferCount];

    Texture2D m_depthBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
    DescriptorHeap m_dsvHeap;

  private:
    POINT currMousPt = {0, 0};
    POINT prevMousePt = {0, 0};

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

    // simulation
  private:
    std::shared_ptr<SPH> m_sph;
    std::shared_ptr<StableFluids> m_stableFluids;
    std::shared_ptr<Noise> m_noise;

    // font
  private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_guiFontHeap;
    bool resetFlag = false;

  private:
    std::vector<std::shared_ptr<MeshBatch>> meshBatchs;
    std::shared_ptr<Material> cubemapMaterial;
    std::vector<CameraComponent*> m_camera;

  private:
	GPUBuffer readbackBuffer;

    // 포트폴리오용 자동 녹화 (고정 dt 오프라인 렌더 → PNG 연번)
    // SENGINE_RECORD 환경변수가 있으면 활성화. SENGINE_RECORD_FRAMES / SENGINE_RECORD_FPS로 조절.
  private:
    bool m_recording = true;
    int m_recordFrame = 0;       // 시작부터의 누적 프레임
    int m_recordWarmup = 60;     // 초반 워밍업 프레임 수 (트레일 형성 대기)
    int m_recordCount = 300;     // 저장할 프레임 수
    float m_recordDt = 1.0f / 60.0f; // 캡처 timestep (= 입력 fps). 1/60으로 바꾸면 진짜 60fps
    int m_outputFps = 60;         // 저장 mp4 fps (0이면 캡처 fps와 동일). 예: 60이면 60fps로 저장

    // ffmpeg 파이프 핸들
    HANDLE m_ffmpegStdin = nullptr;
    HANDLE m_ffmpegProcess = nullptr;
    bool m_videoPipeOpen = false;
    std::string videoName;

  private:
    std::string m_currPSOName = "";
};
