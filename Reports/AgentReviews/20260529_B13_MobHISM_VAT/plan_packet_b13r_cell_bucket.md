# B.13R Plan Packet - HISM Spatial-Cell VAT Rendering

## Working Goal

Plan, review, implement, validate, and document B.13 Mob HISM rendering with VAT per-instance custom data at full resolution, staged through a one-mob de-risk proof before all-family rollout.

## Codex Current Finding

The original B.13 HISM world-transform approach was functionally correct but not complete because it regressed performance. The active goal remains open. The prior isolated packet proved:

- VAT frame selection from `PerInstanceCustomData` works.
- Stage 1 Slime-only proof passed.
- All-family proof passed.
- HISM full-resolution world-transform candidates regressed versus the 189.65 FPS baseline.
- UE 5.7 source shows HISM `BatchUpdateInstancesTransformsInternal` loops through per-instance `UpdateInstanceTransform`; moving instances still mark the HISM out of date and trigger `BuildTreeIfOutdated`.

The next aligned candidate is to preserve HISM and per-instance custom data, but avoid per-frame HISM location updates. Use coarse spatial-cell transforms for HISM culling and drive the per-frame fine movement through per-instance local offset custom data in the VAT material.

## Applicable Instructions

- Root `AGENTS.md`: working goal, folder instruction discovery, plan packet, Claude review, no manual approval after valid Claude `Verdict: APPROVE` unless a human-only decision is present.
- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`: keep performance contracts/docs under `PerformanceSystem/`; runtime gameplay rendering code remains under `Source/T66/Gameplay`.
- Existing B.13 Claude review: `Reports/AgentReviews/20260529_B13_MobHISM_VAT/20260528T222533-pass1/claude_review_pass1.md`.

## PPF Check

Objective: Replace lightweight basic-mob per-actor visual mesh rendering with HISM rendering, with VAT animation driven by per-instance custom data, at full resolution.

Proven process: Existing T66 VAT material process in `/Game/Materials/M_EasyMobVAT_Unlit_UV2`, manager-owned VAT state from Pass A, and the reviewed B.13 HISM de-risk workflow.

My planned implementation: Keep the duplicate instanced VAT material variant. Extend it from frame-only to frame plus local offset. HISM instance transforms represent coarse spatial cells instead of exact actor transforms. Per-frame exact visual placement is `CellTransform + PerInstanceLocalOffset` in the material, while VAT frame remains per-instance custom data. This preserves HISM as the visual carrier and per-instance custom data as the VAT driver.

Same method class: YES

If NO, why: n/a

User approval required before proceeding: NO, if Claude approves. This is a revised plan inside the same B.13 objective and does not introduce a human-only product decision.

Verification evidence:

- Stage 1 Slime-only proof with cell-bucket HISM active and fallback VAT rows for other mobs.
- All-family proof with `FallbackVATStates=0`, independent frame values, and no visible per-mob meshes while HISM is active.
- 3-capture full-resolution after set with median, 1% low, and 0.1% low compared to the existing before baseline.

## Artifact Parity Gate

Reference artifact/category: Current lightweight basic-mob VAT visual mesh rendering.

Role: Primary

Required: YES

Planned artifact/path: `/Game/Materials/M_EasyMobVAT_Unlit_UV2_Instanced` plus HISM components owned by `UT66MobManagerSubsystem`.

Status: EQUIVALENT

Evidence: The existing B.13 proof showed the instanced variant can reproduce independent VAT frames. B.13R must prove the added local-offset path preserves visible placement and animation.

## Mechanism Manifest

Reference/source: Current VAT material custom WPO node and manager-owned VAT state.

Required mechanisms:

1. Mechanism: VAT frame sampling from position texture.
   Required: YES
   Planned implementation: Keep the existing WPO HLSL and feed `Frame` from per-instance custom data slot 0.
   Evidence needed: Independent same-bucket frame values and visible animation in Slime-only and all-family proofs.

2. Mechanism: Exact visual placement follows actor/capsule position.
   Required: YES
   Planned implementation: HISM transform stores a coarse cell center. Custom data slots 1-3 store the local visual offset from that cell center. Material adds the offset after VAT local deformation.
   Evidence needed: Runtime proof/screenshot showing HISM visuals co-located with live mobs and no stale/ghost instances.

3. Mechanism: Avoid per-frame HISM tree rebuild pressure.
   Required: YES
   Planned implementation: Only update HISM transforms when an instance crosses a coarse spatial cell. Per-frame movement within the cell updates custom data only.
   Evidence needed: Capture performance improves over prior HISM world-transform candidates and does not regress versus the before baseline.

4. Mechanism: Rich actor material isolation.
   Required: YES
   Planned implementation: Continue using an instanced material duplicate only for lightweight basic-mob HISM buckets. Original material/MID path remains for rich actors.
   Evidence needed: Original material untouched; rich actors not routed through HISM.

## Anti-Lookalike

Cheapest wrong result: all mobs render at cell centers or at one shared origin while collision continues elsewhere. That would reduce transform churn but break visual/logic alignment.

Discriminator: proof must show HISM visuals visually track actor/capsule positions, and gameplay captures must still show projectile/touch interactions without invisible or displaced mobs. The proof summary must still report `VisibleVisualMeshesWhileHISM=0`, and screenshots must show mobs in plausible world positions rather than clustered at cell anchors.

## Revised Implementation Scope

Files in isolated worktree first:

- `Source/T66/Gameplay/T66MobManagerSubsystem.h`
- `Source/T66/Gameplay/T66MobManagerSubsystem.cpp`
- `Source/T66/Gameplay/T66MobBase.cpp`
- `Content/Materials/M_EasyMobVAT_Unlit_UV2_Instanced.uasset`
- Helper scripts under `Reports/AgentReviews/20260529_B13_MobHISM_VAT/`

No live runtime source copy-back until the isolated candidate passes.

## Technical Plan

1. Recreate/update `/Game/Materials/M_EasyMobVAT_Unlit_UV2_Instanced`:
   - Slot 0: VAT frame.
   - Slot 1: local offset X.
   - Slot 2: local offset Y.
   - Slot 3: local offset Z.
   - WPO returns `TransformLocalVectorToWorld(Parameters, local_delta) + float3(OffsetX, OffsetY, OffsetZ)`.

2. Change HISM instance transform strategy:
   - Compute exact visual world transform as today.
   - Quantize the world location to a coarse cell center (initial candidate: 2000 UU cell size).
   - HISM instance transform uses the cell-center location and the same rotation/scale as the visual.
   - Preserve exact visual yaw/scale in the HISM transform for this candidate rather than moving yaw into material custom data. Pre-review code inspection found manager-driven `SetActorRotation` writes, so this candidate depends on practical yaw stability during the stationary-hero performance scenario; if rotation churn still dirties HISM transforms per frame, the after capture will reject the candidate.
   - Per-instance offset is `ExactVisualWorldLocation - CellCenter`.
   - Only queue HISM transform update when the quantized cell transform changes.
   - Queue offset custom data when the local offset changes.

3. Preserve old safety fixes:
   - Keep VisualMesh hidden and out of main pass while HISM owns rendering.
   - Do not unregister VisualMesh, because that crashed in scene relevance tasks.
   - Keep actor collision capsule/hit zones untouched.

4. De-risk in stages:
   - Stage 1: Slime-only HISM cell-bucket proof.
   - Stage 2: all-family HISM cell-bucket proof.
   - Stage 3: 3-capture full-resolution after set.

## Risks

- Extra per-vertex offset math/custom-data reads may still cost more than saved transform/tree work.
- Coarse cells can hurt culling if too large; too-small cells can reintroduce frequent HISM tree updates.
- Rotating/scaling via HISM transform while offsetting translation may produce alignment bugs if world/local spaces are mixed.
- If lightweight mobs change yaw frequently enough, exact-yaw preservation can still force HISM transform updates and eliminate the intended win. This pass should reject the candidate rather than hiding that cost.

Rollback/revert:

- All implementation remains isolated in `C:\UE\T66_B13_Worktree` until passing.
- If it fails, document it and keep live runtime source on the current per-mob renderer.

## Verification Gates

- Build succeeds.
- Stage succeeds with content-complete standalone.
- Stable staged binary hash per proof/capture set.
- Slime-only proof passes with `Stage1SlimeOnly=1`, one HISM bucket, independent frame values, and no visible VisualMesh while HISM active.
- All-family proof passes with all VAT rows on HISM and fallback count zero.
- Full-resolution after captures: 3/3 accepted, zero HeroDeath, zero overhead rejects, `r.ScreenPercentage=100`, median and lows compared to before baseline.
- If after median and lows do not improve or hold versus before, do not copy runtime source back.

## Out Of Scope

- GPU structured-buffer crowd renderer.
- Moving collision to manager side.
- Rich actor rendering.
- Deleting deprecated paths.
- Any Mini/minigame work.

## Reviewer Request

Review this revised B.13R plan for flawed assumptions, missing files, unsafe material/rendering changes, inadequate verification, or contradictions with the project instructions. Focus especially on whether the spatial-cell transform plus material-offset strategy remains inside the B.13 objective and whether the verification can prove visual alignment and performance.
