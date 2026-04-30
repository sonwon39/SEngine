#define HLSL
#include "DefaultHLSLCompat.h"

ConstantBuffer<DefaultLocalConstant> gLocalCB : register(b0);
Texture2D g_hdrTexture : register(s0);

SamplerState g_wrapLinearSampler;

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};
struct PSInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
};
