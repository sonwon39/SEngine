#pragma once

#include "StructuredBuffer.h"

inline StructuredBuffer::StructuredBuffer()
{
}
inline StructuredBuffer::~StructuredBuffer()
{
}

template <typename DataType>
inline void StructuredBuffer::Initialize(const std::vector<DataType>& data, D3D12_RESOURCE_FLAGS flag,
                                    ID3D12GraphicsCommandList* commandList, std::wstring name)
{
    utility->CreateBuffer<DataType>(data, gpu, upload, flag, commandList);
    bufferSize = UINT(data.size() * sizeof(DataType));
    dataCount = UINT(data.size());
    gpu->SetName(name.c_str());

}
template <typename DataType>
inline void StructuredBuffer::Initialize(DataType* data, long bufferSize, D3D12_RESOURCE_FLAGS flag,
                                         ID3D12GraphicsCommandList* commandList, std::wstring name)
{
    utility->CreateUploadBuffer<DataType>(data, bufferSize, gpu, flag, commandList);
    bufferSize = bufferSize;
    dataCount = UINT(bufferSize / sizeof(DataType));
    gpu->SetName(name.c_str());
}
template <typename DataType> inline void StructuredBuffer::CopyToGpu(const std::vector<DataType>& data)
{
    if (!pGPU)
        return;

	UINT dataSize = data.size() * sizeof(DataType);
    if (dataSize > bufferSize)
        return;

	if (pGPU)
	{
        memcpy(pGPU, data.data(), dataSize);
	}
}
template <typename DataType> inline void StructuredBuffer::CopyToGpu(DataType* data, long dataSize)
{
    if (!pGPU)
        return;

    if (dataSize > bufferSize)
        return;

    if (pGPU)
    {
        memcpy(pGPU, data, dataSize);
    }
}
