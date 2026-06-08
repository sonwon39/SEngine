#define HLSL
#include "../GlobalConstant.h"
#include "PBRHLSLCompat.h"

Texture2D g_albedo : register(t0);
Texture2D g_ambient : register(t1);
Texture2D g_height : register(t2);
Texture2D g_metallic : register(t3);
Texture2D g_normal : register(t4);
Texture2D g_roughness : register(t5);

Texture2D g_brdf : register(t6);
Texture2D g_diff : register(t7);
Texture2D g_cubeMap : register(t8);
Texture2D g_specular : register(t9);

ConstantBuffer<GlobalConstant> gGlobalCB : register(b0);
ConstantBuffer<LocalConstant> gLocalCB : register(b1);

SamplerState g_wrapLinearSampler : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 svPosition : SV_Position;
    float2 uv : TEXCOORD;
};
