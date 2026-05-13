---
name: SPH papers and reference implementations
description: Where to look up SPH algorithms / verify implementations for SEngine's fluid work
type: reference
originSessionId: c5d4498c-1c2b-424f-a858-89376e24a707
---
## Local papers (d:/Users/son/SEngine/Papers/sph/)

- **sph14.pdf** — Ihmsen, Orthmann, Solenthaler, Kolb, Teschner 2014, *"SPH Fluids in Computer Graphics"*, Eurographics STAR
  - Definitive overview through 2014. Covers neighborhood search (§2), incompressibility (§3), boundary (§4), adaptivity (§5), multiphase (§6), surface/rendering (§7), foam/spray (§8)
  - Figure 1 = breaking dam, 20M particles — user's visual reference target

- **sph22.pdf** — Koschier, Bender, Solenthaler, Teschner 2022, *"A Survey on SPH Methods in Computer Graphics"*, CGF
  - Direct successor to sph14 (8 years of advances)
  - Key new content: DFSPH, equivalence theorem (IISPH ≡ PCISPH ≡ PBF ≡ DFSPH), Volume Maps boundary, implicit viscosity, deformable solids, micropolar fluids, ML/data-driven approaches
  - **Defers** neighborhood search / surface reconstruction / rendering to sph14 — sph14 is still the reference for those topics

- sph03.pdf, sph07.pdf, sph11.pdf — older references (not yet read)

## Local notes

- **Papers/sph/sph14_summary.md** — Korean chapter-by-chapter summary of sph14, written 2026-05-09 with mapping to SEngine project state at the bottom

## Reference implementation

- **SPlisHSPlasH** — github.com/InteractiveComputerGraphics/SPlisHSPlasH
  - Bender et al.'s ongoing open-source C++ impl of: DFSPH, IISPH, PBF, PCISPH, WCSPH, Volume Maps, Density Maps, implicit viscosity (Weiler 2018), deformable solids (Peer 2017, Kugelstadt 2021)
  - Use as ground-truth baseline when implementing any of the above
  - When user asks "is X correct?", cross-check against this if possible

## Key external papers (post-2014, not in local folder)

When recommending algorithms beyond sph14, the canonical references are:

- **DFSPH**: Bender & Koschier 2015 (SCA) and 2017 (TVCG) — "Divergence-Free Smoothed Particle Hydrodynamics"
- **PBF**: Macklin & Müller 2013 — "Position Based Fluids"
- **IISPH**: Ihmsen et al. 2014 (TVCG) — "Implicit Incompressible SPH"
- **Volume Maps**: Bender et al. 2020 (TVCG) — "Implicit Frictional Boundary Handling for SPH"
- **Akinci boundary** (sph14 era): Akinci et al. 2012 (SIGGRAPH) — "Versatile Rigid-Fluid Coupling"

## Rendering (sph14 Figure 1 look)

- **Screen-Space Fluid Rendering**: van der Laan, Green, Sainz 2009 — "Screen Space Fluid Rendering with Curvature Flow"
- **NVIDIA Fluid Sample / Green 2010 talk**: simpler SSFR pipeline (depth → bilateral filter → normal → composite), often cited reference impl
