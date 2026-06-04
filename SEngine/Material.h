#pragma once

#include "d3d12.h"

// 머티리얼: 텍스처 SRV들이 묶인 descriptor table을 한 단위로 표현한다.
// 현재는 표면적으로 (heap + GPU table 시작 핸들)만 갖지만, 추후 PSO/blend state 등으로 확장 예정.
class Material
{
public:
	Material() = default;

	void Initialize(ID3D12DescriptorHeap* heap,
		D3D12_GPU_DESCRIPTOR_HANDLE srvTable);

	void Bind(ID3D12GraphicsCommandList* commandList) const;

	bool IsValid() const { return m_heap != nullptr; }

private:
	ID3D12DescriptorHeap* m_heap = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srvTable = { 0 };
};
