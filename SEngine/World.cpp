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
}

void World::Tick(float deltaTime)
{
	if (mouse)
	{
		mouse->Tick();
	}
}
