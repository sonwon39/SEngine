#pragma once

#include "wrl.h"
#include "d3d12.h"
#include <string>


class Texture2D {

public:
	Texture2D();
	virtual ~Texture2D();

public:
	void Initialize(int width, int height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, UINT miplevels, std::wstring name);

public:
	int m_width;
	int m_height;

public:
	ID3D12Resource* GetResource() const { return buffer.Get(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return buffer->GetGPUVirtualAddress(); }
	D3D12_RESOURCE_BARRIER Transition(D3D12_RESOURCE_STATES newState);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
	D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
};
