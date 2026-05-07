#include "GraphicsCommon.h"
#include "PipelineState.h"
#include "RootSignature.h"

namespace Graphics
{
  
	D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;
	D3D12_STATIC_SAMPLER_DESC clampLinearSampler;

	D3D12_RASTERIZER_DESC rasterizerDefault;
	D3D12_RASTERIZER_DESC wireRasterizer;
	D3D12_RASTERIZER_DESC noneCullRasterizer;

    D3D12_BLEND_DESC blendNoColorWrite;		

    D3D12_DEPTH_STENCIL_DESC depthStateDefault;

	RootSignature g_commonRS;
	RootSignature g_U1_RS;
	RootSignature g_U1_C1_RS;
	RootSignature g_S1_RS;

	std::shared_ptr<GraphicsUtils::Utility> utility;
}


void Graphics::InitializeCommonState(const Microsoft::WRL::ComPtr<ID3D12Device5>& device)
{
	wrapLinearSampler = {};
	wrapLinearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	wrapLinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	wrapLinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	wrapLinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	wrapLinearSampler.MipLODBias = 0;
	wrapLinearSampler.MaxAnisotropy = 0;
	wrapLinearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	wrapLinearSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	wrapLinearSampler.MinLOD = 0.0f;
	wrapLinearSampler.MaxLOD = D3D12_FLOAT32_MAX;
	wrapLinearSampler.ShaderRegister = 0;
	wrapLinearSampler.RegisterSpace = 0;
	wrapLinearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
   
	clampLinearSampler = {};
	clampLinearSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	clampLinearSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampLinearSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampLinearSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampLinearSampler.MipLODBias = 0;
	clampLinearSampler.MaxAnisotropy = 0;
	clampLinearSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	clampLinearSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	clampLinearSampler.MinLOD = 0.0f;
	clampLinearSampler.MaxLOD = D3D12_FLOAT32_MAX;
	clampLinearSampler.ShaderRegister = 1;
	clampLinearSampler.RegisterSpace = 0;
	clampLinearSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rasterizerDefault = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	
	noneCullRasterizer = rasterizerDefault;
	noneCullRasterizer.CullMode = D3D12_CULL_MODE_NONE;
	noneCullRasterizer.DepthClipEnable = false;

	wireRasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	wireRasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
	wireRasterizer.CullMode = D3D12_CULL_MODE_NONE;
	wireRasterizer.DepthClipEnable = false;

	blendNoColorWrite = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	depthStateDefault = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

    g_commonRS.Reset(2,1 );
	g_commonRS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	g_commonRS[1].InitCBV(0);
	g_commonRS.InitStaticSampler(0, wrapLinearSampler);
	g_commonRS.Finalize(device, L"CommonRS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	g_U1_RS.Reset(1, 0);
	g_U1_RS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	g_U1_RS.Finalize(device, L"g_U1_RS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	g_U1_C1_RS.Reset(2, 0);
	g_U1_C1_RS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	g_U1_C1_RS[1].InitCBV(0);
	g_U1_C1_RS.Finalize(device, L"g_U1_C1_RS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


	g_S1_RS.Reset(1, 0);
	g_S1_RS[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	g_S1_RS.Finalize(device, L"g_S1_RS", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

}

