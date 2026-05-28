#include "World.h"

World::World()
{
	mouse = std::make_shared<SEngineMouse>();
}

void World::Initialize(ID3D12Device5* device)
{
	m_device = device;

	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	mouse->Initilize();

	// heap 초기화
	{
		m_renderDensityHeap.Initialize(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_addSmokesHeap.Initialize(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_advectionHeap.Initialize(4, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	}
	// 버퍼 초기화
	{
		D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_oldDensityBuffer.Initialize(gridWidth, gridHeight, hdrFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"density Buffer");
		m_oldVelocityBuffer.Initialize(gridWidth, gridHeight, hdrFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"velocity Buffer");
		m_newDensityBuffer.Initialize(gridWidth, gridHeight, hdrFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"density Buffer");
		m_newVelocityBuffer.Initialize(gridWidth, gridHeight, hdrFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"velocity Buffer");

		m_addSmokesHeap.CreateResourceView(m_oldDensityBuffer.GetResource(), DescriptorType::UAV);
		m_addSmokesHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::UAV);

		m_advectionHeap.CreateResourceView(m_oldDensityBuffer.GetResource(), DescriptorType::SRV);
		m_advectionHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::SRV);
		m_advectionHeap.CreateResourceView(m_newDensityBuffer.GetResource(), DescriptorType::UAV);
		m_advectionHeap.CreateResourceView(m_newVelocityBuffer.GetResource(), DescriptorType::UAV);

		m_renderDensityHeap.CreateResourceView(m_newDensityBuffer.GetResource(), DescriptorType::SRV);
	}
	Grid grid;
	grid.gGridDim = Vector3(gridWidth, gridHeight, 1);
	grid.h = 1.f;
	gridCB.Initialize(grid);
}

void World::Tick(float deltaTime)
{
	if (mouse)
	{
		mouse->Tick();
	}

	gridCB.localConstant.deltaTime = deltaTime;
	gridCB.Update();
}
