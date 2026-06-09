# Codex Draft - Fall Guys Physics Ownership Stage 1

## Task Contract

Working task:
Operator: Codex
Validator: Claude
Scope: Implement Stage 1 physics ownership only: create/update repo documentation and ownership routing for the reopened broad Physics architecture, update stale pure-Chaos/PAC-off policy notes, and produce a concrete Stage 2 hero active-ragdoll MVP plan for Hero 1 Chad including rigging/animation. No broad source deletion, model deletion, or Blender/Unreal asset mutation in this pass.
Stop condition: Stage 1 docs/routing are written, Stage 2 plan is written, Claude cross-check is incorporated, and verification is reported.

## Files Changed

- `Gameplay/Physics/PHYSICS_AGENTS.md`
  - New folder router for broad gameplay physics ownership.
  - Routes active ragdoll, PAC, Chaos, PhysicsAsset, knockback/recovery, body reactions, and obstacle/body physics feel.
  - Points Hero 1 Chad raw rigging to `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`.

- `Gameplay/Physics/README.md`
  - New orientation map for the Physics folder.
  - Records that Hero 1 Chad active ragdoll is the current target.
  - Lists live source paths, current Hero 1 Chad model rows, raw GLB source, rigging process, and seed PhysicsAsset report.

- `Gameplay/Physics/MASTER_PHYSICS.md`
  - New policy doc for the reopened broad Physics architecture.
  - Records that the previous pure-Chaos/PAC-off hero direction is historical scaffolding.
  - Defines the target runtime model: pose drive plus world hip/pelvis anchor.
  - Defines mechanisms and anti-lookalike rules.

- `Gameplay/Physics/HeroPhysicsModel.md`
  - New Stage 2 implementation plan.
  - Scopes the MVP to Hero 1 Chad only.
  - Specifies the raw source GLB, current `CharacterVisuals.csv` rows, rigging process, required skeleton, animation clips, PhysicsAsset plan, runtime component plan, movement/forward-leap plan, PPF preflight, and acceptance gates.

- `Gameplay/Physics/PhysicsReactionProfiles.md`
  - New future profile contract for physical reactions.
  - Defines initial hero states and reaction profile fields.
  - Keeps trap damage/progression separate from Physics reaction feel.

- `Gameplay/Physics/PhysicsAssetPipeline.md`
  - New rig/PhysicsAsset readiness contract.
  - Points to raw FriendSlop humanoid rigging and Unreal import validation docs.
  - Records the existing TestRoom PhysicsAsset report as seed evidence, not final active-ragdoll acceptance.

- `Gameplay/Physics/Archive/README.md`
  - New index of historical physics/ragdoll reports.
  - Marks pure-Chaos/PAC-off work as superseded/historical without deleting artifacts.

- `Gameplay/README.md`
  - Added `Physics` to Gameplay areas.

- `Gameplay/GAMEPLAY_AGENTS.md`
  - Added Physics to owned gameplay systems.
  - Added physics/ragdoll/PAC/PhysicsAsset/Chaos/knockback trigger words.

- `Source/T66/Gameplay/pending_issues_Gameplay.md`
  - Renamed the first issue from resolved standing direction to superseded previous direction.
  - Added 2026-06-06 supersede note: user reopened hero physics architecture, broad `Gameplay/Physics` is now the direction, and old pure-Chaos/PAC-off work is fallback/historical scaffolding only.

## Important Scope Notes

- I did not delete source files, reports, model assets, raw generated runs, runtime packages, CSV rows, or old proof artifacts.
- I did not mutate Blender, Unreal assets, C++ runtime behavior, input bindings, animation rows, or model imports.
- I did not change TestRoom wipeout-arm settings or `UT66KnockbackComponent` code.
- Cleanup in this pass means ownership docs, routing, policy supersede note, and archive indexing.

## Stage 2 Plan Summary

The next implementation pass should be a Hero 1 Chad active-ragdoll MVP:

1. Use raw source:
   - `Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb`.

2. Follow raw FriendSlop humanoid rigging:
   - `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`.
   - Required skeleton: root/pelvis/three-spine/head/arms/legs with no helper deform bones.
   - Required QA: facing proof, rest proof, bend proof, hierarchy dump, vertex influence QA, FBX re-import validation.

3. Author simple active-ragdoll pose clips:
   - Idle
   - WalkRun
   - Jump
   - ForwardLeap
   - optional GetUpPose

4. Build/import stricter PhysicsAsset:
   - Use the existing 18-body/17-constraint TestRoom report as a seed only.
   - Validate pelvis as central body, pelvis-heavy mass, spine chain stability, limited joints, CCD/sleep/damping/restitution, and no inversion/stretch.

5. Add new runtime path:
   - New `UT66HeroPhysicsComponent` / `FT66HeroPhysicsProfile`.
   - Capsule remains authoritative mover.
   - Mesh always simulated below pelvis at physics blend 1.
   - PAC uses local simulation pose drive.
   - Hip/pelvis anchor constraint provides world balance.
   - Hit reaction ramps anchor and drive down/up.

6. Keep old paths:
   - Do not remove `UT66KnockbackComponent` in Stage 2.
   - Do not globally replace roll yet. Plan includes forward leap as the target, but first implementation should gate it to Hero 1/TestRoom unless user explicitly broadens.

7. Proof:
   - PPF check before implementation.
   - Blender proof artifacts.
   - Unreal import/PhysicsAsset reports.
   - Focused compile.
   - TestRoom video/log proof.
   - Staged standalone if runtime playable behavior changes.

## Verification Performed

- Read live T66 root and Operator/Validator rules.
- Read `.t66/operator-state.json`.
- Read `Gameplay/GAMEPLAY_AGENTS.md`.
- Read `Gameplay/README.md`.
- Read `Source/T66/Gameplay/pending_issues_Gameplay.md`.
- Read rigging docs:
  - `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md`
  - `Model Generation/Rigging and Animation/README.md`
  - `Model Generation/Rigging and Animation/00_RIGGING_ANIMATION_ROUTING_INSTRUCTIONS.md`
  - `Model Generation/Rigging and Animation/03_FINDINGS_AND_LIMITATIONS_REFERENCE.md`
  - `Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`
  - `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`
  - `Model Generation/README.md`
- Confirmed `Content/Data/CharacterVisuals.csv` Hero 1 Chad rows.
- Confirmed raw source GLB exists.
- Confirmed existing TestRoom PhysicsAsset report.
- Ran Claude independent answer before editing.

No build, staged run, Unreal import, Blender run, or runtime capture was performed because this pass changed documentation/routing only.
