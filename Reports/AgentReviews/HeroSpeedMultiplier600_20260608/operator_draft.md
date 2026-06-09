# Hero Speed Multiplier 600 Operator Draft

## Scope

User tested the 2 Speed = 200 uu/s setting and found it too slow. Requested 2 Speed = 600 uu/s.

## Changes Made

- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`
  - Changed `T66HeroWalkSpeedUnitsPerSpeedPoint` from `100.f` to `300.f`.
  - Runtime formula remains `Max(1, SpeedStat) * UnitsPerSpeedPoint`, so displayed Speed 2 now resolves to 600 uu/s.
- `Source/T66/Gameplay/Movement/T66HeroMovementComponent.h`
  - Changed fallback `BaseWalkSpeed` from `200.f` to `600.f`.
- `Gameplay/Movement/MASTER_MOVEMENT.md`
  - Updated last-updated date, fallback, live formula, and current live numbers to `Speed * 300 UU/s`.
  - Corrected the live-number clamp line to the actual code clamp `[100, 10000]`.
- `Gameplay/Stats/MASTER_STATS.md`
  - Updated movement stat references to `300 UU/s` per Speed point.

No hero data, displayed stat values, fixed per-level gains, or coefficient dampening values were changed in this pass.

## Verification

- Stale-reference scan:
  - No remaining task-relevant `Speed * 100`, `BaseSpeed * 100`, `100 UU/s`, or `T66HeroWalkSpeedUnitsPerSpeedPoint = 100` hits in movement/stats source/docs.
- `git diff --check` on touched files:
  - No whitespace errors.
  - Only existing line-ending warnings.
- Focused build:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReload`
  - Result: succeeded.
- Runtime proof:
  - `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heromovementqa ... -T66Hero=Hero_1 -T66HeroMovementQADisableMove -T66HeroMovementQADisableJump -T66HeroMovementQADisableLeap`
  - Output: `Saved/Automation/HeroSpeedMultiplier600/HeroMovementQA_Hero1_600.mp4`
  - Fresh `Saved/Logs/T66.log` has repeated `[HeroMovementQA]` samples with `maxWalkSpeed=600.0` for Hero_1.
- Staged standalone readiness:
  - `Scripts/RunStagedBuildReadinessGate.ps1`
  - Output: `Saved/StagedBuildReadiness/20260608_154509`
  - StageStandaloneBuild: PASS.
  - Staged executable exists: `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`.
  - ProjectRoot and pinned taskbar shortcut checks: PASS.
  - Smoke suite:
    - `01_FrontendTagClick`: PASS.
    - `02_DurableSaveIntegrity`: PASS.
    - `03_LifecycleTransition`: FAIL/BUILD_CONFIG_UNSUPPORTED because `stress_population.mob_loot_spawned` was `0`, expected `6`.
  - This matches the existing out-of-scope pending issue in `Source/T66/Gameplay/pending_issues_Gameplay.md` for shelved mob loot in lifecycle stress.

## Operator Notes

- Claude independent answer said the arithmetic should be `300.f`, which matches the implementation.
- Claude also suggested docs may be a no-op because it did not find references, but live `Gameplay/Movement/MASTER_MOVEMENT.md` and `Gameplay/Stats/MASTER_STATS.md` did reference the old 100 uu/s scale and were updated.
