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

	void Initialize(ID3D12Device5* device);

public:
	void InitCommands(ID3D12Device5* device);
	void InitCPU();
	void InitGPU(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);


public:
	void Tick(float deltaTime);

private:
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;

private:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;


	// hdr 버퍼
//private:
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_hdrRTVHeap;
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_hdrUAVHeap;
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_hdrSRVHeap;
//	Microsoft::WRL::ComPtr<ID3D12Resource> m_hdrBuffer;

};
