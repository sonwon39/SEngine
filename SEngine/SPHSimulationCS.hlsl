#include "SPHUtility.hlsli"

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int i = DTid.x;
    float dt = gParticleLocalCB.dt;
    
    if (i >= gParticleLocalCB.particleCount) return;
    
    float3 prev_acc = prev_particles[i].acceleration;
    float3 prev_vel = prev_particles[i].velocity;
    float3 prev_pos = prev_particles[i].position;
    
	curr_particles[i].velocity = prev_vel + curr_particles[i].acceleration * dt;
    curr_particles[i].position = prev_pos + curr_particles[i].velocity * dt;

    if (curr_particles[i].position.y <= -0.8f) { 
		curr_particles[i].position.y = -0.8f;
        curr_particles[i].velocity.y = -0.2 * curr_particles[i].velocity.y; 
    }

    if (curr_particles[i].position.x <= -0.95f) { 
		curr_particles[i].position.x = -0.95f;
        curr_particles[i].velocity.x = -0.2 * curr_particles[i].velocity.x; 
    }
    if (curr_particles[i].position.x >= 0.95f) { 
        curr_particles[i].position.x = 0.95f; 
        curr_particles[i].velocity.x = -0.2 * curr_particles[i].velocity.x; 
    }
}
