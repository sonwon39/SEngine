#define HLSL
#include "Particle.h"
#include "GlobalConstant.h"

ConstantBuffer<GlobalConstant> g_globalConstant : register(b0);

struct GSInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
	float radius : PSIZE;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
	float2 uv : TEXCOORD;
	
};
