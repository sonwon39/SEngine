#pragma once

#include <mutex>
#include <atomic>
#include "Utility.h"
#include "imgui.h"

class SamplerDesc;
class RootSignature;
class ComputePSO;
class GraphicsPSO;

namespace Graphics
{
    // sampler, rootsignature 등을 미리 생성
    void InitializeCommonState(const Microsoft::WRL::ComPtr<ID3D12Device5>& device);

    extern D3D12_STATIC_SAMPLER_DESC wrapLinearSampler;

    extern D3D12_RASTERIZER_DESC rasterizerDefault;
    extern D3D12_RASTERIZER_DESC wireRasterizer;
    extern D3D12_RASTERIZER_DESC noneCullRasterizer;

	extern D3D12_BLEND_DESC blendNoColorWrite;
	extern D3D12_BLEND_DESC blendColor;

    extern D3D12_DEPTH_STENCIL_DESC depthStateDefault;

    extern RootSignature g_commonRS;
	extern RootSignature g_U1_RS;
	extern RootSignature g_U1_C1_RS;
	extern RootSignature g_U2_C1_RS;
	extern RootSignature g_S1_RS;
	extern RootSignature g_S1_C1_RS;
   

    extern std::shared_ptr<GraphicsUtils::Utility> utility;
}
