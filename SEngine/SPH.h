#pragma once

#include <vector>
#include "directxtk12\SimpleMath.h"
#include "wrl.h"
#include "d3d12.h"

#include "particle.h"

class SPH
{
public:
	SPH();

	void Initialize(ID3D12Device5* device, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);

public:
	void Initialize();
	void InitParticleCPU();
	void InitParticleGPU(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);


public:
	void Tick(float deltaTime);
	void Compute(ID3D12GraphicsCommandList* commandList);
	void Render(ID3D12GraphicsCommandList* commandList);

	void Reset(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList);

public:
	ID3D12DescriptorHeap* GetParticleUAVHeap(int idx) {	return m_sphParticleUAVHeap[idx].Get();}

private:
	int sphCurrParticleCount = 0;
	float countTick = 0.f;
	float sphCountIncreaseSpeed = 1000.f;
	int sphMaxParticleCount = 20000;

	std::vector<SPHParticle> m_sphParticles;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_sphParticleBuffer[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_sphParticleUpload[2];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_sphParticleUAVHeap[2];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_sphParticleSRVHeap;

	SPHParticleLocalConstant m_sphParticleConstant;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_sphParticleLocalCB;
	void* pSPHParticleLocalCB;

	int m_sphHeapIdx = 0;

// sph particle 계수
public:
	float particleRadius = 0.01f;
	float particleSpacing;
	float h;

	float kernelCoeff = 15.0f / (7.0f * 3.141592f);
	float rho0;       // particleRadius로부터 Initialize()에서 자동 계산
	float k = 1.f;
	float mu = 10.f;

private:
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
};
