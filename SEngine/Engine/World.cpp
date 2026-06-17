#pragma warning(disable : 4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "World.h"
#include "Actors/AMovingPlatform.h"
#include "Actors/ACamera.h"
#include "Actors/ALight.h"
#include <fp16.h>

World::World()
{
    mouse = std::make_shared<SEngineMouse>();
    m_inputHelper = std::make_shared<InputHelper>();

    m_level = std::make_shared<Level>();
    m_textureLoader = std::make_shared<TextureLoader>(texBuildPath);
    m_modelLoader = std::make_shared<ModelLoader<Vertex, uint16_t>>();
    m_simpleModelLoader = std::make_shared<SimpleModelLoader>();
    m_pbrModelLoader = std::make_shared<PBRModelLoader>();

    m_lightManager = std::make_shared<LightManager>();

    m_iblEnv = std::make_shared<IBLEnvironment>();

    m_readbackBuffer = std::make_shared<GPUBuffer>();

	saveThread = std::thread(&World::SaveLoop, this);
}

World::~World()
{
    mouse.reset();
    m_level.reset();
    m_textureLoader.reset();
	{
        std::lock_guard<std::mutex> lock(saveMtx);
        stopThread = true;
    }
    saveCv.notify_one();
    if (saveThread.joinable())
        saveThread.join();
}

void World::Initialize(ID3D12Device5* device, int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    m_device = device;

    m_modelLoader->InitializeCPU();
    m_simpleModelLoader->InitializeCPU();
    m_pbrModelLoader->InitializeCPU();

    m_inputHelper->Initialize();

    SetWindowSize(width, height);

    m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    mouse->Initilize();

    colors = {
        Vector3(0.3f, 0.8f, 0.4f), // Light Green
        Vector3(1.0f, 0.0f, 0.0f), // Red
        Vector3(0.0f, 1.0f, 0.0f), // Green
        Vector3(0.0f, 0.0f, 1.0f), // Blue
        Vector3(1.0f, 1.0f, 0.0f), // Yellow
        Vector3(1.0f, 0.0f, 1.0f), // Magenta
        Vector3(0.0f, 1.0f, 1.0f), // Cyan
        Vector3(1.0f, 0.5f, 0.0f), // Orange
        Vector3(0.5f, 0.0f, 1.0f), // Purple
        Vector3(0.8f, 0.8f, 0.8f)  // Light Gray
    };

    // InitReadbackBuffer();
}

void World::Tick(float deltaTime)
{
    if (mouse)
    {
        mouse->Tick(deltaTime);
    }

    for (auto& actor : m_actors)
    {
        actor->Tick(deltaTime);
        actor->TickComponents(deltaTime);
    }

    m_lightManager->Update();
}

bool World::FindTexture(const std::string& textureName, int& index)
{
    if (!m_textureLoader)
        return false;

    index = m_textureLoader->GetTextureIndex(textureName);
    if (index == -1)
        return false;

    return true;
}

bool World::FindTextureHandle(const std::string& textureName, D3D12_GPU_DESCRIPTOR_HANDLE& handle)
{
    int index;
    if (FindTexture(textureName, index))
    {
        handle = m_textureLoader->GetGPUHandle(index);
        return true;
    }
    return false;
}
void World::AddTexture(const std::string& textureName, Texture2D& texture)
{
    if (!m_textureLoader)
        return;

    m_textureLoader->AddTexture(textureName, texture);
}

void World::InitLevel()
{
    ActorData ad = {};

	if (useSimulation)
	{
        ad.lc.model = DirectX::XMMatrixTranslation(0.f, 0.f, 0.f);
        ad.lc.model = ad.lc.model.Transpose();
        ad.textureName = "sf_density";
        ad.psoName = "defaultPSO";
        ad.useMaterial = false;
        GenerateActor("rect", ad);

        auto orthogonalCamera = std::make_shared<ACamera>();
        orthogonalCamera->Initialize(Vector3(0.f, 0.f, 0.f), false);
        AddActor(orthogonalCamera);

        m_player = orthogonalCamera;
        OnRegister();
        return;
	}
	if (useNoise)
	{
        ad.lc.model = DirectX::XMMatrixTranslation(0.f, 0.f, 0.f);
        ad.lc.model = ad.lc.model.Transpose();
        ad.textureName = "perlinNoise";
        ad.psoName = "defaultPSO";
        ad.useMaterial = false;
        GenerateActor("rect", ad);

        auto orthogonalCamera = std::make_shared<ACamera>();
        orthogonalCamera->Initialize(Vector3(0.f, 0.f, 0.f), false);
        AddActor(orthogonalCamera);

        m_player = orthogonalCamera;
        OnRegister();
	}
	if (renderDefault)
	{
        D3D12_GPU_DESCRIPTOR_HANDLE handle;
        if (FindTextureHandle(skyTextureName + "Brdf", handle))
            m_iblEnv->Initialize(handle);

        ad.lc.model = Matrix(DirectX::XMMatrixTranslation(-1.5f, 1.f, 0.f)).Transpose();
        ad.psoName = "pbrPSO";
        ad.textureName = "Metal052C_4K-PNG_albedo";
        ad.useMaterial = true;
        ad.mc.texTransform = Matrix(DirectX::XMMatrixScaling(2.5f, 2.5f, 1.f)).Transpose();
        ad.mc.heightScale = 0.05f;
        ad.mc.useHeightMap = true;
        ad.mc.useNormalMap = true;
        ad.mc.useMetallicMap = true;
        ad.mc.useRoughnessMap = true;
        // gui 조작용
        m_pbr = GenerateActor("pbr_sphere", ad);

        ad.lc.model = Matrix(DirectX::XMMatrixTranslation(1.5f, 1.f, 0.f)).Transpose();
        ad.textureName = "worn-painted-metal_albedo";
        GenerateActor("pbr_sphere", ad);

        ad.mc.useHeightMap = false;
        ad.lc.model = Matrix(DirectX::XMMatrixTranslation(0.f, 0.f, 0.f)).Transpose();
        ad.mc.texTransform = Matrix(DirectX::XMMatrixScaling(15.f, 15.f, 1.f)).Transpose();

        ad.textureName = "PavingStones145_2K-PNG_Albedo";
        GenerateActor("pbr_plane", ad);

        /* auto mp = std::make_shared<AMovingPlatform>();
         mp->Initialize();
         AddActor(mp);*/

        auto camera = std::make_shared<ACamera>();
        camera->Initialize(Vector3(0.f, 2.f, -2.f), true);
        AddActor(camera);

        auto light = std::make_shared<ALight>();
        light->Initialize();
        AddActor(light);

        ad.textureName = "SkyEnvHDR_CubeMap";
        ad.psoName = "cubeMapPSO";
        GenerateActor("simple_cube", ad);

        m_player = camera;

        OnRegister();
	}
   
}

ID3D12DescriptorHeap* World::GetMainHeap() const
{
    if (!m_textureLoader)
        return nullptr;

    return m_textureLoader->GetDescriptorHeap()->GetHeap();
}

Vector2 World::GetMouseVelocity() const
{
    Vector2 velocity;
    if (mouse)
    {
        return mouse->velocity;
    }
    return velocity;
}

std::shared_ptr<Material> World::GetOrCreateMaterial(const std::string& textureName)
{
    auto it = m_materials.find(textureName);
    if (it != m_materials.end())
        return it->second;

    int index;
    if (!FindTexture(textureName, index))
        return nullptr;

    auto mat = std::make_shared<Material>();
    mat->Initialize(m_textureLoader->GetGPUHandle(index));
    m_materials[textureName] = mat;
    return mat;
}

void World::SetWindowSize(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

}

std::shared_ptr<StaticMesh> World::GetMesh(const std::string& meshName)
{
    auto it = m_meshes.find(meshName);
    if (it == m_meshes.end())
        return nullptr;

    return it->second;
}

std::shared_ptr<Actor> World::GenerateActor(const std::string& meshName, const ActorData& ad)
{
    auto mesh = GetMesh(meshName);
    if (!mesh)
        return nullptr;

    std::shared_ptr<Actor> actor = std::make_shared<Actor>();
    actor->Initialize(mesh, ad);
    AddActor(actor);
    return actor;
}

void World::AddActor(std::shared_ptr<Actor> actor)
{
    m_actors.push_back(actor);
}

void World::AddMesh(const std::string& meshName, std::shared_ptr<StaticMesh> mesh)
{
    m_meshes[meshName] = mesh;
}

void World::OnRegister()
{
    for (auto& a : m_actors)
    {
        a->OnRegister();
    }
}

void World::ClearMeshBlobs()
{
    for (auto& m : m_meshes)
    {
        m.second->Clear();
    }
}

void World::ClearTextureBlobs()
{
    m_textureLoader->ClearBlobs();
}

void World::MoveMouseToWindowCenter()
{
    POINT center = GetCenterPoint();

    // 클라이언트 좌표 → 화면 좌표
    ClientToScreen(m_mainWnd, &center);

    // 마우스 이동
    SetCursorPos(center.x, center.y);
}

POINT World::GetCenterPoint()
{
    RECT rect;
    GetClientRect(m_mainWnd, &rect);

    // 클라이언트 영역 중앙 좌표
    POINT center;
    center.x = (rect.left + rect.right) / 2;
    center.y = (rect.top + rect.bottom) / 2;
    return center;
}

void World::InitReadbackBuffer(UINT64 size)
{
    m_readbackBuffer->Initialize(size, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST,
                                 D3D12_RESOURCE_FLAG_NONE, L"readback");
    m_readbackBuffer->SetResourceStates(D3D12_RESOURCE_STATE_COPY_DEST);
}

// TODO : 현재 f16 텍스쳐만 가능
void World::SaveTextureCPU(const ImageInfo& info)
{
	if (!m_readbackBuffer)
        return;

	std::cout << "SaveTextureCPU\n";

    uint32_t pixelCount = (uint32_t)(info.rowSize * info.numRows / 2);
    std::vector<uint16_t> imagef16(pixelCount);
    std::vector<uint8_t> image(pixelCount);

    m_readbackBuffer->MapForRead();
    m_readbackBuffer->CopyToCpu(imagef16.data(), info.numRows, info.rowSize, info.rowPitch);

    for (size_t i = 0; i < image.size(); i++)
    {
        double c = std::clamp(fp16_ieee_to_fp32_value(imagef16[i]), 0.f, 1.f);
        // tone mapping
        if ((i + 1) % 4 != 0)
            c = std::pow(c, 1 / 2.2);
        image[i] = std::clamp((int)(c * 255.f), 0, 255);
    }

    std::string filename = "Backbuffer" + utility->MakeTimestamp() + ".png";
    std::string fileFullPath = "Results/" + filename;
    UINT width = info.width;
    UINT height = info.height;
    stbi_write_png(fileFullPath.c_str(), (int)width, (int)height, 4, image.data(), (int)(info.rowSize / 2));

    std::cout << fileFullPath << " saved.\n";
}

void World::SaveLoop()
{
	while (true)	{
        ImageInfo local;
        {
            std::unique_lock lock(saveMtx);
            saveCv.wait(lock, [this] { return stopThread || saveFlag; });
            if (stopThread)
                break;

			local = sharedInfo;
            saveFlag = false;
        }
        SaveTextureCPU(local);
	}
}
void World::Notify(const ImageInfo& info)
{
	{
        std::lock_guard<std::mutex> lock(saveMtx);
        sharedInfo = info;
        saveFlag = true;
	}
    saveCv.notify_one();
}
