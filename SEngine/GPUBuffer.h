#pragma once

#include "wrl.h"
#include "d3d12.h"
#include <string>
#include <vector>

class GPUBuffer
{

  public:
    GPUBuffer();
    virtual ~GPUBuffer();

  public:
    bool Transition(D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_BARRIER& outBarrier);
    // upload 버퍼 clear
    virtual void Clear();
    void Reset();
    void Map();

  public:
    uint64_t bufferSize;
    UINT dataCount;

  public:
    ID3D12Resource* Get() const
    {
        return gpu.Get();
    }
    ID3D12Resource** ReleaseAndGetAddressOf()
    {
        return gpu.ReleaseAndGetAddressOf();
    }
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
    {
        return gpu->GetGPUVirtualAddress();
    }

  public:
    void SetResourceStates(D3D12_RESOURCE_STATES newState)
    {
        m_currentState = newState;
    }

  protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> gpu;
    Microsoft::WRL::ComPtr<ID3D12Resource> upload;
    D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
    void* pGPU = nullptr;
};
#include "GPUBuffer.inl"
