#include "Noise.h"
#include "GraphicsCommon.h"
#include "Engine\world.h"
#include "Renderer.h"

Noise::Noise()
{
}

Noise::~Noise()
{
}
void Noise::Initialize()
{
    InitCommands();
}

void Noise::InitCommands()
{
    if (m_world && m_world->GetDevice())
    {
        auto device = m_world->GetDevice();
        ThrowIfFailed(
            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

        ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr,
                                                IID_PPV_ARGS(&m_commandList)));
        m_commandList->Close();
    }
}

void Noise::InitGPU()
{
    m_noiseHeap.Initialize(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    D3D12_RESOURCE_FLAGS allowUAflag = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    m_perlinNoise.Initialize(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, allowUAflag,
                           D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, L"perlinNoise");

	m_noiseHeap.CreateResourceView(m_perlinNoise.Get(), DescriptorType::UAV, ViewDimensionType::TEXTURE2D);

	if(m_world->GetTextureLoader())
        m_world->AddTexture("perlinNoise", m_perlinNoise);
}

void Noise::GenerateNoise()
{
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	m_noiseHeap.Bind(m_commandList.Get());
    m_commandList->SetComputeRootDescriptorTable(0, m_noiseHeap.GetGPUHandle(0));

	Dispatch(N_GROUP_SIZE_X, N_GROUP_SIZE_Y);
}

void Noise::Execute(ID3D12CommandQueue* commandQueue)
{
    m_commandList->Close();
    ID3D12CommandList* commands[] = {m_commandList.Get()};
    commandQueue->ExecuteCommandLists(ARRAYSIZE(commands), commands);
}

void Noise::Dispatch(UINT gs_x, UINT gs_y)
{
    UINT x = (width + gs_x - 1) / gs_x;
    UINT y = (height + gs_y - 1) / gs_y;
    m_commandList->Dispatch(x, y, 1);
}

void Noise::SetCPSO(const std::string psoName)
{
    Renderer::BindCPSO(psoName, m_commandList.Get());
}
