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
	AddSmokes();
}

void StableFluids::AddSmokes()
{
	ComputePSO cpso = Renderer::GetComputePSO("addSmokesCPSO");
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	Texture2D& hdrTexture = m_world->m_hdrBuffer;
	DescriptorHeap & hdrUavHeap = m_world->m_hdrUavHeap;

	m_commandList->SetPipelineState(cpso.GetPSO());
	m_commandList->SetComputeRootSignature(cpso.GetRootSignature()->GetSignature());

	ID3D12DescriptorHeap* heaps[] = { hdrUavHeap.GetHeap() };
	m_commandList->SetDescriptorHeaps(1, heaps);
	m_commandList->SetComputeRootDescriptorTable(0, hdrUavHeap.GetGPUHandle(0));
	m_commandList->SetComputeRootConstantBufferView(1, m_world->gridCB.GetGPUAddress());
	m_commandList->SetComputeRootConstantBufferView(2, m_world->mouse->mouseCB.GetGPUAddress());

	UINT groupCountX = (hdrTexture.m_width + SF_GROUP_SIZE - 1) / SF_GROUP_SIZE;
	UINT groupCountY = hdrTexture.m_height;

	m_commandList->Dispatch(groupCountX, groupCountY, 1);
}

void StableFluids::Execute(ID3D12CommandQueue* commandQueue)
{
	m_commandList->Close();
	ID3D12CommandList* commands[] = { m_commandList.Get() };
	commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);
}
