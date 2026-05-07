#include "ParticleCommon.hlsli"

StructuredBuffer<Particle> particles : register(t0);


PSInput main(uint id : SV_VertexID)
{
	PSInput output;
	float3 pos = particles[id].pos;
	output.pos = float4(pos, 1.0f);
	output.color = particles[id].color;
	return output;
	
}
