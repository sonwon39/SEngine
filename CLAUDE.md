# CLAUDE.md

Behavioral guidelines for graphics programming and paper-implementation work in this repo. Adapted from Karpathy-style coding principles.

**Tradeoff:** These bias toward correctness, numerical stability, and faithfulness to source papers over speed. For trivial tasks, use judgment.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly — coordinate system (LH/RH), row/column-major matrices, units, dimensionality (2D vs 3D kernel), winding order, color space (linear vs sRGB), pre/post-multiplied alpha. Graphics bugs hide in conventions.
- When implementing from a paper: cite the equation number / section. If the paper is ambiguous (notation collision, missing factor, unstated boundary condition), surface it — don't pick silently.
- If multiple interpretations of a formula exist, present them. "The original paper does X, but follow-up Y does Z — which?"
- If a simpler / textbook variant exists (e.g. naive O(N²) before spatial hashing), say so and recommend MVP first.
- If something is unclear (resource state, descriptor layout, what buffer is "prev" vs "curr"), stop and ask.

Watch especially for **silent dimensional / unit mismatches**: rho0 vs kernel-output density, h vs particle spacing, world-space vs NDC, dt vs deltaTime. Always sanity-check the order of magnitude before committing values.

## 2. Simplicity First

**Minimum code that reproduces the paper's claim. Nothing speculative.**

- No optimizations the paper doesn't mention (BVH, spatial hash, tiled compute) until the naive version visibly works.
- No abstractions for "future shader variants" until there are two callers.
- No fallback paths for cases the paper doesn't cover (negative pressures, degenerate Jacobians, etc.) unless they actually arise. Then add them surgically with a comment citing the failure.
- No "flexible" constant buffers / root signature slots for parameters that aren't tuned yet.
- Hardcode constants from the paper first; expose as CB fields only when you're actively sweeping them.

Ask yourself: "Could I show the paper's Figure N with strictly less code?" If yes, cut.

## 3. Surgical Changes

**Touch only what you must. Don't reshape the engine to fit one paper.**

When editing existing rendering / compute code:
- Don't reorder or rename root signature parameters, descriptor heaps, or RWStructuredBuffer registers unless that's the task.
- Don't change resource transition / barrier patterns in code paths you aren't touching.
- Don't migrate HLSL style (e.g. `float3` vs `vector<float,3>`, capitalization) just because you'd write it differently.
- Don't "fix" GPU sync that's working — race conditions are best diagnosed before being papered over.
- If you notice unrelated correctness issues (missing UAV barrier, leaked descriptor, redundant Flush), mention them as a separate item — don't bundle.

When your changes create orphans:
- Remove only the imports / CB fields / shader entries that YOUR changes made unused.
- Don't delete pre-existing unused shaders, dead `#if 0` blocks, or commented-out experiments unless explicitly asked — they often encode prior intent.

The test: every changed line should trace to the requested feature or to a numerical bug surfaced by it.

## 4. Goal-Driven Execution

**Define a *visible* success criterion. Loop until verified.**

Graphics has weak unit-test affordances; substitute concrete observables:
- "Implement SPH" → "Particles spawned in a 100×100 grid settle into a still pool with density ≈ ρ₀ ± 5% within N seconds"
- "Add tone mapping" → "Reference HDR test image renders within ΔE < X of authors' Figure 5"
- "Port shader to compute" → "Output buffer is bit-identical (or within 1 ULP) to PS reference for the same inputs"
- "Fix flickering" → "Capture a 5-second clip; no frame-to-frame variance above threshold T"

For multi-step paper implementations, state a plan with verifiable checkpoints:
```
1. Naive O(N²) kernel evaluation       → density at rest matches 1/dx² ± 5%
2. Pressure + viscosity (no boundary)  → particles in periodic box stay finite for 1000 steps
3. Walls / boundary handling           → free-fall test reaches expected terminal depth
4. Optimization pass (grid / hash)     → same visual + ≥ Nx faster
```

Strong criteria let you self-verify. Weak criteria ("make it look better") require constant user feedback. **If you can't state a criterion before coding, you don't yet understand the task — go back to §1.**

## 5. Numerical Diligence (graphics-specific)

The biggest class of bugs in this repo is silent float blow-up, not logic errors.

When changing math-heavy GPU code:
- Estimate the **range** of every intermediate before running. `ρ ≈ Σ W ≈ N_neighbors × W(0)`. `pressure = k·(ρ/ρ₀)^7` — does it overflow float32 (3.4×10³⁸)?
- For new kernels, dump a single thread's intermediates (`StructuredBuffer<DebugRecord>`, RenderDoc, or PIX) for the first frame and inspect.
- Add a one-line NaN/Inf guard during bring-up:
  ```hlsl
  if (any(isnan(acc)) || any(isinf(acc))) acc = float3(0, -9.8, 0); // TODO: remove
  ```
  and remove it once root-caused. Don't ship the guard as the fix.
- Prefer fixed `dt` + substeps over `dt = deltaTime` for any explicit integrator. Document the CFL bound (`dt < h/c`).

## 6. Paper Fidelity

When implementing a paper, the paper is ground truth — even when "obviously wrong":
- If the paper uses a strange normalization, reproduce it first, then deviate with a comment: `// Paper uses 7/(πh²); we use 15/(7πh²) (2D cubic spline) — see §3.1`.
- Match notation in code where reasonable: paper's `ρ_i` → variable `rhoi`, paper's `∇W_ij` → `w_grad`. Future-you reading both side-by-side will thank you.
- When a paper omits a factor (mass, time-step, kernel constant), assume `=1` *and say so explicitly* in the comment, so the assumption is auditable.
- Keep a one-line citation at the top of any file implementing a specific paper:
  ```hlsl
  // Müller et al. 2003, "Particle-Based Fluid Simulation for Interactive Applications", §4.2
  ```
  This is the one place comments are mandatory in this repo.

---

**These guidelines are working if:** fewer "all particles flew off / went black / are NaN" sessions, fewer cargo-culted constant tweaks without dimensional reasoning, and clarifying questions about coordinate systems / units arrive *before* a 200-line shader is written.
