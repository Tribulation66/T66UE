# Hero Stat Rescale Operator Draft

## Task Contract

- Operator: Codex
- Validator: Claude
- Scope: Implement the clarified stat rescale without multiplying displayed stats by 100. Keep the 1-99 displayed stat model for now. Make Speed convert to 100 uu/s per displayed Speed point, so Hero_1 BaseSpeed=2 resolves to 200 uu/s. Collapse hero per-level gains to fixed values in the 1-5 range and dampen the gameplay coefficients that were making small stat increases too strong.
- Stop condition: Code/data/docs updated, DataTables refreshed, focused compile and runtime proof attempted, staged standalone readiness attempted, and Claude validation incorporated.

## Implementation

- Speed conversion:
  - `Source/T66/Gameplay/Movement/T66HeroMovementComponent.cpp`
    - `T66HeroWalkSpeedUnitsPerSpeedPoint = 100.f`.
    - Added `T66HeroMinimumWalkSpeed = 100.f`.
    - Walk-speed fallback/clamps now use 100 uu/s minimum and retain 10000 uu/s upper clamp.
  - `Source/T66/Gameplay/Movement/T66HeroMovementComponent.h`
    - fallback `BaseWalkSpeed = 200.f`.

- Fixed per-level hero stat gains:
  - `Content/Data/Heroes.csv`
    - All `Lvl*Min` and `Lvl*Max` pairs are fixed equal values in the 1-5 range, derived from each hero's current base stat weight and clamped to 1-5.
    - Hero_1 remains `BaseSpeed=2`, with fixed `LvlSpeedMin=2` and `LvlSpeedMax=2`.
  - `Content/Data/DT_Heroes.uasset`
    - Refreshed via `Scripts/ImportHeroDataTable.py`.
  - Fallback/default data updated in:
    - `Source/T66/Core/T66GameInstance.cpp`
    - `Source/T66/Data/T66DataTypes.h`
    - `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`

- Dampened gameplay coefficients:
  - `Source/T66/Core/RunState/T66RunStateSubsystem_Stats.cpp`
    - Introduced named stat coefficient constants.
    - Primary stat impact reduced:
      - Damage: 0.0015 per point above 1
      - AttackSpeed: 0.0012 per point above 1
      - AttackScale: 0.0008 per point above 1
      - Accuracy: 0.0010 per point above 1
      - Armor reduction: 0.0008 per point above 1
      - Evasion: 0.0006 per point above 1
    - Secondary/proc/range/luck coefficients reduced roughly 10x:
      - common chance: 0.001
      - small chance: 0.0005
      - attack range: 2.5 uu per bonus point
      - movement multiplier: 0.002
      - elemental power: 0.005
      - interactable luck quality tilt: 0.005
      - stealing/gambling luck: 0.001
      - proc luck: 0.0005
  - `Content/Data/PlayerExperience.json`
    - `HeadshotChancePerBonusPoint` changed from 0.005 to 0.0005 across all difficulties.
  - `Content/Data/DT_PlayerExperience.uasset`
    - Refreshed via `Scripts/SetupPlayerExperienceDataTable.py`.

- Documentation updated:
  - `Gameplay/Stats/MASTER_STATS.md`
  - `Gameplay/Stats/MASTER_PLAYER_EXPERIENCE.md`
  - `Gameplay/Movement/MASTER_MOVEMENT.md`
  - stale code comments in `T66GameInstance.h` and `T66RunStateSubsystem.h`.

## Verification Performed

- Static consistency:
  - `rg` for stale 840-speed/range wording against touched stats/movement/data surfaces: no task-relevant stale hits.
  - CSV validation:
    - 12 hero rows checked.
    - No fixed-gain problems.
    - No base display range problems.
    - Hero_1 BaseSpeed = 2.0.
  - `git diff --check` on task-touched files: no whitespace errors, only line-ending warnings.

- Build:
  - `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex -NoHotReload`
  - Result: succeeded.
  - Existing warning: deprecated Niagara `FNiagaraEmitterInstance::IsReadyToRun` in `T66Hero1AxeAOEVFXLabActor.cpp`.

- DataTable reload:
  - `ImportHeroDataTable.py`
    - `DT_Heroes` imported with 0 problems and saved.
    - Log: `Saved/Logs/HeroStatRescale_ImportHeroDataTable.log`.
  - `SetupPlayerExperienceDataTable.py`
    - `DT_PlayerExperience` imported with 0 problems and saved.
    - Log: `Saved/Logs/HeroStatRescale_SetupPlayerExperienceDataTable.log`.

- Runtime speed proof:
  - `Scripts/CaptureT66GameplayVideo.ps1 -CaptureMode heromovementqa ... -T66Hero=Hero_1 -T66HeroMovementQADisableMove -T66HeroMovementQADisableJump -T66HeroMovementQADisableLeap`
  - Output video: `Saved/Automation/HeroStatRescale/HeroMovementQA_Hero1.mp4`.
  - Fresh log: `Saved/Logs/T66.log`.
  - Evidence: repeated `[HeroMovementQA]` samples report `maxWalkSpeed=200.0` for Hero_1.

- Staged standalone readiness:
  - `Scripts/RunStagedBuildReadinessGate.ps1`
  - Output: `Saved/StagedBuildReadiness/20260608_150707`.
  - StageStandaloneBuild: PASS.
  - Staged executable exists: `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`.
  - ProjectRoot and pinned taskbar shortcut target checks: PASS.
  - PreRelease smoke:
    - `01_FrontendTagClick`: PASS.
    - `02_DurableSaveIntegrity`: PASS.
    - `03_LifecycleTransition`: FAIL/BUILD_CONFIG_UNSUPPORTED because `stress_population.mob_loot_spawned` was `0`, expected `6`.
  - This matches the existing out-of-scope pending issue in `Source/T66/Gameplay/pending_issues_Gameplay.md` for shelved mob loot in lifecycle stress, so no duplicate pending issue was added.

## Caveats

- The current implementation preserves the displayed 1-99 stat model; no UI stat x100 conversion was made.
- The staged readiness gate is not fully green because of the existing lifecycle stress mob-loot expectation, but the standalone stage, shortcuts, frontend smoke, durable smoke, editor build, DataTable reloads, and Hero_1 200 uu/s runtime speed proof all passed.
