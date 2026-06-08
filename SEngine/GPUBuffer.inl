#pragma once

#include "GPUBuffer.h"
#include "GraphicsCommon.h"

using namespace Graphics;

inline GPUBuffer::GPUBuffer()
{
}

inline GPUBuffer::~GPUBuffer()
{
    Clear();
}

inline void GPUBuffer::Clear()
{
    if (upload)
        upload.Reset();
}

inline void GPUBuffer::Reset()
{
    if (gpu)
        gpu.Reset();
}

inline bool GPUBuffer::Transition(D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_BARRIER& outBarrier)
{
    if (m_currentState == newState)
    {
        if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {

            outBarrier = CD3DX12_RESOURCE_BARRIER::UAV(gpu.Get());
            return true;
        }

        return false;
    }

    outBarrier = CD3DX12_RESOURCE_BARRIER::Transition(gpu.Get(), m_currentState, newState);

    m_currentState = newState;
    return true;
}
