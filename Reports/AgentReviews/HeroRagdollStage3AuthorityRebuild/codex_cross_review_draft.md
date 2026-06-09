# Codex Draft For Cross-Review - Hero Active-Ragdoll Stage 3 Authority Rebuild

## Task Contract

Operator: Codex
Validator: Claude

Scope: Rebuild Stage 3 Hero 1 active-ragdoll around the agreed authority model, update stale physics docs/process notes, and verify the new infrastructure in the playable/TestRoom path.

Stop condition: Code/docs changed, focused verification run, and final report states what passed, what failed, and what remains tuning work.

## Implemented Authority Model

- `UT66HeroPhysicsComponent` is the Hero 1 Stage 3 owner.
- Capsule remains gameplay/input/navigation authority.
- Skeletal mesh component transform is kinematic.
- Chaos simulates pelvis/body chain below the component.
- Hip anchor constraint binds simulated pelvis to capsule authority.
- Physical Animation Component is local child-body pose muscle, not gameplay root authority.
- Obstacle reaction applies mass-scaled simulated-body impulse plus bounded capsule shove.
- Old normal actor-to-pelvis follow and `SimulationUpdatesComponentTransform` root authority are avoided.
- Runtime distance debug now resolves effective pelvis location by comparing raw body coordinates against component-transformed coordinates to avoid false origin-divergence/flattening diagnosis.

## Main Files Changed

- `C:\UE\T66\Source\T66\Gameplay\Physics\T66HeroPhysicsComponent.h`
- `C:\UE\T66\Source\T66\Gameplay\Physics\T66HeroPhysicsComponent.cpp`
- `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_TestRoom.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66KnockbackComponent.h`
- `C:\UE\T66\Scripts\CaptureT66GameplayVideo.ps1`
- `C:\UE\T66\Gameplay\Physics\README.md`
- `C:\UE\T66\Gameplay\Physics\MASTER_PHYSICS.md`
- `C:\UE\T66\Gameplay\Physics\HeroPhysicsModel.md`
- `C:\UE\T66\Gameplay\Physics\PhysicsAssetPipeline.md`
- `C:\UE\T66\Gameplay\Traps\MASTER_TRAPS.md`
- `C:\UE\T66\Scripts\README.md`
- `C:\UE\T66\Source\T66\Gameplay\Physics\pending_issues_Physics.md`
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- `C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\stage3_authority_rebuild_proof_summary.md`

## Verification

- Focused editor build passed:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReloadFromIDE`
- Staged standalone refreshed successfully:
  - `C:\UE\T66\Scripts\StageStandaloneBuild.ps1`
  - Final staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Both `C:\UE\T66\T66 Standalone.lnk` and pinned taskbar shortcut target that exe.
- Unreal-owned proof video:
  - `C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\heroactiveragdollproof_reaction_testroom_final_framed.mp4`
  - `ffprobe`: 1280x720, 16 fps, 6.0s, 96 frames.
- Final proof frames:
  - `C:\UE\T66\Reports\Proof\Physics\HeroRagdollStage3AuthorityRebuild\frames_reaction_testroom_final_framed\`
- Final proof log gates in `C:\UE\T66\Saved\Logs\T66.log`:
  - `-T66AutomationTestRoom` and `-T66AutoCaptureHeroHPOverride=20000` were injected by `heroactiveragdollproof`.
  - TestRoom direct entry active.
  - Wipeout arm scheduled.
  - `Reaction Applied=1 Source=TestRoomWipeoutArm`.
  - `ActiveTried=1 ActiveApplied=1 LegacyApplied=0`.
  - TestRoom legacy PAC fields now log as `LegacyProfilePAC=0 LegacyDriveMode=0`.
  - State transitions include `Balanced -> KnockedDown`, `Recovering`, and return to `Balanced`.
  - Effective `PelvisCapsuleDist` remains bounded in samples despite raw body distance reading high due component-relative body readback.
  - No `PelvisCapsuleDistance exceeded`, active-ragdoll resync, `PlayerDied`, or fatal markers were found in the final proof scan.

## Known Remaining Risk

This completes the Stage 3 authority/infrastructure rebuild, not final Fall-Guys-like feel tuning. The remaining work is design tuning: PhysicsAsset body shapes/mass/inertia, hip-anchor stiffness/limits, pose-drive strength, rebound, get-up timing, and proof camera framing.

## Draft Final Position

The Stage 3 authority model is rebuilt and verified enough to test in the game. The old flattening/origin-divergence class is addressed by keeping capsule authority, kinematic component transform, simulated pelvis anchored to the capsule, local child-body PAC, bounded capsule shove, and effective pelvis-distance diagnostics. The docs now route future work through `Gameplay/Physics` and the `heroactiveragdollproof` proof path. The result is stable infrastructure, not a final feel pass.
