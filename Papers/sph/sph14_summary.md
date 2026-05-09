# SPH Fluids in Computer Graphics — Chapter Summary

**원논문**: Ihmsen, Orthmann, Solenthaler, Kolb, Teschner — *"SPH Fluids in Computer Graphics"*, Eurographics 2014 STAR (State of the Art Report)

본 요약은 §2부터 §9까지를 자세히 정리한 학습용 노트입니다. 표기는 가능한 한 원논문과 일치시켰고, 식 번호는 원논문 기준입니다.

---

## §2. Neighborhood Search

SPH의 dominant cost. 매 step마다 동적으로 변하는 입자 이웃 집합을 빠르게 찾아야 함. 핵심 요구사항:
- 병렬 생성 / 병렬 query 가능
- 한 입자의 이웃 = 자기 cell + **인접 cell들** 까지 query (≠ 단일 cell)
- 도메인 sparse / non-uniform 분포에도 효율적

가변 kernel support (multi-resolution) 가 필요한 경우엔 kd-tree 같은 hierarchical 구조 ([KAD06, APKG07, SBH09]) 를 쓰지만, **고정 kernel support** 의 표준 SPH 에서는 **uniform grid** 가 사실상 합의된 선택. 빌드 O(n), query O(1).

### §2.1 Uniform Grid

- 공간을 cubic cell 로 분할. 각 입자를 한 cell 에 배치
- **cell 크기 = kernel support (예: 2h)** 일 때 3D에서 query cell = **3³ = 27개** 가 최적 [IABT11]
- 초기 입자 간격 dx 와 support 가 2h 라면 한 cell 당 ~8개 입자, 한 입자당 ~40개 이웃
- 문제: query 는 병렬화 쉬움, **grid 빌드는 race condition** 때문에 단순 병렬화 안 됨

#### §2.1.1 Index Sort

- 입자를 **cell key 기준으로 정렬** ([PDC03, Gre08, KS09])
- cell 은 자기 첫 입자의 인덱스만 저장 (참조 리스트 X) → 메모리 절감
- 같은 cell 입자는 메모리상 인접 → cache coherence ↑
- 단점: 이웃 cell 의 입자는 메모리상 멀 수도 있음

#### §2.1.2 Z-index Sort

- 같은 인덱스 정렬이지만 **Z-curve (Morton order)** 로 정렬 [GSSP10, IABT11]
- 2ⁿ 셀 블록을 재귀적으로 묶어서 정렬 → 공간 인접성을 메모리 인접성으로 보존
- bit-interleaving 으로 인덱스 빠르게 계산
- cache-hit rate 향상으로 query/process 모두 빨라짐
- 한계: 메모리 사용량이 시뮬레이션 도메인 크기에 비례 → 무한 도메인에 불리

#### §2.1.3 Hashing (Spatial Hashing)

- 무한 도메인을 **유한 hash table** 로 매핑 [THM03]
- 해시 함수: `c = ((⌊x/d⌋·p₁) ⊕ (⌊y/d⌋·p₂) ⊕ (⌊z/d⌋·p₃)) mod m`
- 큰 소수: p₁=73856093, p₂=19349663, p₃=83492791
- 충돌(서로 다른 cell이 같은 hash로) 발생 → query 느려짐. 테이블 크게 잡으면 줄어듦 (메모리 대 속도)
- 대부분의 해시 셀이 비어있어도 메모리 미리 할당 → 비효율
- 또한 **메모리상 인접 셀이 공간상 인접 아님** → cache 비효율

#### §2.1.4 Compact Hashing [IABT11]

- **사용된(non-empty) cell 만 secondary list** 에 저장
- hash cell 은 used cell의 핸들만 가짐
- 메모리 사용이 입자 수에 비례 (도메인 크기와 무관)
- 추가로 used cell 리스트를 **Z-curve 순으로 reorder** → cache-hit 향상
- 입자가 frame-to-frame 간 거의 안 움직이므로 reorder는 매 step 안 해도 됨 (1~100 step 마다 한번)

#### §2.1.5 GPUs

- 두 가지 매핑:
  - **Scatter** (rasterization 파이프라인 활용): fragment shader로 입자 기여를 텍스처 슬라이스에 blend [KLRS04, HKK07a/b]. 가속 구조 불필요. 인터랙티브 렌더링에 유리
  - **Gather** (CUDA/OpenCL/compute): 일반 GPGPU API 활용. 시뮬레이션 / 표면 재구성에 유리. **thread coherence + coherent memory access** 가 효율의 핵심
- Gather 방식의 표준: **index sort 또는 Z-index sort + parallel radix sort + stream compaction** [Gre08, GSSP10, ZGHG10, SHG09, SHZO07]
- 멀티-GPU 도메인 분할로 입자 수 늘릴 수 있음 [VBDRC12, ZSL13, RBH13]

---

## §3. Incompressibility

비압축성 보장은 SPH 시뮬 품질의 결정적 요소. 보통 0.1%~1% 밀도 오차 목표 [MM97, SP09b, ICS13]. 트레이드오프: **dt당 비용** vs **dt 크기**. 4개 클래스로 분류:

### §3.1 Non-iterative EOS Solvers (= 사용자의 SEngine 현재 방식)

State equation으로 ρ → p 를 직접 계산. 빠르지만 dt가 작아야 함.

대표 EOS 형태:
- `p = c_s² · ρ` [MM97], cs = 음속
- `p = k·(ρ - ρ₀)` [DC96, MCG03]
- `p = k·(ρ/ρ₀ - 1)` [APKG07]
- `p = k·((ρ/ρ₀)⁷ - 1)` ← Tait, weakly compressible [Mon94, BT07, ...] (가장 널리 쓰임)
- `p = k·((ρ/ρ₀)² - 1)` [ZYF10]

k가 클수록 비압축성 ↑ but 안정 dt ↓. EOS 간 비교는 문헌에 거의 없음.

알고리즘 (Alg.1, 사용자 코드와 동일):
```
for each i: find neighbors
for each i: ρᵢ = Σ mⱼ·Wᵢⱼ ; pᵢ = EOS(ρᵢ)
for each i: F^pressure, F^viscosity, F^other 계산
for each i: 시간적분 (semi-implicit Euler)
```

### §3.2 Non-iterative EOS with Splitting

비압력 force 로 먼저 v* 예측 → v* 로 입자 이동 후 ρ* 계산 → 그 ρ* 로 압력 계산.
즉, 비압력 가속도와 압력 가속도를 **분리(splitting)** 해서 두 step 으로 나눔. 압력이 비압력 결과를 알고 보정하는 셈.

§3.1 과의 비교는 문헌에서 본격적으로 안 됐지만, 반복형 splitting 이 §3.1보다 빠르므로 non-iterative splitting 도 promising 한 옵션으로 남아있음.

### §3.3 Iterative EOS Solvers with Splitting (= PCISPH, LPSPH, PBF, constraint fluids)

§3.2를 반복 적용 — 매 iteration마다 압력으로 위치/속도 갱신, 새 ρ_err 계산, 임계 ε 미만이면 종료.

특징:
- stiffness k 가 user-defined가 아니라 **자동 계산됨**
- 사용자가 직접 정하는 건 **목표 밀도 오차 ε**
- 작은 ε → 더 많은 iter, 큰 ε → 빠름

대표:
- **PCISPH** [Solenthaler & Pajarola 2009 / SP09b]
  - prototype particle 기준으로 k 사전 계산:
    `k = mᵢ²·dt²·(Σ ∇W⁰ᵢⱼ · Σ 2/ρ₀² + Σ(∇W⁰ᵢⱼ · ∇W⁰ᵢⱼ))⁻¹` (Eq. 11)
  - dt가 [BT07] 대비 최대 100배 큼. iter 3~5회 (ε=0.1%~1%). 전체 50배 speedup
- **LPSPH** (Local Poisson) [HLWW12]
  - `k = ρᵢ·rᵢ² / (ρ₀·dt²)` (Eq. 12)
  - PCISPH 대비 1.5x 빠름. 매 iter neighbor 재계산
- **PBF** (Position-Based Fluids) [Macklin & Müller 2013 / MM13]
  - 압력/압력force 누적 X. **위치 변화 Δxᵢ** 누적 (Eq. 13)
  - dt를 PCISPH보다 훨씬 크게 가능 but iter 수 더 많음. overall은 비슷
  - NVIDIA FleX 기반
- **Constraint Fluids** [BLS12]
  - 25~250x speedup 보고. iter 보통 5~15회

성능 분석은 복잡. 최적 dt가 가능한 최대 dt와 일치하지 않음 (iter 수 ↑로 상쇄). 또한 매 iter마다 neighbor search overhead.

### §3.4 Pressure Projection (Incompressible SPH, ISPH; **IISPH 포함**)

EOS 대신 **Pressure Poisson Equation (PPE)** 풀어서 압력장을 직접 결정. v* (intermediate)을 divergence-free로 projection.

PPE 형태:
- `∇²pᵢ = (ρ₀/dt) · ∇·v*` (Eq. 14, 발산 기반)
- `∇²pᵢ = (ρ₀ - ρ*) / dt²` (Eq. 15, 압축 기반)

ρ는 number density `ñᵢ = Σ Wᵢⱼ` 로 대체 가능.

#### **IISPH** (Implicit Incompressible SPH) [ICS13]

본 STAR 논문의 저자(Ihmsen)가 2013년 발표. 핵심 구조:
- 연속방정식의 이산형 + SPH 압력 force 를 PPE 의 이산형으로 결합
- **압력 force 형태가 velocity update 의 압력 force 와 동일** (다른 PPE 접근들이 Laplacian을 직접 이산화하거나 background grid 쓰는 것과 다름)
- linear system 을 **relaxed Jacobi** 로 매트릭스 없이(matrix-free) 반복 해결
- **입자당 7개 scalar 만** 저장
- 한 iter당 **2개 particle loop** 만 필요 (PCISPH는 3개)
- 밀도 오차 0.01% 까지 가능
- PCISPH 대비 압력장 계산 **최대 6x speedup**

저자 측정 (six-core desktop, single step):
| 시나리오 | 입자 수 | neighborhood | pressure | iter | dt | spacing |
|---|---|---|---|---|---|---|
| breaking dam | 20M | 8.5s | 21.5s | 10 | 0.0034s | 0.1m |
| fountain | 17M | 7.0s | 10.0s | 4 | 0.0008s | 0.0125m |
| ships | 19M | 8.0s | 24.0s | 12 | 0.017s | 0.5m |

(Compact Hashing [IABT11] + IISPH 조합)

### §3.5 Performance Comparison

전반적 우열 경향:
```
non-iter EOS (Alg.1)  <  iterative EOS (PCISPH, PBF)  <  pressure projection (IISPH)
```
하지만 시나리오 의존성 큼:
- 입자 1개라면 non-iter EOS 가 가장 빠름 (iter solver 는 최소 iter 수가 있음)
- shallow water 같이 압력이 크게 발산할 일 없는 경우 non-iter EOS도 유용
- **breaking dam 같이 깊은 fluid + 큰 압력 변화** 시나리오에서 IISPH가 압도적

추가 미해결 이슈:
- 최적 dt를 자동으로 찾는 방법은 없음 (시행착오)
- non-iter vs iter 비교 시 같은 ε(밀도 오차) 기준으로 맞추는 게 어려움

---

## §4. Boundary Handling

고체 경계와의 상호작용. 표준 접근은 **boundary particle 로 솔리드 sampling**.

### Distance-based Penalty Force

- Lennard-Jones force 등 거리의 다항식 [Mon94, Mon05, MK09]
- stiffness 튜닝 어려움. 큰 압력 변동 → dt 제약 ↑
- 압축성 SPH + deformable mesh 조합에도 적용됐지만 성능 한계 [MST04, LAD08]

### Direct Forcing [BTT09]

- 1-way / 2-way 결합. predictor-corrector
- slip condition 모델링 가능. 비관통 보장
- penalty 기반 대비 큰 dt 가능

### Boundary Particle 접근

거리 함수 보간 대신 boundary 자체를 입자로 sampling:
- pre-sample [KAD06, SSP07, FG07, IAGT10, SB12, AIS12]
- on-the-fly sample [HA06]

전략:
- **fluid처럼 처리**: boundary 입자도 자체 ρ, p 가짐. solid의 속도로 advect [MM97, SSP07, IAGT10]
- **mirroring**: boundary 입자가 인접 fluid 입자의 quantities 를 미러링 (예: 같은 압력) [HA06, MM97, SB12, AIS12]

문제: 경계 근처 fluid는 **support 영역에 이웃 부족 (particle deficiency)** → 밀도 추정 정확도 ↓
- **fluid가 경계에 들러붙는 sticking 현상** 발생
- [HKK07b]는 wall-weight function 으로 부분 해결, but 경계 밀도 분포 irregular

### Akinci 2012 [AIS12] (=현재 표준에 가까움)

- 경계 입자의 **기여도(contribution)를 동적으로 계산** → inhomogeneous sampling 도 처리
- single-layer 경계로도 충분 → 메모리/얇은 물체 유리
- 비관통 보장
- **밀도 비 1000:1 까지 two-way coupling** 검증 (수면-공기 같은 큰 차이도)
- IISPH 등 어떤 SPH 와도 결합 가능
- 후속 [ACAT13] 에서 deformable solid / cloth 까지 확장
- [OHB13]에서 transport (입자 간 quantity 교환) 까지 모델링

---

## §5. Adaptivity

해상도 ↑ → 표면 디테일 ↑, 수치 dissipation ↓. 그러나 균일하게 늘리면 비용 폭증. 시각적으로 중요한 영역에만 자원 투입하는 adaptive 기법.

### §5.1 Adaptive Spatial Discretization

#### §5.1.1 Dynamic Particle Refinement

- 큰 입자를 **여러 개 작은 child 로 split**, 반대로 **merge** 하는 방식
- 전역 [CPK02] 또는 지역 [FB07]
- 변동 smoothing radius 가 quantity 재현 어렵게 함 [BOT01]
  - 인접 level 간 indirect interaction [KAD06]
  - 여러 layer 에 걸친 점진적 radius 변화 [APKG07]
- non-adaptive 와 동일 dt 유지하려면 **resolution level 간 temporal blending** 필요 [OK12]
- Alg. 5: split/merge 영역 식별 → blend weight로 부드러운 전환

#### §5.1.2 Multi-scale Methods (= Coupled Simulations)

- **여러 해상도 level 을 별도 시뮬레이션 + 결합**
- 각 level 의 입자 반지름 fix
- 기본 [SG11]:
  - L (low) = 전체 fluid coarse 시뮬
  - H (high) = 일부 영역만 fine 시뮬, **boundary particle** 로 L과 연결
  - boundary 입자는 H의 물리에 기여 + L의 가장 가까운 parent 입자 속도로 advect
  - L → H feedback force
- 확장 [HS13]: 다중 level 지원, 총 질량 보존
- Single-scale 대비 **3~12x speedup** (visually interesting subset 크기에 의존)

### §5.2 Adaptive Time Discretization

#### §5.2.1 Globally Adaptive

- CFL 기준으로 dt 자동 조절 [DC96, IAGT10, GP11]
- iterative EOS 방식엔 까다로움 (F_max, v_max 사전에 모름)
- [IAGT10]: dt 변화량을 한 step당 0.2% 로 제한 → shock 처리는 별도 메커니즘 필요

#### §5.2.2 Locally Adaptive

- **입자별로 다른 dt** [DC96, DC99]. action-reaction 대칭성 깨지는 비용
- [DC96]: 안정성 한도 내 최대 dt, 입자 상태는 t 간격으로 동기화
- [GP11]: 거의 정적인 영역 입자를 **deactivate** (속도/표면거리 임계 넘으면 reactivate)
  - 백만 입자 시나리오에서 non-iter EOS 대비 6.7x speedup

---

## §6. Multiphase Fluids

여러 fluid 가 공존 (액-액, 액-기). SPH의 입자 표현은 mesh 기반(Eulerian) 대비 **interface 가 자연히 sharp**한 이점.

### §6.1 Liquid-Liquid Interface

- 작은 밀도비 (≤10) 는 standard SPH로 OK — 입자 mass와 ρ₀를 fluid type 마다 다르게:
  `mₐ/ρ₀ₐ = mᵦ/ρ₀ᵦ` (rest volume 동일)
- 큰 밀도비 (>10) 는 spurious tension / interface gap 발생 [Hoo98, AMS07]

해결책 [TM05b, HA06, SP08]:
- **Number density** `ñᵢ = Σ Wᵢⱼ` 사용 → ρ̃ᵢ = mᵢ·ñᵢ
- 다른 fluid 입자의 **mass 가 ρ̃ᵢ 에 영향 안 줌**, 기하학적 W 기여만 받음
- 압력 force도 수정 (Eq. 16):
  `Fᵢ^pressure = -Σⱼ (p̃ⱼ/ρ̃ⱼ² + p̃ᵢ/ρ̃ᵢ²) · ∇Wᵢⱼ`
- single-fluid 케이스에선 standard 식과 동일

대안: 연속방정식으로 ρ 진화 [OS03, MR13]. dt 작아야 하고 적분 오차로 mass drift 가능 → graphics 에서는 보통 summation 식 선호.

### §6.2 Liquid-Air Interface

액-액보다 어려움 (공기 입자 부족 → density deficiency). 표면 근처에만 air particle 생성 [MSKG05, IBAT11, SB12].

- **One-way coupling** [SB12]: air 는 fluid 의 ρ 추정에만 기여, mass·velocity 는 보간된 값. ρ_air = 0 이지만 fluid에 작용은 안 함
- **Two-way coupling** [MSKG05, IBAT11]: 동적으로 air particle 생성/삭제. [IBAT11]은 압력장 대신 **속도장 결합** → drag/lift 효과를 큰 dt로 안정 시뮬
- 작은 부력 air bubble은 정지상태 도달 후 죽기 쉬움 → 인공 buoyancy force 추가

### §6.3 Surface Tension

#### §6.3.1 Macroscopic (CSF, Continuum Surface Force) [BKZ92]

- color field c (phase 간 jump 있음) 정의
- normal: `n = ∇c` (fluid 안쪽 향함). 다양한 형태로 계산 [MCG03, GAC09, SP08]
- 곡률: `κ = -∇²c`
- Tension force ∝ κ·n̂. 작은 normal magnitude 는 무시 (interface 만 작용시키려)
- 다중 액의 interface 와 free surface 구분하려면 c 정규화 필요 [SP08]
- 대안: stress tensor 의 발산 형태 [HA06, GAC09]

문제: SPH로 Laplacian 추정 → 입자 disorder 에 매우 민감. [YWTY12] 는 재구성된 mesh 로 곡률 → 정확도↑ but 매 step mesh 만들어야 함.

#### §6.3.2 Microscopic (Cohesion Forces)

- 분자 인력을 모방한 입자 간 attractive force [NP00, TM05a, BT07]
- 곡률(2차 미분) 회피 → 입자 disorder에 robust
- [NP00]: Van der Waals EOS, 인력 범위 4h
- [BT07]: 거리 h 내 인력. `F_surface = -Σ mⱼ · xᵢⱼ · Wᵢⱼ`
- [TM05a, AAT13]: short-range repulsion + long-range attraction 결합

[AAT13]: **macroscopic + microscopic 결합** + adhesion(젖음) force → 큰 surface tension, no clustering, no momentum dissipation 동시 달성.

---

## §7. Surface Reconstruction & Rendering

입자 ↔ smooth 표면 사이의 변환. 대부분 **scalar field → polygonization**, 또는 직접 렌더링.

### §7.1 3D Scalar Field

- 표면 = scalar field `φ` 의 zero level set (φ=0)
- **Marching Cubes** [LC87] 로 polygonization. cell size = 입자 간격의 0.5배 권장 [AIAT12]

φ 정의 방식:
- **Blobbies / metaballs** [Bli82]: 각 입자를 Gaussian potential로. 단순. 입자 분포 irregular 시 표면 bumpy
- **Weighted average position** `x̄` [ZB05, SSP07, ...]:
  `φ(x) = |x - x̄| - r`, r = 입자 반지름
  - 영향 반지름 4·입자간격 [AIAT12] 시 최적
  - 큰 영향 반지름은 concave 영역 artifact ↑ → eigenvalue analysis [SSP07] 또는 position-based decay [OCD13]
- **Adaptive 입자** 위해 표면거리 추적 [APKG07]
- **Anisotropic kernels** (PCA로 입자 이웃 분석) [YT10] → sharp feature/edge 재구성에 유리. Laplacian smoothing pass 1회 추가
- thin-plate energy 최적화 [BGB11]

### §7.2 Explicit Methods & Direct Rendering

- 매 frame mesh 새로 만들지 않고 **mesh를 advect** [PTB03, DC98, YWTY12]
  - [YWTY12]: 초기 mesh + 입자 속도로 정점 이동 + 주기적으로 implicit function 으로 재투영 (drift 방지)
- **직접 ray-isosurface 교차** [KSN08, ZSP08, GSSP10] 또는 sphere 교차 [GIK07]
- **Point splatting** [ZPvBG01]: isotropic [MCG03] 또는 anisotropic [MM13] kernel

### §7.3 Screen-Space Approaches (실시간용 표준)

세부 다른데 공통 골격: **3D point cloud → screen-space depth map → smooth → 표면 처리**.

- [MHHR07]: depth map binomial filter → marching squares
- **[vdLGS09] / [Gre10] / [BSW10] (대표)**:
  1. 각 입자를 sphere 로 depth 렌더
  2. depth에 smoothing filter 적용 → continuous surface
  3. depth → normal → lighting/refraction
- [Gre10]: Gaussian + bilateral (silhouette 보존). separated filter 로 가속 (artifact 트레이드오프)
- [vdLGS09]: **Curvature flow smoothing** — 입자 간 curvature 변화를 직접 smooth
- [BSW10]: 카메라 거리에 무관한 smoothing

> **사용자의 SEngine에서 "물처럼 보이는 렌더링"의 표준 경로가 §7.3.** Figure 1 같은 결과는 거의 모두 이 파이프라인.

### §7.4 Volume Rendering

- 입자 quantity field 를 participating medium 으로 보고 **emission-absorption 적분** [EHK06, HLSR08]
- Hybrid splat-slice [vdLGS09, FGE10, FAW10]: 입자 기여를 view-aligned [NMM06] 또는 axis-aligned [SP09a] 텍스처 슬라이스에 scatter
- Ray casting / tracing [SSP07, GSSP10, OKK10, JFSP10, IAAT12]: 공간 분할 (kd-tree, octree) 로 메모리 코히어런스. 큰 입자 수의 인터랙티브는 어려움 → offline용 정도

---

## §8. Secondary Simulation (Foam, Spray, Bubbles)

거센 흐름의 작은 디테일 (foam, spray, air bubble). 메인 fluid 위에 **2차 시뮬레이션** 으로 추가.

- 단순 multi-phase는 dt 제약 + 비용 ↑
- 따라서 implicit 모델 선호: 공기-물 mixture를 **휴리스틱 + 인공 buoyancy** 로 표현

대표 [IAAT12]:
- 메인 fluid 위에 **secondary simulation** (pre-computed fluid 위 또는 인터랙티브 위)
- foam/spray/bubble 분류 기준:
  - **air trap potential** (입자가 공기 가두는 정도)
  - **wave crest likelihood** (파도 마루에 있을 가능성)
  - **kinetic energy**
- 위치에 따라 foam / spray / bubble 분류 → fluid가 diffuse 입자에 미치는 영향 다름
- diffuse 간 force 계산 X → diffuse-diffuse neighbor set 불필요 → frame rate 수준의 큰 dt 가능
- 20초·15M diffuse particle 영상이 desktop CPU 2시간 (저자 측정)
- [MM13]: [IAAT12] 모델을 PBF 인터랙티브 시뮬에 통합

대안 [BSW10]: Weber number 임계로 입자를 water/foam 으로 분류. 입자 수가 baseline 그대로라 디테일 한계.

---

## §9. Conclusions and Future Development

### 도달한 지점
- desktop 에서 수백만~수천만 입자 시뮬 가능 (offline)
- IISPH + Compact Hashing 조합이 high-quality 시뮬의 대표
- 비압축성, 효율, multi-phase, 적응적 해상도, surface 재구성 모두 큰 진전

### 남은 과제

1. **최적 dt 자동 결정** — 가능한 최대 dt가 최적 overall performance 와 같지 않음 (iter 수와의 trade-off). 시행착오 외 자동화 부재.

2. **실시간 시뮬의 dissipation** — 거친 해상도(coarse)는 큰 support radius 가 viscosity 를 과도하게 평균화 → splash·표면 디테일 사라짐. viscosity 식 개선 필요.

3. **Liquid-air interface** — particle deficiency 문제. 기존 해결책 비용 ↑, complexity ↑. 메모리·성능·simplicity 동시 만족하는 더 나은 방식 필요.

4. **자동 파라미터 튜닝** — 모든 SPH 변형이 tedious 한 파라미터 튜닝 요구. artist control 은 좋지만 자동화도 필요.

---

## Appendix A: Lagrangian vs Eulerian (요지)

- **Lagrangian (= SPH)**: sample point xᵢ 가 fluid와 함께 이동 (dxᵢ/dt = vᵢ)
- **Eulerian (= grid)**: sample point xᵢ 가 공간에 고정

Navier-Stokes 의 material derivative:
- Lagrangian: `Dvᵢ/Dt = dvᵢ/dt`
- Eulerian: `Dvᵢ/Dt = ∂vᵢ/∂t + vᵢ·∇vᵢ` (convective term 별도)

따라서:
- Lagrangian Navier-Stokes: `dvᵢ/dt = -(1/ρᵢ)∇pᵢ + ν∇²vᵢ + Fᵢ^other / mᵢ` (Eq. 18)
- Eulerian: 같은 식 + `-vᵢ·∇vᵢ` (Eq. 19)

흔한 오해 정정:
- "Lagrangian 이 비싸다 (neighbor search 때문)" — Eulerian 은 convective term 처리가 overhead. FLIP 의 입자가 이걸 처리하기 위한 장치
- "Lagrangian 은 압축성, Eulerian 은 비압축성" — 일반화는 부정확. graphics SPH 가 대체로 압축적인 건 사실이지만 IISPH 처럼 incompressible Lagrangian 도 존재. 반대로 Eulerian compressible 도 가능
- 압축성: 밀도 진동(volume change due to ρ fluctuation), 비압축성: mass 손실/획득(volume change due to mass change). 둘 다 volume 변화 issue 있음

---

## 사용자 프로젝트(SEngine)와의 매핑

| 본 논문 §  | SEngine 현재 상태 | 다음 단계 후보 |
|---|---|---|
| §3.1 (non-iter EOS) | ✓ Tait `(ρ/ρ₀)⁷` 사용 중, k=1 | 그대로 유지하면서 §2 가속 |
| §2.1 Uniform Grid | ✗ O(N²) all-pairs | **Index Sort + parallel radix sort 도입 (가장 큰 효과)** |
| §3.4 IISPH | ✗ | 백만 입자급 가려면 §3.3 PBF 또는 §3.4 IISPH 로 전환 |
| §4 Akinci 2012 | ✗ wall clamping 만 | sticking / boundary deficiency 해결시 도입 |
| §7.3 Screen Space | ✗ point billboard | 물처럼 보이려면 필수 |
| §6.3 Surface Tension | ✗ | 작은 fluid drop 표현시 |

→ **권장 우선순위: §2(spatial grid) → §7.3(SSFR) → §3.4(IISPH) → §4(Akinci boundary)**

---

*본 요약은 학습용으로 원논문의 핵심만 정리한 것입니다. 구현 디테일은 원 reference 들 ([ICS13], [SP09b], [MM13], [IABT11], [vdLGS09], [AIS12] 등) 을 직접 참조하세요.*
