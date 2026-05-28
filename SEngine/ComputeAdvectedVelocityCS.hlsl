#include "StableFluidsUtility.hlsli"

RWStructuredBuffer<float3> gAdvectedVelocity     : register(u0);

StructuredBuffer<float3>   gVelocity			 : register(t0);

SamplerState 			   gWrapClampSampler     : register(s0);

float3 SampleVelocityTrilinear(float3 pos)
{
    float3 p = pos;

    int3 dim = (int3)gLocalCB.gGridDim;
    int3 i0  = clamp(int3(floor(p)),     int3(0,0,0), dim - 1);
    int3 i1  = clamp(i0 + 1,             int3(0,0,0), dim - 1);
    float3 t = saturate(p - float3(i0));

    float3 v000 = gVelocity[FlatIdx(int3(i0.x, i0.y, i0.z))];
    float3 v100 = gVelocity[FlatIdx(int3(i1.x, i0.y, i0.z))];
    float3 v010 = gVelocity[FlatIdx(int3(i0.x, i1.y, i0.z))];
    float3 v110 = gVelocity[FlatIdx(int3(i1.x, i1.y, i0.z))];
    float3 v001 = gVelocity[FlatIdx(int3(i0.x, i0.y, i1.z))];
    float3 v101 = gVelocity[FlatIdx(int3(i1.x, i0.y, i1.z))];
    float3 v011 = gVelocity[FlatIdx(int3(i0.x, i1.y, i1.z))];
    float3 v111 = gVelocity[FlatIdx(int3(i1.x, i1.y, i1.z))];

    float3 v00 = lerp(v000, v100, t.x);
    float3 v10 = lerp(v010, v110, t.x);
    float3 v01 = lerp(v001, v101, t.x);
    float3 v11 = lerp(v011, v111, t.x);

    float3 v0  = lerp(v00, v10, t.y);
    float3 v1  = lerp(v01, v11, t.y);
    return       lerp(v0,  v1,  t.z);
}

float3 GetOldPosition(int idx, float deltaTime)
{
	uint3 gridDim = gLocalCB.gGridDim;
	int maxIdx = gridDim.x * gridDim.y * gridDim.z - 1;
	float3 velocity = gVelocity[idx];

	uint xyArea = gridDim.x * gridDim.y;
	uint iz = idx / xyArea;
	uint xy = idx % xyArea;
	uint iy = xy / gridDim.x;
	uint ix = xy % gridDim.x;
	float3 currPosition = float3(ix, iy, iz) + 0.5;

	float3 oldPosition = currPosition + velocity * -deltaTime;
	oldPosition -= 0.5f;
	oldPosition = clamp(oldPosition, float3(0,0,0), gridDim - 1);

	return oldPosition;
}

[numthreads(SF_GROUP_SIZE, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint3 gridDim = gLocalCB.gGridDim;
	float deltaTime = gLocalCB.deltaTime;
	int maxIdx = gridDim.x * gridDim.y * gridDim.z - 1;
	int idx = DTid.x;
	
	if (DTid.x > maxIdx)
		return;

	float3 oldPosition = GetOldPosition(idx, deltaTime);
	float3 newVelocity = SampleVelocityTrilinear(oldPosition);
	gAdvectedVelocity [idx] = newVelocity;
}
