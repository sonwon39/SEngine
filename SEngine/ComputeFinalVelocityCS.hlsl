#include "StableFluidsUtility.hlsli"

Texture2D<float>	 gPressure : register(t0);
RWTexture2D<float4>  gVelocity : register(u0);

SamplerState gWrapLinearSampler : register(s0);


[numthreads(SF_GROUP_SIZE_X, SF_GROUP_SIZE_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint3 gridDim = gLocalCB.gGridDim;

	if (DTid.x >= gridDim.x || DTid.y >= gridDim.y)
		return;

	int2 ufidx = uint2(DTid.x % gridDim.x, DTid.y % gridDim.y);
	
	uint2 left = uint2(ufidx.x == 0 ? gridDim.x - 1 : ufidx.x - 1, ufidx.y);
	uint2 right = uint2(ufidx.x == gridDim.x - 1 ? 0 : ufidx.x + 1, ufidx.y);
	uint2 top = uint2(ufidx.x, ufidx.y == 0 ? gridDim.y - 1 : ufidx.y - 1);
	uint2 bottom = uint2(ufidx.x, ufidx.y == gridDim.y - 1 ? 0 : ufidx.y + 1);
	
	
	float leftP = gPressure[left];
	float rightP = gPressure[right];
	float topP = gPressure[top];
	float bottomP = gPressure[bottom];

	float2 divergenceP = float2(rightP - leftP , topP- bottomP) / 2.f;
		
	gVelocity[DTid.xy].xy -= divergenceP;
}
