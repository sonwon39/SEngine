#include "SPHUtility.hlsli"

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int i = DTid.x;
    float dt = gParticleLocalCB.dt;

	float minY = -0.8f;
	float minZ = -0.65f;
	float maxZ = 0.65f;
	float minX = -0.65f;
	float maxX = 0.65f;
	
    if (i >= gParticleLocalCB.particleCount) return;

    float3 prev_vel = prev_particles[i].velocity;
    float3 prev_pos = prev_particles[i].position;
    
	curr_particles[i].velocity = prev_vel + curr_particles[i].acceleration * dt;
    curr_particles[i].position = prev_pos + curr_particles[i].velocity * dt;

	if (curr_particles[i].position.y <= minY && curr_particles[i].velocity.y < 0.f)
	{
		float over = minY - curr_particles[i].position.y; // 박힌 깊이
		curr_particles[i].position.y = minY + over * 0.2f; // 반사 비율만큼 되돌리기
		curr_particles[i].velocity.y *= -0.2f;
    }

	// x축 경계조건
	if (curr_particles[i].position.x <= minX && curr_particles[i].velocity.x < 0.f)
	{
		float over = abs(minX - curr_particles[i].position.x); // 박힌 깊이
		curr_particles[i].position.x = minX + over * 0.2f; // 반사 비율만큼 되돌리기
		curr_particles[i].velocity.x *= -0.2;
	}
	if (curr_particles[i].position.x >= maxX && curr_particles[i].velocity.x > 0.f)
	{
		float over = abs(maxX - curr_particles[i].position.x); // 박힌 깊이
		curr_particles[i].position.x = maxX - over * 0.2f; // 반사 비율만큼 되돌리기
		curr_particles[i].velocity.x *= -0.2;
	}

	// z 축 경계조건
	if (curr_particles[i].position.z <= minZ && curr_particles[i].velocity.z < 0.f)
	{							   
		float over = abs(minZ - curr_particles[i].position.z); // 박힌 깊이
		curr_particles[i].position.z = minZ + over * 0.2f; // 반사 비율만큼 되돌리기
		curr_particles[i].velocity.z *= -0.2;
	}							   
	if (curr_particles[i].position.z >= maxZ && curr_particles[i].velocity.z > 0.f)
	{							   
		float over = abs(maxZ - curr_particles[i].position.z); // 박힌 깊이
		curr_particles[i].position.z = maxZ - over * 0.2f; // 반사 비율만큼 되돌리기
		curr_particles[i].velocity.z *= -0.2;
	}
}
