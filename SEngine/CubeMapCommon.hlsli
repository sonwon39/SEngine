#define HLSL
#include "GlobalConstant.h"

Texture2D					   g_cubeMap : register(t0);
ConstantBuffer<GlobalConstant> gGlobalCB : register(b0);

SamplerState g_wrapLinearSampler : register(s0);

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
