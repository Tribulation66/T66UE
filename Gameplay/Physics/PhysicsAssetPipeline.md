# Physics Asset Pipeline

## Purpose

This document records what makes a Hero 1 Chad rig and PhysicsAsset acceptable for the current ragdoll physics framework.

## Source Process

Raw FriendSlop humanoid rigging starts from:

```text
Model Generation/Instructions/13_FRIENDSLOP_RAW_HUMANOID_RIGGING_INSTRUCTIONS.md
```

Unreal import validation follows:

```text
Model Generation/Instructions/05_UNREAL_IMPORT_AND_VALIDATION_INSTRUCTIONS.md
```

Do not use the retired humanoid bakeoff, archived AccuRig lineup, or legacy Animated ToonStyle bridge for the physics-first Hero 1 Chad foundation unless the user explicitly changes the process.

## Input

Current Hero 1 Chad raw source:

```text
Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Outputs/Hero_1_Chad_Male.glb
```

Current Stage 2 rigging output folder:

```text
Model Generation/Runs/Pixal3D/FriendSlopProbe_Hero1Male_20260604_1415/Blender/PhysicsFirstHero/
```

## Rig Acceptance

Required:

- one deform armature
- one root
- real pelvis under root
- three nonzero spine bones
- distinct head, arm, and leg chains
- no helper/control bones in exported deform skeleton
- no near-zero required bones
- no unweighted vertices
- maximum four influences per vertex
- normalized weights
- preserved generated texture/material identity
- four-axis facing proof
- limb bend proofs
- FBX re-import validation

## PhysicsAsset Acceptance

Current hit-triggered full ragdoll requires:

- pelvis body exists and can be resolved by `UT66HeroPhysicsComponent`
- simulation root body exists and can be resolved by `UT66HeroPhysicsComponent`
- pelvis, spine, head, arms, legs, calves, and feet have enough body coverage for readable tumble
- constraint graph is continuous and sane
- no arbitrary floor/root body becomes the ragdoll center
- joint limits block inversion
- core mass distribution favors the pelvis/body center
- damping/restitution are intentionally authored
- CCD/sleep settings are explicitly reviewed
- all-body simulation is stable during the hit reaction window
- get-up can place the capsule under the pelvis without the body visually snapping across the room
- report records body count, constraint count, body bones, constraint pairs, and known compromises

Current Stage 2 seed report:

```text
Reports/AgentReviews/FallGuysHeroRiggingStage2Implementation/physics_first_hero1_physics_asset_report.json
```

That report is useful seed evidence, not final acceptance for the desired Fall Guys-like feel. Runtime tuning must still validate the asset under hit reaction, floor contact, recovery, obstacle contact, and repeated hits.

## Proof Artifacts

Stage 2 asset work should produce:

- Blender file
- skeletal FBX
- rig report
- facing/rest/bend proof images
- exported hierarchy dump
- vertex influence QA
- imported skeletal mesh path
- animation asset paths
- PhysicsAsset path
- PhysicsAsset report
- focused compile evidence when runtime wiring changes
- DataTable reload/import evidence when runtime assets are changed

Runtime proof should produce:

- TestRoom ragdoll runtime proof video/log
- `Reaction Applied=1 Source=TestRoomWipeoutArm` when testing the arm route
- `ActiveApplied=1` and `LegacyApplied=0` when validating the hero physics route
- state sequence through `Ragdoll` and `GettingUp` back to `Normal`
- simulated body count during ragdoll
- readable obstacle-contact multi-frame proof

Accepted proof command family:

```text
Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof
```

## Cleanup Boundary

Do not delete old source assets, reports, or runtime packages during the rigging/PhysicsAsset pass unless a separate cleanup manifest proves exact references and approval.
