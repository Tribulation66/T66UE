# Performance System Pending Issues

## Framework Overhead Spike In Phase C.1 Regression Capture

Severity: [Major]

What's wrong: `Saved/Codex/Performance/MassC1/phase_c1_90cap_regression_final2_metrics.json` reported 35 `PerformanceSystemOverhead` events with a max framework cost of 4,216.399 us after Mass scaffolding. The rollback validation reduced the longest clean capture to 1,091.100 us in `Saved/Codex/Performance/MassRollback/mass_rollback_final170_quietmove_metrics.json`, but an earlier clean rollback capture still reached 2,965.599 us in `Saved/Codex/Performance/MassRollback/mass_rollback_final110_quietmove_metrics.json`. The follow-up five-run variance check then recorded a much larger 16,659.997 us spike in `Saved/Codex/Performance/VarianceCharacterization/variance_run01_metrics.json`, while runs 2-5 stayed between 797.100 us and 1,233.302 us. The spike is intermittent and not fully isolated.

Why it's out of scope now: Phase C.1 was scoped to scaffolding Mass types and counters. Further reducing framework overhead would require another profiling pass over event JSONL writes, summary flushes, snapshot rotation, and detector self-cost.

What fixing it would entail: Add scoped self-timing inside the PerformanceSystem tick/finalization path, identify which substep owns the >2 ms cost, and apply one targeted reduction before using full instrumentation as the Mass migration comparator.

## Resolved: B.10.1 Synchronous PerformanceSystem File I/O Rejected Captures

Severity: [Resolved - Major]

What's wrong: B.10.1 added scoped substep attribution around `UT66PerformanceSubsystem::TickPerformanceSystem` and `EmitPerformanceEvent`. The RA diagnostic halted after two overhead-rejected captures (`105216.0 us` and `12583.9 us`). A pre-patch attribution run under disk contention attributed a severe stall to synchronous file I/O on the game thread: `EventJsonAppendPeakUs=10086628.3`, `PeriodicSnapshotPeakUs=6477247.0`, and `FrameworkTotalPeakUs=10765523.0`. A clean post-patch attribution rerun stayed below threshold (`MaxPerfSystemOverheadUs=2115.9`), so the issue is intermittent and contention-sensitive rather than a constant per-frame cost.

Resolution: B.10.1B audited the producer path and found no explicit off-thread `EmitPerformanceEvent` or `WritePeriodicSnapshot` callers, then added Development-visible `ensureMsgf` game-thread guards and a fixed-capacity PerformanceSystem write worker. `events.jsonl` appends and non-forced `snapshot.current.json` replacements now leave the game thread through the ordered queue. The forced summary/finalization path flushes the queue before synchronous outputs that require completed data, and shutdown uses the fixed `10.0` second teardown timeout through `StopAndJoin`. Validation passed with no queue failures, fallback writes, or abandoned writes in normal captures: 5-capture attribution max `PerformanceSystemOverheadMaxUs=890.9`, 10-capture RA max `1312.5`, and 10-capture RB max `1049.6`.

Follow-up: Keep the clean-environment check before future capture sets. The queue removes PerformanceSystem file-write stalls from game-thread overhead, but it does not prevent unrelated disk contention from other processes.
