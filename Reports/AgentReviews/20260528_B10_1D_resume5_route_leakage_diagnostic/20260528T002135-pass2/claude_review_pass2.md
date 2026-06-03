Verdict: APPROVE

## Blockers

None.

## Major Issues

- **Schema line length is not validated.** The `[RouteAttributionSummary]` key/value line carries ~50 fields and will be in the ballpark of 1.2-1.5 KB once values populate. UE log emission and the runner's line-based parser should both be confirmed against the worst case before relying on a single emission. If `UE_LOG` truncates or splits the line, every row degrades to `CounterMismatch != 0` and the pass yields nothing. Mitigation: confirm the chosen log category/verbosity does not truncate at the expected line length, or pre-emptively split into two lines (`[RouteAttributionSummaryA]` / `[RouteAttributionSummaryB]`) with the parser stitching by `ResetCount`/`WorldTime`.
- **Runner `-Mode RouteDiagnostic` is described as additive but the existing script's argument surface is not characterized.** The packet does not state whether `run_b101d_projectile_manager_validation.ps1` currently exposes a `-Mode` parameter, what the default mode is, or how non-`RouteDiagnostic` invocations (existing automation, prior acceptance reruns) keep their current behavior. Codex needs to commit to: (a) default mode preserves existing behavior bit-for-bit; (b) `RouteDiagnostic` only relaxes the route-leak halt and adds the new parser fields; (c) hash/overhead/hero-death/exit-code/summary-missing rejects remain identical between modes.

## Minor Issues

- **"Preferably adjacent to the existing ranged aggregate diagnostics" is too soft for a plan commitment.** Codex should commit to placing the route-attribution struct in `UT66MobManagerSubsystem` (header + cpp) and tying its reset/emit to `ResetRangedPressureDiagnostics`/`EmitRangedPressureSummary` outright, not "preferably."
- **Resume4 attempted-CVar-on row count is not pinned.** Evidence says "halted after two `RouteValidity` rejects" and the 5-capture justification cites "two route leaks within five attempted CVar-on rows." The "five attempted" figure is not in the Current State And Evidence section; either restate it there or correct the justification so the 5-vs-10 cadence rests on a verifiable count.
- **Mini-boss promotion vs. measurement contract is acknowledged but the disposition is left implicit.** If a Dungeon Ranged MobID is promoted to mini-boss during `enemywaveperf`, that is current-code-correct but contract-wrong. The acceptance criteria should call out that the pass must report whether this branch is active during the captures, not only categorize it.
- **`AT66PlayerController_Overlays.cpp` automation/debug spawn audit is conflated with non-director production paths.** Automation-only spawns that fire during `enemywaveperf` need to be either disabled for the capture or counted as `RoutedRich_NonDirectorPath`. Either is fine, but the packet should pick one to avoid mystery rich spawns showing up under "production" buckets.
- **`UT66EnemyPoolSubsystem::TryAcquire` is listed for audit but not classified.** Pool reuse on the director path is part of the director pipeline, while pool reuse from non-director callers is non-director. The packet should state which side counts which.
- **Acceptance criterion "leak reason is identified with counter evidence" has no fallback for ambiguous data.** Specify what counts as "identified" (e.g., one reason bucket >= 95% of leaked rich routes for the affected family) and what the pass produces if no bucket dominates.
- **The 50000 automation hero HP cap and HP20000 capture HP are both in play.** Confirm explicitly that HP20000 is the capture's hero HP and the 50000 cap is the ceiling, not a substitution. One sentence avoids a Codex misread.
- **Behavioral-neutrality envelope is anchored to Resume4 row range, which is fine, but no decision rule is given for "outside envelope but close."** Either accept a tolerance (e.g., ±5% on median, full overlap of row range) or state "any row outside the Resume4 envelope halts."

## Clarifying Questions

- Does the existing runner script already expose `-Mode` or any other mode/profile switch? If not, is Codex allowed to add a parameter, or should `RouteDiagnostic` be invoked via a different switch (e.g., `-AllowRouteLeakRows`)?
- Is the standard `enemywaveperf` Dungeon capture guaranteed to contain zero `TowerGuardian` / `Tutorial` / `Lab` / `TestRoom` spawns, or is the pass expected to confirm absence from the counters?
- When the diagnostic build emits `[RouteAttributionSummary]`, should the existing `[RangedDecisionSummary]` continue to gate route validity in non-`RouteDiagnostic` modes unchanged, or does this packet implicitly retire the `RichSpawns>0` halt for all modes?
- Is the 10-capture extension a one-time bump or does it itself extend if leaks remain absent at 10?

## Required Verification

- Pre-implementation: confirm `ANTHROPIC_API_KEY` absent from all scopes (already in the packet).
- Build: `Build.bat T66 Win64 Development` succeeds; staged binary hash captured before captures begin.
- Behavioral neutrality: one CVar-off control with counters compiled in, `T66.Ranged.DiagnosticLogging=1`, hero HP20000, compared to Resume4 CVar-off row envelope. Halt on perturbation rather than proceeding.
- Counter wholeness: every accepted CVar-off and CVar-on row must show `CounterMismatch=0`. Any non-zero is an instrumentation gap and must be fixed before the pass concludes.
- Clean-environment gate before each capture: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, unexpected `git-lfs`, or long-lived broad `git status` workers; no broad Content-tree LFS scans during the Unreal process lifetime.
- Schema sanity: confirm one and only one `[RouteAttributionSummary]` per capture, in full (no truncation).
- Hash stability: staged binary hash recorded at pass start, per row, and pass end; hash-drift rows discarded.
- Single 5-row CVar-on diagnostic batch first; extend to 10 only if zero leakage observed in the first 5.
- Post-pass: documentation updates land in `PerformanceSystem/2026-05-23_T66_LightweightActor_Plan.md`, `Source/T66/Gameplay/pending_issues_Gameplay.md`, and the Resume5 section of `B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`. Pablo go-ahead gate honored before implementation begins.

## Rationale

The packet is goal-focused, scope-bounded, and faithful to the live code paths it cites. Route attribution as additive, CVar-gated, aggregate counters with terminal emission is the right shape: it reuses the existing `[RangedDecisionSummary]` lifecycle, avoids per-spawn log spam, and preserves projectile-manager/HISM/LOS/peer-filter/HP/PerformanceSystem mitigations. The separation of mini-boss promotion, family-lookup failure, lightweight-acquire fallback, and non-director paths is the analytically important distinction Resume4 lacked, and the `CounterMismatch` sanity field gives the runner a way to refuse silently-broken instrumentation. The behavioral-neutrality control before CVar-on diagnostics is the correct guard against instrumentation perturbing the very behavior under measurement. Risks are recognized with concrete mitigations, rollback is trivially additive, out-of-scope is well-fenced (no routing fix, no Ranged behavior change, no B.10 acceptance reattempt), and the Pablo go-ahead gate is preserved.

The major items above are clarifications and small commits Codex should fold in before execution, not architectural concerns. The minor items would tighten the pass but none block it. The packet is safe for Codex to present at the user go-ahead gate.

