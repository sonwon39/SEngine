#pragma once

#include "wrl.h"
#include "d3d12.h"

class Texture2D {

public:
	Texture2D();
	virtual ~Texture2D();

public:
	void Initialize(int width, int height);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_uavHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

};
