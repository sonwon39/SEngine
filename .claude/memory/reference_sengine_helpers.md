---
name: SEngine D3D12 helpers, root signature pool, error handling
description: Reusable utilities — Utility class for buffer/heap/texture creation, the named common root signatures, ThrowIfFailed, descriptor wrappers
type: reference
originSessionId: 6e3c0fa4-2d75-4753-8ad4-d608ca41d634
---
**`GraphicsUtils::Utility`** ([Utility.h]) — wraps the device + a command list and offers boilerplate-free creation. Singleton-ish: `Graphics::utility` (a `shared_ptr<Utility>`) is initialized in `RenderEngine::Initialize` and used across the engine. Key methods:
- `CreateDescriptorHeap(num, type, heap&, nodeMask, flags)` — wraps `CreateDescriptorHeap`.
- `CreateConstantBuffer(size, buf&, void** pCpu)` — UPLOAD-heap CB with persistent map.
- `CreateTextureBuffer(buf&, w, h, format, flags, state, mips, name)` — DEFAULT-heap 2D texture; sets debug name; auto-applies depth clear values for D24S8/D32.
- `CreateResourceView(buf&, format, bUseMsaa, handle, DescriptorType)` — RTV/UAV/SRV/DSV for textures.
- `CreateStructuredResourceView(buf&, format, handle, DescriptorType, count, stride)` — SRV/UAV for structured buffers (only).
- `CreateBuffer<T>(vector<T>, gpu&, upload&, flags[, cmdList])` (template in `Utility.inl`) — 2-stage upload pattern (UPLOAD heap → CopyResource → DEFAULT heap).

**Named common root signatures** (in `Graphics::` namespace, defined in `GraphicsCommon.cpp::InitializeCommonState`):
- `g_commonRS` — table[SRV t0] + CBV b0 + static sampler (wrapLinear). For default mesh PSO.
- `g_U1_RS` — table[UAV u0]. (`defaultCPSO`)
- `g_U1_C1_RS` — table[UAV u0] + CBV b0. (`particleSimulationCPSO`)
- `g_U2_C1_RS` — table[UAV u0..u1] + CBV b0. (SPH compute PSOs)
- `g_S1_RS` — table[SRV t0].
- `g_S1_C1_RS` — table[SRV t0] + CBV b0. (`particleRenderPSO`)
- All use `ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT` flag (fine to reuse for compute).
- These cover the **descriptor-table** style only. For root-descriptor bindings (SetComputeRoot{SRV,UAV,CBV}View) you build your own `D3D12_ROOT_PARAMETER` and serialize directly (see `CountingSortTest.cpp`).

**Static sampler pool**: `wrapLinearSampler` (s0), `clampLinearSampler` (s1) — declared as `D3D12_STATIC_SAMPLER_DESC` in `GraphicsCommon.cpp`.

**Rasterizer/blend/depth presets** (also in `Graphics::`): `rasterizerDefault`, `wireRasterizer`, `noneCullRasterizer`, `blendNoColorWrite`, `blendColor` (additive), `depthStateDefault`.

**Error handling**:
- `ThrowIfFailed(hr)` macro (Utility.h) → wraps any HRESULT call; throws `DxException` with file:line:function on FAILED.
- `DxException::ToString()` formats with `_com_error::ErrorMessage()`.
- `ASSERT(cond)` macro only fires in debug builds; calls `__debugbreak()`.

**PSO wrappers** (`PipelineState.h`):
- `GraphicsPSO` / `ComputePSO` extend a common base `PSO(const wchar_t* name)`.
- Use: `pso.SetRootSignature(rs); pso.SetComputeShader(g_pName, sizeof(g_pName)); pso.Finalize(device);`
- The engine maps PSOs into `m_PSOs["nameP SO"]` / `m_CPSOs["nameCPSO"]` (string keyed) in `Renderer::Initialize`.

**Root signature wrapper** (`RootSignature.h`): `RootSignature::Reset(numParams, numStaticSamplers)` then `rs[i].InitAsDescriptorRange(type, register, count)` / `InitCBV(reg)` then `InitStaticSampler` and `Finalize(device, name, flags)`. Note: the wrapper only supports descriptor tables + root CBVs; root SRV/UAV descriptors aren't exposed (drop to raw D3D12 API).

**Descriptor-size cache**: `m_cbvSrvDescriptorSize`, `m_rtvDescriptorSize`, `m_dsvDescriptorSize` — queried once in `RenderEngine::Initialize` from `GetDescriptorHandleIncrementSize`; same pattern in `SPH::Initialize`. Use `CD3DX12_CPU_DESCRIPTOR_HANDLE(...).Offset(i, size)` to walk a heap.
