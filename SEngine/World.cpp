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

	// hdr 버퍼 초기화
	{
		m_hdrSrvHeap.Initialize(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_hdrUavHeap.Initialize(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

		D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_hdrBuffer.Initialize(hdrWidth, hdrHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"hdr Buffer");

		m_hdrSrvHeap.CreateResourceView(m_hdrBuffer.GetResource(), DescriptorType::SRV);
		m_hdrUavHeap.CreateResourceView(m_hdrBuffer.GetResource(), DescriptorType::UAV);
	}
	Grid grid;
	grid.gGridDim = Vector3(hdrWidth, hdrHeight, 1);
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
