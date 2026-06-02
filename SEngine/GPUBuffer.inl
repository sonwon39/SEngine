#pragma once

#include "GPUBuffer.h"
#include "GraphicsCommon.h"

using namespace Graphics;


inline GPUBuffer::GPUBuffer()
{
}

inline GPUBuffer::~GPUBuffer()
{
}

inline void GPUBuffer::Initialize(D3D12_HEAP_TYPE heapType, UINT size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, std::wstring name)
{
	utility->CreateBuffer(gpu, heapType, size, flags, state, name);
}

template<typename DataType>
inline void GPUBuffer::InitializeWithData(const std::vector<DataType>& data, D3D12_RESOURCE_FLAGS flag, ID3D12GraphicsCommandList* commandList, std::wstring name)
{
	utility->CreateBuffer<DataType>(data, gpu, upload, flag, commandList);
	bufferSize = UINT(data.size() * sizeof(DataType));
	dataCount = UINT(data.size());
	gpu->SetName(name.c_str());
}
