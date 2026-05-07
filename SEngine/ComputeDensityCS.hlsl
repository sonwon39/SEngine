#include "SPHUtility.hlsli"

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int i = DTid.x;
    if (i >= gParticleLocalCB.particleCount) return;

    curr_particles[i].density = 0.f;
    float3 xi = prev_particles[i].position;

	for(uint j = 0; j< gParticleLocalCB.particleCount; j++)
    {
        float3 xj = prev_particles[j].position;
        curr_particles[i].density += W(xi, xj);
    }
}

