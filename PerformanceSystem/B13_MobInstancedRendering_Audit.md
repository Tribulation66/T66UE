# B.13 Mob Instanced Rendering Audit

Status: CLOSED - NO-LAND.

This is the authoritative consolidation of the B.13 mob instanced-rendering
investigation. It supersedes the scattered B.13 sections in the Lightweight
Actor plan and the B.13 combined packet for decision purposes. Those documents
remain valid historical evidence; this file is the single source of truth for
the B.13 outcome and the recorded user decision.

- Owning plan: `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`
- Full evidence packet: `Reports/AgentReviews/20260529_B13_MobHISM_VAT/combined_packet.md`
- Closeout artifacts: `Reports/AgentReviews/20260529_B13_NoLand_Closeout/`
- Preserved raw capture/proof evidence: `Reports/AgentReviews/20260529_B13_NoLand_Closeout/preserved_worktree_evidence/`

## Recorded User Decision

- B.13 is closed as a no-land.
- Instanced rendering (HISM and ISM) empirically regresses full-resolution
  performance for constantly-moving VAT mobs in UE 5.7. Every variant tested
  lost to the existing per-mob static-mesh renderer.
- The per-mob static-mesh-component renderer is the chosen, deliberate renderer
  for basic mobs. It is not a placeholder awaiting instancing.
- A GPU-driven crowd renderer - manager-owned position/frame data in a buffer or
  texture, read in the vertex shader while bypassing UE component instance
  transforms - is the only direction that could plausibly beat the current path.
  It is deferred and should be revisited only if enemy counts exceed the design
  ceiling and rendering becomes the measured bottleneck.

## Attempted Working Goal

Replace the lightweight basic-mob per-actor static-mesh visual with instanced
rendering (HISM, then ISM) driven by per-instance VAT custom data, so that one
draw call could cover many basic mobs while still animating each instance's VAT
frame independently.

This was step 10 (B.13) of the Lightweight Actor plan, the last large
performance pass after B.11 moved VAT state to the manager and B.12 disabled
per-mob actor tick.

## Staged De-Risk Approach

B.13 was treated as a de-risk gate, not a direct live change:

- All work was done in an isolated implementation tree at
  `C:\UE\T66_B13_Worktree` (a robocopy of the live repo, not a git worktree),
  so the dirty live repo never received a regressing renderer.
- The pass created an isolated instanced VAT material variant
  (`/Game/Materials/M_EasyMobVAT_Unlit_UV2_Instanced`) and kept rich actors on
  their existing MID-driven material path.
- Correctness was proven first (VAT deformation via per-instance custom data,
  independent per-instance frames, hidden per-mob meshes while instanced).
- Each candidate was measured against a full-resolution before baseline using
  the same `enemywaveperf` saturation contract.
- Live runtime source was updated only if a candidate beat the baseline. None
  did, so nothing was copied back.

The active lightweight VAT material is `/Game/Materials/M_EasyMobVAT_Unlit_UV2`.
The prompt-era reference to `M_Character_Unlit` was incorrect and is not the
active material for these mobs.

## Empirical Before Baseline

Full-resolution (1920x1080, `r.ScreenPercentage=100`), `HeroHPOverride=20000`,
`enemywaveperf` saturation, no PerformanceSystem overhead rejects.

| Metric | Value |
| --- | ---: |
| Staged SHA256 | `B5C226E1870168430F1FDFCAC91D135C77735999D18666B110DA75B626E78BF8` |
| Accepted captures | 3/3 |
| Median FPS | **189.65** |
| Mean FPS | 187.52 |
| Stdev FPS | 4.58 |
| Median 1% low | **156.16** |
| Median 0.1% low | **72.03** |
| Max PerformanceSystem overhead | 1237.8 us |

Raw: `preserved_worktree_evidence/Before_capture_results.json`.

## Correctness Proofs (Passed)

`Pass` means correctness only: VAT deformation, independent per-instance frames,
and hidden per-mob meshes behaved correctly. It does not mean performance
acceptance; the same binary could pass correctness and still be rejected on FPS.

| Proof | Result |
| --- | --- |
| Stage 1 Slime-only HISM (frame-only sparse custom data) | Pass: `Buckets=1`, `ActiveHISMStates=4`, `FallbackVATStates=56`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0` |
| Full HISM rollout (all basic VAT rows) | Pass: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0` |
| Full ISM rollout (all basic VAT rows) | Pass: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0` |
| Spatial-cell Slime-only | Pass: `Buckets=1`, `ActiveHISMStates=10`, `FallbackVATStates=50`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `CustomDataFloats=4`, `CellSize=2000.0` |
| Spatial-cell all-family | Pass: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `CustomDataFloats=4`, `CellSize=2000.0` |

Proof logs preserved under `preserved_worktree_evidence/T66_B13_*Proof*.log`.

## Candidate Performance Table

All rows ran the same full-resolution `enemywaveperf` saturation contract as the
`189.65` before baseline. Staged hashes differ per row because each candidate
rebuilt the renderer under test. `3/3` rows are full acceptance sets; `1/1`
rows are single rejection probes (a single sub-baseline row was enough to reject
the branch without spending a full set); `0/1` is a crash.

| # | Candidate | Hash (SHA256) | Accepted | Median FPS | Median 1% low | Median 0.1% low | Result |
| --- | --- | --- | ---: | ---: | ---: | ---: | --- |
| 1 | HISM, four-slot world-offset custom data | `3D1852F6A4AB214A637DD3FA7E661AF01A79D0D8466133727D9C672D3DD85990` | 3/3 | 137.43 | 77.02 | 41.98 | Rejected |
| 2 | HISM, world transforms + offset-capable material | `B99F5733620D8421300B64ED755E47765E268F633927D33DF202913B4BBD6D75` | 3/3 | 116.89 | 73.76 | 64.73 | Rejected |
| 3 | HISM, frame-only material, per-frame custom writes | `502A9A0969B373E34231317B5CB2BB609D0246FB1F86A2E773E0028557C27B5A` | 2/3 | 161.39 | 113.61 | 53.88 | Rejected |
| 4 | HISM, frame-only material, sparse custom writes | `523293239A13BC3683579FDBD334981D3EFC8678E312CA110E1481184CDD981B` | 3/3 | 172.64 | 139.61 | 68.98 | Rejected (best plain HISM) |
| 5 | ISM feasibility, frame-only material, sparse custom writes | `4268CE0FB64B53C0A4F1C5FF4C041236E1D80E79D9A79B9E6C681AF29108FAFA` | 3/3 | 176.42 | 151.14 | 67.12 | Rejected (best plain ISM; beats HISM, still below before) |
| 6 | HISM + VisualMesh unregister | `F561071FE925911B911124BF5F68DA969E4C834C8D97A91B26BF88AED4A98C2F` | 0/1 | n/a | n/a | n/a | Rejected: crashed in `FPrimitiveSceneInfo::UpdateComponentLastRenderTime` |
| 7 | HISM + safe VisualMesh render suppression (render-flag, no unregister) | `FD6A427D442BF015E2F7556A565D84EA749E012ABE9226621B497A2BAABC55A4` | 3/3 | 176.83 | 150.36 | 75.33 | Rejected: stale mesh render flags were not the cost center |
| 8 | HISM hidden pool at origin (transform-cache adjacent) | `8D0C9FAC2A978BD1E1E3736858420314B3B46EFE8AC46D067A73FE04F8A928B8` | 3/3 | 169.16 | 143.11 | 56.47 | Rejected: worse than offscreen hidden pool |
| 9 | HISM transform-change cache | `604642337D71835F44990C3BFB94A2647939FFB1A7A552538D75E446CB860841` | 3/3 | 171.53 | 119.51 | 54.62 | Rejected: duplicate same-transform submissions were not the cost center |
| 10 | HISM spatial-cell, 2000 uu cells, four custom floats | `4CAE3738260A946E33A4B4ECFECBADCCDE54A6E8E17BCDBF361F4B8805345292` | 3/3 | 80.42 | 44.67 | 38.98 | Rejected: large cell offsets damaged culling/bounds + shader cost |
| 11 | HISM spatial-cell, 500 uu cells | `C0D8E542693EF644F3BEE89593E186E11CA657CC0B82D5638CA9C4AD42E95F53` | 1/1 probe | 167.21 | 94.64 | 62.28 | Rejected: improved over 2000 uu, still below baseline |
| 12 | HISM spatial-cell 500 uu, shadows/decals disabled (render-flag) | `FD487A2CB24AEFE1C874F212F2377BA8C6D28F3783F8F423A255C55A6101C6DA` | 1/1 probe | 177.31 | 142.78 | 68.46 | Rejected: render flags helped but did not close gap |
| 13 | HISM spatial-cell 500 uu, no shadows/decals, lazy pooled allocation | `479760F29BE932D2C86413B078E01CD1F92FDCF46452F3FA6BA7B777132BD39F` | 1/1 probe | 176.01 | 138.49 | 66.08 | Rejected: reduced preallocation did not recover gap |
| 14 | HISM spatial-cell 500 uu, no shadows/decals, lazy pool, actor mesh nulled on takeover (GPU/render-flag final probe) | `949DD53D44D697DF6BDF5381031D6B7B84991F533C55611AB3F83F54B8068B32` | 1/1 probe | 179.36 | 157.71 | 139.87 | Rejected: best final probe for lows; median still `-10.30` FPS vs `189.65` baseline |

Best result of the entire investigation: candidate 14 at `179.36` median FPS,
still `10.30` FPS below the `189.65` baseline. No variant matched or beat the
existing per-mob static-mesh renderer.

## Root Cause

- The VAT-via-per-instance-custom-data technique is functionally correct. Frame
  selection, independent per-instance frames, and hidden actor meshes all work.
- The renderer regresses because the instances are not static. At saturation
  roughly 90 basic mobs move every frame, so every frame the renderer must take
  per-instance transform updates for all of them.
- HISM is the worst fit: per-frame instance transform updates plus hierarchy
  tree management cost more than the draw-call reduction saves. The hierarchy
  rebuild/update path is the dominant added cost.
- Plain ISM removes the hierarchy cost and is consistently better than HISM, but
  it still loses to the existing per-mob static-mesh renderer.
- The material-side world-offset strategy (move instance transforms to origin,
  carry position in custom data) was the worst of all: it added per-vertex
  custom-data reads and defeated useful instance spatial behavior (culling,
  bounds), badly regressing FPS.
- Eliminated non-causes, each proven by a dedicated candidate: stale per-mob
  mesh visibility (candidate 7), hidden-pool placement (candidate 8), duplicate
  same-transform submissions (candidate 9), render flags / shadows / decals
  (candidates 12-14). None were the cost center.

Net: the implementation is functionally correct but performance-negative for
constantly-moving mobs.

## UE 5.7 Engine-Source Audit

The per-instance transform update cost for moving HISM instances cannot be
disabled at the engine level in UE 5.7:

- `UHierarchicalInstancedStaticMeshComponent::BatchUpdateInstancesTransformsInternal`
  loops through `UpdateInstanceTransform` for each instance.
- `UpdateInstanceTransform` marks `bIsOutOfDate=true` and, for location changes,
  calls `BuildTreeIfOutdated(/*Async*/true, /*ForceUpdate*/false)`.
- Therefore setting `bAutoRebuildTreeOnInstanceChanges=false` does not remove
  per-frame hierarchy/tree update cost for moving instances. The tree update is
  reached through the transform-update path itself, not only through the auto
  rebuild flag.

This confirms HISM tree rebuild/update cost is unavoidable for constantly-moving
instances in this engine version, which is why HISM was structurally the wrong
primitive here.

## Final Decision

- **B.13 is a no-land.** No instanced mob renderer was copied into live source.
- The per-mob static-mesh-component renderer is the chosen, authoritative live
  renderer for basic mobs - a deliberate decision, not a placeholder.
- The reviewed de-risk gate worked exactly as intended: it proved the VAT
  custom-data technique and HISM/ISM visual correctness while preventing a
  slower renderer from entering live runtime source.
- **GPU-crowd escape hatch (deferred):** the only direction that could plausibly
  beat the current path is a GPU-driven crowd renderer with manager-owned
  position/frame data in a structured buffer or texture, read in the vertex
  shader, bypassing UE component instance transforms entirely. Revisit only if
  enemy counts exceed the design ceiling and rendering becomes the measured
  bottleneck. It requires a new reviewed packet (provisionally "B.13R").

## Live Repo State (verified at closeout, 2026-05-29)

- No instanced mob renderer in live source. `UT66MobManagerSubsystem` creates no
  HISM/ISM component; `AT66MobBase` renders through a per-mob
  `UStaticMeshComponent VisualMesh`.
- The only B.13 reference in live source is a forward-looking comment on the
  B.11/B.12 manager VAT custom-data layout
  (`Source/T66/Gameplay/T66MobManagerSubsystem.h`): the flat `CustomData*`
  fields were laid out for a future B.13 to consume. No renderer consumes them
  in an instanced path; the per-mob material does.
- The B.13-created instanced material `M_EasyMobVAT_Unlit_UV2_Instanced` does not
  exist anywhere in live `Content/`. Only the base `M_EasyMobVAT_Unlit_UV2`
  exists. No Pass D orphan to remove.
- Mob renderer source files are not part of the user's current dirty working set.
- The camera-angle invisibility bug observed during the worktree experiments
  cannot be present in live, because no HISM renderer landed.

See `Reports/AgentReviews/20260529_B13_NoLand_Closeout/operator_completion.md`
for exact verification commands and results.
