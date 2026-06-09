# FriendSlop Hero Knockback Infrastructure - Codex Draft

## Scope

User requested hero-only implementation for now, but with data/infrastructure shaped so bosses, elites, and other future actors can adopt the same path later.

## Implementation Summary

- Added `UT66KnockbackComponent` with `FT66KnockbackProfile`, budget class, phase enum, physical animation drive mode, skeletal-ragdoll path, fallback launch path, mass-scaled impulse application, delayed Physical Animation Component activation, actor-follow-to-ragdoll handling, and recovery/restore logic.
- Kept the current gameplay use hero-scoped by attaching the component to `AT66HeroBase`, exposing `IsKnockbackActive`, `IsKnockbackIncapacitated`, and `ApplyKnockbackLaunch`.
- Made the component operate on generic `ACharacter` ownership internally, while hero-only auto-attack suppression is applied only when the owner is actually `AT66HeroBase`. This keeps future boss/elite adoption from requiring a rewrite of the physics component.
- Routed TestRoom wipeout-arm impacts through the hero's knockback component instead of local TestRoom ragdoll state. TestRoom still supplies trap-specific profile overrides through cvars.
- Suppressed hero movement input, movement abilities, auto-attack, and raw `PlayAnimation` driving during active knockback so gameplay control and animation do not fight the physics state.
- Left horde/mob simulation out of scope; profile contains `ET66KnockbackBudgetClass::HordeFallback` for later cheap fallback integration.

## Key Files

- `Source/T66/Gameplay/T66KnockbackComponent.h`
- `Source/T66/Gameplay/T66KnockbackComponent.cpp`
- `Source/T66/Gameplay/T66HeroBase.h`
- `Source/T66/Gameplay/T66HeroBase.cpp`
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`
- `Source/T66/Gameplay/T66PlayerController_Movement.cpp`
- `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`

## Verification

- Focused editor build:
  - Command: `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex`
  - Result: succeeded.
  - Known unrelated warning remains in `T66Hero1AxeAOEVFXLabActor.cpp` for deprecated `FNiagaraEmitterInstance::IsReadyToRun`.
- Unreal-owned gameplay proof:
  - Command: `.\Scripts\CaptureT66GameplayVideo.ps1 -CaptureMode testragdollproof -Output .\Saved\AgentReviews\FriendSlopHeroRagdollInfrastructure\testragdoll_component_path_hero1_final.mp4 -FrameDir .\Saved\AgentReviews\FriendSlopHeroRagdollInfrastructure\frames_component_path_hero1_final -FramePrefix frame -FrameCount 120 -FrameRate 12 -CaptureIntervalSeconds 0.08 -DelaySeconds 4.0 -PostCaptureDelaySeconds 0.2 -TimeoutSeconds 260 -ExtraArgs @('-T66AutomationTestRoom','-T66Hero=Hero_1')`
  - Result: 120 frames, 1280x720, 12 FPS, 10.0 seconds.
  - Video: `C:\UE\T66\Saved\AgentReviews\FriendSlopHeroRagdollInfrastructure\testragdoll_component_path_hero1_final.mp4`
  - Frames: `C:\UE\T66\Saved\AgentReviews\FriendSlopHeroRagdollInfrastructure\frames_component_path_hero1_final`
  - Log confirms direct entry `Hero=Hero_1`.
  - Log confirms skeletal mesh `/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/SK_Hero_1_Chad_Male_FriendSlop`.
  - Log confirms `T66Knockback skeletal launch` with `RuntimeBodies=18`, `PhysicsBodies=18`, `PhysicsConstraints=15`, `PhysicsAsset=/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/Skeletal/PA_Hero_1_Chad_Male_FriendSlop_TestRoom`.
  - Log confirms `T66Knockback physical animation activation` with `PhysicalAnimation=1`, `DriveMode=2`, `DrivenBodies=6`.
  - Log confirms repeated post-restore impacts, recovery, and restore across the 10-second proof window.
  - Log also contains an unrelated startup `LogAutomationTest: Error` about FText serialization and the usual profiler DLL load warnings; the proof path itself did not fatal/assert.
- Staged standalone:
  - Command: `.\Scripts\StageStandaloneBuild.ps1`
  - Result: `BUILD SUCCESSFUL`, AutomationTool `ExitCode=0`.
  - Executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
  - Shortcut verification: both `C:\UE\T66\T66 Standalone.lnk` and the pinned taskbar shortcut target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Source hygiene:
  - Command: `git diff --check -- <touched source files>`
  - Result: no whitespace/check errors; Git only reported line-ending normalization warnings for touched files.

## Caveats / Next Tuning

- This pass builds the infrastructure and proves the final code uses skeletal ragdoll + PAC for Hero 1. It intentionally does not tune the final "feel" values; those remain user feel-tuning.
- The proof camera follows the actor/ragdoll enough to show gameplay continuity, but the arm can occlude the hero in some proof frames. If visual review becomes the next acceptance gate, tune the proof camera separately from gameplay behavior.
- Future bosses/elites should attach the same component and supply their own `FT66KnockbackProfile` or DataTable row; their non-hero attack/state suppression will still need a small owner-specific adapter similar to the hero auto-attack suppression branch.
