# Trap Obstacle Implementation - Codex Draft

## Scope

User approved implementing the first four obstacle traps and integrating them into tower generation:

- Sweeper arm
- Bumper
- Launch pad
- Ceiling hammer

Additional requested behavior:

- Traps spawn randomly in the tower map.
- Traps spawn only on floors 2 and 3, not floors 1 or 4.
- Trap purpose is obstacle/ragdoll disable, not direct trap damage.
- Hero remains enemy-damageable while ragdolled/disabled/downed.

## Implementation Summary

- Added `AT66ObstacleTrapBase` plus `AT66SweeperArmTrap`, `AT66BumperTrap`, `AT66LaunchPadTrap`, and `AT66CeilingHammerTrap` in `Source/T66/Gameplay/Traps/T66ObstacleTrap.h/.cpp`.
- Registered the four obstacle traps in `UT66TrapSubsystem`, with floor pool restricted to tower floors 2 and 3.
- Added config-backed `FT66ObstacleTrapTuning` and obstacle entries in `Config/DefaultT66TrapTuning.ini`.
- Added obstacle placement helper in `T66TowerMapTerrain`, including generated-grid floor fallback so placement does not incorrectly shrink each walkable cell by outer-edge padding.
- Obstacle traps call `UT66HeroPhysicsComponent::ApplyPhysicsReaction` and do not apply trap HP damage.
- Changed ragdoll capsule handling in `UT66HeroPhysicsComponent` so the capsule remains `QueryOnly` using the `Pawn` collision profile during ragdoll, preserving the hero hurtbox for enemy projectile/touch damage paths while avoiding blocking movement.
- Updated trap and physics docs to describe obstacle trap behavior and downed/ragdoll hurtbox behavior.

## Files Changed

- `Config/DefaultT66TrapTuning.ini`
- `Gameplay/Traps/MASTER_TRAPS.md`
- `Gameplay/Physics/README.md`
- `Gameplay/Physics/CURRENT_STATE.md`
- `Gameplay/Physics/MASTER_PHYSICS.md`
- `Gameplay/Physics/PhysicsReactionProfiles.md`
- `Source/T66/Core/T66TrapSubsystem.cpp`
- `Source/T66/Core/T66TrapTuningConfig.h`
- `Source/T66/Core/T66TrapTuningConfig.cpp`
- `Source/T66/Gameplay/T66TowerMapTerrain.h`
- `Source/T66/Gameplay/T66TowerMapTerrain.cpp`
- `Source/T66/Gameplay/Physics/T66HeroPhysicsComponent.cpp`
- `Source/T66/Gameplay/Traps/T66ObstacleTrap.h`
- `Source/T66/Gameplay/Traps/T66ObstacleTrap.cpp`

## Verification

- Focused editor build passed:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -FromMsBuild`
- Staged standalone build passed:
  - `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`
  - staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Shortcut target verification passed:
  - `C:\UE\T66\T66 Standalone.lnk`
  - pinned taskbar `T66 Standalone.lnk`
  - both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Packaged runtime tower smoke passed:
  - log: `C:\UE\T66\Saved\AgentProofs\TrapObstacleImplementation\trap_spawn_packaged_final.log`
  - `ObstacleSweeperArm x2`
  - `ObstacleBumper x3`
  - `ObstacleLaunchPad x2`
  - `ObstacleCeilingHammer x3`
  - `[Traps] Spawned 10 floor-driven traps for tower stage 1 on Floor 2 x6, Floor 3 x4.`
  - `MinibossTraversalProofSummary ... Pass=1`
- `git diff --check` on scoped changed files exited 0 with only line-ending warnings.

## Known Caveats

- The runtime proof confirms tower spawn integration and floor restriction through logs. It does not visually film each trap hitting the hero.
- Downed/ragdoll damageability was verified by code path and compile: enemy projectile/touch paths that use the hero capsule now have a query-only capsule during ragdoll. No dedicated enemy-hit-while-ragdolled automation hook was added in this scope.
- Worktree contains unrelated dirty/untracked files; no unrelated changes were reverted.
