#include "World.h"
#include "Actors/AMovingPlatform.h"
#include "Actors/ACamera.h"

World::World()
{
	mouse = std::make_shared<SEngineMouse>();
	m_inputHelper = std::make_shared<InputHelper>();

	m_level = std::make_shared<Level>();
	m_textureLoader = std::make_shared<TextureLoader>(texBuildPath);
	m_modelLoader = std::make_shared<ModelLoader<Vertex, uint16_t>>();
	m_simpleModelLoader = std::make_shared<SimpleModelLoader>();
	m_pbrModelLoader = std::make_shared<PBRModelLoader>();

	m_iblEnv = std::make_shared<IBLEnvironment>();
}

World::~World()
{
	mouse.reset();
	m_level.reset();
	m_textureLoader.reset();
}

void World::Initialize(ID3D12Device5* device, int width, int height)
{
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
	
	colors =
	{
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
	}
}

bool World::FindTexture(const std::string& textureName, int & index)
{
	if (!m_textureLoader)
		return false;

	index = m_textureLoader->GetTextureIndex(textureName);
	if (index == -1)
		return false;

	return true;
}

bool World::FindTextureHandle(const std::string& textureName, D3D12_GPU_DESCRIPTOR_HANDLE & handle)
{
	int index;
	if (FindTexture(textureName, index))
	{
		handle = m_textureLoader->GetGPUHandle(index);
		return true;
	}
	return false;
}

void World::InitLevel()
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
	if (FindTextureHandle(skyTextureName+"Brdf", handle))
		m_iblEnv->Initialize(handle);

	ActorData ad = {};
	ad.lc.model = DirectX::XMMatrixTranslation(0.f, 1.f, 3.f);
	ad.lc.model = ad.lc.model.Transpose();
	ad.textureName = "PavingStones145_2K-PNG_Albedo";
	ad.psoName = "pbrPSO";
	GenerateActor("pbr_sphere", ad);

	auto mp = std::make_shared<AMovingPlatform>();
	mp->Initialize();
	//AddActor(mp);

	auto camera = std::make_shared<ACamera>();
	camera->Initialize();
	AddActor(camera);

	ad.lc.model = DirectX::XMMatrixTranslation(0.f, 0.f, 0.f);
	ad.lc.model = ad.lc.model.Transpose();
	ad.psoName = "defaultPSO";
	GenerateActor("plane", ad);

	ad.textureName = "SkyEnvHDR_CubeMap";
	ad.psoName = "cubeMapPSO";
	GenerateActor("simple_cube", ad);

	m_player = camera;

	OnRegister();
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
	if (it != m_materials.end()) return it->second;

	int index;
	if (!FindTexture(textureName, index)) return nullptr;

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

void World::GenerateActor(const std::string& meshName, const ActorData &ad)
{
	auto mesh = GetMesh(meshName);
	if (!mesh)
		return;
	
	std::shared_ptr<Actor> actor = std::make_shared<Actor>();
	actor->Initialize(mesh, ad);
	AddActor(actor);
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
