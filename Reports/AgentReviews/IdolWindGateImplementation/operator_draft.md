# Operator Draft: Idol Wind Gate Implementation

## Task Contract

Operator: Codex
Validator: Claude
Scope: Implement the revised idol/tower/gate flow: keep Pierce, add Wind and WindPower parity, 20 idol stock, reduce equipped idol slots to 3, four-floor tower with boss floor included, gate-adjacent weapon/idol selection unlocks, No Idol and No Weapon both valid unlock selections, minibosses remain but no longer drop idols, boss no longer drops idols.
Stop condition: Code/data changes are made, required data reload/build/runtime verification is attempted, and skipped proof is reported.

## Implemented

- Kept Pierce as an idol category and added Wind as a fifth idol element with DOT/AOE/Pierce/Bounce rows in `Content/Data/Idols.csv`.
- Added WindPower as an elemental secondary stat and wired it through stat serialization, backend parsing, run summary UI, boost/drop pools, saint blessing elemental pool, localization, and outgoing traveler visual profile colors.
- Reduced equipped idol slots from 4 to 3 while preserving legacy idol ID migration. Backend/run-summary parsing truncates equipped idol metadata to the normalized 3-slot shape.
- Increased idol stock/altar display from 16 to 20, with Wind idols included and No Idol retained.
- Changed tower layout to 4 floors total including boss floor, with miniboss floors on floors 2 and 3.
- Changed tower descent gates so every descent gate requires an explicit idol altar selection. First-floor descent also requires an explicit weapon altar selection.
- Linked gate-adjacent idol altars to their descent gates. Linked the first-floor weapon altar to the first descent gate.
- Removed miniboss idol reward spawning and boss idol reward spawning.
- Added `Weapon_NoWeapon` to weapon data and generation script. It is white, `SingleTarget`, and distinct from not selecting a weapon.
- Added `ET66AttackCategory::SingleTarget`; it has no AOE/Pierce/DOT/Bounce substat scaling, uses a white placeholder projectile, and the fallback single-target attack path is hard-capped to one target.

## Verification

- Ran `Scripts/SetupIdolsDataTable.py` through `UnrealEditor-Cmd`; exit 0.
- Ran `Scripts/SetupWeaponsDataTable.py` through `UnrealEditor-Cmd`; exit 0.
- Data sanity check: `Idols.csv` has 20 rows with four Wind idol IDs; `Weapons.csv` has 49 rows and one `Weapon_NoWeapon` row with `Branch=SingleTarget`.
- Focused editor build:
  `Build.bat T66Editor Win64 Development -Project=C:\UE\T66\T66.uproject -WaitMutex`
  Result: succeeded.
- Editor run-summary roundtrip:
  `Saved/Automation/run_summary_roundtrip_wind_gate.json`
  Result: ok true, schema 24, three equipped idol slots, WindPower round-trip, No Idol round-trip, legacy idol truncation/migration.
- Editor miniboss traversal proof:
  `Saved/Automation/miniboss_traversal_wind_gate.log`
  Result: `Pass=1`, floors 2 and 3 guardian gates block while alive and unlock after defeat.
- Editor content-corrections smoke:
  `Saved/Automation/content_corrections_wind_gate.log`
  Task-relevant check `BossRewardLayoutSeparatedOneGate` passed. Full smoke still failed the pre-existing documented `SafeZoneVisualBubblePresent` issue in `Source/T66/Gameplay/pending_issues_Gameplay.md`.
- Refreshed staged standalone with `Scripts/StageStandaloneBuild.ps1`; build successful.
- Verified both desktop and taskbar `T66 Standalone.lnk` shortcuts target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged run-summary roundtrip:
  `Saved/Automation/staged_run_summary_roundtrip_wind_gate_final.json`
  Result: ok true, schema 24, three enriched idol slots, No Idol, WindPower values, and weapon structural data round-tripped.
- Staged miniboss traversal proof:
  `Saved/Automation/staged_miniboss_traversal_wind_gate_final.log`
  Result: `Pass=1`, floors 2 and 3 guardian gates block while alive and unlock after death, with `RequestExitWithStatus(0, 0, MinibossTraversalProofComplete)`.

## Caveats

- The first descent gate's dual requirement, weapon selection and idol selection, is enforced in `AT66TowerDescentHole::CanOpenGate` and configured in `T66GameMode_Tower.cpp`, but the runtime proof captured is the miniboss-traversal log, which exercises idol-gate blocking on floors 2 and 3, not the floor-1 weapon plus idol combination. That specific path is covered structurally and by the staged build, not by a bespoke capture.
- Wind uses temporary parity references where no dedicated Wind art/icon exists yet.
