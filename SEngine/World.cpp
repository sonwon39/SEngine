#include "World.h"

World::World()
{
	mouse = std::make_shared<SEngineMouse>();
}

void World::Initialize(ID3D12Device5* device, int width, int height)
{
	m_device = device;

	gridWidth = width;
	gridHeight = height;

	SetWindowSize(width, height);

	m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	mouse->Initilize();

	// heap 초기화
	{
		m_renderDensityHeap.Initialize(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_sourcingHeap.Initialize(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_computeCurlHeap.Initialize(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_vorticityConfinementHeap.Initialize(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_advectionHeap.Initialize(4, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_computeDivergenceHeap.Initialize(4, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_jacobiHeap[0].Initialize(3, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_jacobiHeap[1].Initialize(3, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_computeFinalVelocityHeap.Initialize(2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	}
	// 버퍼 초기화
	{
		D3D12_RESOURCE_FLAGS flag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_oldDensityBuffer.Initialize(gridWidth, gridHeight, densityFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"oldDensity Buffer");
		m_oldVelocityBuffer.Initialize(gridWidth, gridHeight, velocityFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"oldVelocity Buffer");
		m_newDensityBuffer.Initialize(gridWidth, gridHeight, densityFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"density Buffer");
		m_newVelocityBuffer.Initialize(gridWidth, gridHeight, velocityFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"velocity Buffer");
		m_divergenceBuffer.Initialize(gridWidth, gridHeight, divergenceFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"divergence Buffer");
		m_curlBuffer.Initialize(gridWidth, gridHeight, curlFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"divergence Buffer");
		for (size_t i = 0; i < 2; i++)
		{
			std::wstring name = L"pressure Buffer" + std::to_wstring(i);
			m_pressureBuffer[i].Initialize(gridWidth, gridHeight, pressureFormat, flag, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, name);
		}

		m_sourcingHeap.CreateResourceView(m_oldDensityBuffer.GetResource(), DescriptorType::UAV);
		m_sourcingHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::UAV);

		m_computeCurlHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::SRV);
		m_computeCurlHeap.CreateResourceView(m_curlBuffer.GetResource(), DescriptorType::UAV);

		m_vorticityConfinementHeap.CreateResourceView(m_curlBuffer.GetResource(), DescriptorType::SRV);
		m_vorticityConfinementHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::UAV);

		m_advectionHeap.CreateResourceView(m_oldDensityBuffer.GetResource(), DescriptorType::SRV);
		m_advectionHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::SRV);
		m_advectionHeap.CreateResourceView(m_newDensityBuffer.GetResource(), DescriptorType::UAV);
		m_advectionHeap.CreateResourceView(m_newVelocityBuffer.GetResource(), DescriptorType::UAV);

		m_computeDivergenceHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::SRV);
		m_computeDivergenceHeap.CreateResourceView(m_divergenceBuffer.GetResource(), DescriptorType::UAV);
		m_computeDivergenceHeap.CreateResourceView(m_pressureBuffer[0].GetResource(), DescriptorType::UAV);
		m_computeDivergenceHeap.CreateResourceView(m_pressureBuffer[1].GetResource(), DescriptorType::UAV);

		m_jacobiHeap[0].CreateResourceView(m_divergenceBuffer.GetResource(), DescriptorType::SRV);
		m_jacobiHeap[0].CreateResourceView(m_pressureBuffer[0].GetResource(), DescriptorType::SRV);
		m_jacobiHeap[0].CreateResourceView(m_pressureBuffer[1].GetResource(), DescriptorType::UAV);

		m_jacobiHeap[1].CreateResourceView(m_divergenceBuffer.GetResource(), DescriptorType::SRV);
		m_jacobiHeap[1].CreateResourceView(m_pressureBuffer[1].GetResource(), DescriptorType::SRV);
		m_jacobiHeap[1].CreateResourceView(m_pressureBuffer[0].GetResource(), DescriptorType::UAV);


		m_computeFinalVelocityHeap.CreateResourceView(m_pressureBuffer[1].GetResource(), DescriptorType::SRV);
		m_computeFinalVelocityHeap.CreateResourceView(m_oldVelocityBuffer.GetResource(), DescriptorType::UAV);

		m_renderDensityHeap.CreateResourceView(m_newDensityBuffer.GetResource(), DescriptorType::SRV);
	}

	Grid grid;
	grid.gGridDim = Vector3(gridWidth, gridHeight, 1);
	grid.h = 1.f;
	gridCB.Initialize(grid);


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

	gridCB.localConstant.deltaTime = deltaTime;
	gridCB.Update();
}

void World::SetWindowSize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

}
