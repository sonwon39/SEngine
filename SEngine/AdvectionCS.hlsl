#include "StableFluidsUtility.hlsli"

Texture2D<float4> gOldDensity : register(t0);
Texture2D<float4> gOldVelocity : register(t1);

RWTexture2D<float4> gNewDensity : register(u0);
RWTexture2D<float4> gNewVelocity : register(u1);

SamplerState gWarpLinearSampler : register(s0);


[numthreads(SF_GROUP_SIZE, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint width, height;
	gNewDensity.GetDimensions(width, height);

	if (DTid.x >= width || DTid.y >= height)
		return;

	float deltaTime = gLocalCB.deltaTime;

	float2 dx = float2(1.f / width, 1.f / height);
	float2 currPos = (DTid.xy + 0.5f) * dx;
	
	float2 velocity = gOldVelocity.SampleLevel(gWarpLinearSampler, currPos, 0).xy;
	float2 prevPos = currPos - deltaTime * velocity;

	float2 newVelocity = gOldVelocity.SampleLevel(gWarpLinearSampler, prevPos, 0).xy;
	float4 newDensity = gOldDensity.SampleLevel(gWarpLinearSampler, prevPos, 0);
	
	gNewVelocity[DTid.xy].xy = newVelocity;
	gNewDensity[DTid.xy] = newDensity;

}
