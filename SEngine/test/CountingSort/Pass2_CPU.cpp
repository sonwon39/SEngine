// Pass 2a / Pass 2b 를 CPU에서 for 문으로 풀어쓴 시뮬레이션.
// HLSL 셰이더의 각 thread 가 곧 한 iteration 이 된다.
//
// 매핑:
//   [numthreads(GROUP_SIZE,1,1)]    →   for (GTid = 0; GTid < GROUP_SIZE; GTid++)
//   Dispatch(numGroups,1,1)         →   for (Gid  = 0; Gid  < numGroups;  Gid++)
//   SV_DispatchThreadID             →   DTid = Gid * GROUP_SIZE + GTid
//   groupshared                     →   각 group 안에서만 살아있는 지역 배열
//   GroupMemoryBarrierWithGroupSync →   phase 와 phase 사이의 "다음 for 문 시작"
//   WavePrefixSum / WaveActiveSum   →   wave 단위 inner 루프로 직접 계산
//
// 작아서 머리로 따라갈 수 있도록 GROUP_SIZE=8, LANE_COUNT=4 (wave 2개) 로 설정.

#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;

constexpr uint32_t GROUP_SIZE = 8;
constexpr uint32_t LANE_COUNT = 4;
constexpr uint32_t WAVE_COUNT = GROUP_SIZE / LANE_COUNT;  // = 2

// ─────────────────────────────────────────────────────────────────────────────
// Pass 2a: per-block exclusive scan + emit block total.
// HLSL 의 Pass2a_ScanBlocks.hlsl 에 대응.
// ─────────────────────────────────────────────────────────────────────────────
void Pass2a_ScanBlocks(
    const vector<uint32_t>& gInput,
    vector<uint32_t>&       gOutput,
    vector<uint32_t>&       gBlockSums,
    uint32_t                gScanCount)
{
    const uint32_t numGroups = (gScanCount + GROUP_SIZE - 1) / GROUP_SIZE;

    // ── 바깥 for: GPU 가 numGroups 개의 threadgroup 을 병렬로 돌리는 것에 해당 ──
    for (uint32_t Gid = 0; Gid < numGroups; Gid++)
    {
        // groupshared : 한 group 안에서만 공유되는 메모리. 매 group 마다 새로 시작.
        uint32_t sWaveSums[WAVE_COUNT] = {};

        // 각 thread 가 자기 자리에 들고 있는 값들. GPU 에선 register, 여기선 배열로 흉내.
        uint32_t val[GROUP_SIZE]         = {};
        uint32_t wavePrefix[GROUP_SIZE]  = {};
        uint32_t waveTotal[GROUP_SIZE]   = {};

        // ── Phase 1: 입력 로드 ─────────────────────────────────────────────
        //    HLSL:  uint val = (DTid.x < gScanCount) ? gInput[DTid.x] : 0u;
        for (uint32_t GTid = 0; GTid < GROUP_SIZE; GTid++)
        {
            uint32_t DTid = Gid * GROUP_SIZE + GTid;
            val[GTid] = (DTid < gScanCount) ? gInput[DTid] : 0u;
        }

        // ── Phase 2: wave 단위 exclusive scan + total ────────────────────
        //    HLSL:  uint wavePrefix = WavePrefixSum(val);
        //           uint waveTotal  = WaveActiveSum(val);
        //    GPU 는 instruction 하나에 끝내지만, CPU 에선 wave 안을 직접 훑는다.
        for (uint32_t waveIdx = 0; waveIdx < WAVE_COUNT; waveIdx++)
        {
            uint32_t base = waveIdx * LANE_COUNT;
            uint32_t total = 0;
            for (uint32_t lane = 0; lane < LANE_COUNT; lane++)
            {
                wavePrefix[base + lane] = total;   // exclusive
                total += val[base + lane];
            }
            // waveTotal 은 wave 전체 lane 이 같은 값을 broadcast 받음
            for (uint32_t lane = 0; lane < LANE_COUNT; lane++)
                waveTotal[base + lane] = total;
        }

        // ── Phase 3: lane 0 만 자기 wave 총합을 groupshared 에 기록 ───────
        //    HLSL:  if (laneIdx == 0) sWaveSums[waveIdx] = waveTotal;
        for (uint32_t GTid = 0; GTid < GROUP_SIZE; GTid++)
        {
            uint32_t laneIdx = GTid % LANE_COUNT;
            uint32_t waveIdx = GTid / LANE_COUNT;
            if (laneIdx == 0)
                sWaveSums[waveIdx] = waveTotal[GTid];
        }
        // [GroupMemoryBarrierWithGroupSync] 와 동치 — 다음 단계는 위가 다 끝난 뒤 시작.

        // ── Phase 4: wave 0 이 sWaveSums 를 또 한 번 scan ─────────────────
        //    HLSL:  if (waveIdx == 0) {
        //               uint x = (laneIdx < waveCount) ? sWaveSums[laneIdx] : 0;
        //               uint scanned = WavePrefixSum(x);
        //               if (laneIdx < waveCount) sWaveSums[laneIdx] = scanned;
        //           }
        //    "wave 안에서의 scan" 을 또 흉내내는 inner 루프.
        {
            uint32_t scanIn[LANE_COUNT]  = {};
            uint32_t scanOut[LANE_COUNT] = {};
            for (uint32_t lane = 0; lane < LANE_COUNT; lane++)
                scanIn[lane] = (lane < WAVE_COUNT) ? sWaveSums[lane] : 0;

            uint32_t total = 0;
            for (uint32_t lane = 0; lane < LANE_COUNT; lane++)
            {
                scanOut[lane] = total;          // exclusive
                total += scanIn[lane];
            }
            for (uint32_t lane = 0; lane < WAVE_COUNT; lane++)
                sWaveSums[lane] = scanOut[lane];
        }
        // [GroupMemoryBarrierWithGroupSync]

        // ── Phase 5: 합치기 + 출력 + 블록 총합 ────────────────────────────
        //    HLSL:  uint blockExclusive = sWaveSums[waveIdx] + wavePrefix;
        //           if (DTid.x < gScanCount) gOutput[DTid.x] = blockExclusive;
        //           if (GTid == GROUP_SIZE - 1) gBlockSums[Gid.x] = blockExclusive + val;
        for (uint32_t GTid = 0; GTid < GROUP_SIZE; GTid++)
        {
            uint32_t DTid    = Gid * GROUP_SIZE + GTid;
            uint32_t waveIdx = GTid / LANE_COUNT;
            uint32_t blockExclusive = sWaveSums[waveIdx] + wavePrefix[GTid];

            if (DTid < gScanCount)
                gOutput[DTid] = blockExclusive;

            if (GTid == GROUP_SIZE - 1)
                gBlockSums[Gid] = blockExclusive + val[GTid];
        }
    } // 다음 group
}

// ─────────────────────────────────────────────────────────────────────────────
// Pass 2b: scanned block-sum 을 각 block 의 모든 원소에 더한다.
// HLSL 의 Pass2b_AddBlockOffsets.hlsl 에 대응.
// ─────────────────────────────────────────────────────────────────────────────
void Pass2b_AddBlockOffsets(
    vector<uint32_t>&       gPartial,
    const vector<uint32_t>& gScannedBlockSums,
    uint32_t                gScanCount)
{
    const uint32_t numGroups = (gScanCount + GROUP_SIZE - 1) / GROUP_SIZE;

    for (uint32_t Gid = 0; Gid < numGroups; Gid++)
    {
        for (uint32_t GTid = 0; GTid < GROUP_SIZE; GTid++)
        {
            uint32_t DTid = Gid * GROUP_SIZE + GTid;
            if (DTid < gScanCount)
                gPartial[DTid] += gScannedBlockSums[Gid];
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 호스트 측 흐름. HLSL 으로 옮기면 그대로 Dispatch 시퀀스가 됨.
// ─────────────────────────────────────────────────────────────────────────────
int main()
{
    // 16 개 입력 → GROUP_SIZE=8 이므로 block 2 개 → blockSums 2 개는 1 group 안에 들어감.
    vector<uint32_t> input = {
        3, 1, 4, 1, 5, 9, 2, 6,     // block 0
        5, 3, 5, 8, 9, 7, 9, 3      // block 1
    };
    const uint32_t N = (uint32_t)input.size();

    // ── Level 0: input → output_L0 + blockSums_L1 ──────────────────────────
    vector<uint32_t> output_L0(N, 0);
    const uint32_t numBlocks_L0 = (N + GROUP_SIZE - 1) / GROUP_SIZE;
    vector<uint32_t> blockSums_L1(numBlocks_L0, 0);

    Pass2a_ScanBlocks(input, output_L0, blockSums_L1, N);

    cout << "[L0] output_L0 (block-internal exclusive scan):\n  ";
    for (auto v : output_L0) cout << v << " ";
    cout << "\n[L0] blockSums_L1 (block totals):\n  ";
    for (auto v : blockSums_L1) cout << v << " ";
    cout << "\n\n";

    // ── Level 1: blockSums_L1 자체를 scan ──────────────────────────────────
    //    원소가 GROUP_SIZE 이하라서 1 group 으로 끝남. (그래서 blockSums_L2 는 더미)
    vector<uint32_t> scannedBlockSums_L1(numBlocks_L0, 0);
    vector<uint32_t> blockSums_L2(1, 0);  // 한 group 짜리 — 안 씀
    Pass2a_ScanBlocks(blockSums_L1, scannedBlockSums_L1, blockSums_L2, numBlocks_L0);

    cout << "[L1] scannedBlockSums_L1 (exclusive scan of block totals):\n  ";
    for (auto v : scannedBlockSums_L1) cout << v << " ";
    cout << "\n\n";

    // ── Pass 2b: scannedBlockSums_L1 을 각 block 에 더해 최종 결과 ──────────
    Pass2b_AddBlockOffsets(output_L0, scannedBlockSums_L1, N);

    cout << "[Final] output_L0 (global exclusive scan):\n  ";
    for (auto v : output_L0) cout << v << " ";
    cout << "\n";

    // ── 검증: naive sequential 결과와 비교 ─────────────────────────────────
    vector<uint32_t> expected(N, 0);
    {
        uint32_t acc = 0;
        for (uint32_t i = 0; i < N; i++) { expected[i] = acc; acc += input[i]; }
    }
    cout << "[Check] expected:\n  ";
    for (auto v : expected) cout << v << " ";
    cout << "\n\n";

    cout << (output_L0 == expected ? "PASS\n" : "FAIL\n");
    return 0;
}
