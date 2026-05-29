#include "StableFluidsUtility.hlsli"

Texture2D<float>   gDivergence	: register(t0);
Texture2D<float>   gOldPressure : register(t1);
RWTexture2D<float> gNewPressure	: register(u0);

SamplerState gWrapLinearSampler : register(s0);


[numthreads(SF_GROUP_SIZE_X, SF_GROUP_SIZE_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{	
	uint3 gridDim = gLocalCB.gGridDim;

	if (DTid.x >= gridDim.x || DTid.y >= gridDim.y)
		return;

	int2 ufidx = uint2(DTid.x % gridDim.x, DTid.y % gridDim.y);
	float2 dx = float2(1.f / (gridDim.x - 1.f), 1.f / (gridDim.y - 1.f));
	
	uint2 left = uint2(ufidx.x == 0 ? gridDim.x - 1 : ufidx.x - 1, ufidx.y) * dx;
	uint2 right = uint2(ufidx.x == gridDim.x - 1 ? 0 : ufidx.x + 1, ufidx.y) * dx;
	uint2 top = uint2(ufidx.x, ufidx.y == 0 ? gridDim.y - 1 : ufidx.y - 1) * dx;
	uint2 bottom = uint2(ufidx.x, ufidx.y == gridDim.y - 1 ? 0 : ufidx.y + 1) * dx;

	float2 leftP = gOldPressure[left];
	float2 rightP = gOldPressure[right];
	float2 topP = gOldPressure[top];
	float2 bottomP = gOldPressure[bottom];

	float divergence = gDivergence[DTid.xy];
	
	float pressure = (leftP + rightP + topP + bottomP + divergence) / 4.f;
	gNewPressure[DTid.xy] = pressure;
}
