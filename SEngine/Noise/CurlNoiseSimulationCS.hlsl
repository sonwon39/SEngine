#include "NoiseCommon.hlsli"

Texture2D<float4>				  gCurl		 : register(t0);
RWStructuredBuffer<NoiseParticle> gParticles : register(u0);

ConstantBuffer<NoiseLocalConstant> gLCB		 : register(b0);

SamplerState gWrapLinearSampler : register(s0);

[numthreads(SIMULATION_GROUP_SIZE_X, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x >= gLCB.particleCount)
		return;
	// position range [-1, 1] uv range[0, 1]
	float3 pos = gParticles[DTid.x].position;
	float2 uv = (pos.xy + 1.f) * 0.5f;
	//float2 velocity = gCurl.SampleLevel(gWrapLinearSampler, uv, 0.f).xy;
	//velocity = float2(-1.f, 0.f);
	float dt = gLCB.deltaTime;
	float2 dx = float2(1.f / gLCB.grid.x, 1.f / gLCB.grid.y);
	gParticles[DTid.x].position.xy += getCurl(gParticles[DTid.x].position.xy, dx) * dt;
	gParticles[DTid.x].position.xy = clamp(gParticles[DTid.x].position.xy, -1.f, 1.f);

}
