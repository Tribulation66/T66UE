# Codex Operator Draft - Hero Active Ragdoll Stage 3

## Scope

Implemented the Stage 3 Hero 1 active-ragdoll MVP runtime ownership path:

- Added `UT66HeroPhysicsComponent` under `Source/T66/Gameplay/Physics`.
- Wired `AT66HeroBase` to own and initialize the hero physics component after visual application.
- Routed the TestRoom wipeout arm through active hero physics first, with legacy knockback retained as fallback.
- Expanded the existing ragdoll proof overlay mode to support `heroactiveragdollproof` and command-line proof positioning.
- Added a focused pending issue for the remaining Hero 1 PhysicsAsset/rig stability gap.

## Changed Files

- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.h`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- `Source/T66/Gameplay/Physics/pending_issues_Physics.md`
- `Source/T66/Gameplay/T66HeroBase.h`
- `Source/T66/Gameplay/T66HeroBase.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`
- `Source/T66/Gameplay/T66PlayerController_Overlays.cpp`

## Runtime Behavior Implemented

`UT66HeroPhysicsComponent` gates the MVP to Hero 1 Chad PhysicsFirst mesh usage, configures the skeletal mesh for query-and-physics collision, enables simulation below pelvis, applies a local PhysicalAnimationComponent pose drive, creates a hidden hip anchor and pelvis constraint, tracks Balanced/Staggered/KnockedDown/Recovering states, and exposes `ApplyPhysicsReaction` for obstacle impacts.

The TestRoom wipeout arm now attempts active ragdoll first. If active ragdoll is unavailable or rejected, it falls back to the existing knockback component.

## Verification

Focused editor compile passed:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex
```

Staged standalone build passed:

```powershell
& 'C:\UE\T66\Scripts\StageStandaloneBuild.ps1' -ClientConfig Development
```

The staged executable exists at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`, and `C:\UE\T66\T66 Standalone.lnk` targets that executable.

Runtime proof with Hero 1 override produced active-ragdoll routing and state transitions:

- Active component initialized on `SK_Hero_1_Chad_PhysicsFirst` with `PA_Hero_1_Chad_PhysicsFirst_Stage2Seed`.
- TestRoom impact log reported `ActiveTried=1`, `ActiveApplied=1`, and `LegacyApplied=0`.
- State machine logged `Balanced -> KnockedDown -> Recovering -> Balanced`.
- Evidence video: `C:\UE\T66\Saved\AgentReviews\HeroActiveRagdollStage3Implementation\hero_active_ragdoll_testroom_hero1_bodydistance.mp4`
- Evidence contact sheet: `C:\UE\T66\Saved\AgentReviews\HeroActiveRagdollStage3Implementation\TestRoomHero1BodyDistanceEvidence\contact_sheet.png`
- Evidence bundle: `C:\UE\T66\Saved\AgentReviews\HeroActiveRagdollStage3Implementation\TestRoomHero1BodyDistanceEvidence`

## Known Partial Result

This is a partial Stage 3 result with a major stability gap, not a fully acceptable Fall Guys-like feel. The architecture and active obstacle path are in place, but Hero 1's current PhysicsFirst PhysicsAsset/rig is not stable enough under impact. The contact sheet shows a visible active body and wipeout-arm interaction, but the mesh can collapse into a flattened/spiky silhouette, and logs still show body divergence/resync oscillation during recovery.

The next engineering step is a dedicated PhysicsAsset/rig tuning loop, not another knockback code pass.

## PPF Status

Declared method class is partially met. Runtime ownership, active simulated body, PAC drive, pelvis/hip anchor, obstacle impulse routing, and state recovery are present. Visual artifact stability and readable active-ragdoll body parity remain a major partial gap because the seed Hero 1 PhysicsAsset is not tuned enough yet.
