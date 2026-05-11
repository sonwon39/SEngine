// Teaching example: exclusive prefix sum (scan) over particleCounter -> cellStart
// using SM 6.0 wave intrinsics. Single-threadgroup version: handles up to
// GROUP_SIZE elements. For larger arrays you need a multi-dispatch version
// (per-block scan -> scan-of-block-totals -> add back). See note at bottom.
//
// Compile target: cs_6_0 or higher (wave intrinsics require SM 6.0+).

#define GROUP_SIZE 256
// Max waves per group at the smallest wave size we expect (32).
// Used to size groupshared. If you know the GPU is AMD (wave64), 4 is enough,
// but 8 is a safe upper bound for wave32 hardware.
#define MAX_WAVES_PER_GROUP (GROUP_SIZE / 32)

RWStructuredBuffer<uint> inputCount  : register(u0); // particleCounter
RWStructuredBuffer<uint> outputStart : register(u1); // cellStart (exclusive scan output)

groupshared uint waveSums[MAX_WAVES_PER_GROUP];

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
          uint  GTid : SV_GroupIndex)
{
    uint laneCount = WaveGetLaneCount();      // 32 on NV, 32/64 on AMD
    uint laneIdx   = WaveGetLaneIndex();      // 0..laneCount-1
    uint waveIdx   = GTid / laneCount;        // which wave inside this group
    uint waveCount = GROUP_SIZE / laneCount;  // how many waves in this group

    uint val = inputCount[DTid.x];

    // ── Step 1: exclusive scan WITHIN the wave (hardware, 1 instruction) ──
    //   waveLocalPrefix[lane] = sum of val[0..lane-1] inside this wave
    uint waveLocalPrefix = WavePrefixSum(val);
    uint waveTotal       = WaveActiveSum(val); // total of this wave (broadcast)

    // ── Step 2: lane 0 of each wave writes its wave's total to groupshared ──
    if (laneIdx == 0)
    {
        waveSums[waveIdx] = waveTotal;
    }
    GroupMemoryBarrierWithGroupSync();

    // ── Step 3: scan the wave totals. There are waveCount of them (<= 32 for
    //   a 1024-thread group with wave32), so a SINGLE wave can do it with
    //   another WavePrefixSum. We use the first wave of the group.          ──
    if (waveIdx == 0)
    {
        uint x = (laneIdx < waveCount) ? waveSums[laneIdx] : 0u;
        uint scanned = WavePrefixSum(x);
        if (laneIdx < waveCount)
        {
            waveSums[laneIdx] = scanned; // overwrite with exclusive prefix
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // ── Step 4: combine. Each thread's exclusive prefix over the group =
    //   (offset of my wave) + (my exclusive prefix within my wave)           ──
    uint groupExclusivePrefix = waveSums[waveIdx] + waveLocalPrefix;

    outputStart[DTid.x] = groupExclusivePrefix;
}

// NOTE on scaling beyond one threadgroup (e.g. 100^3 = 1,000,000 cells):
//   This shader only scans GROUP_SIZE elements. For a full array of N elements
//   you run THREE dispatches:
//     A. this shader on each block of GROUP_SIZE inputs, writing per-block
//        exclusive scan to outputStart AND each block's total to a small
//        blockSums[N / GROUP_SIZE] buffer.
//     B. scan blockSums (same shader, or recursively if it's still > GROUP_SIZE).
//     C. add scanned blockSums[blockIdx] back to every element of block blockIdx.
//   That gives a full exclusive scan in O(N) work, O(log N) depth.
