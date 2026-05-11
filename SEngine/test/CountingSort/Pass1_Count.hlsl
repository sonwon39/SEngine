// Pass 1: count particles per cell, cache each particle's cellId.
//
// Dispatch:  ceil(gParticleCount / GROUP_SIZE)
// Inputs:    positions[N]
// Outputs:   cellCount[M]     (atomically incremented)
//            particleCellId[N] (avoids recomputing cellId in Pass 3)

#include "CountingSortCommon.hlsli"

StructuredBuffer<float3>  gPositions       : register(t0);
RWStructuredBuffer<uint>  gCellCount       : register(u0);
RWStructuredBuffer<uint>  gParticleCellId  : register(u1);

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= gParticleCount) return;

    uint cellId = CellIdFromPos(gPositions[DTid.x]);
    gParticleCellId[DTid.x] = cellId;

    uint _;
    InterlockedAdd(gCellCount[cellId], 1, _);
}
