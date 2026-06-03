# Miniboss Placed Encounter Combined Packet

Date: 2026-05-28

## Working Goal

Replace random per-wave miniboss promotion with deliberate placed Slime miniboss encounters on the tower descent holes for floor transitions 2->3, 3->4, and 4->5. Minibosses remain rich actors, gate the exit hole while alive, and are excluded from ordinary trickle-wave promotion.

## Review Gate

- Plan packet: `C:\UE\T66\Reports\AgentReviews\20260528_MinibossPlacedEncounter\plan_packet.md`
- Claude pass 1: `C:\UE\T66\Reports\AgentReviews\20260528_MinibossPlacedEncounter\20260528T030848-pass1\claude_review_pass1.md`
  - Verdict: REVISE
  - Main correction: the floor-transition owner is `AT66TowerDescentHole`, not `AT66StageGate`; spawn timing should be floor-entry, not stage-start.
- Claude pass 2: `C:\UE\T66\Reports\AgentReviews\20260528_MinibossPlacedEncounter\20260528T031149-pass2\claude_review_pass2.md`
  - Verdict: APPROVE
  - Accepted caveat: the live tower map maps the locked floor 4 encounter to the 4->5 boss-floor entry. This was kept because Pablo explicitly locked floors 2, 3, and 4.

## Implementation Summary

### Random Promotion Disabled

- File: `C:\UE\T66\Source\T66\Gameplay\T66EnemyDirector.cpp`
- Random wave miniboss promotion now leaves `MiniBossIndex = INDEX_NONE`.
- Result: no ordinary director slot can become a miniboss through the old `MiniBossChancePerWave` roll.

### Placed Miniboss Rule

- File: `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp`
- Placed miniboss constants:
  - MobID: `Slime`
  - HP scalar: `3.0`
  - Damage scalar: `2.0`
  - Actor scale: `1.75`
- `IsPlacedTowerMinibossFloor()` returns true only for normal tower gameplay floors 2, 3, and 4.
- `EnsurePlacedTowerMinibossForFloor()` spawns one placed miniboss when the player enters an eligible floor.
- `HandleTowerDescentHoleTriggered()` calls the placed-spawn helper before unpausing director spawning for the entered floor.

### Door Gating

- Files:
  - `C:\UE\T66\Source\T66\Gameplay\T66TowerDescentHole.h`
  - `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp`
- The existing `AT66TowerDescentHole` guardian gate is reused.
- Descent holes for floors 2, 3, and 4 are created with `bRequiresGuardianDefeated=true`.
- The placed Slime is assigned with `SetGuardianEnemy()`.
- On guardian death, `HandleTowerGateGuardianDefeated()` records the floor defeated and preserves the existing idol altar side effect.

### State Reset

- File: `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_MainMap.cpp`
- `TowerPlacedMinibossSpawnedFloors` and `TowerPlacedMinibossDefeatedFloors` reset with tower descent hole state during main-map cleanup/regeneration.

### Documentation

- File: `C:\UE\T66\Gameplay\World\T66_MAP_DESIGN_REFERENCE.md`
- Added placed-miniboss descent gate rules, floor mapping, placeholder Slime model/scaling, spawn timing, boss-rush finale exclusion, and random-promotion removal.

- File: `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- Updated the route-leakage/Ranged acceptance issue to note that family-neutral random miniboss promotion is replaced by deliberate placed encounters.

## Verification

### Narrow Diff Hygiene

Command:

```powershell
git diff --check -- Source/T66/Gameplay/T66TowerDescentHole.h Source/T66/Gameplay/T66GameMode.h Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp Source/T66/Gameplay/GameMode/T66GameMode_MainMap.cpp Source/T66/Gameplay/T66EnemyDirector.cpp Gameplay/World/T66_MAP_DESIGN_REFERENCE.md Source/T66/Gameplay/pending_issues_Gameplay.md
```

Result:

- Exit code: `0`
- Only line-ending normalization warnings were reported; no whitespace errors.

### Focused Build

Command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex
```

Result:

- Exit code: `0`
- Known existing warnings:
  - `T66Mini.Build.cs` references missing `Source\T66Mini\Public\UI\Components`.
  - Niagara C4996 warning in `T66Hero1AxeAOEVFXLabActor.cpp`.

### Staged Standalone

Command:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1
```

Result:

- Exit code: `0`
- Staged exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- SHA256: `0C3B7A308A67C6C08BAC72E3957CE967461731D0030447A9868870174174216E`
- Shortcut target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Shortcut args: `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush`

### Gameplay Smoke

Command shape:

```powershell
T66.exe -T66Entry=Run:Tower -T66GameplayAutoCapture=rangedsmoke -T66MobUseLightweight=0 -T66GameplayAutoScreenshot="C:\UE\T66\Saved\Codex\Gameplay\MinibossPlacedEncounter\floor2_miniboss_smoke.png" -T66GameplayAutoScreenshotDelay=1 -T66GameplayAutoPostCaptureScreenshotDelay=1 -abslog="C:\UE\T66\Saved\StandaloneLogs\T66_MinibossPlacedEncounter_Smoke.log" -forcelogflush
```

Artifacts:

- Log: `C:\UE\T66\Saved\StandaloneLogs\T66_MinibossPlacedEncounter_Smoke.log`
- Screenshot: `C:\UE\T66\Saved\Codex\Gameplay\MinibossPlacedEncounter\floor2_miniboss_smoke.png`

Key evidence:

- Direct entry reached gameplay.
- Floor 2 placed miniboss spawned:
  - `Tower placed miniboss spawned floor=2 mob=Slime hp=150 scale=1.75`
- Ranged smoke still passed projectile travel and hero damage assertions:
  - `ProjectileTravelAssertion ... result=PASS`
  - `HeroDamageAssertion ... result=PASS`
- Process exited cleanly via automation exit request.

Note: the screenshot was written, but the existing ranged smoke camera is not door-focused. The log is the reliable proof for placed-miniboss spawn in this run.

### Wave Route Sanity

Command shape:

```powershell
T66.exe -T66Entry=Run:Tower -T66GameplayAutoCapture=enemywaveperf -T66MobUseLightweight=1 -T66AutoCaptureHeroHPOverride=20000 -T66RangedDiagnosticLogging=1 -T66GameplayAutoScreenshot="C:\UE\T66\Saved\Codex\Gameplay\MinibossPlacedEncounter\route_sanity.png" -T66GameplayAutoScreenshotDelay=1 -T66GameplayAutoPostCaptureScreenshotDelay=30 -abslog="C:\UE\T66\Saved\StandaloneLogs\T66_MinibossPlacedEncounter_RouteSanity.log" -forcelogflush
```

Artifacts:

- Log: `C:\UE\T66\Saved\StandaloneLogs\T66_MinibossPlacedEncounter_RouteSanity.log`
- Screenshot: `C:\UE\T66\Saved\Codex\Gameplay\MinibossPlacedEncounter\route_sanity.png`

Key `RouteAttributionSummary` evidence:

- `UseLightweight=1`
- `TotalObservedSpawns=91`
- `DirectorObservedSpawns=90`
- `MiniBossPromotionSlots=0`
- `SpecialOrGoblinSlots=0`
- `BossOrGuardianObserved=1`
- `MeleeRoutedLightweightBasic=27`
- `RushRoutedLightweightBasic=11`
- `FlyingRoutedLightweightBasic=17`
- `RangedRoutedLightweightBasic=35`
- `RangedRoutedRichMiniBossPromotion=0`
- `RangedRoutedRichFallbackBranch=0`
- `RangedRoutedRichNonDirectorPath=0`

Interpretation:

- All 90 director wave spawns route lightweight under CVar-on.
- The old random miniboss promotion path is absent from the wave.
- The single rich guardian observation is the deliberate placed Slime gate, not a wave-promotion leak.

## Coverage Notes

- Existing automation enters the first gameplay floor, so the staged smoke directly proves floor 2 placed spawn and route-contract cleanup.
- Floors 3 and 4 use the same `HandleTowerDescentHoleTriggered()` -> `EnsurePlacedTowerMinibossForFloor()` path and are selected by the same explicit floor predicate. They were verified by code path and build, not by a manual multi-floor playthrough in this pass.
- Boss-rush finale exclusion is implemented by the `IsBossRushFinaleStage()` guard in `IsPlacedTowerMinibossFloor()` and `EnsurePlacedTowerMinibossForFloor()`. It was verified by code path and build, not by a staged finale-stage smoke run.
- No new validation-only automation hook was added, to keep the implementation within the reviewed scope.

## Acceptance Status

- Random per-wave miniboss promotion disabled: PASS
- Fixed rich Slime placed miniboss implemented: PASS
- Door gating reuses `AT66TowerDescentHole` guardian flow: PASS
- Staged build and shortcut refresh: PASS
- Floor 2 staged spawn smoke: PASS
- Wave route sanity, no random miniboss promotion: PASS
- Manual/full-runtime floors 3 and 4 playthrough: NOT RUN
- Boss-rush finale staged runtime smoke: NOT RUN

## Next Follow-Up

If the team wants runtime proof for floors 3 and 4 plus boss-rush finale without manual play, add a small reviewed non-shipping automation mode that steps floor transitions, kills the placed guardian, asserts `Interact()` is blocked/unblocked, and runs a finale-stage no-miniboss check.
