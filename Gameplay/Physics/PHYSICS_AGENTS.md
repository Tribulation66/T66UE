# Physics Agents

## Owns

Broad gameplay physics ownership for T66: Hero 1 ragdoll reactions, Chaos body simulation, PhysicsAsset readiness, physics reaction profiles, knockback/recovery policy, physical obstacle interactions, and proof standards for physics-driven player feel.

This is not an obstacle-only folder. Obstacles and traps may call into Physics, but Physics owns the shared feel contract.

## Trigger Words

Physics, ragdoll, active ragdoll, hit-triggered ragdoll, Chaos, PhysicsAsset, knockback, bounce, shove, impulse, simulation, pelvis, get-up, Fall Guys, wobble, obstacle physics.

Also route here when a task mentions PAC, Physical Animation, hip anchor, or active-ragdoll state machines, because those are historical/future physics topics that must be checked against the current source before use.

## Read First

1. `README.md`
2. `CURRENT_STATE.md`
3. `HISTORY.md`
4. `MASTER_PHYSICS.md`
5. `HeroPhysicsModel.md`
6. `PhysicsReactionProfiles.md`
7. `PhysicsAssetPipeline.md`
8. `Archive/README.md`
9. `../../Source/T66/Gameplay/Physics/pending_issues_Physics.md`

For related owners:

- Runtime locomotion/control input: `../Movement/MASTER_MOVEMENT.md`
- Trap spawning, damage, and progression: `../Traps/MASTER_TRAPS.md`
- Raw Hero 1 Chad FriendSlop rigging: `../../Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`
- Unreal import validation for generated assets: `../../Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md`

## Hard Rules

- Live source is the authority for current behavior. Do not preserve an older architecture claim if `UT66HeroPhysicsComponent` no longer implements it.
- The current implemented Hero 1 system is hit-triggered full ragdoll: normal capsule/CharacterMovement control, full skeletal simulation on qualifying hit, pelvis/floor get-up, then return to normal.
- Names containing `ActiveRagdoll` are compatibility names in the current code. They do not imply always-on active-ragdoll simulation by themselves.
- Do not describe the PAC/hip-anchor always-on authority model as current unless the source includes the hip anchor, PAC drive, and active state machine again.
- Do not revive the retired humanoid bakeoff, AccuRig lineup, or Animated ToonStyle bridge for raw FriendSlop Hero 1 Chad unless the user explicitly approves that legacy path.
- For raw FriendSlop humanoid rigs, use the current raw-humanoid process in `Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md`.
- Treat rigging, animation, PhysicsAsset, and runtime physics as coupled for heroes. A model is not physics-ready until pelvis/spine hierarchy, body layout, weights, clips, PhysicsAsset, and runtime proof all pass.
- Prefer data-authored physics profiles over hardcoded C++ feel values when expanding beyond the current MVP.
- Keep `UT66KnockbackComponent` as legacy/prototype support until the new hero ragdoll path has full accepted proof. Do not remove old source or assets without a separate cleanup manifest and reference audit.
- Runtime-facing physics changes require focused compile, staged standalone validation when playable behavior changes, and Unreal-owned capture/video proof for visual/temporal mechanisms.
- Do not accept desktop screenshots as physics proof. Use Unreal-owned capture paths and multi-frame evidence for wobble, hit reaction, tumble, rebound, and recovery.

## Verification

For docs-only ownership changes, report the files edited and skipped runtime verification.

For runtime physics changes, report:

- focused compile/build command and result
- staged standalone path and shortcut verification when playable content changes
- generated/imported asset paths
- profile/data rows changed
- Unreal-owned capture/video/log paths
- frame or log evidence for required temporal mechanisms
- current state-machine/log markers for `Normal`, `Ragdoll`, and `GettingUp`
- remaining partial/deferred mechanisms
