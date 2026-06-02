#pragma once

#include "IndexBuffer.h"

inline IndexBuffer::IndexBuffer()
{
}

inline IndexBuffer::~IndexBuffer()
{
}

template<typename DataType>
inline void IndexBuffer::Initialize(const std::vector<DataType>& data, D3D12_RESOURCE_FLAGS flag, ID3D12GraphicsCommandList* commandList, std::wstring name)
{
	GPUBuffer::InitializeWithData<DataType>(data, flag, commandList, name);
	indexBufferView.BufferLocation = gpu->GetGPUVirtualAddress();
	indexBufferView.Format = ((sizeof(DataType) == sizeof(uint16_t)) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
	indexBufferView.SizeInBytes = (UINT)(data.size() * sizeof(DataType));

}
