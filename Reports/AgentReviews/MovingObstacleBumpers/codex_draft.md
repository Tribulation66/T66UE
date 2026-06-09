# Codex Draft: Moving Floor And Wall Bumpers

## Scope

Latest user request: make sure no trap is static. The floor bumper must move up and down, and the wall bumper must move out and back. The rotating arm and swinging axe/hammer are already acceptable. Simplify the prior bumper/launch-pad pair into a floor bumper and wall bumper, and make the change in the production trap actors, not only the TestRoom.

Operator: Codex
Validator: Claude

## Implementation

- Updated `Source/T66/Gameplay/Traps/T66ObstacleTrap.h` and `.cpp`.
  - `AT66BumperTrap` is now the production `ObstacleFloorBumper`.
  - It owns a `BumperMotionRoot`, ticks while enabled, and moves that root on local Z using a sine cycle.
  - It logs `[ObstacleBumperMotion] Type=ObstacleFloorBumper Axis=Up ...` on BeginPlay.
  - Added `AT66WallBumperTrap` as the production wall bumper.
  - It owns a static wall mount plus a moving `BumperMotionRoot`, ticks while enabled, and moves that root on local X using the same sine-cycle motion.
  - It logs `[ObstacleBumperMotion] Type=ObstacleWallBumper Axis=Forward ...` on BeginPlay.
  - Kept `AT66LaunchPadTrap` only as a legacy compatibility subclass of `AT66WallBumperTrap`.

- Updated production trap tuning in `Source/T66/Core/T66TrapTuningConfig.h`, `Source/T66/Core/T66TrapTuningConfig.cpp`, and `Config/DefaultT66TrapTuning.ini`.
  - Replaced live tuning entries with `ObstacleFloorBumper` and `ObstacleWallBumper`.
  - Mapped old `ObstacleBumper` config reads to `ObstacleFloorBumper`.
  - Mapped old `ObstacleLaunchPad` config reads to `ObstacleWallBumper`.
  - `SecondarySize` now controls bumper travel distance for both bumper variants; `SpeedOrPeriod` controls cycle period.

- Updated tower spawning in `Source/T66/Core/T66TrapSubsystem.cpp`.
  - Active obstacle pool is `ObstacleSweeperArm`, `ObstacleFloorBumper`, `ObstacleWallBumper`, and `ObstacleCeilingHammer`.
  - The obstacle pool is active only on tower floors 2 and 3.
  - Floor bumper uses tile-center placement.
  - Wall bumper uses `MazeWall` placement via `T66TowerMapTerrain::TryGetMazeWallSpawnLocation`, with actor forward pointed by the wall normal.
  - Spawn tuning applies movement period and travel distance to both bumper variants.

- Updated `Source/T66/Gameplay/GameMode/T66GameMode_TestRoom.cpp`.
  - Side rooms now include sweeper, floor bumper, wall bumper, and ceiling hammer.
  - The existing middle wipeout arm remains stationary per prior user request.
  - TestRoom startup logs all four side-room traps and the stationary middle trap.

- Updated `Gameplay/Traps/MASTER_TRAPS.md`.
  - Documents floor bumper and wall bumper as the live simplified bumper pair.
  - Documents that concrete obstacle traps should be moving, not static.

## Verification

- Focused editor build:
  - Command: `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReload`
  - Result: succeeded.
  - Caveat: existing unrelated C4996 Niagara warning in `T66Hero1AxeAOEVFXLabActor.cpp`.

- Staged standalone build:
  - Command: `& 'C:\UE\T66\Scripts\StageStandaloneBuild.ps1'`
  - Result: succeeded.
  - Staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
  - Shortcut verified: `C:\UE\T66\T66 Standalone.lnk` targets the staged exe.

- Packaged TestRoom runtime smoke:
  - Staged `T66.exe` launched with `-T66AutomationTestRoom` and a screenshot sequence.
  - Result: exit code 0.
  - Log: `C:\UE\T66\Saved\Logs\TrapMotionSmoke_20260607_142519.log`.
  - Frames: `C:\UE\T66\Saved\Automation\TrapMotionSmoke_20260607_142519`, 6 PNGs.
  - Log evidence:
    - `[ObstacleBumperMotion] Type=ObstacleFloorBumper Axis=Up Travel=300.0 Period=1.10 ...`
    - `[ObstacleBumperMotion] Type=ObstacleWallBumper Axis=Forward Travel=430.0 Period=1.10 ...`
    - `TestRoom side-room obstacle traps spawned: Sweeper=1 FloorBumper=1 WallBumper=1 CeilingHammer=1.`
    - `TestRoom stationary wipeout arm trap scheduled ...`

## Caveats

- The TestRoom screenshot sequence proves packaged startup and frame capture, but its camera did not visually frame the side-room bumpers. Motion proof is from the production C++ tick/update path plus runtime initialization logs.
- `Source/T66/Gameplay/Traps/T66ObstacleTrap.h` and `.cpp` are currently untracked in git, and normal `git status` hides untracked files because `status.showUntrackedFiles=no`. They are present on disk and were compiled into both editor and staged builds.
