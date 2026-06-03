#include "World.h"

World::World()
{
	mouse = std::make_shared<SEngineMouse>();
	m_level = std::make_shared<Level>();
	m_textureLoader = std::make_shared<TextureLoader>(texBuildPath);
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


	SetWindowSize(width, height);

	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	mouse->Initilize();

	UINT testSize = width * height * sizeof(float) * 4;
	m_test.Initialize(D3D12_HEAP_TYPE_READBACK, testSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, L"test");

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
}

void World::SetWindowSize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
}
