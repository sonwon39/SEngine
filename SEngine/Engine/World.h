#pragma once

#include "d3d12.h"
#include <memory>
#include <string>
#include <unordered_map>
#include "directxtk12\SimpleMath.h"

#include "SEngineMouse.h"
#include "Texture2D.h"
#include "DescriptorHeap.h"
#include "StableFluids/Grid.h"
#include "ConstantBuffer.h"
#include "GPUBuffer.h"
#include "Level.h"
#include "Material.h"
#include "AssetManager/TextureLoader.h"
#include "AssetManager/ModelLoader.h"
#include "GameFramework/Actor.h"

class World {
public:
	World();
	virtual ~World();

public:
	void Initialize(ID3D12Device5* device, int width, int height);
	void Tick(float deltaTime);

	bool FindTexture(const std::string& textureName, int& index);

public:
	ID3D12Device5* GetDevice() { return m_device; }
	std::shared_ptr<TextureLoader> GetTextureLoader() const { return m_textureLoader; }
	std::shared_ptr<ModelLoader<Vertex, uint16_t>> GetModelLoader() const { return m_modelLoader; }
	std::shared_ptr<SimpleModelLoader> GetSimpleModelLoader() const { return m_simpleModelLoader; }
	ID3D12DescriptorHeap* GetMainHeap() const;


public:
	void SetWindowSize(int width, int height);

public:
	void AddActor(std::shared_ptr<StaticMesh> mesh, const ActorData& ad);
	void OnRegister();

	// 텍스처 이름으로 Material을 얻는다. 같은 이름이면 같은 Material을 반환(캐시).
	// 텍스처가 로드돼 있지 않으면 nullptr.
	std::shared_ptr<Material> GetOrCreateMaterial(const std::string& textureName);

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
	//GPUBuffer m_test;
	DXGI_FORMAT testFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

public:
	bool m_captureDirty = false;

private:
	std::shared_ptr<Level> m_level;

private:
	std::string texBuildPath = "Assets/Build/";
	std::shared_ptr<TextureLoader> m_textureLoader;

private:
	std::shared_ptr <ModelLoader<Vertex, uint16_t>> m_modelLoader;
	std::shared_ptr <SimpleModelLoader> m_simpleModelLoader;

private:
	std::vector<Actor> m_actors;

	std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
};
