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

public:
	void Initialize();
	void InitCommands();
	void InitCPU();
	void InitGPU(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);


public:
	void Tick(float deltaTime);

	void CopyDensityAndVelocity();
	void Sourcing();
	void Advection();
	void Projection();
	void ComputeDivergence();
	void Jacobi(int idx);
	void CopyPressure();
	void Finalize();
	void Execute(ID3D12CommandQueue* commandQueue);

	void Dispatch();

	void SetCPSO(const std::string psoName);


private:
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;

private:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;



};
