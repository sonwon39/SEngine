#pragma once

#include "d3d12.h"
#include <memory>
#include <string>
#include "directxtk12\SimpleMath.h"

#include "SEngineMouse.h"
#include "Texture2D.h"
#include "DescriptorHeap.h"
#include "StableFluids/Grid.h"
#include "ConstantBuffer.h"
#include "GPUBuffer.h"
#include "Level.h"
#include "AssetManager/TextureLoader.h"

class World {
public:
	World();
	virtual ~World();

public:
	void Initialize(ID3D12Device5* device, int width, int height);
	//void InitStableFluidsResources(int width, int height);
	void Tick(float deltaTime);

public:
	ID3D12Device5* GetDevice() { return m_device; }
	std::shared_ptr<TextureLoader> GetTextureLoader() { return m_textureLoader; }

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
	std::vector<DirectX::SimpleMath::Vector3> colors;

public:
	GPUBuffer m_test;
	DXGI_FORMAT testFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

public:
	bool m_captureDirty = false;

private:
	std::shared_ptr<Level> m_level;

private:
	std::string texBuildPath = "Assets/Build/";
	std::shared_ptr<TextureLoader> m_textureLoader;
};
