#pragma once

#include <vector>
#include "directxtk12\SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"

#include "PipelineState.h"

class StableFluids
{
public:
	StableFluids();

	void Initialize(ID3D12Device5* device, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);

public:
	void InitCPU();
	void InitGPU(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);


public:
	void Tick(float deltaTime);

private:
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
};
