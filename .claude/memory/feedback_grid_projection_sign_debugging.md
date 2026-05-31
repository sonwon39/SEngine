---
name: feedback-grid-projection-sign-debugging
description: "When debugging a grid pressure-projection fluid sim, verify divergence/gradient/Laplacian sign conventions as one matched set before suspecting convergence or aspect ratio"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 0f15e8cd-74d1-4a39-92c1-d0598bfed15f
---

When reviewing SEngine's Stable Fluids, I missed a sign bug in the y-derivative: divergence and gradient both computed `(smaller index − larger index)` for the y axis (`top(y-1) − bottom(y+1)`), while x was correct. I wrongly judged "div and grad are consistent with each other, so it's just a y-mirror, mathematically fine" and moved on to suspecting Jacobi iteration count and 16:9 aspect ratio. The user found it themselves by flipping up/down.

**Why:** The Jacobi Laplacian (`(left+right+up+down − 4p)`) is sign-fixed to the POSITIVE isotropic operator regardless of how you label neighbors. Projection only cancels divergence if div AND grad use the SAME convention as that fixed Laplacian, i.e. both must be `(larger index − smaller index)/2 = +∂/∂axis`. Internal div/grad consistency is NOT enough — there's a third operator (the Laplacian) they must agree with. If div/grad are both flipped on one axis, projection AMPLIFIES that axis's divergence → blows up aligned to the grid axes → the classic "sharp diamond/rhombus" smoke shape.

**How to apply:** For any grid pressure-projection (Stable Fluids, MAC solvers), check div, grad, and Laplacian as ONE matched sign set first. Hand-expand `div(v − ∇p) = div·v − ∇²p` to confirm they compose to zero. Treat a grid-axis-aligned diamond/blowup as a sign-mismatch signature, not a convergence problem. Central difference of `∂/∂(index)` is ALWAYS `(larger index cell − smaller index cell)`; screen up/down labels are display-only and irrelevant to correctness. See [[reference-grid-derivative-sign-convention]].
