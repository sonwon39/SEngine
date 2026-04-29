#define HLSL
#include "DefaultHLSLCompat.h"

ConstantBuffer<DefaultLocalConstant> gLocalCB : register(b0);

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