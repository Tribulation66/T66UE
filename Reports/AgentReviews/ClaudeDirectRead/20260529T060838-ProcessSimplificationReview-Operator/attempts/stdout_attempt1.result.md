The report content is ready, but writing the artifact file requires your permission (the write is pending approval). Rather than re-attempt blindly, here's the full recommendation inline — approve the write if you want it persisted to `claude_process_simplification_opinion.md`.

## Bottom line

**The timeout should not be removed today.** It's the only hard wall-clock guard on the Claude subprocess, and `MaxTurns` defaults to `0` (no turn cap) — so an unbounded run could hang with no kill path. Relax and document it instead of deleting it.

## Q1 — Can the timeout be removed with the current helper?

No. Key anchors:
- `Invoke-ClaudeDirectRead.ps1:45` — `[ValidateRange(15, 1200)] $TimeoutSeconds = 180` (this rejected `1800`).
- `:37-38` — `$MaxTurns = 0`, **no default turn cap**.
- `:555-574` — the timeout is the sole mechanism that `Kill()`s a stalled run.

It's also **per-attempt/per-continuation**, not a total budget (`Attempts=2`, `MaxTurnContinuations=3`), so the binding constraint is the per-turn ceiling. Fix instead: raise the ceiling + add `0 = unbounded` opt-in; bump the Operator default (the baseline command in `OPERATOR_VALIDATOR_PROTOCOL.md:78` passes no `-TimeoutSeconds`, so heavy authoring runs under **180s** today); document the range.

## Q2 — Real vs stale

| # | Verdict | Why |
|---|---|---|
| 1 | **Doc ambiguity, not enforcement** | `Assert-CodexApproval` runs **only** for `FullOperator` (`:700-701`). Read-only never requires it in code. |
| 2 | **Real helper bug** | `FullOperator` uses `--tools default` (`:532-534`) so `AllowedTools` is ignored, yet the manifest still emits `Read,Grep,Glob` (`:928`, `:976`). |
| 3 | **Real limitation** | `ValidateRange(15,1200)` cap; only discoverable by reading the script or hitting the error. |
| 4 | **Real, but behavioral** | `AGENTS.md:26-27` already prohibit stale-evidence substitution; this is discipline/prompt, not a helper bug. |
| 5 | **Architectural, not a bug** | Token total is computed by the wrapper *after* the subprocess exits (`:909-910`). Inner Claude cannot self-report; manifest `ClaudeTokensSpent` is the only source. |
| 6 | **Already in docs; host-driven** | `AGENTS.md:6,19,25` already say host goal/tray is telemetry only. Reinforce, don't redesign. |
| 7 | **Real doc gap** | `AGENTS.md:236-241` gates deletion on "durable summary exists" but never lists required proof classes. |

## Q3 — Safe simplification
Relax the timeout (keep the guard); make the manifest report the effective tool surface; state the read-only/no-approval rule once; enumerate required proof classes before raw-folder deletion. Keep the API-key guard, the `FullOperator` approval gate, and the plan-mode prohibition — cheap, high-value safety.

## Q4 — Helper code vs docs
- **Code:** widen/relax `TimeoutSeconds` + raise Operator default + document range (#3); stop emitting unused `AllowedTools` for `FullOperator`, record effective surface (#2).
- **Docs:** clarify read-only needs no approval (#1); add verification-freshness rule to the Operator header + `AGENTS.md:26` (#4); document that token self-report is impossible by design (#5); reinforce host-goal-is-telemetry (#6); add a proof-class checklist to the retention rule (#7).

**Open for Codex:** confirm `--tools default` truly overrides `AllowedTools`; decide the new ceiling / default policy; check no other caller depends on the old `1200` cap before raising it.

The write to `Reports\AgentReviews\20260529_ProcessSimplificationReview\claude_process_simplification_opinion.md` is pending your approval — grant it and I'll persist this artifact for Codex to validate.
