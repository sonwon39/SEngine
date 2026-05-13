---
name: SEngine shader build pipeline (FxCompile)
description: How HLSL gets compiled and consumed by C++ — vcxproj rules, generated header convention, include resolution, SM matrix
type: reference
originSessionId: 6e3c0fa4-2d75-4753-8ad4-d608ca41d634
---
**Build pipeline**: shaders are compiled offline by msbuild's `FxCompile` task during the C++ build. There is **no runtime DXC/D3DCompile usage** in this engine.

**Per-config FxCompile rule** (in `SEngine.vcxproj`):
```
<FxCompile>
  <ShaderModel>6.0</ShaderModel>                              <!-- x64 only -->
  <VariableName>g_p%(Filename)</VariableName>
  <HeaderFileOutput>$(OutDir)CompiledShaders\%(Filename).h</HeaderFileOutput>
  <ObjectFileOutput>$(OutDir)CompiledShaders\%(Filename).cso</ObjectFileOutput>
</FxCompile>
```
- Output: `$(OutDir)CompiledShaders/<Basename>.h` exposing `g_p<Basename>` as a `BYTE[]` bytecode.
- `%(Filename)` strips path, so files with the same basename in different folders **collide**. Keep basenames unique.
- `AdditionalIncludeDirectories` is `$(OutDir)`, so C++ sources include with `#include "CompiledShaders/<Basename>.h"`.
- **SM matrix**: x64 = SM 6.0 (DXC, wave intrinsics OK). Win32 = SM 4.0 (FXC, no wave intrinsics). Most shaders only run on x64; some (like `DefaultCS.hlsl`) declare per-config `ShaderModel` overrides for Win32 compatibility.

**Per-shader vcxproj entry** must set `ShaderType` (Compute / Vertex / Pixel / Geometry) — usually conditioned on `'$(Configuration)|$(Platform)'=='Debug|x64'` (and Release variants when the shader is shipped).

**`.hlsli` includes**: tracked as `<None Include="..." />` (not FxCompile). HLSL `#include "Foo.hlsli"` resolves relative to the source `.hlsl`'s directory, so co-locate the .hlsli with its consumers (or add include search paths if shared).

**Consumption pattern in C++**: see `Renderer.cpp::Initialize` — for each PSO, `#include "CompiledShaders/<Name>.h"` then `pso.SetComputeShader(g_p<Name>, sizeof(g_p<Name>))`. The bytecode array is embedded into the .exe.

**Existing shader inventory** (under SEngine root): `DefaultVS/PS/CS`, `ParticleRenderVS/GS/PS`, `ParticleSimulationCS`, `ComputeDensityCS`, `ComputePressureCS`, `ComputeForcesCS`, `SPHSimulationCS`. Shared `.hlsli`: `DefaultCommon.hlsli`, `ParticleCommon.hlsli`, `SPHUtility.hlsli`.

**Shaders outside SEngine/ root**: vcxproj `FxCompile Include="test\CountingSort\Pass2a_ScanBlocks.hlsl"` style works — msbuild compiles them in place; output still lands at `$(OutDir)CompiledShaders/Pass2a_ScanBlocks.h`. The `.hlsli` next to them resolves automatically.
