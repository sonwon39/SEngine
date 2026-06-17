#pragma once

#include "d3d12.h"
#include "directxtk12\SimpleMath.h"
#include <vector>

#include "Texture2D.h"
#include "StructuredBuffer.h"
#include "DescriptorHeap.h"
#include "NoiseLocalConstant.h"

class Noise
{
  public:
    Noise();
    virtual ~Noise();

    void Initialize(UINT width, UINT height);
    void InitGPU(UINT width, UINT height);
    void GeneratePerlinNoise();
    void GenerateCurlNoise();
    void ResetCommand();
    void InitCommands();
    void InitCPU();
    void CurlNoiseSimulation();

	// partice 위치 갱신
    void ComputeParticles();
    void RenderParticles(ID3D12GraphicsCommandList* c);

  public:
    void Execute(ID3D12CommandQueue* commandQueue);
    void Dispatch(UINT width, UINT height, UINT gs_x, UINT gs_y);
    void SetCPSO(const std::string psoName);

  private:
    DescriptorHeap m_noiseHeap;
    Texture2D m_perlinNoise;
    Texture2D m_curlNoise;

    UINT perlinWidth = 256;
    UINT perlinHeight = 256;

    // curlNoise (속도장) 크기
    UINT m_width = 0;
    UINT m_height = 0;

  private:
    UINT particleCount = 1;
    StructuredBuffer particles;
    std::vector<NoiseParticle> particleCPU;

  private:
    std::string perlinCPSOName = "perlinNoiseCPSO";
    std::string curlCPSOName = "curlNoiseCPSO";
    std::string curlSimulationCPSOName = "curlSimulationCPSO";

  private:
    DXGI_FORMAT perlinFormat = DXGI_FORMAT_R16_FLOAT;
    DXGI_FORMAT curlFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

  private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
};
