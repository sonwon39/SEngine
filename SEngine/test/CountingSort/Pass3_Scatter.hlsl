// Pass 3: write each particle index into its sorted slot.
//
// Dispatch:  ceil(gParticleCount / GROUP_SIZE)
// Inputs:    particleCellId[N]   (from Pass 1)
//            cellStart[M]        (from Pass 2, immutable here)
// Output:    sortedIndices[N]    sorted by cellId
//
// gScatterCounter[M] must be cleared to 0 before this dispatch
// (ClearUnorderedAccessViewUint). It serves as a per-cell write cursor so we
// don't mutate cellStart — cellStart needs to stay valid for the neighbor
// query that follows.

#include "CountingSortCommon.hlsli"

StructuredBuffer<uint>   gParticleCellId  : register(t0);
StructuredBuffer<uint>   gCellStart       : register(t1);
RWStructuredBuffer<uint> gScatterCounter  : register(u0);
RWStructuredBuffer<uint> gSortedIndices   : register(u1);

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= gParticleCount) return;

    uint cellId = gParticleCellId[DTid.x];

    uint localSlot;
    InterlockedAdd(gScatterCounter[cellId], 1, localSlot);

    uint dst = gCellStart[cellId] + localSlot;
    gSortedIndices[dst] = DTid.x;
}
