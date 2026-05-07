#define HLSL
#include "Particle.h"

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
};
