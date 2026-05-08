#include "ParticleCommon.hlsli"

StructuredBuffer<SPHParticle> particles : register(t0);


GSInput main(uint id : SV_VertexID)
{
	GSInput output;
	output.pos = particles[id].position;
	output.color = particles[id].color;
	output.radius = particles[id].radius;
	
	return output;
}
