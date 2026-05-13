---
name: SEngine code layout and execution flow
description: How SEngine.exe is structured — entry chain, frame loop, command list pools, integration points for new GPU work
type: reference
originSessionId: 6e3c0fa4-2d75-4753-8ad4-d608ca41d634
---
**Project shape**: single `SEngine.vcxproj` building `SEngine.exe` (Application, Console subsystem, x64 + Win32; only x64 enables SM 6.0). Source files all sit directly under `SEngine/`. C++20 (`stdcpp20`), `/utf-8`.

**Entry chain**:
- `main.cpp::main` → `Core::SimpleApp app(1280,720,0); app.Initialize(); app.Run();`
- `BaseApp::Initialize` runs `InitWindow` → `InitDirectX` → `InitGUI`.
- `SimpleApp::InitDirectX` creates the `ID3D12Device5` (hw, falling back to WARP), calls `Graphics::InitializeCommonState(device)` then `Renderer::Initialize(device)`, then constructs `RenderEngine` and calls `m_renderEngine->Initialize(...)`.
- `RenderEngine::Initialize` → command objects, descriptor heaps, swap chain RTVs, textures, depth, `InitScene` (creates `m_mesh` and `m_sph`), camera.
- `SimpleApp::Run` is the Win32 message pump; on idle it calls `m_renderEngine->Tick(deltaTime)`.
- `RenderEngine::Tick` → `SPHSimulation()` which dispatches `ComputeSPH` PSOs (`computeDensityCPSO`, `computePressureCPSO`, `computeForcesCPSO`, `sphSimulationCPSO`) then `RenderSPH("particleRenderPSO", clear=true, isFinal=false)` then `RenderGUI(isFinal=true)`.

**Command list pools** (all DIRECT type):
- `m_commandList` + `m_commandAllocator` — main frame rendering.
- `m_resourceCommandList` + `m_resourceCommandAllocator` — buffer/texture upload, also reused by `m_sph->Reset`.
- `gui_commandList` + `gui_commandAllocator` — ImGui pass.
- `m_computeCommandLists[0..3]` + matching allocators — one per SPH compute pass, indexed by `ComputeSPH(psoName, idx)`.
- Two fences: `m_fence` (frame) and `m_createBufferfence` (resource).

**Where to add new GPU work**:
- Long-running per-frame compute: register a `ComputePSO` in `Renderer::Initialize` (matching root signature from `Graphics::g_*_RS` pool) and dispatch it from `Tick`/`SPHSimulation` similar to `ComputeSPH`.
- One-shot validation / test work (no per-frame cost): tail-call from `RenderEngine::Initialize` after camera setup, creating its own queue/alloc/cmdList/fence (see `CountingSortTest::Run` for the pattern).

**Test folder convention**: `SEngine/test/<topic>/` holds standalone `.cpp` + HLSL study files. By default these aren't in `ClCompile` (e.g. `CountingSort.cpp`, `Pass2_CPU.cpp` each declare their own `main()`). Add to vcxproj only when you actually want them linked.

**Async-style fence sync** uses `Signal` → `GetCompletedValue` check → `SetEventOnCompletion` + `WaitForSingleObject`. Both `FlushCommands` and `FlushResourceCommands` follow this pattern.
