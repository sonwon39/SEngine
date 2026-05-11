// Shared constants for the counting-sort neighbor-search pipeline.
// Compile with -T cs_6_0 or higher (wave intrinsics require SM 6.0+).
//
// Pipeline overview (1M particles, ~125K cells example with GROUP_SIZE=256):
//
//   Pass 1  Count           dispatch( ceil(N / 256) )      → cellCount, particleCellId
//   Pass 2  ExclusiveScan(cellCount → cellStart)
//     2a  ScanBlocks         dispatch( ceil(M  / 256) )    → cellStart (partial), blockSums_L1
//     2a  ScanBlocks         dispatch( ceil(M1 / 256) )    → blockSums_L1 (partial), blockSums_L2
//     ... recurse until level fits in a single threadgroup ...
//     2b  AddBlockOffsets   dispatch( ceil(M1 / 256) )    → blockSums_L1 final
//     2b  AddBlockOffsets   dispatch( ceil(M  / 256) )    → cellStart final
//   ZeroCellCount (or ClearUAV) before pass 3
//   Pass 3  Scatter          dispatch( ceil(N / 256) )    → sortedIndices
//
// With GROUP_SIZE=256, two levels of scan handle up to 256*256 = 65,536 cells.
// For 125K cells you need three levels (M=125K → 489 → 2 → 1). The shader is
// the same at every level; only the buffer bindings and dispatch counts change.

#define GROUP_SIZE 256
#define MAX_WAVES_PER_GROUP (GROUP_SIZE / 32)  // upper bound for wave32 hardware

cbuffer GridCB : register(b0)
{
    float3 gGridMin;        // world-space corner of the grid
    float  gCellSize;       // h (uniform; equal to SPH support radius)
    uint3  gGridDim;        // cells per axis
    uint   gParticleCount;  // N
    uint   gCellCount;      // M = gGridDim.x * y * z
    uint   gScanCount;      // length of input to current scan dispatch (varies per level)
    uint   _pad0;
    uint   _pad1;
};

uint LinearCellId(int3 c)
{
    return uint(c.x) + uint(c.y) * gGridDim.x + uint(c.z) * gGridDim.x * gGridDim.y;
}

uint CellIdFromPos(float3 p)
{
    int3 c = int3(floor((p - gGridMin) / gCellSize));
    c = clamp(c, int3(0, 0, 0), int3(gGridDim) - 1);
    return LinearCellId(c);
}
