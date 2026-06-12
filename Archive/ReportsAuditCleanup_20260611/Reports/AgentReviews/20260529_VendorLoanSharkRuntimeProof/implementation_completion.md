# Vendor Failed-Steal & Loan Shark Debt Runtime Proof — Implementation Completion

Status: COMPLETE — both proofs PASS on the staged standalone binary, clean process exit (0).

## Staged binary under test

- Path: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- SHA256: `b63678827d40a8ea48b75053b8b120cf0d2fc3e1faa42c6fbd3ac6803571fde4`
- SHA record: `logs/staged_t66_exe_sha256.txt`
- Both proof captures below were run on this exact binary (no rebuild between SHA capture and runs).

## Build & stage

- Editor compile check: `Build.bat T66Editor Win64 Development` → Succeeded (`logs/build_t66editor.log`).
- Standalone stage: `RunUAT BuildCookRun -project=T66.uproject -noP4 -platform=Win64 -clientconfig=Development -stage -pak -package -build -cook` → BUILD SUCCESSFUL, ExitCode=0 (`logs/stage_standalone.log`).

## Proof invocation

Each mode launched on the staged `T66.exe`:

```
-T66Entry=Run:Tower -T66GameplayAutoCapture=<mode> -unattended -nop4 -nosplash -forcelogflush -abslog=<log>
```

## Vendor failed-steal proof — PASS

`logs/vendorfailedstealproof.log`

```
[VendorFailedStealProofSummary] Terminal=1 StealAttempted=1 StealFailed=1 VendorBossSpawned=1 VendorBossIdentity=1 VendorBossDefeated=1 VendorTokenDropped=1 HiddenBossCount=1 CasinoAngerSpawnedBoss=0 Pass=1
FPlatformMisc::RequestExitWithStatus(0, 0, VendorFailedStealProofComplete)
```

- Process exit code: 0.
- All required markers present and correct: `StealAttempted=1`, `StealFailed=1`, `VendorBossSpawned=1`, `VendorBossIdentity=1` (BossID == `VendorBoss`), `VendorBossDefeated=1`, `VendorTokenDropped=1`, `HiddenBossCount=1`, `CasinoAngerSpawnedBoss=0`, `Pass=1`.

## Loan Shark debt proof — PASS

`logs/loansharkdebtproof.log`

```
[LoanSharkDebtProofSummary] Terminal=1 DebtSet=1 PendingSet=1 LoanSharkSpawned=1 ChasingHero=1 LowDebtSpeed=700.0 HighDebtSpeed=950.0 LowDebtDamageHearts=1 HighDebtDamageHearts=4 SpeedScaledWithDebt=1 DamageScaledWithDebt=1 TouchDamageApplied=1 LoanSharkDespawned=1 Pass=1
FPlatformMisc::RequestExitWithStatus(0, 0, LoanSharkDebtProofComplete)
```

- Process exit code: 0.
- All required markers present and correct: `DebtSet=1`, `PendingSet=1`, `LoanSharkSpawned=1`, `ChasingHero=1`, low/high speed (700.0 / 950.0), low/high damage hearts (1 / 4), `SpeedScaledWithDebt=1`, `DamageScaledWithDebt=1`, `TouchDamageApplied=1`, `LoanSharkDespawned=1`, `Pass=1`.
- Touch damage is corroborated by two `[CombatDamage] Delivery=LoanSharkTouch` lines: the shark's natural spawn overlap (DamageHearts=1, HP 100→80) and the explicit proof hit after the invuln reset (DamageHearts=4, HP 80→3).

## Diagnosis notes (issues found and fixed during bring-up)

1. **Vendor boss never died / token never dropped.** `TakeDamageFromHeroHit` resolves to one hit-zone part per call and the boss only dies (running `Die()`, which drops `Item_VendorToken`) once every part is gone. Fix: `ForceAwaken()` the freshly spawned boss (damage is rejected while `bAwakened==false`), then loop `TakeDamageFromHeroHit` until it returns true.
2. **`CasinoAngerSpawnedBoss` false positive.** Original code counted every non-vendor `AT66BossBase`, which flagged the run's legitimate stage boss. Fix: count only `AT66VendorBoss` — pre-spawn count → `CasinoAngerSpawnedBoss` (expect 0), post-spawn count → `HiddenBossCount` (expect 1).
3. **`TouchDamageApplied=0`.** The hero was never in a safe zone (an early teleport hypothesis was wrong and self-defeating — teleporting onto the shark fired a real overlap touch that armed the 0.75s post-hit invuln window, blocking the measurement). Root cause: the shark's spawn overlap already lands a touch this tick. Fix: added a non-shipping `UT66RunStateSubsystem::AutomationResetDamageInvuln()` and clear the invuln window immediately before the explicit touch measurement.

## Files changed (test-only / non-shipping)

- `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp` — added `#include "Gameplay/T66VendorBoss.h"`; added `vendorfailedstealproof` and `loansharkdebtproof` dispatch blocks to the existing `#if !UE_BUILD_SHIPPING` AutoQA next-tick scheduler.
- `Source/T66/Gameplay/T66LoanShark.h` / `.cpp` — `#if !UE_BUILD_SHIPPING` automation accessors (`AutomationRefreshTuningFromDebt`, `AutomationGetCurrentDamageHearts`, `AutomationGetMaxWalkSpeed`, `AutomationIsChasingHero`, `AutomationApplyTouchDamageToHero`).
- `Source/T66/Core/T66RunStateSubsystem.h` — `#if !UE_BUILD_SHIPPING` `AutomationResetDamageInvuln()` accessor.

No production behavior changed: all proof logic and accessors are compiled out of shipping builds and only run when the matching `-T66GameplayAutoCapture=` CLI arg is present. No data/config/content edits other than the normal generated build/cook/stage outputs. No git operations performed.
