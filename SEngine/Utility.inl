#pragma once

#include "Utility.h"

namespace GraphicsUtils {
	template<typename DataType>
	inline void Utility::CreateBuffer(const std::vector<DataType>& data, Microsoft::WRL::ComPtr<ID3D12Resource>& gpu, Microsoft::WRL::ComPtr<ID3D12Resource>& upload)
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Data) * data.size()),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		));

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Data) * data.size()),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(gpuBuffer.GetAddressOf())
		));

		D3D12_SUBRESOURCE_DATA subData;
		subData.pData = data.data();
		subData.RowPitch = sizeof(Data) * data.size();
		subData.SlicePitch = subData.RowPitch;

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources(
			m_commandList,
			gpuBuffer.Get(),
			uploadBuffer.Get(),
			0, 0, 1, &subData
		);

		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gpuBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));

	}
}