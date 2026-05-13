---
name: No LaTeX math in responses
description: User's Claude Code UI does not render LaTeX ($$...$$ or $...$); use Unicode/ASCII math instead
type: feedback
originSessionId: c5d4498c-1c2b-424f-a858-89376e24a707
---
Do NOT use LaTeX delimiters (`$...$` or `$$...$$`) in responses. The user's Claude Code UI renders CommonMark only, so LaTeX blocks show up as raw `$$` text and the math is invisible.

**Why:** The user has fed this back at least twice. Repeating it is a real cost — they have to re-explain and the response loses its main content.

**How to apply:** For math expressions, use:
- Unicode symbols: ρ, ∑, ∫, ∇, Δ, ², ³, ½, ≈, ≤, ≥, π, μ, etc.
- Code fences for block equations (multi-line, aligned)
- Inline backticks for short formulas: `ρ = Σ m_j · W(r_ij, h)`
- Plain text with Unicode subscripts where helpful: x_i, r_ij

When tempted to write `$$\rho = \sum_j m_j W_{ij}$$`, instead write:
```
ρ = Σⱼ mⱼ · W(rᵢⱼ, h)
```
or just inline: `ρ ≈ 1/dx²`.

This applies to ALL responses regardless of topic (math, physics, ML, etc.).
