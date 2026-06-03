# B.10.1D Projectile Manager + HISM Rendering - Consolidated Packet

Date: 2026-05-27  
Working goal: replace actor-spawned enemy projectiles with a manager-owned flat-array projectile system rendered by HISM, wire both rich and lightweight Ranged through it, then run smoke plus 3-capture CVar-off/on acceptance.

## 1. Review Gate

Plan packet:

- `C:\UE\T66\Saved\AgentReviews\20260527T_b101d_projectile_manager_hism\plan_packet.md`

Claude review:

- `C:\UE\T66\Saved\AgentReviews\20260527T_b101d_projectile_manager_hism\20260527T071750-pass8\claude_review_pass8.md`
- Verdict: APPROVE

Accepted caveats and hard gates:

- Prove rich and lightweight Ranged both fire through the manager, render via HISM, hit the hero, and preserve damage source attribution.
- `DroppedFires > 0` is a hard fail.
- `ManagerTickMaxUs > 7500`, `HISMUpdateMaxUs > 5000`, or `HISMUpdateAvgUs > 1000` is a hard fail / follow-up.
- CVar-off Task 9 median must be at least `120.0 FPS`.
- CVar-on Task 10 median must be at least `95%` of the new CVar-off median.
- Zero non-zero exits, zero overhead rejects, stable staged-binary hash.
- Pre-capture clean environment and binary hash checks are mandatory.

## 2. Implementation Summary

New runtime system:

- Added `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.h`
- Added `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.cpp`
- `UT66ProjectileManagerSubsystem` owns enemy projectile state in a fixed 256-slot flat array.
- Each slot tracks active state, position, velocity, lifetime, radius, damage, source actor, cached source ID, projectile type index, and HISM instance index.
- Manager tick advances projectiles, performs hero capsule segment checks, performs optional non-hero/world sweep, applies hero damage through the existing run-state damage path, deactivates expired/hit projectiles, and batches HISM transform updates.

Wiring:

- Rich `AT66RangedEnemy` now calls `UT66ProjectileManagerSubsystem::FireProjectile`.
- Lightweight `AT66MobBase` Ranged now calls the same manager API.
- `AT66EnemyProjectileBase` is retained for asset/data compatibility and marked deprecated for future cleanup.
- Ranged aggregate diagnostics now record manager projectile outcomes.
- PerformanceSystem board samples include manager-owned active projectiles.

Important implementation note:

- Initial non-hero sweep testing showed false terminal impacts on trigger volumes such as `T66TowerDescentHole.TriggerBox`.
- Final manager filtering skips non-hero hits whose component ignores the projectile object channel, matching the old overlap-path behavior more closely.

## 3. Build And Stage

Build:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development 'C:\UE\T66\T66.uproject' -WaitMutex
```

Result: passed.

Known warning:

- Pre-existing `Source\T66Mini\T66Mini.Build.cs` warning about missing `Public\UI\Components`.

Stage:

```powershell
& 'C:\UE\T66\Scripts\StageStandaloneBuild.ps1' -ClientConfig Development -SkipBuild
```

Result: passed.

Shortcut verification:

- `C:\UE\T66\T66 Standalone.lnk` targets `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- pinned taskbar shortcut was refreshed to the same staged executable.

Final staged executable:

- Path: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- SHA256: `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B`
- Size: `320273408`
- Last write UTC: `2026-05-27T11:20:05.3619032Z`

## 4. Smoke Evidence

### Rich Ranged Smoke

Artifacts:

- Log: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1D\T66_B101D_RichRangedSmoke_Isolated_Final.log`
- Screenshot: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\projectile_manager_smoke\isolated_rich_final.png`

Result: passed.

Evidence:

- `ProjectileTravelAssertion managerFired=2 active=2 hitHero=0 hitWorld=0 expired=0 result=PASS`
- `HeroDamageAssertion initialHP=100.0 currentHP=80.0 result=PASS`
- `[CombatDamage] SourceID=HexSlinger Delivery=EnemyProjectile SourceClass=T66RangedEnemy`
- `[ProjectileManagerSummary] Fired=4 ActivePeak=2 HitHero=4 HitWorld=0 DroppedFires=0 ApplyDamageReturnedFalse=2 ManagerTickMaxUs=342.7 ManagerTickAvgUs=40.8 HISMUpdateMaxUs=60.3 HISMUpdateAvgUs=2.1`

### Lightweight Ranged Smoke

Artifacts:

- Log: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1D\T66_B101D_LightweightRangedSmoke_Isolated_Final.log`
- Screenshot: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\projectile_manager_smoke\isolated_lightweight_final.png`

Result: passed.

Evidence:

- `ProjectileTravelAssertion managerFired=1 active=1 hitHero=0 hitWorld=0 expired=0 result=PASS`
- `HeroDamageAssertion initialHP=100.0 currentHP=80.0 result=PASS`
- `[CombatDamage] SourceID=HexSlinger Delivery=EnemyProjectile SourceClass=T66MobBase`
- `[ProjectileManagerSummary] Fired=2 ActivePeak=1 HitHero=2 HitWorld=0 DroppedFires=0 ApplyDamageReturnedFalse=0 ManagerTickMaxUs=447.4 ManagerTickAvgUs=34.5 HISMUpdateMaxUs=83.0 HISMUpdateAvgUs=3.1`

Smoke conclusion:

- Rich and lightweight Ranged both fire through the manager.
- Projectiles render and move via HISM.
- Hero damage attribution works for both paths.
- No smoke row dropped fires.

## 5. Acceptance Attempt 1 - Incomplete Historical Evidence

This attempt started clean and produced useful rows, but the full acceptance matrix did not complete. The run later aborted after CVar-on capture 2 because two long-lived `git status --porcelain` processes exceeded the runner's original transient Git wait. These rows are not acceptance proof.

Executable hash:

- `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B`

Rows captured before abort:

| Set | Run | Terminal | Hero HP | Hero hits | Fired | Hit hero | Dropped | Avg FPS | Overhead us | Accepted |
| --- | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| CVarOff | 1 | Completed | 240 | 13 | 23 | 13 | 0 | 150.77 | 908.0 | Yes |
| CVarOff | 2 | Completed | 300 | 12 | 26 | 12 | 0 | 151.58 | 1186.8 | Yes |
| CVarOff | 3 | Completed | 200 | 19 | 21 | 19 | 0 | 171.52 | 741.2 | Yes |
| CVarOn | 1 | Completed | 400 | 6 | 9 | 6 | 0 | 172.25 | 714.0 | Yes |

Infrastructure correction:

- The validation runner now records status-only Git waits as non-fatal instead of aborting the pass.
- `git-lfs.exe` activity remains waited/recorded.
- The stale status processes were killed before the final clean rerun:
  - PID `11856`, created `2026-05-27 08:36:58`, `git.exe -c core.hooksPath=NUL -c core.fsmonitor=false status --porcelain`
  - PID `14196`, created `2026-05-27 08:36:58`, `git.exe -c core.hooksPath=NUL -c core.fsmonitor=false status --porcelain`
- Two fresh status workers later respawned during documentation verification and were also killed:
  - PID `31724`, created `2026-05-27 09:06:28`, `git.exe -c core.hooksPath=NUL -c core.fsmonitor=false status --porcelain`
  - PID `1664`, created `2026-05-27 09:06:28`, `git.exe -c core.hooksPath=NUL -c core.fsmonitor=false status --porcelain`
- The recurring source of background status workers remains unresolved and should be handled before the next acceptance-grade capture pass.

## 6. Acceptance Attempt 2 - Final Clean Rerun

Command:

```powershell
& 'C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1' -Mode Acceptance -AcceptanceCount 3 -HeroHPOverride 500
```

Result: blocked. CVar-off halted after two rejected captures, per standing methodology. CVar-on was not run because no valid CVar-off baseline was established.

Clean-environment/provenance:

- No `git-lfs.exe`, `git status`, `RunUAT`, `UnrealEditor-Cmd`, or staged `T66.exe` conflict was recorded during the final clean rerun.
- Pass-start, pre-capture, post-capture executable hashes matched: `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B`

Rows:

| Set | Run | Terminal | Reject reason | Hero HP | Hero hits | Projectile damage | Fired | Hit hero | Hit world | Rich LOS passed | Rich LOS blocked | Avg FPS | Overhead us | Manager max us | HISM max us |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOff | 1 | HeroDied | HeroDeath | 0 | 34 | 680 | 41 | 33 | 6 | 41 | 85 | 85.77 | 1529.7 | 709.6 | 630.0 |
| CVarOff | 2 | Completed | None | 460 | 2 | 40 | 7 | 2 | 5 | 7 | 1532 | 87.10 | 1434.7 | 360.5 | 91.7 |
| CVarOff | 3 | Completed | NoProjectilesFired | 500 | 0 | 0 | 0 | 0 | 0 | 0 | 3840 | 92.37 | 1318.2 | 0.0 | 0.0 |

Gate status:

| Gate | Status | Evidence |
| --- | --- | --- |
| Stable binary hash | Pass | `CF70BC...F20B` before/after every clean row |
| Clean process environment | Pass | no Git/LFS/RunUAT/UnrealEditor-Cmd/staged T66 conflicts during clean rerun |
| PerformanceSystem overhead under 10ms | Pass | max `1529.7 us` |
| Dropped fires | Pass | `DroppedFires=0` in all rows |
| Manager tick timing | Pass | max `709.6 us`, below `7500 us` gate |
| HISM update timing | Pass | max `630.0 us`, below `5000 us` gate |
| CVar-off baseline established | Fail | two rejected captures; set halted |
| CVar-on comparison | Not run | blocked by missing CVar-off baseline |

## 7. Analysis

What worked:

- The manager architecture replaces the rich `SpawnActor` failure path.
- Rich and lightweight Ranged can both produce visible, damaging projectiles via HISM.
- The manager timing gates are comfortably below thresholds.
- PerformanceSystem I/O mitigation stayed healthy; no overhead rejection occurred.

What blocked acceptance:

- The fixed HP500 stationary hero can die once rich Ranged projectiles are functional. The clean CVar-off run 1 delivered 34 hero projectile hits / 680 damage by `WorldTime=52.11`.
- Rich Ranged can also produce saturated no-fire captures. Clean CVar-off run 3 had `RichDistancePassed=3840`, `RichLOSBlocked=3840`, `RichLOSPassed=0`, and `Fired=0`.
- Because the CVar-off baseline is now both ranged-active and highly variable, the prior broken-rich comparators remain invalid, but this pass could not establish the replacement baseline.

Interpretation:

- B.10.1D successfully retires the actor-spawned enemy projectile path.
- The remaining blocker is not manager capacity, HISM rendering, or PerformanceSystem overhead.
- The remaining blocker is the measurement/gameplay contract for ranged-active rich captures: HP500 is sometimes too low, while rich LOS/positioning can also sometimes result in zero fired projectiles.

## 8. Documentation Updates

Updated:

- `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`

Gameplay pending issue update:

- Rich `SpawnActor` failure is resolved by architecture replacement.
- New blocker is ranged-active acceptance: HP500 hero death plus rich LOS/no-fire variability.

PerformanceSystem pending issue update:

- B.10.1D did not reproduce PerformanceSystem-owned overhead rejects.
- Git status contamination was identified and handled as infrastructure provenance.
- Final clean rerun failure was gameplay/measurement, not Git/LFS or PerformanceSystem overhead.

## 9. Recommended Next Packet

Do not re-run B.10 acceptance without first deciding the ranged-active measurement contract.

Recommended scope:

1. Keep `UT66ProjectileManagerSubsystem` as the enemy projectile architecture.
2. Decide whether to raise `T66.AutoCapture.HeroHPOverride` for ranged-active performance captures or revise the stationary saturated scenario.
3. Investigate/fix rich Ranged LOS/positioning variance enough that CVar-off cannot produce `NoProjectilesFired` captures under the chosen measurement contract.
4. Re-run 3-capture CVar-off/CVar-on acceptance after those gates are reviewed.

Out of scope for the next packet unless explicitly approved:

- Reverting the projectile manager.
- Patching the deprecated `AT66EnemyProjectileBase` actor spawn path.
- Changing lightweight Ranged pressure.
- B.11/B.12/B.13 work.

## 10. Bottom Line

B.10.1D implementation landed and smoke-passed. The acceptance gate did not pass. The next reviewed decision is measurement-contract plus rich LOS stability, not projectile-manager architecture.

## 11. Resume Attempt - HP2000 Measurement Contract Rerun

Review and go-ahead:

- Continuation packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume_measurement_contract\plan_packet.md`
- Claude review: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume_measurement_contract\20260527T091338-pass1\claude_review_pass1.md`
- Verdict: APPROVE

Command:

```powershell
& 'C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1' -Mode Acceptance -AcceptanceCount 3 -HeroHPOverride 2000
```

Preflight:

- Initial preflight found one `git-lfs.exe` worker and one `git status --porcelain` worker.
- `git-lfs.exe` drained before launch.
- The `git status --porcelain` worker PID `46904` persisted through a five-minute wait and was explicitly terminated before launch.
- Staged executable hash before launch remained `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B`.

Result:

- Halted. The first CVar-off row reproduced the rich LOS/no-fire blocker, and the runner then refused to proceed to capture 2 after `UnrealEditor-Cmd.exe` appeared in the clean-environment check.
- No CVar-off baseline was established.
- CVar-on was not run.

Row captured before halt:

| Set | Run | Terminal | Reject reason | Requested HP | Applied HP | Hero hits | Fired | Rich distance passed | Rich LOS passed | Rich LOS blocked | Avg FPS | Overhead us |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOff | 1 | Completed | NoProjectilesFired | 2000 | 1000 | 0 | 0 | 103 | 0 | 103 | 157.93 | 689.0 |

Key evidence:

- Command line and route flags correctly requested `T66AutoCaptureHeroHPOverride=2000`.
- `ApplyAutomationHeroHPOverride` capped the applied value to `1000.0`:
  - `AutoCaptureHeroHPOverride AppliedHP=1000.0 RequestedHP=2000.0 MaxHP=1000.0 CurrentHP=1000.0`
- The rich Ranged fire pipeline reached distance-passed checks but never passed LOS:
  - `RichDistancePassed=103`
  - `RichLOSBlocked=103`
  - `RichLOSPassed=0`
  - `RichProjectilesDispatched=0`
  - `ProjectileManagerSummary Fired=0`
- Framework overhead was healthy at `689.0 us`; this was not a PerformanceSystem overhead failure.

New findings:

- The measurement HP override is not actually capable of applying HP2000 in the current binary because `UT66RunStateSubsystem::ApplyAutomationHeroHPOverride` has a hard cap of `1000.f`.
- Rich LOS starvation reproduced immediately on the next reviewed rerun. Raising HP alone cannot establish the B.10.1D baseline.
- Current aggregate counters do not preserve blocker names, so the next fix packet needs enough bounded attribution to distinguish enemy-body occlusion, world geometry, trigger volumes, and hero/capsule targeting issues without reintroducing per-frame log spam.
- A transient `UnrealEditor-Cmd.exe` process appeared between captures and correctly stopped the runner's clean-environment gate. It was gone by inspection, but this is still capture-environment contamination and must remain a hard preflight concern.

Next reviewed fix scope:

1. Make `T66.AutoCapture.HeroHPOverride` able to apply the reviewed ranged-active measurement value, likely by raising or parameterizing the automation-only cap while keeping normal gameplay unchanged.
2. Add bounded aggregate LOS blocker attribution, not per-frame logs.
3. Fix rich Ranged LOS/occlusion behavior so CVar-off cannot produce `NoProjectilesFired` saturated captures.
4. Rebuild, stage, smoke rich/lightweight projectiles, then rerun B.10.1D acceptance.

## 12. Resume2 - Rich Ranged LOS Blocker Attribution Probe

Status: diagnostic implementation and three-probe CVar-off run completed. This pass does not change HP caps, LOS behavior, projectile sweep behavior, or B.10.1D acceptance status.

Review and go-ahead:

- Packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_hp_cap_los_fix\plan_packet.md`
- Claude review: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_hp_cap_los_fix\20260527T095018-pass7\claude_review_pass7.md`
- Verdict: `APPROVE`

Reviewed scope:

- Add bounded aggregate LOS blocker buckets for rich and lightweight Ranged.
- Preserve `T66.Ranged.DiagnosticLogging` as the diagnostics gate.
- Add a bounded clean-environment wait in the validation runner for transient `RunUAT`, `UnrealEditor-Cmd`, or staged `T66.exe`.
- Run short CVar-off probes to attribute the `NoProjectilesFired` mode.
- Stop before any HP-cap change, LOS/collision behavior fix, projectile sweep change, or B.10.1D acceptance rerun.

Implementation summary:

- `UT66MobManagerSubsystem::RecordRangedLosBlocked` now receives the existing LOS trace blocker actor/component and classifies it into fixed buckets: `WorldStatic`, `WorldDynamic`, `RichEnemy`, `LightweightMob`, `OtherPawn`, and `Unknown`.
- Both firing paths emit symmetric terminal summary fields, for example `RichLOSBlockerRichEnemy` and `LightweightLOSBlockerLightweightMob`.
- The classifier null-checks the component fallback before reading object type, satisfying the Claude minor caveat.
- Rich `AT66RangedEnemy::HasProjectileLineOfSightToPlayer` and lightweight `AT66MobBase::HasProjectileLineOfSightToHero` reuse their existing `LineTraceSingleByChannel` hit result; no second trace was added.
- Diagnostics remain CVar-gated. With `T66.Ranged.DiagnosticLogging=0`, `RecordRangedLosBlocked` returns before classification and no terminal `RangedDecisionSummary` is emitted.
- Validation runner `CVarOffProbe` mode was added after smoke to run the reviewed CVar-off-only probe through the existing runner instead of invoking the full acceptance matrix. It is a harness-only addition under `Saved\Codex\Performance\LightweightActorB10_1D`.

Build, stage, and provenance:

- Focused Development build passed: `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex`
- Build warning observed: `Source\T66\Gameplay\T66Hero1AxeAOEVFXLabActor.cpp(353,3): warning C4996: FNiagaraEmitterInstance::IsReadyToRun`. This warning is pre-existing/out of scope for this projectile-manager diagnostic pass.
- Stage passed: `C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`
- Shortcut verification passed for `C:\UE\T66\T66 Standalone.lnk` and `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`; both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Stable staged executable SHA256: `5FB9EECF1C2B25B79DDFC5CA07D221AA1C4BE986636FA732409AD011B7A13589`

Diagnostic-off guard:

- Log: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1D\T66_B101D_DiagnosticOffGuard_CVarOff.log`
- Result: exit `0`, completed, no timeout.
- `RangedDecisionSummaryLines=0`
- `RangedFireDecisionLines=0`
- `RangedPressureDiagnosticLines=0`
- A plain text search for `LOSBlocker` found only pre-existing `[CombatDamage] ... LOSBlocker=None` fields, not the new aggregate bucket output.
- Conclusion: the new attribution path is not always-on in diagnostic-off runs.

Smoke after attribution build:

| Set | Exit | Hero HP | Hero hits | Fired | Hit hero | Hit world | Rich spawns | Lightweight spawns | Avg FPS | Overhead us | Manager max us | HISM max us | Summary lines |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 0 | 640 | 19 | 33 | 19 | 14 | 35 | 0 | 145.14 | 515.6 | 533.6 | 163.7 | 1 |
| CVarOnSmoke | 0 | 940 | 3 | 4 | 3 | 1 | 0 | 17 | 172.39 | 561.3 | 319.5 | 56.7 | 1 |

Smoke blocker sample:

| Set | LOS blocked | Blocked by rich enemy | Blocked by lightweight mob | LOS passed |
| --- | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 256 | 256 | 0 | 33 |
| CVarOnSmoke | 801 | 0 | 801 | 4 |

This proves both rich and lightweight paths are wired into the new buckets. It also shows the expected symmetry: rich projectiles are mainly blocked by rich enemy capsules in CVar-off, while lightweight projectiles are mainly blocked by lightweight mob capsules in CVar-on.

CVar-off LOS blocker probe command:

```powershell
& 'C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1' -Mode CVarOffProbe -AcceptanceCount 3 -HeroHPOverride 1000
```

Rows:

| Run | Terminal | Rejected | Reject reason | Hero HP | Rich spawns | Rich attempts | Distance passed | LOS blocked | Blocked by WorldStatic | Blocked by RichEnemy | LOS passed | Fired | Hit hero | Hit world | Damage HP | Avg FPS | Overhead us |
| ---: | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | Completed | No | None | 820 | 33 | 206343 | 457 | 403 | 12 | 391 | 54 | 54 | 16 | 37 | 320 | 139.65 | 526.5 |
| 2 | Completed | No | None | 880 | 19 | 125162 | 42 | 28 | 0 | 28 | 14 | 14 | 6 | 8 | 120 | 144.43 | 1827.7 |
| 3 | Completed | Yes | NoProjectilesFired | 1000 | 25 | 157205 | 815 | 815 | 0 | 815 | 0 | 0 | 0 | 0 | 0 | 144.88 | 0.0 |

Hygiene:

- Exit code was `0` for all three probes.
- No hero deaths occurred.
- No PerformanceSystem overhead rejection occurred; max observed overhead was `1827.7 us`.
- Write queue accounting stayed balanced, with `FallbackWrites=0` and `FailedWrites=0`.
- No per-attempt `RangedFireDecision` or `RangedPressureDiagnostic` lines appeared.
- Hash stayed stable at `5FB9EECF1C2B25B79DDFC5CA07D221AA1C4BE986636FA732409AD011B7A13589`.

Finding:

- The no-fire failure mode is now attributed.
- In the rejected probe, rich Ranged reached the distance-passed fire decision `815` times, but every one of those checks failed LOS.
- All `815` blocked LOS checks were blocked by `RichEnemy`.
- World static, world dynamic, lightweight mob, other pawn, and unknown blocker buckets were all `0`.
- Therefore the intermittent CVar-off `NoProjectilesFired` mode is peer rich enemy capsule self-occlusion on the visibility LOS trace, not a projectile manager failure, PerformanceSystem issue, HP issue, or spawn failure.
- Accepted runs also show the same dominant blocker pattern: run 1 had `391/403` rich LOS blocks caused by rich enemies, and run 2 had `28/28`.

Recommended next packet:

1. Keep the projectile manager and HISM rendering architecture.
2. Keep the bounded aggregate diagnostics; do not reintroduce per-frame fire-decision logs.
3. Decide and implement the measurement HP cap change separately so the requested ranged-active HP value can apply.
4. Fix rich Ranged LOS/collision behavior so peer rich enemy capsules do not starve all LOS checks in saturated CVar-off captures. Candidate shapes include ignoring same-team/enemy bodies for the LOS trace, using a dedicated projectile-LOS collision channel/profile, or otherwise narrowing the visibility trace so geometry still blocks but peer enemies do not.
5. Separately review projectile sweep semantics if enemy bodies should not deactivate projectiles after fire; this pass only attributed LOS starvation and did not change sweep behavior.
6. Rebuild/stage, rerun smoke, then rerun B.10.1D acceptance only after HP cap and rich LOS behavior are reviewed and fixed.

## 13. Resume3 - HP2000 Cap, Rich LOS Ignore, Projectile Peer Filtering

Status: implementation completed; smoke passed; mandatory acceptance rerun halted in CVar-off after the approved hero-death stop condition. No CVar-on acceptance rows were run after the halt.

Review and go-ahead:

- Packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume3_hp_cap_rich_los_acceptance\plan_packet.md`
- Claude review: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume3_hp_cap_rich_los_acceptance\20260527T105609-pass7\claude_review_pass7.md`
- Verdict: `APPROVE`
- User go-ahead: approved the permanent rich-enemy LOS ignore, permanent enemy-projectile peer-body pass-through, non-regression acceptance framing, mandatory `10 + 10` capture attempt, and HP2000 stop/report rule.

Implementation:

- `Source\T66\Core\RunState\T66RunStateSubsystem_Combat.cpp`
  - Raised the automation-only hero HP override cap from `1000.f` to `2000.f`.
  - Normal gameplay HP remains unchanged because the override path is still gated behind autocapture command-line flow.
- `Source\T66\Gameplay\Enemies\T66RangedEnemy.cpp`
  - Rich Ranged projectile LOS now adds registered rich enemy actors to the visibility-trace ignore list once per frame through `UT66ActorRegistrySubsystem::GetEnemies()`.
  - World geometry and the hero remain traceable; the change prevents peer rich bodies from starving saturated CVar-off LOS checks.
- `Source\T66\Gameplay\T66ProjectileManagerSubsystem.cpp`
  - Enemy projectile non-hero sweeps now ignore `AT66EnemyBase` and `AT66MobBase` peer bodies.
  - Hero collision and world collision remain active.

Build, stage, shortcut, and provenance:

- Focused Development build passed:
  - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex`
- Existing warning only:
  - `Source\T66Mini\T66Mini.Build.cs: Referenced directory ... Source\T66Mini\Public\UI\Components does not exist.`
- Stage refresh passed:
  - `C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`
- Shortcut verification passed:
  - `C:\UE\T66\T66 Standalone.lnk`
  - `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`
  - Both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Stable staged executable SHA256:
  - `782F98AAA4D5269D450A94FBA7345E54B335C91F53BFDD61D324258569962C9D`

Pre-capture hygiene:

- Before smoke: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66`, or `git-lfs` processes.
- Before acceptance: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66`, or `git-lfs` processes.
- Acceptance rows recorded no Git/LFS or Git status overlap before or after the Unreal process.
- Binary hash stayed stable for every accepted and rejected acceptance row.

Smoke command:

```powershell
& 'C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1' -Mode Smoke -HeroHPOverride 2000
```

Smoke result:

| Set | Exit | Terminal | Hero HP | Hero hits | Fired | Hit hero | Hit world | Rich LOS passed | Rich LOS blocked | RichEnemy LOS blockers | Avg FPS | Manager max us | HISM max us | Overhead us |
| --- | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 0 | Completed | 1640 | 18 | 18 | 18 | 0 | 18 | 0 | 0 | 166.47 | 569.3 | 64.9 | 0.0 |
| CVarOnSmoke | 0 | Completed | 1740 | 19 | 19 | 19 | 0 | 0 | 0 | 0 | 208.96 | 385.7 | 116.3 | 0.0 |

Smoke evidence:

- HP override applied as requested in both logs:
  - `AutoCaptureHeroHPOverride AppliedHP=2000.0 RequestedHP=2000.0 MaxHP=2000.0 CurrentHP=2000.0`
- Rich Ranged smoke used the manager path and damaged the hero:
  - `SourceClass=T66RangedEnemy`, `Delivery=EnemyProjectile`, `ProjectileManagerFired=18`, `ProjectileManagerHitHero=18`
- Lightweight Ranged smoke used the manager path and damaged the hero:
  - `SourceClass=T66MobBase`, `Delivery=EnemyProjectile`, `ProjectileManagerFired=19`, `ProjectileManagerHitHero=19`
- Projectile/HISM smoke screenshots were written under:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\projectile_manager_smoke\CVarOffSmoke_01`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\projectile_manager_smoke\CVarOnSmoke_01`

Acceptance command:

```powershell
& 'C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1' -Mode Acceptance -AcceptanceCount 10 -HeroHPOverride 2000
```

CVar-off acceptance rows before halt:

| Run | Terminal | Rejected | Reject reason | Hero HP | Hero hits | Fired | Hit hero | Hit world | Rich LOS passed | Rich LOS blocked | RichEnemy LOS blockers | Avg FPS | Overhead us | Manager max us | HISM max us |
| ---: | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | Completed | No | None | 800 | 118 | 119 | 118 | 0 | 119 | 0 | 0 | 178.36 | 754.2 | 627.6 | 154.8 |
| 2 | Completed | No | None | 800 | 118 | 119 | 118 | 0 | 119 | 0 | 0 | 180.47 | 888.8 | 445.4 | 127.8 |
| 3 | HeroDied | Yes | HeroDeath | 0 | 194 | 194 | 193 | 0 | 194 | 0 | 0 | 163.85 | 686.4 | 445.5 | 187.3 |
| 4 | Completed | No | None | 760 | 62 | 62 | 62 | 0 | 62 | 0 | 0 | 173.52 | 845.5 | 503.7 | 175.1 |
| 5 | Completed | No | None | 1600 | 39 | 40 | 39 | 0 | 40 | 0 | 0 | 172.97 | 936.6 | 460.0 | 127.0 |
| 6 | HeroDied | Yes | HeroDeath | 0 | 262 | 264 | 261 | 0 | 264 | 0 | 0 | 166.20 | 531.6 | 639.9 | 179.9 |

Acceptance summary:

- CVar-off rows attempted: `6`
- CVar-off accepted rows before halt: `4`
- CVar-off rejected rows: `2`, both `HeroDeath`
- CVar-on rows attempted: `0`
- Accepted CVar-off median before halt: `175.94 FPS` from four clean rows; not an authoritative baseline because the required 10-row set did not complete.
- Max PerformanceSystem framework overhead: `936.6 us`, below the `10000 us` rejection threshold.
- Max projectile manager tick: `639.9 us`
- Max HISM update: `187.3 us`
- Dropped fires: `0` in every row.
- Rich LOS starvation is no longer present in this build: `RichLOSBlocked=0` and `RichLOSBlockerRichEnemy=0` in every CVar-off row.
- The halt is therefore a ranged-active measurement/gameplay-pressure blocker, not a projectile manager, HISM, binary provenance, Git/LFS, or PerformanceSystem overhead failure.

Finding:

- Resume3 fixed the two reviewed implementation blockers: HP2000 applies, and peer rich enemy bodies no longer block rich Ranged LOS.
- The projectile manager remains functional under both rich and lightweight smoke.
- Once rich Ranged can fire freely in CVar-off, the stationary HP2000 autocapture hero can still die under saturated rich projectile pressure. The failing rows recorded `194` and `262` hero projectile hits before `OnPlayerDied`.
- This invalidates adopting a new CVar-off baseline from the partial set and blocks the B.10.1D acceptance gate.

Next reviewed scope:

1. Decide the ranged-active performance measurement contract now that both rich and lightweight projectiles are functional. HP2000 is not enough for all CVar-off saturated rows.
2. Decide whether rich saturated projectile pressure is intended at this rate or whether rich Ranged cadence/selection/positioning should be brought closer to the target modest pressure level before acceptance.
3. If the contract changes, update the validation runner to halt on the first `HeroDeath` reject for this pass family; this run continued until the standing second-reject halt, despite the user-approved stop/report rule.
4. Rerun B.10.1D acceptance only after the measurement contract or rich-pressure behavior is reviewed and fixed.

## 14. Resume4 - Measurement HP Contract and HeroDeath Harness Halt

Status: implementation completed; HP survival fixed; smoke passed; 3-capture acceptance escalated to 10 because the 3+3 result was borderline/incomplete; escalated acceptance halted in CVar-on after two `RouteValidity` rejects. B.10.1D acceptance remains blocked, now by intermittent CVar-on route leakage rather than HP, rich LOS starvation, PerformanceSystem overhead, Git/LFS, or projectile manager/HISM behavior.

Review and go-ahead:

- Packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume4_measurement_contract_acceptance\plan_packet.md`
- Claude review: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume4_measurement_contract_acceptance\20260527T223640-pass2\claude_review_pass2.md`
- Verdict: `APPROVE`

Implementation:

- `Source\T66\Core\RunState\T66RunStateSubsystem_Combat.cpp`
  - Raised the automation-only HP override cap from `2000.f` to `50000.f`.
  - Added a code comment documenting that the high cap is only for stationary performance captures under saturated ranged projectile pressure.
  - Pre-existing dirty state in the same file remains unrelated: the earlier deletion of `GrantQuickReviveCharge` and `ClearQuickReviveCharge` was already present before Resume4 and was not touched by this pass.
- `Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1`
  - Preserved the existing rejected-count increment.
  - Added immediate halt when a rejected row has `TerminalStatus=HeroDied`, `RejectReasons=HeroDeath`, or `RejectReasons=HeroDied`.
  - Other reject types still follow the existing second-reject halt behavior.

HP callsite audit:

- Narrow audit command:
  - `rg -n "ApplyAutomationHeroHPOverride|T66AutoCaptureHeroHPOverride|AutoCaptureHeroHPOverride" Source/T66`
- Runtime callsite remains autocapture-only:
  - `Source\T66\Gameplay\T66PlayerController_Overlays.cpp` reads `CVarT66AutoCaptureHeroHPOverride` inside the `enemywaveperf`/autocapture overlay flow.
  - It calls `ApplyAutomationHeroHPOverride` only when `RequestedHeroHPOverride > 0.0f`.
- Other hits were the declaration, the implementation, and pending-issue documentation.

Build, stage, shortcut, and provenance:

- Focused Development build passed:
  - `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex`
- Stage refresh passed:
  - `C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`
- Stage log observations:
  - Existing cook warnings/noise included NTFS journal wrapping and an invalid cached include path for `/Project/ToonStyle/ToonShadingCommon.ush`; packaging still completed with `BUILD SUCCESSFUL`.
- Shortcut verification passed:
  - `C:\UE\T66\T66 Standalone.lnk`
  - `C:\Users\DoPra\AppData\Roaming\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar\T66 Standalone.lnk`
  - Both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Stable staged executable SHA256 for this pass:
  - `0A0AC836F224B898353CD7FA59B5A58ECC24D7676F6903DFD765CA9A3D9252EB`
- The hash differs from Resume3 as expected because this pass rebuilt C++.

Smoke command:

```powershell
& 'C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1' -Mode Smoke -HeroHPOverride 20000
```

Smoke result:

| Set | Exit | Terminal | Hero HP | Hero hits | Damage HP | Fired | Hit hero | Dropped | Avg FPS | Overhead us | Manager max us | HISM max us |
| --- | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 0 | Completed | 19260 | 112 | 2240 | 112 | 112 | 0 | 153.64 | 513.3 | 562.9 | 178.4 |
| CVarOnSmoke | 0 | Completed | 19780 | 25 | 500 | 25 | 25 | 0 | 171.86 | 0.0 | 458.5 | 92.1 |

Smoke HP proof:

- CVar-off and CVar-on logs both reported:
  - `AutoCapture hero HP override applied: starting HP = 20000.0`
  - `AutoCaptureHeroHPOverride AppliedHP=20000.0 RequestedHP=20000.0 MaxHP=20000.0 CurrentHP=20000.0`

3+3 acceptance attempts:

- Attempt 1 artifact:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows_acceptance_hp20000_attempt1_partial.jsonl`
- Attempt 1 result:
  - CVar-off: 3 rows, 2 accepted, 1 rejected `NoProjectilesFired`.
  - CVar-on: 3 rows, 3 accepted.
  - CVar-off did not establish a valid 3-row accepted baseline.
- Attempt 2 artifact:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows_acceptance_hp20000_attempt2_partial_borderline.jsonl`
- Attempt 2 result:
  - CVar-off: 3 rows, 2 accepted, 1 rejected `NoProjectilesFired`.
  - CVar-on: 3 rows, 3 accepted, but stdev was high enough to trigger the reviewed 10-capture escalation condition.

10-capture escalation command:

```powershell
& 'C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1' -Mode Acceptance -AcceptanceCount 10 -HeroHPOverride 20000
```

Escalated artifacts:

- Rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows_acceptance_hp20000_attempt3_escalated_halted_routevalidity.jsonl`
- Progress: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_progress_acceptance_hp20000_attempt3_escalated_halted_routevalidity.jsonl`
- `capture_results.json` was stale after the exception, so the row file is the authoritative artifact for the halted escalation.

CVar-off escalated baseline candidate:

| Run | Hero HP | Hero hits | Damage HP | Fired | Hit hero | Avg FPS | Overhead us |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 18600 | 139 | 2780 | 140 | 139 | 159.68 | 706.2 |
| 2 | 18660 | 130 | 2600 | 131 | 130 | 167.76 | 805.9 |
| 3 | 18000 | 198 | 3960 | 199 | 198 | 167.09 | 714.6 |
| 4 | 18400 | 152 | 3040 | 153 | 152 | 147.74 | 755.4 |
| 5 | 18440 | 112 | 2240 | 112 | 112 | 155.07 | 644.4 |
| 6 | 18640 | 196 | 3920 | 196 | 196 | 146.89 | 775.8 |
| 7 | 17380 | 387 | 7740 | 389 | 387 | 146.89 | 652.9 |
| 8 | 17400 | 494 | 9880 | 496 | 494 | 156.87 | 665.9 |
| 9 | 18880 | 107 | 2140 | 108 | 107 | 159.72 | 710.3 |
| 10 | 17280 | 591 | 11820 | 595 | 591 | 158.49 | 711.3 |

CVar-off escalated summary:

- Rows: `10`
- Accepted: `10`
- Hero deaths: `0`
- Rejected: `0`
- Median FPS: `157.68`
- Mean FPS: `156.62`
- Stdev FPS: `7.65`
- Max overhead: `805.9 us`
- Total fired: `2519`
- Total hit hero: `2506`
- Dropped fires: `0`
- Rich LOS starvation remained fixed: each row had `RichLOSBlocked=0`.

CVar-on escalated rows before halt:

| Run | Rejected | Reason | Hero HP | Hero hits | Damage HP | Fired | Hit hero | Rich spawns | Rich attempts | Lightweight spawns | Avg FPS | Overhead us |
| ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | No | None | 19540 | 23 | 460 | 23 | 23 | 0 | 0 | 18 | 166.82 | 751.3 |
| 2 | No | None | 19840 | 10 | 200 | 10 | 10 | 0 | 0 | 28 | 169.62 | 941.9 |
| 3 | Yes | RouteValidity | 19880 | 6 | 120 | 6 | 6 | 1 | 19767 | 22 | 162.47 | 684.3 |
| 4 | No | None | 19940 | 3 | 60 | 3 | 3 | 0 | 0 | 22 | 165.29 | 773.5 |
| 5 | Yes | RouteValidity | 19980 | 1 | 20 | 1 | 1 | 1 | 21327 | 27 | 169.04 | 988.8 |

CVar-on halted summary:

- Rows before halt: `5`
- Accepted rows: `3`
- Rejected rows: `2`, both `RouteValidity`
- Hero deaths: `0`
- Max overhead: `988.8 us`
- Dropped fires: `0`
- All rows kept stable staged hash `0A0AC836F224B898353CD7FA59B5A58ECC24D7676F6903DFD765CA9A3D9252EB`.
- Rejected rows had `UseLightweight=1`, `RouteRanged=1`, and `LightweightSpawns>0`, but also `RichSpawns=1` plus nonzero rich fire attempts. The runner rejected them because CVar-on is expected to route Ranged through lightweight only.

Finding:

- Resume4 solved the measurement HP blocker. HP20000 survives saturated rich and lightweight Ranged pressure in smoke and all attempted acceptance rows.
- Resume4 solved the HeroDeath harness policy in code, though no post-fix HeroDeath occurred during validation.
- B.10.1D acceptance still did not pass:
  - Initial 3+3 attempts could not produce a complete 3-row accepted CVar-off baseline because intermittent CVar-off `NoProjectilesFired` rows still occurred when rich Ranged never entered firing distance.
  - Escalation produced a clean 10-row CVar-off baseline candidate, but CVar-on halted after two `RouteValidity` rejects.
  - The escalated CVar-on rejects show intermittent rich Ranged leakage under `T66.Mob.UseLightweight=1` / `RouteRanged=1`, which violates the route contract for the lightweight acceptance set.
- This is now a route/measurement harness blocker, not an HP, projectile manager, HISM, PerformanceSystem overhead, Git/LFS, or binary provenance blocker.

Next reviewed scope:

1. Investigate why CVar-on can still report `RichSpawns=1` and rich fire attempts under `UseLightweight=1` with `RouteRanged=1`.
2. Decide whether CVar-off `NoProjectilesFired` rows should be treated as replacement captures, route/positioning diagnostics, or scenario-invalid rows. The current runner counts fixed total attempts rather than collecting until the requested accepted-row count is reached.
3. After route validity is fixed or the harness replacement semantics are reviewed, rerun acceptance on a fresh staged binary.
## 15. Resume5 - Route Leakage Diagnostic

Status: diagnostic complete. No routing fix or B.10 acceptance reattempt was performed.

Review:

- Plan packet: `C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\plan_packet.md`
- Claude approval: `C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\20260528T002135-pass2\claude_review_pass2.md`
- Gate amendment approval: `C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\20260528T004840-pass5\claude_review_pass5.md`

Implementation:

- Added aggregate `RouteAttributionSummary` counters gated by `T66.Ranged.DiagnosticLogging=1`.
- Instrumented director initial population/runtime trickle and known non-director rich spawn paths.
- Added runner `RouteDiagnostic` mode plus opt-in `-AllowHighFpsControlAdvisory`.
- Gate self-test passed: high FPS blocks by default, advisory high FPS passes only with the switch, low FPS still blocks.

Build/stage:

- `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development` passed.
- Stable staged SHA256: `D1E3235ED789C2596626BF6748F3DE49018B883D99F941B6160D860C535192FF`.
- Existing warnings only: missing `T66Mini.Build.cs` referenced directory and the known Niagara `IsReadyToRun` deprecation.

Artifacts:

- Rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows.jsonl`
- Results: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_results.json`

Control notes:

- Discarded high-FPS control: `172.04 FPS`, stale `git status --porcelain` active at launch.
- Clean strict-control retry: `168.93 FPS`, structurally clean, halted only by old high-FPS gate.
- Final advisory control: `170.63 FPS`, structurally clean, accepted with `RouteControlHighFpsAdvisory`.

Final CVar-on aggregate route attribution across 10 captures:

| Family | Total observed | Routed lightweight basic | Rich special/mini-boss | Rich mini-boss promotion | Rich fallback | Rich non-director |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Melee | 372 | 372 | 0 | 0 | 0 | 0 |
| Rush | 119 | 119 | 0 | 0 | 0 | 0 |
| Flying | 139 | 138 | 0 | 1 | 0 | 0 |
| Ranged | 259 | 259 | 0 | 0 | 0 | 0 |
| SpecialUnknown | 11 | 0 | 11 | 0 | 0 | 0 |

Finding:

- The specific Ranged rich-route leak did not reproduce in 10 CVar-on captures: all Ranged rich buckets were `0`, `RichSpawns=0`, and rich fire attempts were `0`.
- A planned rich mini-boss route did reproduce once, landing on Flying.
- Source audit shows mini-boss promotion is family-neutral: `ShouldRouteSpawnToLightweightMob` returns false for `bIsMiniBoss`, and runtime waves choose the mini-boss slot before rolling the final `MobID`. Therefore the earlier Resume4 `RichSpawns=1` Ranged route-validity rejects are most likely mini-boss promotion landing on a Ranged `MobID`.
- Special wave spawns are expected rich routes (`SpecialUnknownRoutedRichSpecialOrMiniBoss=11`) and are not a second spawn path.
- No evidence was found for routing race, family lookup failure, fallback-branch leakage, lightweight acquire failure, or non-director spawn leakage in standard `enemywaveperf`.

Next reviewed scope:

1. Decide whether `enemywaveperf` acceptance should disable special/mini-boss spawns for a pure basic-family lightweight population, or whether route validity should allow planned rich special/mini-boss routes.
2. Update the route-validity gate accordingly.
3. Rerun B.10.1D acceptance on a fresh staged binary.
