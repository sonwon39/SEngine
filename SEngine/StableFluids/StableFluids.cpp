#include "StableFluids.h"
#include "GraphicsCommon.h"
#include "World.h"
#include "Renderer.h"
#include "RootSignature.h"
#include <vector>

using namespace Graphics;
using namespace std;
StableFluids::StableFluids()
{
}

void StableFluids::Initialize()
{
	InitCommands();
}

void StableFluids::InitCommands()
{
	if (m_world && m_world->GetDevice())
	{
		auto device = m_world->GetDevice();
		ThrowIfFailed(
			device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_commandAllocator)
			));

		ThrowIfFailed(
			device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				m_commandAllocator.Get(),
				nullptr,
				IID_PPV_ARGS(&m_commandList)
			));
		m_commandList->Close();
	}
}

void StableFluids::InitCPU()
{

}

void StableFluids::InitGPU(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{

}

void StableFluids::Tick(float deltaTime)
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	Sourcing();
	
	Projection();
	Advection();

	CopyDensityAndVelocity();
}

void StableFluids::CopyDensityAndVelocity()
{
	vector<D3D12_RESOURCE_BARRIER> barriers0;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_oldDensityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_DEST, barrier)) barriers0.push_back(barrier);
	if (m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_DEST, barrier)) barriers0.push_back(barrier);
	if (m_world->m_newDensityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_SOURCE, barrier)) barriers0.push_back(barrier);
	if (m_world->m_newVelocityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_SOURCE, barrier)) barriers0.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers0.size(), barriers0.data());

	m_commandList->CopyResource(m_world->m_oldDensityBuffer.GetResource(), m_world->m_newDensityBuffer.GetResource());
	m_commandList->CopyResource(m_world->m_oldVelocityBuffer.GetResource(), m_world->m_newVelocityBuffer.GetResource());
}

void StableFluids::Sourcing()
{
	AddSource();
	AddVorticity();
}

void StableFluids::AddSource()
{
	SetCPSO("sourcingCPSO");

	DescriptorHeap& sourcingHeap = m_world->m_sourcingHeap;
	vector<D3D12_RESOURCE_BARRIER> barriers0;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_oldDensityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers0.push_back(barrier);
	if (m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers0.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers0.size(), barriers0.data());

	ID3D12DescriptorHeap* heaps[] = { sourcingHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, sourcingHeap.GetGPUHandle(0));
	m_commandList->SetComputeRootConstantBufferView(1, m_world->gridCB.GetGPUAddress());
	m_commandList->SetComputeRootConstantBufferView(2, m_world->mouse->mouseCB.GetGPUAddress());

	Dispatch();
}

void StableFluids::AddVorticity()
{
	ComputeVelocityCurl();
	VorticityConfinement();
}

void StableFluids::ComputeVelocityCurl()
{
	SetCPSO("computeCurlCPSO");

	DescriptorHeap& heap = m_world->m_computeCurlHeap;

	vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_curlBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers.push_back(barrier);
	if (m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	ID3D12DescriptorHeap* heaps[] = { heap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, heap.GetGPUHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, heap.GetGPUHandle(1));
	m_commandList->SetComputeRootConstantBufferView(2, m_world->gridCB.GetGPUAddress());

	Dispatch();
}

void StableFluids::VorticityConfinement()
{
	SetCPSO("vorticityConfinementCPSO");

	DescriptorHeap& heap = m_world->m_vorticityConfinementHeap;

	vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_curlBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	if (m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	ID3D12DescriptorHeap* heaps[] = { heap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, heap.GetGPUHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, heap.GetGPUHandle(1));
	m_commandList->SetComputeRootConstantBufferView(2, m_world->gridCB.GetGPUAddress());

	Dispatch();
}

void StableFluids::Advection()
{
	SetCPSO("advectionCPSO");

	DescriptorHeap& advectionHeap = m_world->m_advectionHeap;

	vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_oldDensityBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	if (m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	if (m_world->m_newDensityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers.push_back(barrier);
	if (m_world->m_newVelocityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	ID3D12DescriptorHeap* heaps[] = { advectionHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, advectionHeap.GetGPUHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, advectionHeap.GetGPUHandle(2));
	m_commandList->SetComputeRootConstantBufferView(2, m_world->gridCB.GetGPUAddress());

	Dispatch();
}

void StableFluids::Projection()
{
	ComputeDivergence();

	for (int i = 0; i < 100; i++)
	{
		Jacobi(i);
	}

	Finalize();
}

void StableFluids::ComputeDivergence()
{
	SetCPSO("computeDivergenceCPSO");

	DescriptorHeap& computeDivergenceHeap = m_world->m_computeDivergenceHeap;

	vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	if (m_world->m_divergenceBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	ID3D12DescriptorHeap* heaps[] = { computeDivergenceHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, computeDivergenceHeap.GetGPUHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, computeDivergenceHeap.GetGPUHandle(1));
	m_commandList->SetComputeRootConstantBufferView(2, m_world->gridCB.GetGPUAddress());

	Dispatch();
}


void StableFluids::Jacobi(int idx)
{
	SetCPSO("jacobiCPSO");

	int index0 = idx % 2;
	int index1 = (idx + 1) % 2;

	DescriptorHeap& jacobiHeap = m_world->m_jacobiHeap[index0];

	vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_pressureBuffer[index0].Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	if (m_world->m_pressureBuffer[index1].Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers.push_back(barrier);
	if (m_world->m_divergenceBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	ID3D12DescriptorHeap* heaps[] = { jacobiHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, jacobiHeap.GetGPUHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, jacobiHeap.GetGPUHandle(2));
	m_commandList->SetComputeRootConstantBufferView(2, m_world->gridCB.GetGPUAddress());

	Dispatch();
	//CopyPressure();
}

void StableFluids::CopyPressure()
{

	vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_pressureBuffer[0].Transition(D3D12_RESOURCE_STATE_COPY_DEST, barrier)) barriers.push_back(barrier);
	if (m_world->m_pressureBuffer[1].Transition(D3D12_RESOURCE_STATE_COPY_SOURCE, barrier)) barriers.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	m_commandList->CopyResource(m_world->m_pressureBuffer[0].GetResource(), m_world->m_pressureBuffer[1].GetResource());
}

void StableFluids::Finalize()
{
	SetCPSO("computeFinalVelocityCPSO");

	DescriptorHeap& heap = m_world->m_computeFinalVelocityHeap;

	vector<D3D12_RESOURCE_BARRIER> barriers;
	D3D12_RESOURCE_BARRIER barrier;

	if (m_world->m_pressureBuffer[1].Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, barrier)) barriers.push_back(barrier);
	if (m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, barrier)) barriers.push_back(barrier);
	m_commandList->ResourceBarrier((UINT)barriers.size(), barriers.data());

	ID3D12DescriptorHeap* heaps[] = { heap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, heap.GetGPUHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, heap.GetGPUHandle(1));
	m_commandList->SetComputeRootConstantBufferView(2, m_world->gridCB.GetGPUAddress());

	Dispatch();
}

void StableFluids::Execute(ID3D12CommandQueue* commandQueue)
{
	m_commandList->Close();
	ID3D12CommandList* commands[] = { m_commandList.Get() };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);
}

void StableFluids::Dispatch()
{
	UINT width = (UINT)m_world->gridCB.localConstant.gGridDim.x;
	UINT height = (UINT)m_world->gridCB.localConstant.gGridDim.y;
	UINT groupCountX = (width + SF_GROUP_SIZE_X - 1) / SF_GROUP_SIZE_X;
	UINT groupCountY = (height + SF_GROUP_SIZE_Y - 1) / SF_GROUP_SIZE_Y;
	m_commandList->Dispatch(groupCountX, groupCountY, 1);
}

void StableFluids::SetCPSO(const std::string psoName)
{
	ComputePSO cpso = Renderer::GetComputePSO(psoName);
	m_commandList->SetPipelineState(cpso.GetPSO());
	m_commandList->SetComputeRootSignature(cpso.GetRootSignature()->GetSignature());

}
