#include "SPHUtility.hlsli"

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int i = DTid.x;
    float dt = gParticleLocalCB.dt;
    
    if (i >= gParticleLocalCB.particleCount) return;

    float3 prev_vel = prev_particles[i].velocity;
    float3 prev_pos = prev_particles[i].position;
    
	curr_particles[i].velocity = prev_vel + curr_particles[i].acceleration * dt;
    curr_particles[i].position = prev_pos + curr_particles[i].velocity * dt;

	if (curr_particles[i].position.y <= -0.8f && curr_particles[i].velocity.y < 0.f)
	{
		float over = -0.8f - curr_particles[i].position.y;   // 박힌 깊이
    curr_particles[i].position.y = -0.8f + over * 0.2f;  // 반사 비율만큼 되돌리기
    curr_particles[i].velocity.y *= -0.2f;
    }

	if (curr_particles[i].position.x <= -0.95f && curr_particles[i].velocity.x < 0.f)
	{
		curr_particles[i].position.x = -0.95f;
		curr_particles[i].velocity.x *= -0.2;
	}
	if (curr_particles[i].position.x >= 0.95f && curr_particles[i].velocity.x > 0.f)
	{
        curr_particles[i].position.x = 0.95f; 
		curr_particles[i].velocity.x *= -0.2;
	}
}
