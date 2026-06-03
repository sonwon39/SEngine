#pragma once

#include "wrl.h"
#include "d3d12.h"
#include <string>
#include <vector>
#include "Directxtk12/DDSTextureLoader.h"
#include <fstream>

class Texture2D {

public:
	Texture2D();
	virtual ~Texture2D();

public:
	void Initialize(int width, int height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, UINT miplevels, std::wstring name);
	void Initialize(std::ifstream & bin, uint64_t size, DirectX::DX12::DDS_LOADER_FLAGS flags, ID3D12GraphicsCommandList* commandList, std::wstring name = L"");
	bool Transition(D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_BARRIER& outBarrier);
	void Clear();

public:
	int m_width;
	int m_height;

public:
	ID3D12Resource* GetResource() const { return buffer.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return buffer->GetGPUVirtualAddress(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> upload;
	std::vector<uint8_t> ddsBlob;

	D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
};
