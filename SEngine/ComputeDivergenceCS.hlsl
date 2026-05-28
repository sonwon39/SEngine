#include "StableFluidsUtility.hlsli"

StructuredBuffer<float3>  gAdvectedVelocity : register(t0);
RWStructuredBuffer<float> gDivergence       : register(u0);

[numthreads(SF_GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint3 gridDim = gLocalCB.gGridDim;
	int maxIdx = gridDim.x * gridDim.y * gridDim.z - 1;
	int idx = DTid.x;
	
	if (idx > maxIdx)
		return;

	float h2 = gLocalCB.h * 2.f;

	float3 vels[6]; // left, right, bottom, top, backward, forward
	uint offsets[3] = { 1, gridDim.x, gridDim.x * gridDim.y };

	
	float divergence = 0.f;
	uint3 ufIdx = UnflattenIdx(idx);
	for (uint i = 0; i < 3; i++)
	{
		uint j = i * 2;
		uint offset = offsets[i];
		
		vels[j] = (ufIdx[i] > 0) ? gAdvectedVelocity[idx - offset] : gAdvectedVelocity[idx];
		vels[j + 1] = (ufIdx[i] != gridDim[i] - 1) ?
			gAdvectedVelocity[idx + offset] :
			gAdvectedVelocity[idx];

		divergence += (vels[j + 1][i] - vels[j][i])/ h2;

	}
	gDivergence[idx] = divergence;
}
