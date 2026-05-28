#include "StableFluids.h"
#include "GraphicsCommon.h"
#include "World.h"
#include "Renderer.h"
#include "RootSignature.h"

using namespace Graphics;

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
	CopyDensityAndVelocity();
	AddSmokes();
	Advection();
}

void StableFluids::CopyDensityAndVelocity()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	D3D12_RESOURCE_BARRIER barriers0[] =
	{
		m_world->m_oldDensityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_DEST),
		m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_DEST),
		m_world->m_newDensityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_SOURCE),
		m_world->m_newVelocityBuffer.Transition(D3D12_RESOURCE_STATE_COPY_SOURCE)
	};
	m_commandList->ResourceBarrier(ARRAYSIZE(barriers0), barriers0);

	m_commandList->CopyResource(m_world->m_oldDensityBuffer.GetResource(), m_world->m_newDensityBuffer.GetResource());
	m_commandList->CopyResource(m_world->m_oldVelocityBuffer.GetResource(), m_world->m_newVelocityBuffer.GetResource());
}

void StableFluids::AddSmokes()
{
	ComputePSO cpso = Renderer::GetComputePSO("addSmokesCPSO");

	Texture2D& oldDensity = m_world->m_oldDensityBuffer;
	DescriptorHeap & addSmokesHeap = m_world->m_addSmokesHeap;

	D3D12_RESOURCE_BARRIER barriers0[] =
	{
		m_world->m_oldDensityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	};
	m_commandList->ResourceBarrier(ARRAYSIZE(barriers0), barriers0);

	m_commandList->SetPipelineState(cpso.GetPSO());
	m_commandList->SetComputeRootSignature(cpso.GetRootSignature()->GetSignature());

	ID3D12DescriptorHeap* heaps[] = { addSmokesHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, addSmokesHeap.GetGPUHandle(0));
	m_commandList->SetComputeRootConstantBufferView(1, m_world->gridCB.GetGPUAddress());
	m_commandList->SetComputeRootConstantBufferView(2, m_world->mouse->mouseCB.GetGPUAddress());

	UINT groupCountX = (oldDensity.m_width + SF_GROUP_SIZE - 1) / SF_GROUP_SIZE;
	UINT groupCountY = oldDensity.m_height;

	m_commandList->Dispatch(groupCountX, groupCountY, 1);

	
}

void StableFluids::Advection()
{
	ComputePSO cpso = Renderer::GetComputePSO("advectionCPSO");

	Texture2D& oldDensity = m_world->m_oldDensityBuffer;
	DescriptorHeap& advectionHeap = m_world->m_advectionHeap;

	D3D12_RESOURCE_BARRIER barriers0[] =
	{
		m_world->m_oldDensityBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE),
		m_world->m_oldVelocityBuffer.Transition(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE),
		m_world->m_newDensityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
		m_world->m_newVelocityBuffer.Transition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
	};
	m_commandList->ResourceBarrier(ARRAYSIZE(barriers0), barriers0);

	m_commandList->SetPipelineState(cpso.GetPSO());
	m_commandList->SetComputeRootSignature(cpso.GetRootSignature()->GetSignature());

	ID3D12DescriptorHeap* heaps[] = { advectionHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, advectionHeap.GetGPUHandle(0));
	m_commandList->SetComputeRootDescriptorTable(1, advectionHeap.GetGPUHandle(2));
	m_commandList->SetComputeRootConstantBufferView(2, m_world->gridCB.GetGPUAddress());

	UINT groupCountX = (oldDensity.m_width + SF_GROUP_SIZE - 1) / SF_GROUP_SIZE;
	UINT groupCountY = oldDensity.m_height;

	m_commandList->Dispatch(groupCountX, groupCountY, 1);
}

void StableFluids::Execute(ID3D12CommandQueue* commandQueue)
{
	m_commandList->Close();
	ID3D12CommandList* commands[] = { m_commandList.Get() };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);
}
