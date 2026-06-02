#pragma once

#include "VertexBuffer.h"


inline VertexBuffer::VertexBuffer()
{
}

inline VertexBuffer::~VertexBuffer()
{
}

template<typename DataType>
inline void VertexBuffer::Initialize(const std::vector<DataType>& data, D3D12_RESOURCE_FLAGS flag, ID3D12GraphicsCommandList* commandList, std::wstring name)
{
	GPUBuffer::InitializeWithData<DataType>(data, flag, commandList, name);
	vertexBufferView.BufferLocation = gpu->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = (UINT)(data.size() * sizeof(DataType));
	vertexBufferView.StrideInBytes = (UINT)(sizeof(DataType));
}
