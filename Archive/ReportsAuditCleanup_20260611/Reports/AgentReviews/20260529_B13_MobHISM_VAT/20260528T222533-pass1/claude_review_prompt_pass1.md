You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\plan_packet.md
- Output scope: review of the packet below only.

<review_packet>
# B.13 Mob HISM Rendering with VAT - Plan Packet

Date: 2026-05-29

## Working Goal

Replace lightweight basic-mob per-actor visual mesh + dynamic-material-instance VAT rendering with manager-owned HISM rendering, one HISM bucket per VAT mesh/material combination, driven by per-instance custom data, while preserving actor-owned collision, hit zones, and gameplay logic. Prove VAT via per-instance custom data on one mob type before rolling out to all VAT-enabled basic mobs, then measure full-resolution before/after FPS median plus 1% and 0.1% lows.

## Operator / Validator

- Operator: Codex in `C:\UE\T66`.
- Validator: Claude Code CLI via `Scripts\Invoke-ClaudePlanReview.ps1`.
- Review mode: read-only plan review. A valid first-line `Verdict: APPROVE` from Claude authorizes implementation for this packet under the current `AGENTS.md` process. Codex fallback review, if needed, still requires manual user confirmation.

## Applicable Instructions Read

- `AGENTS.md`
- `PerformanceSystem/PERFORMANCE_SYSTEM_AGENTS.md`
- `Gameplay/GAMEPLAY_AGENTS.md`
- `Gameplay/README.md`
- `Reports/AGENTS.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `PerformanceSystem/pending_issues_PerformanceSystem.md`

## Scope Boundaries

In scope:

- Lightweight `AT66MobBase` rendering only.
- VAT-enabled basic mobs in `Content/Data/MobVertexAnimations.csv`.
- New instanced VAT material variant for HISM use.
- Manager-owned HISM instance allocation, transform updates, and per-instance custom data upload.
- Full-resolution before/after packaged standalone capture using `r.ScreenPercentage=100`.
- Runtime visual proof that independent instances of the same type can animate at different frames.

Out of scope:

- Moving collision, hit zones, or touch damage out of actors.
- Rich actors: minibosses, bosses, specials, guardians.
- Changing rich actor materials or converting rich actor rendering.
- Deleting deprecated rich-basic routing, old projectile actor classes, or old CVars.
- Hero/trap/idol projectile managers.
- Designing new mob art or adding non-Dungeon VAT rows.
- Mini/minigame systems.

## Current Source-State / Dirty Worktree Handling

The live worktree is dirty with unrelated source/content/doc work. B.13 needs an isolated A/B where the only variable between before and after is mob HISM rendering.

Plan:

1. Before staging or measuring, write a source-state classification artifact under `Reports/AgentReviews/20260529_B13_MobHISM_VAT/source_state_classification.md`.
2. Create/use an isolated source tree `C:\UE\T66_B13_Worktree` copied from the current live source state. This intentionally keeps unrelated current work constant for both before and after captures, instead of reverting user-owned changes.
3. Measure the pre-B.13 baseline in the isolated tree before B.13 edits.
4. Implement B.13 in the isolated tree.
5. Measure the after state in the same isolated tree.
6. Copy only reviewed B.13 files/assets back to live `C:\UE\T66` after checking that the live target files have not changed relative to the classification snapshot. If a target file changed, stop and reconcile instead of overwriting.

The read-only Unreal material audit scripts created for this packet are report artifacts, not production runtime changes.

## Audit Findings

### Current lightweight visual setup

- `AT66MobBase` owns a `UStaticMeshComponent* VisualMesh` attached to the capsule root. Actor tick and component ticks are already disabled.
- `AT66MobBase::ApplyConfiguredVisual` calls `TryApplyMobVertexAnimationVisual`.
- `TryApplyMobVertexAnimationVisual` calls `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual(CharacterVisualID, VisualMesh, DynamicMaterial, Row)` and then registers VAT state with `UT66MobManagerSubsystem::ConfigureMobVertexAnimationState`.
- `UT66MobManagerSubsystem` owns `FT66MobVertexAnimationRuntimeState` and currently advances clip/frame state in `TickMobVertexAnimationState`.
- The manager still applies the frame by calling `State.Material->SetScalarParameterValue("Frame", CurrentFrame)` on each mob's MID.
- The main manager tick can reach `TickMobVertexAnimationState` through several branch exits, but each active mob takes exactly one branch per tick. B.13 will preserve and prove at-most-one VAT advancement per active mob per manager tick.

### Current VAT material and sampling

Read-only Unreal Python audit outputs:

- `Saved/Codex/Performance/B13_MobHISM/vat_material_audit.json`
- `Saved/Codex/Performance/B13_MobHISM/material_graph_probe.json`

Important correction: the live mob VAT material instances do not parent directly to `M_Character_Unlit`. All ten current VAT MIs parent to:

- `/Game/Materials/M_EasyMobVAT_Unlit_UV2.M_EasyMobVAT_Unlit_UV2`

`M_Character_Unlit` still exists and is used by other character/static visual paths, so it must remain untouched. The original VAT parent also remains untouched because rich placed slimes or any other actor/MID path may still use the existing material instances.

`M_EasyMobVAT_Unlit_UV2` is unlit. Its base/emissive color path is:

- `BaseColorTexture * Tint * Brightness`

Its world-position-offset path is a custom material expression with inputs:

- `UV1`
- `PositionTexture`
- `Frame`
- `RowsPerFrame`
- `MinBBox`
- `SizeBBox`

Audited custom expression code:

```hlsl
uint tex_width;
uint tex_height;
PositionTexture.GetDimensions(tex_width, tex_height);
float frame_index = floor(Frame + 0.0001);
float2 sample_uv = UV1;
sample_uv.y += (frame_index * RowsPerFrame) / max(1.0, (float)tex_height);
float3 packed_delta = Texture2DSample(PositionTexture, PositionTextureSampler, sample_uv).rgb;
float3 local_delta = packed_delta * SizeBBox.rgb + MinBBox.rgb;
return TransformLocalVectorToWorld(Parameters, local_delta);
```

B.13 will duplicate this material to `/Game/Materials/M_EasyMobVAT_Unlit_UV2_Instanced` and replace only the `Frame` scalar input to the WPO custom node with `PerInstanceCustomData(DataIndex=0, ConstDefaultValue=0)`. All texture/BBox constants remain component material parameters set once per HISM bucket.

### Per-instance custom-data slot map

HISM components will set `NumCustomDataFloats = 6`.

| Slot | Manager field | Material use in B.13 |
| ---: | --- | --- |
| 0 | `CustomDataFrame` | Drives WPO `Frame` through `PerInstanceCustomData` |
| 1 | `CustomDataStartFrame` | Written for proof/debug and B.13+ readiness |
| 2 | `CustomDataEndFrame` | Written for proof/debug and B.13+ readiness |
| 3 | `CustomDataClipIndex` | Written for proof/debug and B.13+ readiness |
| 4 | `CustomDataPlayRate` | Written for proof/debug and B.13+ readiness |
| 5 | `CustomDataFlags` | Written for proof/debug and visibility/state readiness |

The material only needs slot 0 to match the current WPO path. Writing all six keeps the manager-owned layout Pass A prepared and leaves B.13 evidence aligned with the future custom-data contract.

### Mob VAT inventory

Authoritative current VAT source: `Content/Data/MobVertexAnimations.csv`.

There are ten VAT-enabled Dungeon basic-mob rows, each with a distinct static mesh, position texture, normal texture, and material instance. These cover all four migrated families used by the current Dungeon enemywaveperf/stage flow:

| Family | VAT rows |
| --- | --- |
| Melee | `Slime`, `BoneWalker`, `TombSpider`, `CryptWraith` |
| Rush | `RatPack`, `MimicLure` |
| Flying | `CaveBat` |
| Ranged | `HexSlinger`, `StoneSentinel`, `BoneConjurer` |

`Content/Data/Enemies.csv` contains additional Forest/Ocean/Martian/Hell basic rows marked `MeshReady`, but they currently have no corresponding rows in `MobVertexAnimations.csv`. B.13 will HISM-convert the current VAT-enabled basic-mob rendering path. If a non-VAT basic mob is encountered at runtime, it remains on the existing fallback static visual path and is documented as a content-authoring/future rendering gap rather than silently forcing an invalid VAT bucket.

### HISM bucket key

One `UHierarchicalInstancedStaticMeshComponent` per unique VAT render key:

- `StaticMesh`
- instanced VAT parent material path
- pixel/base-color texture
- position texture
- normal texture
- row constants (`RowsPerFrame`, `NumFrames`, `MinBBox`, `SizeBBox`, `MeshRelativeTransform`)

The current table means up to ten buckets. Stage 1 will enable only `Slime`; Stage 2 rolls out all ten rows.

## PPF Check

```text
PPF CHECK
Objective: Preserve the existing VAT deformation method while moving the per-mob frame source from MID scalar to HISM per-instance custom data.
Proven process: Existing T66 EasyMob VAT pipeline using M_EasyMobVAT_Unlit_UV2 world-position-offset texture sampling from MobVertexAnimations.csv rows, manager-driven VAT state from Pass A, and Unreal HISM per-instance custom data.
My planned implementation: Duplicate the existing VAT material, keep the same WPO custom HLSL and texture/BBox parameters, replace only the Frame scalar input with PerInstanceCustomData slot 0, then feed the existing manager-owned frame value into HISM instance custom data.
Same method class: YES
If NO, why: N/A
User approval required before proceeding: NO after valid Claude approval
Verification evidence: Material audit, Stage 1 one-type proof, multi-frame instance-independent VAT proof, all-family visual proof, full-resolution before/after captures.
```

## Artifact Parity Gate

```text
ARTIFACT PARITY GATE
Reference artifact/category: Existing VAT static mesh + position texture + unlit VAT material instance.
Role: Primary
Required: YES
Planned artifact/path: Existing SM/TX rows stay in Content/Characters/MobsVAT/*; new /Game/Materials/M_EasyMobVAT_Unlit_UV2_Instanced uses the same WPO algorithm with per-instance frame input.
Status: SAME method, new material variant only
Evidence: material graph audit and Stage 1 multi-frame capture.

Reference artifact/category: Rich actor/MID material path.
Role: Primary for miniboss/special/boss/rich actors
Required: YES
Planned artifact/path: Leave M_Character_Unlit, M_EasyMobVAT_Unlit_UV2, and all existing MI_EasyMobVAT_* material instances unchanged.
Status: SAME
Evidence: asset diff plus smoke that placed slime/rich actors still render.
```

## Mechanism Manifest

```text
MECHANISM MANIFEST
Reference/source: Existing M_EasyMobVAT_Unlit_UV2 WPO custom node and UT66MobManagerSubsystem VAT state.
Required mechanisms:
  1. Mechanism: Position texture VAT sampling
     Required: YES
     Planned implementation: Keep the same custom HLSL formula and texture/BBox parameter set in the duplicated material variant.
     Evidence needed: Stage 1 visual proof matches Slime pre-B.13 deformation.
  2. Mechanism: Per-mob independent frame selection
     Required: YES
     Planned implementation: Write manager-owned CustomDataFrame to HISM PerInstanceCustomData slot 0 for each active instance.
     Evidence needed: multiple same-type instances display different frames/phases in the same frame sequence.
  3. Mechanism: Clip switching and override timing
     Required: YES
     Planned implementation: Preserve manager clip selection/override code; upload resulting frame/start/end/clip fields to HISM custom data.
     Evidence needed: idle/move/attack/death/status proof per family.
  4. Mechanism: Spawn/release visual lifecycle
     Required: YES
     Planned implementation: allocate persistent HISM instance on acquire, keep visible while active/dying, play death clip, then hide and recycle without RemoveInstance churn.
     Evidence needed: pool reuse proof showing no ghost/stale transform/custom-data state.
  5. Mechanism: Transform sync
     Required: YES
     Planned implementation: actor capsule remains authoritative for collision; manager batches HISM transforms from actor/capsule location plus row relative transform.
     Evidence needed: visual/capsule co-location proof in smoke and no hit-zone regression.
```

Anti-lookalike discriminator:

- Cheapest wrong result: all instances use one component/MID `Frame` value and therefore animate in lockstep, or the HISM renders static meshes with no VAT deformation.
- Discriminator: Stage 1 must show at least three same-type instances with different frame values and visible different poses in one multi-frame capture/log proof; Stage 2 repeats across all four families.

## Implementation Plan

### Stage 0: Pre-B.13 full-resolution baseline

1. Classify current dirty source state.
2. Create isolated `C:\UE\T66_B13_Worktree` source tree from current live state.
3. Build and stage packaged Win64 Development standalone in the isolated tree.
4. Confirm RetroFX is off and `r.ScreenPercentage=100`.
5. Run 3 lightweight enemywaveperf captures at 1920x1080/full res with `HeroHPOverride=20000`.
6. Record median, 1% low, 0.1% low, overhead max, projectile counters, route counters, binary hash.

This supersedes the prior single-capture 146.30 FPS full-res sanity read as the authoritative pre-B.13 baseline.

### Stage 1: One-type VAT custom-data proof

One-type target: `Slime`.

Implementation:

1. Add an editor automation script under `Reports/AgentReviews/20260529_B13_MobHISM_VAT/` to duplicate `/Game/Materials/M_EasyMobVAT_Unlit_UV2` to `/Game/Materials/M_EasyMobVAT_Unlit_UV2_Instanced`.
2. In the duplicate only, replace the WPO custom node's `Frame` input source with `UMaterialExpressionPerInstanceCustomData` `DataIndex=0`.
3. Add manager-owned HISM bucket structs/components for VAT rows, initially gating HISM rendering to `Slime` only.
4. On Slime acquire/configure:
   - allocate a persistent pre-created or lazily grown HISM instance from the Slime bucket;
   - set bucket mesh and runtime MID parameters from the Slime VAT row;
   - set custom data slots 0-5 from the manager VAT state;
   - hide the actor `VisualMesh` after HISM assignment.
5. On Slime tick:
   - continue normal actor movement/collision;
   - advance VAT state once;
   - batch/update the HISM transform and custom-data values.
6. On Slime death/release:
   - preserve death clip playback;
   - hide/recycle the HISM instance after existing release timing.

Gate:

- If Slime VAT custom-data proof fails, stop and document the failure before touching other rows.
- Failure examples: no WPO deformation, all Slimes lockstep, rich placed Slime breaks, material compile failure, HISM custom data ignored.

Proof:

- Non-shipping proof hook or smoke command that spawns multiple Slimes with forced different clips/frames.
- Capture >=3 samples >=0.10s apart.
- Evidence saved under `Saved/Codex/Performance/B13_MobHISM/stage1_slime_proof/`.

### Stage 2: Roll out to all VAT-enabled basic mob rows

1. Generalize the HISM bucket map to all ten `MobVertexAnimations.csv` rows.
2. One HISM per unique VAT render key.
3. Use preallocated persistent instances per bucket with hide/reuse. Do not use `RemoveInstance` in active gameplay.
4. Upload transforms in batches per bucket using `BatchUpdateInstancesTransforms` or the closest UE 5.7 equivalent.
5. Upload custom data with `SetCustomDataValue(..., bMarkRenderStateDirty=false)` and mark the HISM render state dirty once per updated bucket per frame.
6. Keep `AT66MobBase` capsule, hit zones, lock indicator, actor registry, health, combat, projectiles, and behavior untouched.
7. Keep actor `VisualMesh` component present but hidden/disabled for HISM-rendered mobs. It remains as a conservative fallback for non-VAT rows and debugging.

### Stage 3: Finalize visual routing and proof hooks

1. Ensure HISM assignment resets correctly on pool reuse.
2. Ensure death clip completes before hiding/recycling the instance.
3. Ensure actor hidden state does not hide the HISM component, since HISM lives on the manager/world subsystem rather than the actor.
4. Add terminal summary counters for proof only:
   - HISM buckets active
   - active instances
   - peak instances
   - transform batch updates
   - custom-data updates
   - fallback visual count
   - instance reuse resets
5. Keep summaries aggregate only. No per-frame/per-mob log emission.

### Stage 4: Full rollout validation and measurement

1. Build and stage the isolated B.13 binary.
2. Run all visual proof hooks:
   - Stage 1 Slime independent-phase proof.
   - All-family idle/move/attack/death/status proof.
   - Pool-reuse reset proof.
   - Rich Slime/miniboss or other rich actor visual smoke to confirm original MID path still renders.
3. Run 3 after captures at full resolution with the same settings as Stage 0.
4. Compare before vs after:
   - median FPS
   - mean/stdev
   - 1% low FPS
   - 0.1% low FPS
   - draw-call/HISM bucket count proxy
   - PerformanceSystem overhead max
   - binary hash stability

Acceptance expectation:

- Median improves.
- 1% and 0.1% lows improve or at least hold. If lows regress, investigate before claiming success.

## Files / Assets Expected To Change

Source:

- `Source/T66/Gameplay/T66MobManagerSubsystem.h`
- `Source/T66/Gameplay/T66MobManagerSubsystem.cpp`
- `Source/T66/Gameplay/T66MobBase.h`
- `Source/T66/Gameplay/T66MobBase.cpp`
- `Source/T66/Core/T66CharacterVisualSubsystem.h` (only if a clean row-parameter helper is needed)
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp` (only if a clean row-parameter helper is needed)

Content:

- `Content/Materials/M_EasyMobVAT_Unlit_UV2_Instanced.uasset` (new)

Docs/reports:

- `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`
- `PerformanceSystem/pending_issues_PerformanceSystem.md` if capture methodology or overhead status changes
- `Source/T66/Gameplay/pending_issues_Gameplay.md` if a visual fallback or runtime rendering gap is found
- `Reports/AgentReviews/20260529_B13_MobHISM_VAT/*`
- `Saved/Codex/Performance/B13_MobHISM/*`

## Risks and Mitigations

| Risk | Mitigation |
| --- | --- |
| Per-instance custom data does not affect WPO in this material setup | Stage 1 Slime-only proof before rollout |
| Editing shared material breaks rich actors | Duplicate VAT parent to instanced variant; do not alter original parent or existing MIs |
| HISM custom-data update cost offsets draw-call win | Batch transforms, mark render state dirty once per bucket, measure median and lows |
| Actor hidden state hides logic/collision or HISM unexpectedly | HISM components live under manager; actor keeps collision and registry, actor VisualMesh hidden only |
| Death clip disappears immediately due existing release-on-next-tick behavior | Preserve or adjust visual lifecycle so HISM instance remains through death proof; stop if gameplay lifecycle requires a separate reviewed fix |
| Non-VAT rows still render with per-mob fallback | Document as existing content coverage gap; do not invent VAT rows or convert unrelated static paths in B.13 |
| Dirty live worktree contaminates A/B | Use isolated source tree and copy back only reviewed B.13 paths after target-file drift checks |
| Measurement contaminated by build/editor/git-lfs | Existing clean-environment gate, binary hash before/per row/after, PerformanceSystem overhead rejection |

## Verification Plan

Build/stage:

- `Build.bat T66Editor Win64 Development` if editor/material scripts are used.
- `Build.bat T66 Win64 Development`.
- `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development` in isolated tree.
- Verify staged `T66.exe` SHA256 before, per capture, and after each set.
- Verify taskbar shortcut target after live integration if staged standalone in live tree is refreshed.

Material proof:

- Unreal Python asset audit after creation confirms:
  - new material exists;
  - original material still exists and is unmodified;
  - instanced material WPO `Frame` input comes from `MaterialExpressionPerInstanceCustomData` data index 0.

Runtime visual proof:

- Slime one-type independent-phase proof.
- All-family proof: Melee/Rush/Flying/Ranged.
- Clip proof: idle, move, attack, death, status/freeze.
- Pool reuse: reacquire resets transform and custom data; no ghost/stale instance.
- Rich actor/MID path smoke: placed Slime guardian or equivalent still renders using original material path.

Performance proof:

- Full-res before: 3 captures, median + 1%/0.1% lows.
- Full-res after: 3 captures, median + 1%/0.1% lows.
- Rejection rules:
  - `PerformanceSystemOverheadMaxUs > 10000` rejects;
  - first HeroDeath halts;
  - binary hash drift rejects;
  - non-zero exit rejects.
- Escalate only if the after result is a borderline fail, not for a noisy pass.

Documentation:

- Append `B.13 Mob HISM Rendering with VAT` to `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`.
- Update baseline table with full-res before and after.
- Include material mapping, slot map, HISM bucket inventory, binary hashes, capture tables, screenshots/proof paths, and remaining gaps.

## Acceptance Criteria

- Stage 1 proves Slime VAT via HISM per-instance custom data before all-row rollout.
- Original shared materials/MIs remain intact for rich actors.
- HISM rendering replaces per-mob visual mesh rendering for all current VAT-enabled lightweight basic mobs.
- Actor collision capsule, body/head hit zones, lock indicator, registry, health, combat, and logic remain actor-owned.
- Instances of the same type animate at independent phases.
- Idle/move/attack/death/status visuals are correct across all four families.
- Instance pooling uses hide/reuse, not active-gameplay `RemoveInstance` churn.
- Transform updates are batched per HISM bucket.
- Full-res before/after median, 1% low, and 0.1% low are recorded with stable binary hashes and zero accepted-row overhead rejects.
- Lows improve or hold; any regression is investigated before success is claimed.


</review_packet>
