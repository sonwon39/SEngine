#include "SPH.h"
#include <random>
#include "GraphicsCommon.h"

SPH::SPH()
{
}

void SPH::Initialize(ID3D12Device5* device, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{
	using namespace Graphics;

	m_cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// heap 생성
	utility->CreateDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_sphParticleUAVHeap[0], 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	utility->CreateDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_sphParticleUAVHeap[1], 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	utility->CreateDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_sphParticleSRVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	InitParticleCPU();
	InitParticleGPU(cmdAlloc, cmdList);
	
	utility->CreateConstantBuffer(sizeof(SPHParticleLocalConstant), m_sphParticleLocalCB, &pSPHParticleLocalCB);

	m_sphParticleConstant.particleCount = sphCurrParticleCount;

	// particleRadius 하나로부터 시뮬레이션 상수 자동 도출 (mass=1, 2D cubic spline)
	//   dx_eq = 2·radius      평형 packing 시 입자 중심 간 간격 (= 직경)
	//   h     = 1.3·dx_eq     cubic spline 2D 권장 — support 안에 이웃 ~21개
	//   rho0  = 1/dx_eq²      연속체 한계 ρ ≈ m·n = m/dx² (m=1)
	particleSpacing = 2.0f * particleRadius;
	h               = 1.3f * particleSpacing;
	rho0            = 1.0f / (particleSpacing * particleSpacing);

	m_sphParticleConstant.h    = h;
	m_sphParticleConstant.hd   = 1.0f / (h * h);
	m_sphParticleConstant.kernelCoefficient = kernelCoeff;
	m_sphParticleConstant.rho0 = rho0;
	m_sphParticleConstant.k    = k;
	m_sphParticleConstant.mu   = mu;
	memcpy(pSPHParticleLocalCB, &m_sphParticleConstant, sizeof(SPHParticleLocalConstant));
}

void SPH::Tick(float deltaTime)
{	
	if (countTick < sphMaxParticleCount)
	{
		countTick += sphCountIncreaseSpeed * deltaTime;

		if (countTick >= sphMaxParticleCount)
			countTick = float(sphMaxParticleCount);

		sphCurrParticleCount = int(countTick);
	}

	m_sphParticleConstant.particleCount = sphCurrParticleCount;
	m_sphParticleConstant.dt = 1 / 300.f;
	memcpy(pSPHParticleLocalCB, &m_sphParticleConstant, sizeof(SPHParticleLocalConstant));
}


void SPH::Compute(ID3D12GraphicsCommandList* commandList)
{
	ID3D12DescriptorHeap* heap = GetParticleUAVHeap(m_sphHeapIdx);
	ID3D12DescriptorHeap* heaps[] = {
		heap
	};

	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetComputeRootDescriptorTable(0, heap->GetGPUDescriptorHandleForHeapStart());
	commandList->SetComputeRootConstantBufferView(1, m_sphParticleLocalCB->GetGPUVirtualAddress());

	UINT numThreadsX = 1024;
	UINT threadXCount = (sphMaxParticleCount + numThreadsX - 1) / numThreadsX;

	commandList->Dispatch(threadXCount, 1, 1);
}


void SPH::Render(ID3D12GraphicsCommandList* commandList)
{
	m_sphHeapIdx = (m_sphHeapIdx + 1) % 2;

	ID3D12DescriptorHeap* heaps[] = {
		m_sphParticleSRVHeap.Get()
	};

	CD3DX12_GPU_DESCRIPTOR_HANDLE sphHandle(m_sphParticleSRVHeap->GetGPUDescriptorHandleForHeapStart(), m_sphHeapIdx, m_cbvSrvDescriptorSize);
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(0, sphHandle);


	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	commandList->DrawInstanced(sphCurrParticleCount, 1, 0, 0);
}

void SPH::Reset(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{
	InitParticleCPU();
	InitParticleGPU(cmdAlloc, cmdList);

	memcpy(pSPHParticleLocalCB, &m_sphParticleConstant, sizeof(SPHParticleLocalConstant));
}

void SPH::InitParticleCPU()
{
	sphCurrParticleCount = 0;
	countTick = 0.f;

	Vector3 basePosition(-0.8f, 0.7f, 1.f);
	Vector3 baseVelocity(1.f, 0.f, 0.f);
	std::random_device rd;
	std::mt19937 gen(rd());

	m_sphParticles.resize(sphMaxParticleCount);
	std::vector<Vector3> colors =
	{
		Vector3(1.0f, 0.0f, 0.0f), // Red
		Vector3(0.0f, 1.0f, 0.0f), // Green
		Vector3(0.0f, 0.0f, 1.0f), // Blue
		Vector3(1.0f, 1.0f, 0.0f), // Yellow
		Vector3(1.0f, 0.0f, 1.0f), // Magenta
		Vector3(0.0f, 1.0f, 1.0f), // Cyan
		Vector3(1.0f, 0.5f, 0.0f), // Orange
		Vector3(0.6f, 0.2f, 1.0f), // Purple
		Vector3(1.0f, 0.75f, 0.8f), // Pink
		Vector3(0.5f, 1.0f, 0.5f), // Light Green
	};

	float PI = 3.141592f;
	std::uniform_real_distribution<float> randomTheta(0, PI / 2.f);
	std::uniform_int_distribution<int> randomColor(0, 9);
	std::uniform_real_distribution<float> randomDist(0.f, 0.4f);
	std::uniform_real_distribution<float> velDist(1.f, 2.f);

	// structured buffer cpu 데이터 초기화
	for (size_t i = 0; i < sphMaxParticleCount; i++)
	{
		SPHParticle& p = m_sphParticles[i];
		float theta = randomTheta(gen);
		p.position = basePosition + Vector3(std::cos(theta), -std::sin(theta), 0.f) * randomDist(gen);
		p.velocity = baseVelocity * velDist(gen);
		p.color = Vector3(0.0f, 0.0f, 1.0f);
		//p.color = colors[randomColor(gen)];
		p.acceleration = Vector3(0.f, 0.f, 0.f);
		p.radius = particleRadius;
	}

}
void SPH::InitParticleGPU(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{
	using namespace Graphics;

	cmdAlloc->Reset();
	cmdList->Reset(cmdAlloc, nullptr);

	// structured gpu buffer 생성
	D3D12_RESOURCE_FLAGS uavFlag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	for (size_t i = 0; i < 2; i++)
	{
		Graphics::utility->CreateBuffer(m_sphParticles, m_sphParticleBuffer[i], m_sphParticleUpload[i], uavFlag, cmdList);
	}

	cmdList->Close();

	// view 생성
	CD3DX12_CPU_DESCRIPTOR_HANDLE prev_handle(m_sphParticleUAVHeap[0]->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE curr_handle(m_sphParticleUAVHeap[1]->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle(m_sphParticleSRVHeap->GetCPUDescriptorHandleForHeapStart());
	utility->CreateStructuredResourceView(m_sphParticleBuffer[0], DXGI_FORMAT_UNKNOWN, prev_handle, DescriptorType::UAV, sphMaxParticleCount, sizeof(SPHParticle));
	utility->CreateStructuredResourceView(m_sphParticleBuffer[1], DXGI_FORMAT_UNKNOWN, curr_handle, DescriptorType::UAV, sphMaxParticleCount, sizeof(SPHParticle));
	utility->CreateStructuredResourceView(m_sphParticleBuffer[0], DXGI_FORMAT_UNKNOWN, srv_handle, DescriptorType::SRV, sphMaxParticleCount, sizeof(SPHParticle));

	prev_handle.Offset(1, m_cbvSrvDescriptorSize);
	curr_handle.Offset(1, m_cbvSrvDescriptorSize);
	srv_handle.Offset(1, m_cbvSrvDescriptorSize);

	utility->CreateStructuredResourceView(m_sphParticleBuffer[1], DXGI_FORMAT_UNKNOWN, prev_handle, DescriptorType::UAV, sphMaxParticleCount, sizeof(SPHParticle));
	utility->CreateStructuredResourceView(m_sphParticleBuffer[0], DXGI_FORMAT_UNKNOWN, curr_handle, DescriptorType::UAV, sphMaxParticleCount, sizeof(SPHParticle));
	utility->CreateStructuredResourceView(m_sphParticleBuffer[1], DXGI_FORMAT_UNKNOWN, srv_handle, DescriptorType::SRV, sphMaxParticleCount, sizeof(SPHParticle));

}
