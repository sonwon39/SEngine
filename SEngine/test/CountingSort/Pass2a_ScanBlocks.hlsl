// Pass 2a: per-threadgroup exclusive scan + emit per-block total.
//
// Re-used at every scan level. The host wires the bindings differently each call:
//   Level 0:  in = cellCount    , out = cellStart    , blockSums = blockSums_L1
//   Level 1:  in = blockSums_L1 , out = blockSums_L1 , blockSums = blockSums_L2   (in-place OK)
//   ...
//
// Dispatch:  ceil(gScanCount / GROUP_SIZE)
//
// At lane 0 of each wave we stash that wave's total in groupshared. The first
// wave then scans those wave totals with one more WavePrefixSum. Step count
// per element is O(1) on the wave path; the only barriers are the two
// GroupMemoryBarrierWithGroupSync below.

#include "CountingSortCommon.hlsli"

StructuredBuffer<uint>   gInput     : register(t0);
RWStructuredBuffer<uint> gOutput    : register(u0);  // exclusive scan within block
RWStructuredBuffer<uint> gBlockSums : register(u1);  // one inclusive total per block

groupshared uint sWaveSums[MAX_WAVES_PER_GROUP];

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
          uint3 Gid  : SV_GroupID,
          uint  GTid : SV_GroupIndex)
{
    const uint laneCount = WaveGetLaneCount();
    const uint laneIdx   = WaveGetLaneIndex();
    const uint waveIdx   = GTid / laneCount;
    const uint waveCount = GROUP_SIZE / laneCount;

    // OOB lanes contribute 0 — keeps the last partial block correct.
    uint val = (DTid.x < gScanCount) ? gInput[DTid.x] : 0u;

    // ── 1. exclusive scan inside the wave (one hardware instruction) ──
    uint wavePrefix = WavePrefixSum(val);
    uint waveTotal  = WaveActiveSum(val);

    if (laneIdx == 0) sWaveSums[waveIdx] = waveTotal;
    GroupMemoryBarrierWithGroupSync();

    // ── 2. scan the wave totals using wave 0 (waveCount ≤ laneCount usually) ──
    if (waveIdx == 0)
    {
        uint x = (laneIdx < waveCount) ? sWaveSums[laneIdx] : 0u;
        uint scanned = WavePrefixSum(x);
        if (laneIdx < waveCount) sWaveSums[laneIdx] = scanned;
    }
    GroupMemoryBarrierWithGroupSync();

    // ── 3. combine: my exclusive prefix in the block = waveOffset + wavePrefix ──
    uint blockExclusive = sWaveSums[waveIdx] + wavePrefix;

    if (DTid.x < gScanCount) gOutput[DTid.x] = blockExclusive;

    // ── 4. last active lane of the block writes the inclusive block total ──
    // For a full block this is GTid == GROUP_SIZE-1.
    // For the tail block this is also fine because OOB lanes have val=0.
    if (GTid == GROUP_SIZE - 1)
    {
        gBlockSums[Gid.x] = blockExclusive + val;
    }
}
