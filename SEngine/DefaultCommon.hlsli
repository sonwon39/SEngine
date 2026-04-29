#define HLSL
#include "DefaultHLSLCompat.h"

ConstantBuffer<DefaultLocalConstant> gLocalCB : register(b0);

struct VSInput
{
    float3 position;
    float2 uv;
};
struct PSInput
{
    float4 svPosition : SV_Position;
};