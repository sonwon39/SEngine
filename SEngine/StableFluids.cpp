#include "StableFluids.h"
#include "GraphicsCommon.h"
#include "World.h"

using namespace Graphics;

StableFluids::StableFluids()
{
}



void StableFluids::Initialize(ID3D12Device5* device)
{
	InitCommands(device);

}


void StableFluids::InitCommands(ID3D12Device5* device)
{
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

void StableFluids::InitCPU()
{

}

void StableFluids::InitGPU(ID3D12CommandAllocator* cmdAlloc, ID3D12GraphicsCommandList* cmdList)
{

}

void StableFluids::Tick(float deltaTime)
{
}
