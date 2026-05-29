#include "StableFluidsUtility.hlsli"

Texture2D<float4>   gVelocity   : register(t0);
RWTexture2D<float>  gDivergence : register(u0);

SamplerState gWrapLinearSampler : register(s0);

[numthreads(SF_GROUP_SIZE_X, SF_GROUP_SIZE_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	
	uint3 gridDim = gLocalCB.gGridDim;

	if (DTid.x >= gridDim.x || DTid.y >= gridDim.y)
		return;

	float deltaTime = gLocalCB.deltaTime;

	float divergence = 0.f;

	float2 dx = float2(1.f / (gridDim.x - 1.f), 1.f / (gridDim.y - 1.f));

	int2 ufidx	 = uint2(DTid.x % gridDim.x, DTid.y % gridDim.y);
	uint2 left	 = uint2(ufidx.x == 0 ? gridDim.x - 1 : ufidx.x - 1, ufidx.y);
	uint2 right  = uint2(ufidx.x == gridDim.x - 1 ? 0 : ufidx.x + 1,	ufidx.y);
	uint2 top	 = uint2(ufidx.x, ufidx.y == 0 ? gridDim.y - 1 : ufidx.y - 1);
	uint2 bottom = uint2(ufidx.x, ufidx.y == gridDim.y - 1 ? 0 : ufidx.y + 1);

	float2 leftVel	 = gVelocity[left].xy;
	float2 rightVel = gVelocity[right].xy;
	float2 topVel = gVelocity[top].xy;
	float2 bottomVel = gVelocity[bottom].xy;

	
	divergence += (rightVel.x - leftVel.x) / 2.f;
	divergence += (topVel.y - bottomVel.y) / 2.f;
	gDivergence[DTid.xy] = divergence;

}
