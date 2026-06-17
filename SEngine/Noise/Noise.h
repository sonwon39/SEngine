#pragma once

#include "d3d12.h"
#include "directxtk12\SimpleMath.h"

#include "Texture2D.h"
#include "DescriptorHeap.h"
#include "NoiseLocalConstant.h"

class Noise
{
  public:
    Noise();
    virtual ~Noise();

    void Initialize();
    void InitGPU();
    void GenerateNoise();
    void InitCommands();

  public:
    void Execute(ID3D12CommandQueue* commandQueue);
    void Dispatch(UINT gs_x, UINT gs_y);
    void SetCPSO(const std::string psoName);

  private:
    DescriptorHeap m_noiseHeap;
    Texture2D m_perlinNoise;
    UINT width = 256;
    UINT height = 256;

  private:
    std::string perlinCPSOName = "perlinCPSO";

  private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
};
