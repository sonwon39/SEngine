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
	void Initialize(ID3D12Device5* device, int width, int height);
	void Tick(float deltaTime);

public:
	ID3D12Device5* GetDevice() {
		return m_device;
	}

public:
	void SetWindowSize(int width, int height);

public:
	UINT m_cbvSrvDescriptorSize = 0;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;

public:
	HWND m_mainWnd;
	std::shared_ptr<SEngineMouse> mouse;
	UINT windowWidth;
	UINT windowHeight;

private:
	ID3D12Device5* m_device;

public:
	// grid 버퍼
	Texture2D m_oldDensityBuffer;
	Texture2D m_oldVelocityBuffer;

	Texture2D m_newDensityBuffer;
	Texture2D m_newVelocityBuffer;

	Texture2D m_divergenceBuffer;

	Texture2D m_pressureBuffer[2];

	DescriptorHeap m_renderDensityHeap;
	DescriptorHeap m_sourcingHeap;
	DescriptorHeap m_advectionHeap;
	DescriptorHeap m_computeDivergenceHeap;
	DescriptorHeap m_jacobiHeap[2];
	DescriptorHeap m_computeFinalVelocityHeap;

	UINT gridWidth = 512;
	UINT gridHeight = 512;
	
	DXGI_FORMAT densityFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	DXGI_FORMAT velocityFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	DXGI_FORMAT divergenceFormat = DXGI_FORMAT_R32_FLOAT;
	DXGI_FORMAT pressureFormat = DXGI_FORMAT_R32_FLOAT;

	ConstantBuffer<Grid> gridCB;

public:
	std::vector<DirectX::SimpleMath::Vector3> colors;
};
