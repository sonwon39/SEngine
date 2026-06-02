#pragma once

#include "wrl.h"
#include "d3d12.h"
#include <string>
#include <vector>

class GPUBuffer {

public:
	GPUBuffer();
	virtual ~GPUBuffer();

public:
	template<typename DataType>
	void InitializeWithData(const std::vector<DataType>& data, D3D12_RESOURCE_FLAGS flag, ID3D12GraphicsCommandList* commandList, std::wstring name = L"");
	void Initialize(D3D12_HEAP_TYPE heapType, UINT size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, std::wstring name = L"");

public:
	UINT bufferSize;
	UINT dataCount;

public:
	ID3D12Resource* GetResource() const { return gpu.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return gpu->GetGPUVirtualAddress(); }

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> gpu;
	Microsoft::WRL::ComPtr<ID3D12Resource> upload;
	D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
};
#include "GPUBuffer.inl"
