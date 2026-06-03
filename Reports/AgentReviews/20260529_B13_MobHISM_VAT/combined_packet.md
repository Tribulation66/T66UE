# B.13 Mob HISM Rendering With VAT - Combined Packet

## Review Status

- Working goal: replace lightweight basic-mob per-actor visual mesh rendering with instanced rendering driven by per-instance VAT custom data, staged through one-mob proof before all-family rollout.
- Plan packet: `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\plan_packet.md`
- Claude review: `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\20260528T222533-pass1\claude_review_pass1.md`
- Verdict: `APPROVE`
- Implementation sandbox: `C:\UE\T66_B13_Worktree`
- Live runtime source copied back: no. The de-risk pass found a performance regression, so the live repo was not given the regressing renderer.

## Material And Rendering Audit

- Actual lightweight VAT material: `/Game/Materials/M_EasyMobVAT_Unlit_UV2`
- Prompt-mentioned `M_Character_Unlit` is not the active lightweight VAT material for these mobs.
- Rich actors remain on their existing MID-driven material path.
- Created isolated instanced variant: `/Game/Materials/M_EasyMobVAT_Unlit_UV2_Instanced`
- Final viable correctness variant used one per-instance custom-data float:
  - Slot 0: `CustomDataFrame`
- The attempted four-slot world-offset variant was rejected because it worsened performance.

## Stage 1 Proof

Stage 1 proved the core VAT-via-custom-data technique on one mob type.

`Pass` in the proof tables means correctness-only proof: VAT deformation, independent per-instance frames, and hidden actor meshes behaved correctly. It does not mean performance acceptance; the same binary can pass correctness proof and still be rejected by the full-resolution FPS table below.

| Variant | Hash | Result | Evidence |
| --- | --- | --- | --- |
| HISM frame-only sparse custom-data, Slime only | `523293239A13BC3683579FDBD334981D3EFC8678E312CA110E1481184CDD981B` | Pass | `T66_B13_Stage1Slime_HISMProof_frameonly_sparsecustom.log`: `Buckets=1`, `ActiveHISMStates=4`, `FallbackVATStates=56`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `Pass=1` |

## Full Rollout Proof

| Variant | Hash | Result | Evidence |
| --- | --- | --- | --- |
| HISM frame-only sparse custom-data, all basic VAT rows | `523293239A13BC3683579FDBD334981D3EFC8678E312CA110E1481184CDD981B` | Pass | `T66_B13_Full_HISMProof_frameonly_sparsecustom.log`: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `Pass=1` |
| ISM feasibility variant, all basic VAT rows | `4268CE0FB64B53C0A4F1C5FF4C041236E1D80E79D9A79B9E6C681AF29108FAFA` | Pass | `T66_B13_Full_ISMProof_frameonly_sparsecustom.log`: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `Pass=1` |

## Measurement

All captures were full-resolution 1920x1080 with `r.ScreenPercentage=100`, `HeroHPOverride=20000`, stable binary hashes within each set, and no PerformanceSystem overhead rejects. Baseline and candidates ran the same `enemywaveperf` scene/saturation contract in the same isolated source tree; the staged binary hash changed only because each rejected candidate rebuilt the renderer under test.

### Before Baseline

| Metric | Value |
| --- | ---: |
| Staged SHA256 | `B5C226E1870168430F1FDFCAC91D135C77735999D18666B110DA75B626E78BF8` |
| Accepted captures | 3/3 |
| Median FPS | 189.65 |
| Mean FPS | 187.52 |
| Stdev FPS | 4.58 |
| Median 1% low | 156.16 |
| Median 0.1% low | 72.03 |
| Max PerformanceSystem overhead | 1237.8 us |

### Candidate Results

| Candidate | Hash | Accepted | Median FPS | Median 1% low | Median 0.1% low | Finding |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| HISM, four-slot world-offset material | `3D1852F6A4AB214A637DD3FA7E661AF01A79D0D8466133727D9C672D3DD85990` | 3/3 | 137.43 | 77.02 | 41.98 | Rejected. Keeping instance transforms at origin defeated useful spatial behavior and extra custom-data reads were costly. |
| HISM, world transforms but offset-capable material | `B99F5733620D8421300B64ED755E47765E268F633927D33DF202913B4BBD6D75` | 3/3 | 116.89 | 73.76 | 64.73 | Rejected. Offset-capable shader stayed too expensive even with zero offsets. |
| HISM, frame-only material, per-frame custom-data writes | `502A9A0969B373E34231317B5CB2BB609D0246FB1F86A2E773E0028557C27B5A` | 2/3 | 161.39 | 113.61 | 53.88 | Rejected. One row rejected for `NoProjectilesFired`; accepted rows still regressed. |
| HISM, frame-only material, sparse custom-data writes | `523293239A13BC3683579FDBD334981D3EFC8678E312CA110E1481184CDD981B` | 3/3 | 172.64 | 139.61 | 68.98 | Correct but not acceptable. Median and 1% low regressed vs before. |
| ISM feasibility, frame-only material, sparse custom-data writes | `4268CE0FB64B53C0A4F1C5FF4C041236E1D80E79D9A79B9E6C681AF29108FAFA` | 3/3 | 176.42 | 151.14 | 67.12 | Better than HISM, but still below the pre-B.13 baseline. |

## Root Cause Finding

The custom-data VAT technique works functionally, but dynamic instanced rendering is not currently a performance win for T66 basic mobs because the instances are not static. Every active mob still moves every frame, so the renderer must receive per-instance transform updates for roughly 90 moving instances at saturation.

HISM is especially poor for this use case: the hierarchy/tree management and per-instance transform update path cost more than the draw-call reduction saves. Plain ISM removes the hierarchy cost and improves the result, but still fails to beat the current per-mob static-mesh-component path.

The material-side world-offset experiment was also rejected. Moving all instance transforms to the origin avoids transform-location churn, but it breaks useful spatial behavior and adds extra per-vertex custom-data reads. The performance result was substantially worse.

## Decision

B.13 is not landed. The accepted plan's de-risk stage prevented a bad renderer from being copied into the live repo.

Do not replace the live per-mob visual mesh path with the tested HISM implementation. The live repo should stay on the current per-mob mesh rendering path until a revised rendering strategy is reviewed.

## Proposed Next Scope

Run a focused B.13R renderer rethink before any live rendering swap. Candidate directions:

1. Keep per-mob mesh components for moving mobs and only instance non-moving/death/pooled visual states.
2. Use ISM instead of HISM only if a revised plan accepts the scope change and can prove a median win.
3. Reduce transform update frequency or interpolate in material only if visual parity can be proven; this is a gameplay-visual tradeoff and needs review.
4. Explore GPU/VAT crowd rendering with manager-owned positions in a texture or structured buffer rather than UE component instance transforms.

## Verification Artifacts

- Before results: `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\Before_capture_results.json`
- HISM final candidate results: `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\After_FrameOnlySparseCustom_capture_results.json`
- ISM feasibility results: `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\After_ISMFeasibility2_capture_results.json`
- Stage 1 screenshot: `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\T66_B13_Stage1Slime_HISMProof_frameonly_sparsecustom.png`
- Full proof screenshot: `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\T66_B13_Full_HISMProof_frameonly_sparsecustom.png`

## Continuation Verification

After the initial packet, the isolated worktree tested the remaining plausible HISM explanations before closing the pass.

Engine-source audit:

- `UHierarchicalInstancedStaticMeshComponent::BatchUpdateInstancesTransformsInternal` loops through `UpdateInstanceTransform` for each instance.
- `UpdateInstanceTransform` marks `bIsOutOfDate=true` and, for location changes, calls `BuildTreeIfOutdated(/*Async*/true, /*ForceUpdate*/false)`.
- Therefore `bAutoRebuildTreeOnInstanceChanges=false` does not remove all per-frame HISM tree/update cost for moving instances.

Additional candidates:

| Candidate | Hash | Accepted | Median FPS | Median 1% low | Median 0.1% low | Finding |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| HISM plus VisualMesh unregister | `F561071FE925911B911124BF5F68DA969E4C834C8D97A91B26BF88AED4A98C2F` | 0/1 | n/a | n/a | n/a | Rejected. Crashed in `FPrimitiveSceneInfo::UpdateComponentLastRenderTime`; unregistering the mesh component while scene relevance tasks are active is unsafe. |
| HISM plus safe VisualMesh render suppression (`RenderInMainPass=false`, no unregister) | `FD6A427D442BF015E2F7556A565D84EA749E012ABE9226621B497A2BAABC55A4` | 3/3 | 176.83 | 150.36 | 75.33 | Stable but below before baseline; stale mesh render flags were not the main regression. |
| HISM hidden pool at origin instead of offscreen | `8D0C9FAC2A978BD1E1E3736858420314B3B46EFE8AC46D067A73FE04F8A928B8` | 3/3 | 169.16 | 143.11 | 56.47 | Worse. Offscreen hidden pool bounds were not the main regression. |
| HISM transform-change cache | `604642337D71835F44990C3BFB94A2647939FFB1A7A552538D75E446CB860841` | 3/3 | 171.53 | 119.51 | 54.62 | Worse. Redundant same-transform submissions were not the main regression. |

## B.13R Spatial-Cell Continuation

Claude reviewed a follow-up HISM spatial-cell plan intended to avoid per-frame HISM location updates by storing a coarse cell transform in the HISM instance and the fine world offset in per-instance custom data.

- Revised plan packet: `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\plan_packet_b13r_cell_bucket.md`
- Claude review: `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\20260529T031018-pass2\claude_review_pass2.md`
- Verdict: `APPROVE`
- Live runtime source copied back: no.

Correctness proof passed:

| Proof | Evidence |
| --- | --- |
| Slime-only spatial-cell proof | `T66_B13_Stage1Slime_HISMProof_cellbucket.log`: `Buckets=1`, `ActiveHISMStates=10`, `FallbackVATStates=50`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `CustomDataFloats=4`, `CellSize=2000.0`, `Pass=1` |
| All-family spatial-cell proof | `T66_B13_Full_HISMProof_cellbucket.log`: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `CustomDataFloats=4`, `CellSize=2000.0`, `Pass=1` |
| Saturated visual sanity screenshot | `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\screenshots\T66_B13_After_HISMCellBucket_02.png` showed many HISM-rendered mobs visible and co-located during the capture band. |

Spatial-cell and final render-flag probes:

The `1/1` rows below are rejection probes only. They are not promoted to positive baselines; a one-row probe below the established `189.65` median baseline was sufficient to reject that branch without spending a full acceptance set.

| Candidate | Hash | Accepted | Median FPS | Median 1% low | Median 0.1% low | Finding |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| HISM spatial-cell, `2000` uu cells, four custom floats | `4CAE3738260A946E33A4B4ECFECBADCCDE54A6E8E17BCDBF361F4B8805345292` | 3/3 | 80.42 | 44.67 | 38.98 | Correct but catastrophically slower. Large cell offsets were likely damaging culling/bounds and shader cost. |
| HISM spatial-cell, `500` uu cells | `C0D8E542693EF644F3BEE89593E186E11CA657CC0B82D5638CA9C4AD42E95F53` | 1/1 probe | 167.21 | 94.64 | 62.28 | Smaller cells improved the branch but still missed the 189.65 median baseline. |
| HISM spatial-cell `500` uu, shadows/decals disabled | `FD487A2CB24AEFE1C874F212F2377BA8C6D28F3783F8F423A255C55A6101C6DA` | 1/1 probe | 177.31 | 142.78 | 68.46 | Render flags helped but still failed median and 1% low. |
| HISM spatial-cell `500` uu, no shadows/decals, lazy pooled allocation | `479760F29BE932D2C86413B078E01CD1F92FDCF46452F3FA6BA7B777132BD39F` | 1/1 probe | 176.01 | 138.49 | 66.08 | Reducing hidden preallocation did not recover the gap. |
| HISM spatial-cell `500` uu, no shadows/decals, lazy pool, actor mesh nulled on takeover | `949DD53D44D697DF6BDF5381031D6B7B84991F533C55611AB3F83F54B8068B32` | 1/1 probe | 179.36 | 157.71 | 139.87 | Best final probe for lows, but median still regressed by `10.30` FPS versus the baseline. |

Updated conclusion:

- HISM VAT rendering is functionally proven but should not land.
- The root issue is not stale per-mob visual mesh visibility, hidden-pool placement, or duplicate same-transform submissions.
- The spatial-cell/material-offset continuation also does not land. It proves HISM visuals can be positioned through per-instance custom data, but the material-offset and HISM render path still cannot beat the existing per-mob mesh renderer at full resolution.
- The live renderer remains unchanged because every tested HISM/ISM dynamic-instancing path either regressed full-resolution FPS or crashed.
- A future B.13R needs a new reviewed rendering architecture rather than more tuning of this HISM approach.
