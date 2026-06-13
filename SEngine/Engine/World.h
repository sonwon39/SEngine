#pragma once

#include "d3d12.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "directxtk12\SimpleMath.h"

#include "InputHelper.h"
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
#include "AssetManager/LightManager.h"
#include "GameFramework/Actor.h"
#include "IBLEnvironment.h"
#include "ImageInfo.h"

class World
{
  public:
    World();
    virtual ~World();

  public:
    // model loader 및 마우스 초기화
    void Initialize(ID3D12Device5* device, int width, int height);
    void Tick(float deltaTime);

    bool FindTexture(const std::string& textureName, int& index);
    bool FindTextureHandle(const std::string& textureName, D3D12_GPU_DESCRIPTOR_HANDLE& handle);

  public:
    // gpu 버퍼 생성 후 (vertex, index) level 초기화
    void InitLevel();
    bool useSimulation = true;

  public:
    ID3D12Device5* GetDevice()
    {
        return m_device;
    }
    std::shared_ptr<TextureLoader> GetTextureLoader() const
    {
        return m_textureLoader;
    }
    std::shared_ptr<ModelLoader<Vertex, uint16_t>> GetModelLoader() const
    {
        return m_modelLoader;
    }
    std::shared_ptr<PBRModelLoader> GetPBRModelLoader() const
    {
        return m_pbrModelLoader;
    }
    std::shared_ptr<SimpleModelLoader> GetSimpleModelLoader() const
    {
        return m_simpleModelLoader;
    }
    std::shared_ptr<LightManager> GetLightManger() const
    {
        return m_lightManager;
    }
    std::shared_ptr<GPUBuffer> GetReadBackBuffer() const
    {
        return m_readbackBuffer;
    }
    ID3D12DescriptorHeap* GetMainHeap() const;
    Vector2 GetMouseVelocity() const;

  public:
    void SetWindowSize(int width, int height);

  public:
    void SetFPSMode(bool newState)
    {
        m_fpsMode = newState;
        if (m_fpsMode)
        {
            MoveMouseToWindowCenter();
        }
    }
    bool GetFPSMode() const
    {
        return m_fpsMode;
    }

  public:
    std::shared_ptr<StaticMesh> GetMesh(const std::string& meshName);
    std::shared_ptr<Actor> GenerateActor(const std::string& meshName, const ActorData& ad);
    void AddActor(std::shared_ptr<Actor> actor);
    void AddMesh(const std::string& meshName, std::shared_ptr<StaticMesh> mesh);
    void OnRegister();

    // mesh 내의 vertex index buffer blob 제거
    void ClearMeshBlobs();
    void ClearTextureBlobs();

    void MoveMouseToWindowCenter();

    POINT GetCenterPoint();

    void InitReadbackBuffer(UINT64 size);

    // 텍스처 이름으로 Material을 얻는다. 같은 이름이면 같은 Material을 반환(캐시).
    // 텍스처가 로드돼 있지 않으면 nullptr.
    std::shared_ptr<Material> GetOrCreateMaterial(const std::string& textureName);
    std::shared_ptr<Actor> GetPlayer() const
    {
        return m_player;
    }
    std::shared_ptr<Actor> GetPBRModel() const
    {
        return m_pbr;
    }
    std::shared_ptr<IBLEnvironment> GetIBL() const
    {
        return m_iblEnv;
    }

  private:
    void SaveTextureCPU(const ImageInfo& info);
    void SaveLoop();

  public:
    void Notify(const ImageInfo& info);

  public:
    UINT m_cbvSrvDescriptorSize = 0;
    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;

  public:
    HWND m_mainWnd;
    std::shared_ptr<SEngineMouse> mouse;
    std::shared_ptr<InputHelper> m_inputHelper;
    UINT windowWidth;
    UINT windowHeight;
    std::vector<DirectX::SimpleMath::Vector3> colors;

  private:
    ID3D12Device5* m_device;

  public:
    bool m_fpsMode = false;

  private:
    std::shared_ptr<Level> m_level;

  private:
    std::string texBuildPath = "Assets/Build/";
    std::shared_ptr<TextureLoader> m_textureLoader;
    std::string skyTextureName = "Sky";

  private:
    std::shared_ptr<ModelLoader<Vertex, uint16_t>> m_modelLoader;
    std::shared_ptr<SimpleModelLoader> m_simpleModelLoader;
    std::shared_ptr<PBRModelLoader> m_pbrModelLoader;

  private:
    std::shared_ptr<LightManager> m_lightManager;

  private:
    std::vector<std::shared_ptr<Actor>> m_actors;
    std::shared_ptr<Actor> m_player;
    std::shared_ptr<Actor> m_pbr;
    std::shared_ptr<IBLEnvironment> m_iblEnv;

    std::unordered_map<std::string, std::shared_ptr<StaticMesh>> m_meshes;
    std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;

    // save 용
  private:
    std::shared_ptr<GPUBuffer> m_readbackBuffer;
    std::thread saveThread;
    bool saveFlag = false;
    bool stopThread = false;
    std::mutex saveMtx;
    std::condition_variable saveCv;

	ImageInfo sharedInfo;
};
