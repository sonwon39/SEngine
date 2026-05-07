#include "SPHUtility.hlsli"


[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int i = DTid.x;
    if (i >= gParticleLocalCB.particleCount) return;

    float3 acc = float3(0.f,0.f,0.f);
    float rhoi = curr_particles[i].density;
    float rhoi_2 = rhoi*rhoi;
    float pi = curr_particles[i].pressure;
    float3 xi = prev_particles[i].position;
    float3 vi = prev_particles[i].velocity;

    float h = gParticleLocalCB.h;
    float dt = gParticleLocalCB.dt;
    float mu = gParticleLocalCB.mu;

    for(uint j = 0; j< gParticleLocalCB.particleCount; j++)
    {
        if(i == j)
        {
            continue;
        }
        else{
            float3 xj = prev_particles[j].position;
            float3 xij = xi - xj;

            float3 vj = prev_particles[j].velocity;
            float3 vij = vi - vj;

            float3 w_grad = W_Grad(xi, xj);

            float rhoj = curr_particles[j].density;
            float rhoj_2 = rhoj*rhoj;
            float pj = curr_particles[j].pressure;
            
            float3 pressure_acc = -((pi / rhoi_2) + (pj / rhoj_2)) * w_grad;
            float3 viscosity_acc = 2 * (mu / (rhoi * rhoj)) * vij * dot(xij, w_grad) / (dot(xij, xij) + 0.01 * h*h );
            acc += pressure_acc + viscosity_acc;
        }
    }
    float3 gravity_acc = float3(0.f,-9.8f, 0.f);

    curr_particles[i].acceleration = acc + gravity_acc;
	//curr_particles[i].acceleration = gravity_acc;
}

