#include "SPH.h"
#include <random>
#include <iostream>

#include "GraphicsCommon.h"
#include "Renderer.h"
#include "RootSignature.h"

using namespace Renderer;

void SetPSO(const std::string& psoName, ComputePSO& pso)
{
	if (m_CPSOs.find(psoName) != m_CPSOs.end())
	{
		pso = m_CPSOs[psoName];
	}
	else
	{
		std::cout << "can't find pso -" << psoName << '\n';
	}
}

SPH::SPH()
{
}

void SPH::Initialize(ID3D12Device5* device, ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{
	using namespace Graphics;

	m_cbvSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	m_sphParticleConstant.particleCount = sphCurrParticleCount;
	// particleRadius 하나로부터 시뮬레이션 상수 자동 도출 (mass=1, 2D cubic spline)
	//   dx_eq = 2·radius      평형 packing 시 입자 중심 간 간격 (= 직경)
	//   h     = 1.3·dx_eq     cubic spline 2D 권장 — support 안에 이웃 ~21개
	//   rho0  = 1/dx_eq²      연속체 한계 ρ ≈ m·n = m/dx² (m=1)
	particleSpacing = 2.0f * particleRadius;
	h = 1.3f * particleSpacing;
	rho0 = 1.0f / (particleSpacing * particleSpacing* particleSpacing);

	m_sphParticleConstant.h = h;
	m_sphParticleConstant.hd = 1.0f / (h * h* h);
	m_sphParticleConstant.kernelCoefficient = kernelCoeff;
	m_sphParticleConstant.rho0 = rho0;
	m_sphParticleConstant.k = k;
	m_sphParticleConstant.mu = mu;

	m_sphParticleConstant.gGridMin = Vector3(-0.5f, -0.5f, -0.5f);
	m_sphParticleConstant.gGridMax = Vector3(0.5f, 0.5f, 0.5f);
	m_sphParticleConstant.gCellSize = 2.f * h;

	Vector3 gridLen = m_sphParticleConstant.gGridMax - m_sphParticleConstant.gGridMin;
	gridDim = Vector3(int(gridLen.x / h), int(gridLen.y / h), int(gridLen.z / h));
	cellCount = int(gridDim.x * gridDim.y * gridDim.z);

	m_sphParticleConstant.gGridDim = gridDim;
	m_sphParticleConstant.gCellCount = cellCount;
	m_sphParticleConstant.gScanCount = cellCount;

	// Pass2 재귀 scan 레벨 산정.
	// 매 level 의 blockCount(=ceil(scanCount/GROUP_SIZE)) 가 다음 level 의 scanCount 가 됨.
	// blockCount<=1 인 level 은 단일 블록 안에서 끝나는 terminal — 더 이상 재귀 안 함.
	m_pass2Levels.clear();
	{
		UINT sc = (UINT)cellCount;
		while (true)
		{
			Pass2Level lvl;
			lvl.scanCount  = sc;
			lvl.blockCount = (sc + GROUP_SIZE - 1) / GROUP_SIZE;
			m_pass2Levels.push_back(std::move(lvl));
			if (m_pass2Levels.back().blockCount <= 1) break;
			sc = m_pass2Levels.back().blockCount;
		}
	}

	// heap 생성
	utility->CreateDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_sphParticleUAVHeap[0], 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	utility->CreateDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_sphParticleUAVHeap[1], 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	utility->CreateDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_sphParticleSRVHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	utility->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_sortedSphParticleHeap, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	
	InitParticleCPU();
	InitParticleGPU(cmdAlloc, cmdList);
	
	utility->CreateConstantBuffer(sizeof(SPHParticleLocalConstant), m_sphParticleLocalCB, &pSPHParticleLocalCB);
	memcpy(pSPHParticleLocalCB, &m_sphParticleConstant, sizeof(SPHParticleLocalConstant));

	// Pass2 레벨별 CB. gScanCount 만 다르고 나머지는 m_sphParticleConstant 와 동일.
	// Pass2 셰이더는 gScanCount 만 읽으므로 Tick 의 particleCount/dt 갱신과 무관.
	for (auto& lvl : m_pass2Levels)
	{
		utility->CreateConstantBuffer(sizeof(SPHParticleLocalConstant), lvl.cb, &lvl.pCb);
		SPHParticleLocalConstant scanCB = m_sphParticleConstant;
		scanCB.gScanCount = lvl.scanCount;
		memcpy(lvl.pCb, &scanCB, sizeof(SPHParticleLocalConstant));
	}
	SetPSO(pass0CPSOname, pass0CPSO);
	SetPSO(pass1CPSOname, pass1CPSO);
	SetPSO(pass2aCPSOname, pass2aCPSO);
	SetPSO(pass2bCPSOname, pass2bCPSO);
	SetPSO(pass3CPSOname, pass3CPSO);
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

// 버퍼 clear용
void SPH::Pass0(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetPipelineState(pass0CPSO.GetPSO());
	commandList->SetComputeRootSignature(pass0CPSO.GetRootSignature()->GetSignature());
	
	commandList->SetComputeRootUnorderedAccessView(0, m_cellCounterBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, m_scatterCounterBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootConstantBufferView(2, m_sphParticleLocalCB->GetGPUVirtualAddress());

	UINT groupCount = (cellCount + GROUP_SIZE - 1) / GROUP_SIZE;
	commandList->Dispatch(groupCount, 1, 1);
}

void SPH::Pass1(ID3D12GraphicsCommandList* commandList)
{
	auto& particle = m_sphParticleBuffer[m_sphHeapIdx];

	commandList->SetPipelineState(pass1CPSO.GetPSO());
	commandList->SetComputeRootSignature(pass1CPSO.GetRootSignature()->GetSignature());

	commandList->SetComputeRootUnorderedAccessView(0, particle->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, m_cellCounterBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, m_particleCellIdBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootConstantBufferView(3, m_sphParticleLocalCB->GetGPUVirtualAddress());

	UINT groupXCount = (sphMaxParticleCount + GROUP_SIZE - 1) / GROUP_SIZE;
	commandList->Dispatch(groupXCount, 1, 1);
}

// Pass2a: per-block exclusive scan + emit block totals.
//   bindings (ScanBlocksCS.hlsl): t0=gInput(SRV), u0=gOutput(UAV), u1=gBlockSums(UAV), b0=GridCB(CBV)
//   input:  level 0 → m_cellCounterBuffer,  level k>0 → m_pass2Levels[k-1].blockSums
//   output, blockSums, cb 는 모두 m_pass2Levels[level] 안에 있음.
void SPH::Pass2a(ID3D12GraphicsCommandList* commandList, int level)
{
	auto& lvl = m_pass2Levels[level];
	D3D12_GPU_VIRTUAL_ADDRESS in_va = (level == 0)
		? m_cellCounterBuffer->GetGPUVirtualAddress()
		: m_pass2Levels[level - 1].blockSums->GetGPUVirtualAddress();

	commandList->SetComputeRootShaderResourceView (0, in_va);
	commandList->SetComputeRootUnorderedAccessView(1, lvl.output->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, lvl.blockSums->GetGPUVirtualAddress());
	commandList->SetComputeRootConstantBufferView (3, lvl.cb->GetGPUVirtualAddress());
	commandList->Dispatch(lvl.blockCount, 1, 1);
}

// Pass2b: m_pass2Levels[level].output[i] += m_pass2Levels[level+1].output[i / GROUP_SIZE]
//   bindings (AddBlockOffsetsCS.hlsl): u0=gPartial(UAV), t0=gScannedBlockSums(SRV), b0=GridCB(CBV)
//   terminal level (lastLevel) 에서는 호출하지 않음 — 재귀 종료.
void SPH::Pass2b(ID3D12GraphicsCommandList* commandList, int level)
{
	auto& lvl = m_pass2Levels[level];
	auto& sub = m_pass2Levels[level + 1];   // 한 level 깊은 곳의 output = scanned block sums

	commandList->SetComputeRootUnorderedAccessView(0, lvl.output->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView (1, sub.output->GetGPUVirtualAddress());
	commandList->SetComputeRootConstantBufferView (2, lvl.cb->GetGPUVirtualAddress());
	commandList->Dispatch(lvl.blockCount, 1, 1);
}

// 재귀 전역 exclusive scan.
//   scan(level):
//     Pass2a(level)
//     if terminal: return
//     transition blockSums[level] UAV→SRV     (다음 level 의 input)
//     scan(level + 1)                          ← 재귀
//     transition output[level+1] UAV→SRV      (Pass2b 의 gScannedBlockSums)
//     Pass2b(level)
//     transition들 다시 UAV 로 복원            (다음 프레임 호출 위해)
//
// 진입 조건: m_cellCounterBuffer 가 NON_PIXEL_SHADER_RESOURCE 상태.
//           m_pass2Levels 모든 buffer 들이 UNORDERED_ACCESS 상태.
// 종료 조건: 모든 buffer 상태가 진입 때와 동일하게 복원됨 (per-frame 재실행 가능).
void SPH::Pass2(ID3D12GraphicsCommandList* commandList,	int level)
{
	auto& lvl = m_pass2Levels[level];
	const int lastLevel = (int)m_pass2Levels.size() - 1;

	// ── Pass2a dispatch ──────────────────────────────────────────────────
	commandList->SetPipelineState(pass2aCPSO.GetPSO());
	commandList->SetComputeRootSignature(pass2aCPSO.GetRootSignature()->GetSignature());

	Pass2a(commandList, level);

	// terminal level: blockCount==1 → 단일 블록 안에서 scan 완료. 더 재귀 안 함.
	if (level == lastLevel) return;

	// 다음 level 이 blockSums 를 SRV 로 읽어야 함. Transition 자체가 UAV write 완료까지 implicit sync.
	{
		auto b = CD3DX12_RESOURCE_BARRIER::Transition(lvl.blockSums.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &b);
	}

	// ── 재귀 ─────────────────────────────────────────────────────────────
	Pass2(commandList, level + 1);

	// 재귀에서 돌아온 시점: m_pass2Levels[level+1].output 가 전역 scan 완료 상태 (UAV).
	// Pass2b 는 이걸 SRV 로 읽음.
	{
		auto b = CD3DX12_RESOURCE_BARRIER::Transition(m_pass2Levels[level + 1].output.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandList->ResourceBarrier(1, &b);
	}

	// ── Pass2b dispatch ──────────────────────────────────────────────────
	commandList->SetPipelineState(pass2bCPSO.GetPSO());
	commandList->SetComputeRootSignature(pass2bCPSO.GetRootSignature()->GetSignature());
	Pass2b(commandList, level);

	// ── 상태 복원 (다음 프레임 호출에서 UAV 로 다시 쓸 수 있도록) ─────────
	{
		D3D12_RESOURCE_BARRIER barriers[2] = {
			CD3DX12_RESOURCE_BARRIER::Transition(lvl.blockSums.Get(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
			CD3DX12_RESOURCE_BARRIER::Transition(m_pass2Levels[level + 1].output.Get(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		};
		commandList->ResourceBarrier(2, barriers);
	}
}

// Pass3: scatter — particle 들을 cellId 기준 정렬된 슬롯에 복사.
//   bindings (ScatterCS.hlsl):
//     u0 = gParticles       (RW, source)
//     u1 = gScatterCounter  (RW, per-cell write cursor, 호출 전 0 으로 클리어 필수)
//     u2 = gSortedParticles (RW, destination)
//     t0 = gParticleCellId  (SRV, from Pass1)
//     t1 = gCellStart       (SRV, = m_pass2Levels[0].output, from Pass2)
//     b0 = gCB              (CBV, gCB.particleCount 으로 early-exit)
//   Dispatch ceil(sphMaxParticleCount / GROUP_SIZE) — 셰이더가 particleCount 로 early-exit.
void SPH::Pass3(ID3D12GraphicsCommandList* commandList)
{
	auto& particle = m_sphParticleBuffer[m_sphHeapIdx];
	commandList->SetPipelineState(pass3CPSO.GetPSO());
	commandList->SetComputeRootSignature(pass3CPSO.GetRootSignature()->GetSignature());

	commandList->SetComputeRootUnorderedAccessView(0, particle->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, m_scatterCounterBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, m_sortedSphParticleBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(3, m_sortedIndicesBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView (4, m_particleCellIdBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView (5, m_pass2Levels[0].output->GetGPUVirtualAddress());
	commandList->SetComputeRootConstantBufferView (6, m_sphParticleLocalCB->GetGPUVirtualAddress());

	UINT groupXCount = (sphMaxParticleCount + GROUP_SIZE - 1) / GROUP_SIZE;
	commandList->Dispatch(groupXCount, 1, 1);
}

void SPH::Sort(ID3D12GraphicsCommandList* commandList)
{

}

void SPH::Compute(ID3D12GraphicsCommandList* commandList)
{
	auto& prevParticles = m_sphParticleBuffer[m_sphHeapIdx];
	auto& currParticles = m_sphParticleBuffer[1 - m_sphHeapIdx];

	commandList->SetComputeRootUnorderedAccessView(0, prevParticles->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(1, currParticles->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(2, m_sortedSphParticleBuffer->GetGPUVirtualAddress());
	commandList->SetComputeRootUnorderedAccessView(3, m_sortedIndicesBuffer->GetGPUVirtualAddress());

	commandList->SetComputeRootShaderResourceView(4, m_pass2Levels[0].output->GetGPUVirtualAddress());
	commandList->SetComputeRootShaderResourceView(5, m_cellCounterBuffer->GetGPUVirtualAddress());

	commandList->SetComputeRootConstantBufferView(6, m_sphParticleLocalCB->GetGPUVirtualAddress());

	UINT threadXCount = (sphMaxParticleCount + GROUP_SIZE - 1) / GROUP_SIZE;
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
	commandList->SetGraphicsRootConstantBufferView(2, m_sphParticleLocalCB->GetGPUVirtualAddress());

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
	//Vector3 basePosition(0.f, 0.f, 0.f);  // [-1,1]
	Vector3 basePosition(-0.45f, 0.2f, 0.f);  // [-0.5,0.5]
	Vector3 baseVelocity(1.f, 0.f, 0.f);

	float PI = 3.141592f;
	std::uniform_real_distribution<float> randomTheta(-PI, PI);
	std::uniform_int_distribution<int> randomColor(0, 9);
	std::uniform_real_distribution<float> randomDist(0.f, 0.25f);
	std::uniform_real_distribution<float> randomDistX(-0.2f, 0.2f);
	//std::uniform_real_distribution<float> randomDist(-0.48f, 0.48f);
	std::uniform_real_distribution<float> velDist(1.f, 2.f);

	// structured buffer cpu 데이터 초기화
	for (size_t i = 0; i < sphMaxParticleCount; i++)
	{
		SPHParticle& p = m_sphParticles[i];
		float theta = randomTheta(gen);
		p.position = basePosition + Vector3(randomDistX(gen), std::cos(theta), -std::sin(theta)) * randomDist(gen);
		//p.position = basePosition + Vector3(randomDist(gen), randomDist(gen), randomDist(gen));
		p.velocity = baseVelocity * velDist(gen);
		p.color = Vector3(0.0f, 0.0f, 1.0f);
		//p.color = colors[randomColor(gen)];
		p.acceleration = Vector3(0.f, 0.f, 0.f);
		p.radius = particleRadius;
	}
	UINT groupCount = (cellCount + GROUP_SIZE - 1) / GROUP_SIZE;

	m_cellCounter.resize(cellCount, 0);
	m_particleCellId.resize(sphMaxParticleCount, 0);
	m_scatterCounter.resize(cellCount, 0);
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
	Graphics::utility->CreateBuffer(m_cellCounter, m_cellCounterBuffer, m_cellCounterUpload, uavFlag, cmdList);
	Graphics::utility->CreateBuffer(m_particleCellId, m_sortedIndicesBuffer, m_sortedIndicesUpload, uavFlag, cmdList);
	Graphics::utility->CreateBuffer(m_particleCellId, m_particleCellIdBuffer, m_particleCellIdUpload, uavFlag, cmdList);
	Graphics::utility->CreateBuffer(m_sphParticles, m_sortedSphParticleBuffer, m_sortedSphParticleUpload, uavFlag, cmdList);
	Graphics::utility->CreateBuffer(m_scatterCounter, m_scatterCounterBuffer, m_scatterCounterUpload, uavFlag, cmdList);

	// Pass2 각 level의 output[scanCount] + blockSums[blockCount] 버퍼를 0으로 초기화 생성.
	// 실제 값은 Pass2a/Pass2b dispatch 가 채움.
	for (auto& lvl : m_pass2Levels)
	{
		std::vector<UINT> outInit(lvl.scanCount, 0);
		std::vector<UINT> bsInit (lvl.blockCount, 0);
		Graphics::utility->CreateBuffer(outInit, lvl.output,    lvl.outputUpload,    uavFlag, cmdList);
		Graphics::utility->CreateBuffer(bsInit,  lvl.blockSums, lvl.blockSumsUpload, uavFlag, cmdList);
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

	// 정렬용 view 생성
	utility->CreateStructuredResourceView(m_sortedSphParticleBuffer, DXGI_FORMAT_UNKNOWN, m_sortedSphParticleHeap->GetCPUDescriptorHandleForHeapStart(), DescriptorType::SRV, sphMaxParticleCount, sizeof(SPHParticle));

}
