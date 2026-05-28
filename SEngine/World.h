#pragma once

#include "d3d12.h"
#include <memory>

#include "directxtk12\SimpleMath.h"
#include "SEngineMouse.h"
#include "Texture2D.h"
#include "DescriptorHeap.h"
#include "Grid.h"
#include "ConstantBuffer.h"

class World {
public:
	World();
	virtual ~World() {};

public:
	void Initialize(ID3D12Device5* device);
	void Tick(float deltaTime);
	ID3D12Device5* GetDevice() {
		return m_device;
	}

public:
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;

public:
	HWND m_mainWnd;
	std::shared_ptr<SEngineMouse> mouse;

private:
	ID3D12Device5* m_device;

public:
	// hdr 버퍼
	Texture2D m_hdrBuffer;
	DescriptorHeap m_hdrSrvHeap;
	DescriptorHeap m_hdrUavHeap;
	UINT hdrWidth = 1280;
	UINT hdrHeight = 720;

	ConstantBuffer<Grid> gridCB;
};
