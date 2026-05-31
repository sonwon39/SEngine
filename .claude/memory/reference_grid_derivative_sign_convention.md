---
name: reference-grid-derivative-sign-convention
description: "SEngine Stable Fluids grid derivative sign rule — central difference is always (larger index − smaller index), and div/grad/Laplacian must share that convention"
metadata: 
  node_type: memory
  type: reference
  originSessionId: 0f15e8cd-74d1-4a39-92c1-d0598bfed15f
---

SEngine Stable Fluids lives in SEngine/*.hlsl: ComputeDivergenceCS, JacobiCS, ComputeFinalVelocityCS (pressure projection), plus AdvectionCS, SourcingCS. Buffers/heaps set up in World.cpp; orchestration in StableFluids.cpp.

**The sign rule:** central-difference derivative `∂q/∂(index)` is by definition `(q[index+1] − q[index−1]) / 2` — larger index minus smaller index, because the derivative is defined along the direction the index INCREASES. Screen "up/down" (0,0 = top-left, y−1 = visually up) is just a display label; the GPU only sees index numbers.

Correct, consistent state (verified working): both
- ComputeDivergenceCS: `divergence = (rightVel.x − leftVel.x + upVel.y − downVel.y) * 0.5` with `up = y+1`, `down = y−1`
- ComputeFinalVelocityCS: `divergenceP = float2(rightP − leftP, upP − downP) * 0.5` with same up/down

The original bug was the y term written as `(top(y-1) − bottom(y+1))` = smaller − larger = `−∂/∂y`, on BOTH div and grad. x was always correct (`right(x+1) − left(x−1)`). Jacobi's Laplacian is sign-fixed positive, so the flipped div/grad broke projection and produced grid-axis-aligned "diamond" artifacts. Fix = make div and grad both `(larger − smaller)` to match Jacobi. Always change div/grad as a PAIR. See [[feedback-grid-projection-sign-debugging]].
