# B.10.1C-Rerun Consolidated Packet

## Scope

Working goal: complete the original B.10.1C diagnostic objective with measurement-cheap aggregate counters: establish the HP-override measurement contract, rerun RA-D/RB-D, identify why rich `AT66RangedEnemy` produces zero hero hits, and stop before the rich-Ranged fix.

No rich-Ranged fix, lightweight behavior change, or B.10 acceptance reattempt was performed.

## Review And Go-Ahead

- Main aggregate-diagnostics plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_aggregate_diagnostics\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_aggregate_diagnostics\20260527T022620-pass2\claude_review_pass2.md`
- Post-capture Git/LFS provenance plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_option2_gitlfs_provenance\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_option2_gitlfs_provenance\20260527T055211-pass2\claude_review_pass2.md`

## Implementation

- Replaced the B.10.1C per-decision Ranged diagnostic log approach with aggregate counters.
- `T66.Ranged.DiagnosticLogging=1` now tracks counters and emits one terminal `[RangedDecisionSummary]` line per capture.
- Removed per-frame/per-mob Ranged fire decision log emission from rich and lightweight paths.
- Added a runner-side `PostCaptureGitContaminated` flag. Post-capture `git-lfs`/Git status activity is provenance for this diagnostic pass, not a hard reject. Pre-capture Git/LFS quiet waits and staged-binary hash checks remain hard hygiene controls.

## Artifacts

- Runner: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\run_b101c_rerun_aggregate_diagnostic_captures.ps1`
- RA-D rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\capture_rows_RA-D_accepted_gitlfs_provenance_6212558B.jsonl`
- RA-D progress: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\capture_progress_RA-D_accepted_gitlfs_provenance_6212558B.jsonl`
- RA-D results: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\capture_results_RA-D_accepted_gitlfs_provenance_6212558B.json`
- RB-D rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\capture_rows_RB-D_accepted_6212558B.jsonl`
- RB-D results: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\capture_results_RB-D_accepted_6212558B.json`
- Logs: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1C_Rerun\`

## Binary Provenance

- Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- SHA256 for RA-D and RB-D: `6212558B842942338D3071098D094703542CB473C9DAADDCF5B96C49FFC71ACA`
- RA-D pass-start hash, per-capture hash, and pass-end hash matched.
- RB-D accepted set used the same staged hash.

## RA-D Matrix

Configuration: `T66.Mob.UseLightweight=1`, Rush/Flying lightweight, `T66.Mob.Diagnostics.RouteRangedLightweight=0`, `T66.AutoCapture.HeroHPOverride=500`, `T66.Ranged.DiagnosticLogging=1`.

| Run | HP | Hits | Overhead us | Git contaminated | Route | Rich attempts | Cooldown | Dist pass | LOS pass | Dispatch | Spawned | Spawn fail | Avg FPS |
| ---: | ---: | ---: | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 500 | 0 | 1184.7 | True | True | 385834 | 377342 | 862 | 33 | 33 | 0 | 33 | 178.53 |
| 2 | 500 | 0 | 941.7 | False | True | 536236 | 524358 | 430 | 3 | 3 | 0 | 3 | 177.11 |
| 3 | 500 | 0 | 952.5 | False | True | 633083 | 619106 | 3236 | 0 | 0 | 0 | 0 | 178.15 |
| 4 | 500 | 0 | 920.6 | True | True | 566440 | 553807 | 2382 | 0 | 0 | 0 | 0 | 176.13 |
| 5 | 500 | 0 | 928.8 | False | True | 582892 | 570128 | 804 | 0 | 0 | 0 | 0 | 179.65 |
| 6 | 500 | 0 | 1102.8 | False | True | 518431 | 507131 | 444 | 30 | 30 | 0 | 30 | 180.16 |
| 7 | 500 | 0 | 997.5 | False | True | 493499 | 482610 | 497 | 83 | 83 | 0 | 83 | 177.79 |
| 8 | 500 | 0 | 926.4 | False | True | 549417 | 537270 | 1631 | 72 | 72 | 0 | 72 | 178.42 |
| 9 | 500 | 0 | 698.4 | True | True | 460189 | 449835 | 0 | 0 | 0 | 0 | 0 | 174.37 |
| 10 | 500 | 0 | 1297.0 | False | True | 668993 | 654416 | 1493 | 0 | 0 | 0 | 0 | 180.41 |

RA-D summary: 10/10 accepted, 10/10 hero survived, 0 hero hits, max framework overhead `1297.0 us`, 3 rows marked `PostCaptureGitContaminated=true`, median FPS `178.28` diagnostic-only. Rich totals: `5,395,014` attempts, `11,779` distance-passed, `221` LOS-passed, `221` dispatched, `0` spawned, `221` spawn failed, `0` hero hits.

## RB-D Matrix

Configuration: `T66.Mob.UseLightweight=1`, Rush/Flying/Ranged lightweight, `T66.AutoCapture.HeroHPOverride=500`, `T66.Ranged.DiagnosticLogging=1`.

| Run | HP | Hits | Overhead us | Route | Lightweight attempts | Cooldown | Dist pass | LOS pass | Dispatch | Spawned | Spawn fail | Avg FPS |
| ---: | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 240 | 13 | 1036.3 | True | 570705 | 558090 | 9571 | 17 | 17 | 17 | 0 | 176.29 |
| 2 | 420 | 5 | 929.3 | True | 522802 | 511110 | 8536 | 6 | 6 | 6 | 0 | 174.91 |
| 3 | 480 | 1 | 999.5 | True | 508574 | 497700 | 7777 | 8 | 8 | 8 | 0 | 182.93 |
| 4 | 180 | 17 | 1119.1 | True | 481888 | 471626 | 7607 | 27 | 27 | 27 | 0 | 181.79 |
| 5 | 500 | 0 | 916.0 | True | 490160 | 479340 | 7501 | 1 | 1 | 1 | 0 | 177.79 |
| 6 | 320 | 17 | 891.0 | True | 578362 | 565720 | 9373 | 27 | 27 | 27 | 0 | 177.54 |
| 7 | 340 | 8 | 925.7 | True | 682861 | 668302 | 10928 | 14 | 14 | 14 | 0 | 183.38 |
| 8 | 380 | 6 | 941.2 | True | 568122 | 555846 | 9092 | 56 | 56 | 8 | 48 | 180.94 |
| 9 | 460 | 2 | 1022.2 | True | 535754 | 524037 | 8045 | 9 | 9 | 9 | 0 | 178.87 |
| 10 | 440 | 4 | 1228.2 | True | 534360 | 522687 | 8552 | 18 | 18 | 13 | 5 | 178.67 |

RB-D summary: 10/10 accepted, 10/10 hero survived under HP override, 73 hero hits, 1460 HP projectile damage, max framework overhead `1228.2 us`, median FPS `178.77` diagnostic-only. Lightweight totals: `5,473,588` attempts, `86,982` distance-passed, `183` LOS-passed, `183` dispatched, `130` spawned, `53` spawn failed, `73` hero hits.

## Root Cause

Rich Ranged is not failing because its tick never runs, cooldown is permanently blocked, status gating is stuck, or distance/LOS never allow fire. RA-D proves rich Ranged reaches the fire dispatch path:

- `RichLOSPassed=221`
- `RichProjectilesDispatched=221`
- `RichProjectilesSpawned=0`
- `RichSpawnFailed=221`
- `RichProjectilesHitHero=0`

The divergence from lightweight happens at projectile spawn/dispatch. On the same staged binary, lightweight Ranged reached dispatch `183` times, spawned `130` projectiles, and hit the hero `73` times. Rich Ranged reached dispatch `221` times and failed every spawn.

Therefore B.10.1D should focus on rich `AT66RangedEnemy` projectile spawn failure in staged standalone: projectile class resolution, cooked class availability, spawn transform, owner/instigator, collision handling, or class/type constraints in `FireProjectileAtPlayer`.

## Measurement Caveat

The post-capture Git/LFS relaxation is valid only for diagnostic counters. Three RA-D rows observed `git-lfs.exe` after the Unreal process exited and are marked `PostCaptureGitContaminated=true`. Those rows are acceptable for in-process decision-counter evidence because the `[RangedDecisionSummary]` line is emitted before process exit and cannot be retroactively altered by post-exit `git-lfs`; their FPS values are not B.10 acceptance-grade.

## B.10.1D Proposed Scope

- Fix rich `AT66RangedEnemy` projectile spawn failure without changing lightweight Ranged pressure.
- Add focused verification that rich Ranged spawns projectiles and can hit the HP-overridden stationary autocapture hero.
- Re-run B.10 acceptance only after the rich fix lands: CVar-off and CVar-on 5-capture sets, escalating per standing methodology.
- Keep binary-hash provenance and clean Git/LFS pre-capture waits.

## Verification Performed

- Claude review pass2 approved aggregate counter implementation.
- Claude review pass2 approved post-capture Git/LFS provenance-only policy for this diagnostic runner.
- RA-D rerun completed 10/10 accepted rows on staged hash `6212558B842942338D3071098D094703542CB473C9DAADDCF5B96C49FFC71ACA`.
- RB-D accepted backup was parsed and aggregated on the same staged hash.
- All accepted rows had one `RangedDecisionSummary`, zero legacy `RangedFireDecision` lines, zero legacy `RangedPressureDiagnostic` lines, route validity, hero survival, and framework overhead below `10000us`.
