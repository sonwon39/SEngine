#include "Renderer.h"
#include "RootSignature.h"
#include "PipelineState.h"

#include "CompiledShaders/DefaultPS.h"
#include "CompiledShaders/DefaultVS.h"
#include "CompiledShaders/DefaultCS.h"

#include "CompiledShaders/ParticleRenderVS.h"
#include "CompiledShaders/ParticleRenderGS.h"
#include "CompiledShaders/ParticleRenderPS.h"
#include "CompiledShaders/ParticleSimulationCS.h"

#include "CompiledShaders/ComputeDensityCS.h"
#include "CompiledShaders/ComputeForcesCS.h"
#include "CompiledShaders/ComputePressureCS.h"
#include "CompiledShaders/SPHSimulationCS.h"


using namespace Graphics;
using namespace Renderer;


namespace Renderer
{
	std::map<std::string, GraphicsPSO> m_PSOs;
	std::map<std::string, ComputePSO> m_CPSOs;

	std::vector<std::string> psoNames;
	std::vector<std::string> cpsoNames;

	DXGI_FORMAT hdrFormat;
	DXGI_FORMAT backBufferFormat;
	DXGI_FORMAT dsBufferFormat;
	DXGI_FORMAT dsOnlyFormat;
	DXGI_FORMAT dsOnlyDsvFormat;
	DXGI_FORMAT dsOnlySrvFormat;

}

void Renderer::Initialize(const Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
	GraphicsPSO defaultPSO(L"default PSO");
	GraphicsPSO particleRenderPSO(L"particleRender PSO");

	ComputePSO defaultCPSO(L"default CPSO");

	ComputePSO computeDensityCPSO(L"computeDensity CPSO");
	ComputePSO computePressureCPSO(L"computePressure CPSO");
	ComputePSO computeForcesCPSO(L"computeForces CPSO");
	ComputePSO sphSimulationCPSO(L"sphSimulation CPSO");

	ComputePSO particleSimulationCPSO(L"particleSimulation CPSO");


	hdrFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//backBufferFormat  = hdrFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	dsBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsOnlyFormat = DXGI_FORMAT_R32_TYPELESS;
	dsOnlyDsvFormat = DXGI_FORMAT_D32_FLOAT;
	dsOnlySrvFormat = DXGI_FORMAT_R32_FLOAT;

	D3D12_INPUT_ELEMENT_DESC posOnlyIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_ELEMENT_DESC simpleIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}

	};
	D3D12_INPUT_ELEMENT_DESC textIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_ELEMENT_DESC phongIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_ELEMENT_DESC pbrIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_ELEMENT_DESC skinnedMeshIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}

	};
	D3D12_INPUT_ELEMENT_DESC pointCloudIL[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	defaultPSO.SetInputLayout(_countof(simpleIL), simpleIL);
	defaultPSO.SetRootSignature(g_commonRS);
	defaultPSO.SetRasterizerState(rasterizerDefault);
	defaultPSO.SetBlendState(blendNoColorWrite);
	defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	defaultPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
	defaultPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));
	defaultPSO.SetSampleMask(UINT_MAX);
	defaultPSO.SetRenderTargetFormat(backBufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
	defaultPSO.Finalize(device);
	m_PSOs["defaultPSO"] = defaultPSO;
	psoNames.push_back("defaultPSO");

	particleRenderPSO.SetInputLayout(0, nullptr);
	particleRenderPSO.SetRootSignature(g_S1_C1_RS);
	particleRenderPSO.SetRasterizerState(rasterizerDefault);
	particleRenderPSO.SetBlendState(blendColor);
	particleRenderPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	particleRenderPSO.SetVertexShader(g_pParticleRenderVS, sizeof(g_pParticleRenderVS));
	particleRenderPSO.SetGeometryShader(g_pParticleRenderGS, sizeof(g_pParticleRenderGS));
	particleRenderPSO.SetPixelShader(g_pParticleRenderPS, sizeof(g_pParticleRenderPS));
	particleRenderPSO.SetSampleMask(UINT_MAX);
	particleRenderPSO.SetRenderTargetFormat(backBufferFormat, DXGI_FORMAT_UNKNOWN, 1, 0);
	particleRenderPSO.Finalize(device);
	m_PSOs["particleRenderPSO"] = particleRenderPSO;
	psoNames.push_back("particleRenderPSO");

	defaultCPSO.SetRootSignature(g_U1_RS);
	defaultCPSO.SetComputeShader(g_pDefaultCS, sizeof(g_pDefaultCS));
	defaultCPSO.Finalize(device);
	m_CPSOs["defaultCPSO"] = defaultCPSO;
	cpsoNames.push_back("defaultCPSO");

	particleSimulationCPSO.SetRootSignature(g_U1_C1_RS);
	particleSimulationCPSO.SetComputeShader(g_pParticleSimulationCS, sizeof(g_pParticleSimulationCS));
	particleSimulationCPSO.Finalize(device);
	m_CPSOs["particleSimulationCPSO"] = particleSimulationCPSO;
	cpsoNames.push_back("particleSimulationCPSO");

	computeDensityCPSO.SetRootSignature(g_U2_C1_RS);
	computeDensityCPSO.SetComputeShader(g_pComputeDensityCS, sizeof(g_pComputeDensityCS));
	computeDensityCPSO.Finalize(device);
	m_CPSOs["computeDensityCPSO"] = computeDensityCPSO;
	cpsoNames.push_back("computeDensityCPSO");

	computePressureCPSO.SetRootSignature(g_U2_C1_RS);
	computePressureCPSO.SetComputeShader(g_pComputePressureCS, sizeof(g_pComputePressureCS));
	computePressureCPSO.Finalize(device);
	m_CPSOs["computePressureCPSO"] = computePressureCPSO;
	cpsoNames.push_back("computePressureCPSO");

	computeForcesCPSO.SetRootSignature(g_U2_C1_RS);
	computeForcesCPSO.SetComputeShader(g_pComputeForcesCS, sizeof(g_pComputeForcesCS));
	computeForcesCPSO.Finalize(device);
	m_CPSOs["computeForcesCPSO"] = computeForcesCPSO;
	cpsoNames.push_back("computeForcesCPSO");

	sphSimulationCPSO.SetRootSignature(g_U2_C1_RS);
	sphSimulationCPSO.SetComputeShader(g_pSPHSimulationCS, sizeof(g_pSPHSimulationCS));
	sphSimulationCPSO.Finalize(device);
	m_CPSOs["sphSimulationCPSO"] = sphSimulationCPSO;
	cpsoNames.push_back("sphSimulationCPSO");
}

ID3D12PipelineState* Renderer::GetPSO(std::string psoName)
{
	return m_PSOs[psoName].GetPSO();
}
