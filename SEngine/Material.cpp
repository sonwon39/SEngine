#include "Material.h"

void Material::Initialize(ID3D12DescriptorHeap* heap,
	D3D12_GPU_DESCRIPTOR_HANDLE srvTable)
{
	m_heap = heap;
	m_srvTable = srvTable;
}

void Material::Bind(ID3D12GraphicsCommandList* commandList) const
{
	if (!m_heap) return;

	ID3D12DescriptorHeap* heaps[] = { m_heap };
	commandList->SetDescriptorHeaps(1, heaps);
	commandList->SetGraphicsRootDescriptorTable(0, m_srvTable);
}
