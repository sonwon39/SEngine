---
name: SPH project goal and phased roadmap
description: SEngine's SPH targets sph14 Figure 1 visual quality; agreed phased plan (Phase 0 → 7)
type: project
originSessionId: c5d4498c-1c2b-424f-a858-89376e24a707
---
User's stated goal for SEngine's SPH module: reproduce the visual quality of **sph14.pdf Figure 1** (breaking dam scene, water-like rendering).

**Why:** User wants their realtime renderer to demonstrate fluid that looks like real water, not point-cloud particles. Discussed and confirmed 2026-05-09.

**Realistic scope:** NOT 20M particles real-time — even the original paper measures single step ≈ 30s on 6-core CPU. The "Figure 1 look" is reachable at **100K–1M particles in 3D** with proper rendering pipeline.

## Phased plan (presented to user, implicitly accepted)

| Phase | Item | Why this order |
|---|---|---|
| 1 | Spatial grid + parallel radix sort (Index Sort, sph14 §2.1.1) | replaces O(N²) all-pairs — gates everything else |
| 2 | Screen-Space Fluid Rendering (sph14 §7.3) | highest visual ROI; independent of sim, can run parallel with Phase 1 |
| 3 | 3D extension (kernel coef 8/π, hd=1/h³, neighbor cells 9→27) | needed before scaling further |
| 4 | **DFSPH** (sph22 update over IISPH) — replaces current Tait EOS | enables larger dt, stable at high particle count |
| 5 | **Volume Maps** boundary (sph22 [BKWK20], replaces sph14's Akinci 2012) | fixes wall-sticking, no bumpy surface |
| 6 | Compact Hashing (sph14 §2.1.4) + Z-curve reorder | memory efficiency for 1M+ |
| 7 | (Optional polish) Foam/spray [IAAT12], surface tension, adaptive resolution |

## Algorithm choices (post sph22 reading)

- **DFSPH ≻ IISPH**: per sph22, all global pressure solvers (IISPH, PCISPH, PBF, DFSPH) are mathematically equivalent (Jacobi PPE solvers). Practical differences come from impl. DFSPH is current standard, often 2–4x faster.
- For learning/first impl: **PBF is simplest** — recommend that as Phase 4 stepping stone before DFSPH.
- **Volume Maps ≻ Akinci 2012 particle boundary**: implicit boundary, no bumpy surface, single SDF lookup, decouples particle size from boundary.
- ML/data-driven approaches in sph22 §7: explicitly skip — bad ROI for indie graphics project.

## Current state snapshot (2026-05-09, may drift — verify before assuming)

- 2D simulation, ~20K particles working stably
- O(N²) all-pairs (Phase 1 not started)
- Tait EOS, k=1, mass=1, particleRadius drives h/rho0
- Point billboard rendering with radial fade (Phase 2 not started)
- Wall coefficient -0.2 with `velocity·normal < 0` check
- dt = 1/300 fixed
- No pressure clamp (negative pressures still allowed; tensile instability possible)

## How to apply

When user asks "next step" / "what should I do" / "어떻게 개선" about SPH:
- Suggest the **next un-completed phase** (verify by reading code)
- Don't recommend skipping phases (e.g., don't push DFSPH before spatial grid exists)
- Each phase has concrete visual/numeric verification criteria — point those out so the user has a stopping condition
- For Phase 4 specifically: recommend PBF as first impl, DFSPH as second iteration
- For rendering improvements (Figure 1 look): SSFR (Phase 2) is the dominant lever, NOT particle count or simulation algorithm
