#define HLSL
#include "Particle.h"

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
