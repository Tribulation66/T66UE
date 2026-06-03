# B.10.1C Freeze Consolidated Packet

Point-in-time snapshot. The source files listed below remain the source of truth; this packet is for one-file review/copy convenience.

## Status

- Scope: investigation-only aggregation for the completed B.10.1C-Freeze diagnostic.
- Runtime/code changes in this packet: none.
- Capture reruns in this packet: none.
- B.10 acceptance reattempt: not included.
- Snapshot generated: `2026-05-27T02:03:43.9859732-03:00`.

## Source Index

- Lightweight Actor plan source: `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`
- Gameplay pending issues source: `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`
- PerformanceSystem pending issues source: `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`
- Matrix summary JSON: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\freeze_matrix_summary_simple.json` (2781143 bytes)
- Matrix rows JSONL: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\freeze_matrix_rows_simple.jsonl` (6 rows, 2324845 bytes)
- Provenance JSON: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\provenance_simple.json` (1707 bytes)

## Review Packet

- Consolidation plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_freeze_consolidation\plan_packet.md`
- Claude review greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T020056-pass2\claude_review_pass2.md`

## Artifact Summary

- Final save matched baseline: `True`.
- Baseline save fingerprint: `985FF2C366B3D3781469AEF94AB2DD4430DAE88A7CAA256E50B79F3E36EAFE91`.
- Final save fingerprint: `985FF2C366B3D3781469AEF94AB2DD4430DAE88A7CAA256E50B79F3E36EAFE91`.
- Final matrix exe: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Final matrix exe SHA256: `C4E6053B00CFFF722CFEB2FC7E976EEC0504CD7B9D1B21608A6335A1305752E5`.
- Final matrix exe timestamp UTC: `2026-05-27T04:26:21.5788180Z`.

## Copied Plan Section

Source: `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md`; LastWriteTime: `2026-05-27T01:55:20.6354761-03:00`; SnapshotAsOf: `2026-05-27T02:03:43.9859732-03:00`.

## Pass B.10.1C-Freeze Diagnostic

Status: investigation-only freeze diagnostic completed. No production behavior fix, revert, Ranged parity fix, or B.10 acceptance reattempt was performed.

Artifacts:

- Reviewed plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_freeze_diagnostic\plan_packet.md`
- Claude review greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T004207-pass5\claude_review_pass5.md`
- Matrix runner: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\run_b101c_freeze_matrix_simple.ps1`
- Matrix rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\freeze_matrix_rows_simple.jsonl`
- Matrix summary: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\freeze_matrix_summary_simple.json`
- Provenance: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\provenance_simple.json`
- Logs: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1C_Freeze\`
- Process samples: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\process_samples\`
- Tail files: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\log_tails\`

Reviewed-method deviation used for this investigation: freeze evidence was retained even if it would normally violate acceptance-capture hygiene. In practice, none of the matrix runs exceeded `PerformanceSystemOverheadMaxUs > 10000`, and none hit the 180s wall cap. One ambiguous rerun was allowed per cell; Config 1 and Config 3 each reran once after an early no-HP hero death.

Staged-binary provenance:

- Final matrix executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Size: `311056896`
- Timestamp: `2026-05-27T04:26:21.5788180Z`
- SHA256: `C4E6053B00CFFF722CFEB2FC7E976EEC0504CD7B9D1B21608A6335A1305752E5`
- Source timestamps were earlier than the staged executable: `T66MobManagerSubsystem.cpp` `2026-05-27T01:11:54Z`, `T66MobBase.cpp` `2026-05-27T01:12:04Z`, `T66RangedEnemy.cpp` `2026-05-27T01:12:04Z`, `T66PlayerController_Overlays.cpp` `2026-05-27T01:17:34Z`.
- Binary string scan was inconclusive: it did not find `Stage=CooldownBlocked`, `LogT66RangedDiagnostics`, or `RangedFireDecision`. The logs are the authoritative proof that this staged binary honored `T66.Ranged.DiagnosticLogging` and used `LogT66RangedDiagnostics`.
- Broader observation: staged executable provenance drifted during the investigation window. Earlier preflight saw a staged exe timestamp `2026-05-27T02:57:27Z` and SHA `631972E6135572D8A4484B094017DF9828528C66AE08E1C49C523AB44190E452`; an aborted runner later saw timestamp `2026-05-27T04:01:28Z` and SHA `C16431BAE717DFE3DA160E26AF8A904AE91B8CA9F7E65F5BFE0E8AC84CB752CC`; the final matrix used the `2026-05-27T04:26:21Z` binary above. The final matrix is internally consistent and provenance-recorded, but cross-run comparisons should use the final binary only.

Save-state isolation:

- The staged `SaveGames` directory was snapshotted and restored before each config.
- Final staged save fingerprint matched baseline: `985FF2C366B3D3781469AEF94AB2DD4430DAE88A7CAA256E50B79F3E36EAFE91`.
- No matrix run mutated the staged save state after restore.

Matrix: full RB routing (`UseLightweight=1`, Rush/Flying/Ranged lightweight), 30.0s intended game time, 180s wall cap.

| Config | Attempt | HP override | Diag logging | Exit | Wall s | World s | Game/wall | Log KB | Decisions | Hero hits | Damage lines | Perf overhead us | Process state | Save |
| --- | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| Config1 | 1 | 0 | 0 | Early death | 40.13 | 24.68 | 0.615 | 98.4 | 0 | 5 | 5 | 2211.0 | Responding, CPU active | Restored |
| Config1 | 2 | 0 | 0 | Natural exit | 44.08 | 30.03 | 0.681 | 102.5 | 0 | 2 | 2 | 2677.8 | Responding, CPU active | Restored |
| Config2 | 1 | 500 | 0 | Natural exit | 43.94 | 30.04 | 0.684 | 105.6 | 0 | 4 | 4 | 802.2 | Responding, CPU active | Restored |
| Config3 | 1 | 0 | 1 | Early death | 42.05 | 26.84 | 0.638 | 362.8 | 1467 | 5 | 5 | 2039.5 | Responding, CPU active | Restored |
| Config3 | 2 | 0 | 1 | Natural exit | 49.35 | 30.02 | 0.608 | 457.9 | 2012 | 2 | 2 | 891.4 | CPU active; one transient not-responding sample near exit | Restored |
| Config4 | 1 | 500 | 1 | Natural exit | 53.82 | 30.02 | 0.558 | 548.5 | 2506 | 0 | 0 | 3273.3 | CPU active; transient not-responding sample observed during run | Restored |

Toggle confirmation:

- Every config logged `MobRoutingFlags UseLightweight=1 RouteRush=1 RouteFlying=1 RouteRanged=1 UseTouchDamageOverlap=1 ManagerTickProfile=0`.
- Diagnostic-off configs logged `RangedDiagnosticLogging=0` and emitted `0` `[RangedFireDecision]` lines.
- Diagnostic-on configs logged `RangedDiagnosticLogging=1` and emitted `[RangedFireDecision]` lines through `LogT66RangedDiagnostics`.
- Override-on configs logged `AutoCaptureHeroHPOverride AppliedHP=500.0 RequestedHP=500.0 MaxHP=500.0 CurrentHP=500.0 CurrentHearts=25 MaxHearts=25`.
- Override-off configs logged `HeroHPOverride=0.0` and no override application.

Process-level symptoms:

- No matrix run deadlocked or hit the 180s wall cap.
- T66 consumed CPU throughout the samples; this was slow-but-live behavior, not an idle deadlock.
- Disk writes were not elevated enough to implicate the PerformanceSystem queue. Peak observed process write rate in the matrix was about `395 KB/s` in Config4.
- PerformanceSystem queue accounting remained clean in all six matrix rows: `FailedWrites=0`, `FallbackWrites=0`, `AbandonedWrites=0`, `AccountingBalanced=true`; max queue depth was `1` or `2`.
- Worker write peaks were below `7.3 ms` in the matrix (`7212.9 us` max), while framework overhead stayed below the `10 ms` rejection threshold (`3273.3 us` max).
- No stack/dump was taken in this pass. The process evidence does not support a mutex/file-write deadlock; if future full-length diagnostics hard-freeze in the post-mitigation binary, a reviewed dump-based packet would be needed to prove the exact wait owner.

Isolation finding:

- HP override is not the freeze cause. Config1 attempt 2 and Config2 attempt 1 had nearly identical game/wall ratios (`0.681` vs `0.684`) and similar wall time (`44.08s` vs `43.94s`).
- Diagnostic logging is the current slowdown cause. With HP override enabled, diagnostic-on Config4 took `53.82s` wall for the same `30.02s` world time where diagnostic-off Config2 took `43.94s`, a `+22.5%` wall-time increase and an `18.4%` lower game/wall ratio.
- The short matrix did not reproduce the user-visible hard freeze on the final staged binary. It reproduced a slow-but-live diagnostic overhead.
- The original hard-freeze signature is explained by earlier pre-final B.10.1C diagnostic logs. `T66_B101C_Override_DiagOn_RB_rerun.log` was `8.23 MB` with `46415` `[RangedFireDecision]` lines, `45470` of them `Stage=CooldownBlocked`, all through legacy `LogT66MobManager: VeryVerbose`, and no terminal `RangedPressureSummary`. `T66_B101C_Override_DiagOn_RB_20260526_215506.log` was `11.58 MB` with `65556` decision lines, `64135` `CooldownBlocked`, again no terminal summary. Their tails are continuous per-frame `CooldownBlocked` output. That log flood under `-forcelogflush` matches the frozen-window symptom.
- The later B.10.1C gate logs prove the mitigation shape: `T66_B101C_Override_DiagOn_RB_Gate_20260526_222022.log` was `0.22 MB`, had `768` decision lines, `0` `CooldownBlocked` log lines, used `LogT66RangedDiagnostics`, and completed with a terminal summary.

Broader B.10.1C observations:

| Severity | Observation | What fixing would entail |
| --- | --- | --- |
| Major | Pre-final diagnostic logging enabled legacy `LogT66MobManager VeryVerbose` and emitted per-frame `CooldownBlocked` lines under `-forcelogflush`, producing 46k to 65k decision lines in short smokes and no terminal summary. | Keep cooldown-blocked data aggregate-only; never enable broad manager VeryVerbose for capture diagnostics; use sampled/throttled decision traces if full logs are required. |
| Major | Current post-mitigation `LogT66RangedDiagnostics VeryVerbose` still slows short captures materially, even though it no longer hard-freezes: Config4 was `53.82s` wall for `30.02s` world time. | Convert long diagnostic captures to counters-first output with bounded examples, or add a reviewed throttle/sample CVar for `[RangedFireDecision]` lines. |
| Major | No-HP full-lightweight 30s captures can still kill the stationary hero. Config1 attempt 1 died at `24.68s` after 5 projectile hits and 100 HP damage; Config3 attempt 1 died at `26.84s`. | Use the explicit HP override for ranged-active diagnostics, and keep acceptance captures separate from diagnostic survivability captures. |
| Major | Staged executable provenance drifted during the investigation window while unrelated tooling was observed earlier. The final matrix is internally valid, but the changing binary means older partial runs are not comparable without provenance. | Capture workflows should record exe hash/timestamp before every matrix and block if any build/stage process appears or if the staged exe changes mid-pass. |
| Major | The first local freeze-matrix runner had investigation-harness bugs: `$Pid` conflicted with PowerShell's read-only `$PID`, and a restore attempt copied `.sav` files into staged `Saved` root before correction. The final simple runner restored the baseline and verified final save fingerprint equality. | Keep the simple scalar-output runner or promote a hardened master capture harness; include save-root assertions and post-run root `.sav` checks. |
| Minor | Binary string scanning did not find diagnostic strings despite logs proving the category was active. | Treat staged exe string scans as corroborating only; prefer source/stage timestamps and log signatures. |
| Minor | Staged logs repeated existing non-blocking warnings: Steam unavailable, missing audio packages (`/Game/Audio/SC_Music`, `/Game/Audio/SC_SFX`, theme assets), invalid community item rows (`Item_Alchemy`), and the already-tracked `LogT66TrapProjectile` hot-path fire/impact logs. | Audit data/package references in a separate cleanup pass; trap projectile log demotion is already tracked. |

Root cause:

- The original B.10.1C visible freeze was caused by diagnostic log volume, specifically pre-final per-frame `CooldownBlocked` `[RangedFireDecision]` output through `LogT66MobManager VeryVerbose` under `-forcelogflush`.
- The HP override did not cause the freeze.
- PerformanceSystem write queue/I/O did not cause the freeze in the post-mitigation matrix.
- Current diagnostic logging is improved but still not cheap enough to treat as performance-neutral in long captures.

Proposed next-packet fix scope:

- Do not touch Ranged parity yet unless the next packet is explicitly the B.10.1D rich-Ranged fix.
- Add a reviewed Ranged diagnostic hygiene pass before any long diagnostic matrix that needs full decision traces:
  - Keep all hot-path counters.
  - Keep cooldown-blocked as counters only, not per-event logs.
  - Gate `[RangedFireDecision]` lines behind a bounded sample/throttle such as first N per stage/path per capture or one-per-N-seconds per stage/path.
  - Keep the dedicated `LogT66RangedDiagnostics` category; do not enable broad `LogT66MobManager VeryVerbose` in capture scripts.
  - Add a short smoke proving diagnostic-on capture reaches terminal summary, keeps overhead below `10 ms`, and does not produce unbounded logs.
- Separately, the B.10.1D packet can proceed with rich-Ranged firing diagnosis/fix using HP override, counters, and bounded logs rather than full unthrottled traces.

## Copied Gameplay Pending Issue Sections

Source: `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`; LastWriteTime: `2026-05-27T01:55:41.0997949-03:00`; SnapshotAsOf: `2026-05-27T02:03:43.9859732-03:00`.

## Lightweight Ranged Autocapture Can Kill The Stationary EnemyWavePerf Hero

- Severity tag: [Blocker]
- What's wrong: Pass B.10 moved Ranged-family mobs onto `AT66MobBase` and proved the projectile path in smoke, but the CVar-on `enemywaveperf` acceptance set halted after two non-zero exits. Both non-zero exits were real hero deaths from lightweight `EnemyProjectile` hits, not shutdown crashes. Example fatal lines show `SourceID=StoneSentinel Delivery=EnemyProjectile SourceActor=T66MobBase_* HeroHP=20.0->0.0` at world times `17.58` and `28.56`. A clean CVar-on attempt also had to be rejected because `PerformanceSystemOverheadMaxUs=268567.0`.
- B.10.1 update: route diagnostics landed and the partial same-binary comparison points at a lightweight Ranged parity gap. RA (`UseLightweight=1`, Rush/Flying lightweight, Ranged rich) produced 6/6 route-valid survivals, 0 projectile hits, and 0 projectile damage before the set halted on two PerformanceSystem overhead rejections. A post-patch full-lightweight attribution rerun produced 5/5 route-valid captures, 21 projectile hits, 420 HP of projectile damage, and 3/5 hero deaths. A single post-patch RB preflight also showed full-lightweight Ranged landing a projectile hit while the hero survived at 80 HP. This is not enough for B.10 acceptance, but it strongly favors "lightweight Ranged is more aggressive than rich Ranged in practice" over a pure measurement-contract issue.
- B.10.1B update: after the PerformanceSystem write-queue fix, the clean 10x10 same-binary diagnostic showed a route-dependent split in observed Ranged outcomes without overhead rejects. RA completed 10/10 route-valid captures with 0 hero deaths, 0 projectile hits, and 0 projectile damage. RB completed 10/10 route-valid captures with 4 hero deaths, 31 projectile hits, and 620 projectile damage. B.10 remains blocked on Ranged parity rather than a measurement-contract change in this pass; the next reviewed packet needs to explain RA's zero projectile activity and make lightweight Ranged match the rich path's effective pressure.
- B.10.1C-freeze update: the reviewed short freeze matrix showed that the explicit `T66.AutoCapture.HeroHPOverride=500` path is not the freeze source and is needed for ranged-active diagnostics. No-override full-lightweight runs can still kill the stationary hero within 30s (`5` projectile hits / `100` HP damage at `WorldTime=24.68` in Config1 attempt 1; `5` hits / `100` HP damage at `WorldTime=26.84` in Config3 attempt 1). HP override runs applied `500` HP cleanly and did not mutate staged saves. Ranged parity remains unresolved; this update only confirms the measurement-survivability behavior.
- Why it's out of scope now: Fixing this requires a reviewed gameplay parity packet after the PerformanceSystem overhead fix unblocks clean RA/RB capture sets. Adding hero safety or parking the hero remains out of scope and would hide the regression signal.
- What fixing it would entail: Drill into rich-vs-lightweight Ranged movement efficiency, LOS/fire cadence, projectile spawn failure, and distance-band behavior. Then either make lightweight Ranged match rich pressure or intentionally rebalance the Ranged data with Pablo go-ahead, followed by a clean 10x10 RA/RB diagnostic and the original B.10 acceptance rerun.

## Ranged Diagnostic VeryVerbose Logging Can Stall Or Distort Autocapture

- Severity tag: [Major]
- What's wrong: The pre-final B.10.1C diagnostic path enabled legacy `LogT66MobManager VeryVerbose` and emitted per-frame `CooldownBlocked` `[RangedFireDecision]` lines under `-forcelogflush`. Two diagnostic-on smoke logs produced `46415` and `65556` decision lines, mostly `CooldownBlocked`, and no terminal `RangedPressureSummary`, matching the frozen-window symptom. The current post-mitigation dedicated category no longer hard-freezes in the 30s matrix, but `LogT66RangedDiagnostics VeryVerbose` still slowed HP500 full-lightweight capture from `43.94s` wall for 30.04s world time to `53.82s` wall for 30.02s world time.
- Why it's out of scope now: The current pass is investigation-only and cannot change diagnostics or gameplay behavior. The B.10.1D rich-Ranged fix also should not be mixed with logging-infrastructure changes unless explicitly reviewed.
- What fixing it would entail: Keep Ranged decision counters, keep cooldown-blocked as counter-only data, and add a reviewed bounded sampling/throttle strategy for `[RangedFireDecision]` logs. Capture scripts should use `LogT66RangedDiagnostics`, not broad `LogT66MobManager VeryVerbose`, and should smoke-prove terminal summary output with overhead below `10 ms`.

## Copied PerformanceSystem Pending Issue Sections

Source: `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md`; LastWriteTime: `2026-05-27T01:56:00.2665683-03:00`; SnapshotAsOf: `2026-05-27T02:03:43.9859732-03:00`.

## Intermittent PerformanceSystemOverhead Spikes Still Pollute Capture Sets

- Severity tag: [Major]
- Status: Resolved for the B.10.1B capture path; keep monitoring future capture sets for non-PerformanceSystem disk contention or a broader normal-overhead distribution shift.
- What's wrong: B.9 clean capture sets still produced large intermittent `PerformanceSystemOverhead` events, including `505246.8 us` in `off_clean08` and `21614.4 us` in `on_clean05`, while neighboring captures in the same binary were near the normal `800-2000 us` range. These spikes do not appear to explain the B.9 Flying median miss by themselves, but they make tail metrics and individual-run interpretation noisy.
- B.9.1 diagnostic update: the issue reproduced across different isolation configurations. Set B run 2 reported `396142.5 us`, but the session directory rotated out before inspection. Set E run 7 retained session `20260526T025650Z_j9S6TUHPwxXzvKiou5wGcg` and reported `74944.2 us` at `GameTimeSeconds=108.441`, followed by a `SingleFrameHitch` at `108.523` with `FrameTimeMs=81.647`. This occurred with `LogCharacterMovement` suppressed and with a stable saturated board (`90` live, `23` rich, `67` lightweight, `0` pending, `0` projectiles), so the spike is not caused by Flying routing or character-movement log spam.
- B.10 update: the issue reproduced under the explicit overhead-rejection policy. CVar-off run 1 attempt 2 was rejected at `23216.803 us`; CVar-on run 1 attempt 2 was rejected at `268567.0 us` even though the process exited cleanly. This directly prevented B.10 from producing a usable CVar-on replacement after the two hero-death exits.
- B.10.1 update: scoped substep attribution landed behind `-T66PerfSubstepAttribution=1`. The RA diagnostic halted at 6 captures after two overhead rejections (`105216.0 us` and `12583.9 us`). A pre-patch attribution run under disk contention showed the dominant costs in file I/O: `EventJsonAppendPeakUs=10086628.3`, `PeriodicSnapshotPeakUs=6477247.0`, and `FrameworkTotalPeakUs=10765523.0`. A clean post-patch attribution rerun with probes enabled completed 5 captures below the 10 ms rejection threshold (`MaxPerfSystemOverheadUs=2115.9`), proving the probes are usable but not proving the underlying I/O stall is fixed.
- B.10.1B resolution update: a reviewed fixed-capacity (`4096`) single-worker write queue now owns `events.jsonl` appends and non-forced `snapshot.current.json` replacements off the game thread. Forced snapshots and final reports flush before synchronous file operations, and shutdown uses the fixed `10.0` second teardown timeout through `StopAndJoin`. The mandatory ordering self-test intentionally forced queue-full fallback and passed after fixing a false-idle race (`EventIds=1..10`, `QueueFullFallbackWrites=2`, invalid JSONL lines `0`). Probe-on validation completed 5/5 route-valid captures with `MaxPerfSystemOverheadUs=890.9`, no overhead rejects, no failed/fallback/abandoned normal-capacity writes, and balanced queue accounting. Clean RA then completed 10/10 with max overhead `1312.5 us`; clean RB completed 10/10 with max overhead `1049.6 us`. The worker observed a `27822.7 us` write in RB run 8 while game-thread framework overhead stayed `1049.6 us`, proving PerformanceSystem write latency is no longer charged to the game-thread framework budget.
- B.10.1C-freeze update: the short freeze matrix did not reproduce a PerformanceSystem-overhead freeze on the final staged binary. All six matrix rows stayed below the `10 ms` rejection threshold (`3273.3 us` max), queue accounting remained balanced, `FailedWrites=0`, `FallbackWrites=0`, `AbandonedWrites=0`, and max queue depth was `1` or `2`. The visible-freeze evidence instead points to Gameplay diagnostic log volume before the final B.10.1C mitigation. However, staged executable provenance drifted during the investigation window, so future capture workflows need to keep hash/timestamp provenance as a hard preflight and detect mid-pass binary changes.
- Why it remains listed: Clean-environment controls still matter. The queue fixes PerformanceSystem-owned file-write stalls; it cannot prevent unrelated disk contention from other processes.
- What future work would entail: If a future capture set shows overhead rejects with clean environment and balanced queue counters, add a new attribution pass for the remaining owner. If only non-PerformanceSystem disk contention is present, fix the launch methodology rather than the in-engine write path.

## Staged Executable Provenance Can Drift During Diagnostics

- Severity tag: [Major]
- What's wrong: The B.10.1C-freeze investigation observed multiple staged `T66.exe` hashes/timestamps during the same diagnostic window before the final matrix was run. Earlier preflight saw SHA `631972E6135572D8A4484B094017DF9828528C66AE08E1C49C523AB44190E452` at `2026-05-27T02:57:27Z`; an aborted runner saw SHA `C16431BAE717DFE3DA160E26AF8A904AE91B8CA9F7E65F5BFE0E8AC84CB752CC` at `2026-05-27T04:01:28Z`; the final matrix used SHA `C4E6053B00CFFF722CFEB2FC7E976EEC0504CD7B9D1B21608A6335A1305752E5` at `2026-05-27T04:26:21Z`. The final matrix is internally valid because it recorded provenance and ran under a clean process check, but cross-run comparisons become unsafe when the staged executable changes mid-pass.
- Why it's out of scope now: This pass is investigation-only and cannot redesign the capture harness or coordinate other local build/stage processes.
- What fixing it would entail: Promote binary hash/timestamp recording into the standard capture harness, recheck it before every set and after every set, and fail the capture set if `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, or executable hash drift appears during the run.

## Matrix Rows Artifact

The full `freeze_matrix_rows_simple.jsonl` file is intentionally referenced rather than embedded to keep this handoff readable. It contains the per-run scalar rows used by the copied plan section, including command lines, log paths, session paths, tail paths, process sample paths, save fingerprints, and overhead fields.

Rows artifact: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\freeze_matrix_rows_simple.jsonl`

