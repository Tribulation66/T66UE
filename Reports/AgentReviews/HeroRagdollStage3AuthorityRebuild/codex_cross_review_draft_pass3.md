# Codex Draft For Claude Cross-Review Pass 3

## Task Contract

Operator: Codex
Validator: Claude

Scope: Rebuild Stage 3 Hero 1 active ragdoll around the single authority model agreed in the diagnostic: capsule gameplay authority, kinematic mesh component authority, simulated pelvis/body chain, hip-anchor constraint, child PAC as muscle, bounded capsule shove, no normal actor-to-pelvis follow/body teleport loop. Update docs to remove stale process guidance and verify the new infrastructure.

Stop condition: Code/docs changed, focused build/stage/proof run, and final answer reports passed/failed/deferred mechanisms.

## Implementation Summary

- `UT66HeroPhysicsComponent` now keeps capsule gameplay authority and synchronizes the active skeletal mesh component transform to the capsule-owned active relative transform during tick/reaction.
- The pelvis body is required to be simulating before `ApplyPhysicsReaction` consumes the hit cooldown.
- Obstacle reactions apply simulated-body impulse plus bounded capsule shove. The TestRoom wipeout arm routes active-first and logs legacy fallback separately.
- Normal runtime no longer performs `EmergencyPelvisResync` body teleport as the movement-stability answer. When divergence is detected, the component strengthens/logs the anchor path instead.
- Pelvis-distance telemetry now reports raw, component-space, and component-bone candidates and uses the effective pelvis location closest to the capsule hip anchor.
- `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heroactiveragdollproof` injects `-T66AutomationTestRoom` and `-T66AutoCaptureHeroHPOverride=20000`.
- Physics docs now state the standing authority model and explicitly reject actor-to-pelvis follow, `SimulationUpdatesComponentTransform`, and normal-runtime pelvis/body teleport loops.
- Pending issues now record that subjective Fall-Guys-like feel and proof-camera framing are still not production-final.

## Important Files

- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
- `Scripts/CaptureT66GameplayVideo.ps1`
- `Gameplay/Physics/README.md`
- `Gameplay/Physics/MASTER_PHYSICS.md`
- `Gameplay/Physics/HeroPhysicsModel.md`
- `Gameplay/Physics/PhysicsReactionProfiles.md`
- `Gameplay/Physics/PhysicsAssetPipeline.md`
- `Gameplay/Physics/PHYSICS_AGENTS.md`
- `Source/T66/Gameplay/Physics/pending_issues_Physics.md`
- `Source/T66/Gameplay/pending_issues_Gameplay.md`
- `Reports/Proof/Physics/HeroRagdollStage3AuthorityRebuild/stage3_authority_rebuild_proof_summary.md`

## Verification

Focused editor build:

```text
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66Editor Win64 Development -Project="C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
Result: Succeeded / target up to date
```

Primary obstacle proof:

```text
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\heroactiveragdollproof_reaction_testroom_high_no_runtime_resync.mp4
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\frames_reaction_testroom_high_no_runtime_resync\
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\T66_heroactiveragdollproof_high_no_runtime_resync.log
```

MP4 probe:

```text
width=1280
height=720
r_frame_rate=16/1
duration=6.000000
nb_frames=96
```

Primary obstacle log markers:

```text
PelvisSimulating=1
Reaction Applied=1 Source=TestRoomWipeoutArm
ActiveTried=1 ActiveApplied=1 LegacyApplied=0
State Balanced -> KnockedDown
State KnockedDown -> Recovering
State Recovering -> Balanced
```

Zero-match scan on primary obstacle proof log for:

```text
EmergencyPelvisResync|PelvisResync Reason=EmergencyPelvisResync|Fatal error|Access violation|Assertion failed
```

Supplemental movement telemetry:

```text
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\hero_movementqa_active_ragdoll_no_runtime_resync_60f.mp4
C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\T66_hero_movementqa_active_ragdoll_no_runtime_resync_60f.log
```

Movement telemetry shows active mesh/capsule sync after the MovementQA harness teleport, no emergency resync/fatal/assertion markers, and later settles back to `PelvisCapsuleDist=84.1`. It still logs seven `Pelvis divergence ... anchor strengthened without runtime body teleport` warnings during the harness teleport transition.

Staged standalone:

```text
& C:\UE\T66\Scripts\StageStandaloneBuild.ps1
BUILD SUCCESSFUL
Standalone build ready at C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe
```

Shortcut verification:

```text
C:\UE\T66\T66 Standalone.lnk -> C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe TargetExists=True
C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk -> C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe TargetExists=True
```

## Known Caveats

- The TestRoom proof camera is still not good subjective-feel evidence: the arm can occlude Hero 1 during the contact window and the shove can move the hero out of the best view. This is now recorded as a gameplay pending issue.
- The MovementQA proof is telemetry-first, not visual acceptance, because its camera framing is weak and the route can be contaminated by enemies. It is useful for proving no runtime body teleport loop, not for judging final feel.
- The Fall-Guys-like feel is intentionally still first-pass. PhysicsAsset/body mass/inertia, drive strengths, recovery timing, obstacle geometry/speed, and camera proof all need tuning after this authority rebuild.

## Draft Final Position

The Stage 3 authority model is rebuilt enough to test in-game. It now has one gameplay authority, a simulated pelvis/body chain, active mesh/capsule synchronization, active-first obstacle routing, and proof that the old flattening/origin-divergence emergency-resync loop is not the normal runtime path. This is infrastructure-complete but feel-partial.
