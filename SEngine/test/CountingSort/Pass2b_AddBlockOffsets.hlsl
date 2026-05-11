// Pass 2b: add the scanned block-sum array back to each element of the
// partially-scanned data, completing the exclusive scan at this level.
//
// Dispatch:  ceil(gScanCount / GROUP_SIZE)   (same shape as the matching 2a)
// Inputs:    gPartial[i]            = exclusive scan within its block
//            gScannedBlockSums[Gid] = exclusive prefix of all earlier blocks
// Output:    gPartial[i] += gScannedBlockSums[Gid]   (now a global exclusive scan)

#include "CountingSortCommon.hlsli"

RWStructuredBuffer<uint> gPartial          : register(u0);
StructuredBuffer<uint>   gScannedBlockSums : register(t0);

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
          uint3 Gid  : SV_GroupID)
{
    if (DTid.x >= gScanCount) return;
    gPartial[DTid.x] += gScannedBlockSums[Gid.x];
}
