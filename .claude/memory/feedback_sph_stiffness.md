---
name: SPH stiffness recommendation calibration
description: For this project's explicit-Euler SPH on the GPU, recommend k (Tait stiffness) lower than the weakly-compressible theoretical value
type: feedback
originSessionId: c5d4498c-1c2b-424f-a858-89376e24a707
---
When recommending Tait EOS stiffness `k` for the SEngine SPH simulation, start much lower than the weakly-compressible theoretical bound (which would suggest k ≈ ρ₀·g·H_pool / ((1+η)^7 - 1) for η ≈ 0.1, often 10⁴–10⁵ range).

User reported that my prior recommendation (k = 1000 ~ 10000 for ρ₀ ≈ 4500, mass=1, h=0.02) was too high and unstable; lowering k significantly produced stable behavior.

**Why:** This SPH uses naive explicit Euler with variable dt = deltaTime, no substepping, and naive O(N²) all-pairs neighbor search. CFL margin is poor, so high `c = √(7k/ρ₀)` blows up before the fluid can equilibrate. The user prioritizes visible stability over physical incompressibility.

**How to apply:** When suggesting k for this project, start at **k ≈ 10–100** range (allowing larger compression), and tell the user to ramp up only if the fluid looks too "gassy". Also flag that proper weakly-compressible k requires substepping + fixed dt, which this project does not have yet.
