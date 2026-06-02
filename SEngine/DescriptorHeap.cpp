#include "DescriptorHeap.h"
#include "GraphicsCommon.h"
#include "Engine/World.h"

using namespace Graphics;

DescriptorHeap::DescriptorHeap()
{
}

DescriptorHeap::~DescriptorHeap()
{
}

void DescriptorHeap::Initialize(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT nodeMask, D3D12_DESCRIPTOR_HEAP_FLAGS flag)
{
	m_numDescriptors = numDescriptors;
	m_descriptorType = type;
	if (m_descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)	{
		descriptorIncrementSize = m_world->m_cbvSrvDescriptorSize;
	}
	else if (m_descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV) {
		descriptorIncrementSize = m_world->m_dsvDescriptorSize;
	}
	else {
		descriptorIncrementSize = m_world->m_rtvDescriptorSize;
	}
	utility->CreateDescriptorHeap(numDescriptors, type, m_heap, nodeMask, flag);
}

void DescriptorHeap::CreateResourceView(ID3D12Resource* resource, const DescriptorType& descriptorType)
{
	DXGI_FORMAT format = resource->GetDesc().Format;
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_heap->GetCPUDescriptorHandleForHeapStart(), m_heapIdx, descriptorIncrementSize);
	utility->CreateResourceView(resource, format, false, handle, descriptorType);
	m_heapIdx++;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandle(int offset)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_heap->GetGPUDescriptorHandleForHeapStart(), offset, descriptorIncrementSize);
	return handle;
}
