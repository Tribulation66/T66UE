Verdict: APPROVE

## Blockers
- None. The packet is planning-only, respects the user's explicit "do not build yet" instruction, and the deferred execution order requires a fresh go-ahead before scaffolding.

## Major Issues
- None blocking. One item to watch: the plan reads `.claude/.credentials.json` and calls the undocumented `https://api.anthropic.com/api/oauth/usage` endpoint. Scope/security handling is sound (in-memory only, no token logging, read-only access, clear auth-error states), so this is acceptable under the reviewed scope — but Codex must treat the endpoint as unstable/private and isolate it exactly as described.

## Minor Issues
- The Claude `utilization` 0-100 interpretation is pinned only by two low-value observations (`0.0` and a prior `3.0`). The fallback (mark `ParseError`, hold last value as stale, never re-scale) is the correct safety behavior, so this is acceptable — but a weekly value that is genuinely low could look indistinguishable from a parse failure. A brief log note (redacted) at parse boundaries would help diagnose without violating the token-logging ban.
- Codex short-lived `app-server` per 5-min poll plus persistent-mode fallback is reasonable, but the packet should ensure orphan-process cleanup is covered by an actual test, not just the smoke step (it is listed in UI smoke #3 — keep it there).
- Logo/brand assets remain an open gate. Plan correctly defers to a check/ask step (#3), so no action needed now.

## Clarifying Questions
- Source location (personal `C:\Users\DoPra\Tools\AIUsageTray` vs repo-local `Tools/UsageTray`) is the one genuine user-owned decision. The plan correctly defers it to execution step 1 with a confirmation before scaffolding, so this does not require a NEEDS_HUMAN_DECISION verdict — but Codex must actually pause and confirm at step 1 rather than proceeding on the default silently.

## Required Verification
- The packet's verification plan is sufficient: collector unit tests with fixture JSON, live read-only probes for both providers, UI screenshot, orphan-process check, redaction helper test, and negative tests via config overrides (not by moving real credential files). No gaps that would block approval.

## Rationale
The packet satisfies AGENTS.md requirements: goal defined first, live machine state checked, planning-only boundary respected, Claude cross-review performed, quota values taken from structured provider data (no denominator guessing), PPF check present with same-method-class YES, and report placed under `Reports/AgentReviews`. The default placement outside the repo correctly honors `Tools/README.md` scoping, with repo-local offered only as an explicit user-selected alternate. Security boundaries around credentials are explicit and correct. The single user-owned decision (source location) is properly deferred to a confirmation step rather than assumed. Safe for Codex to proceed to implementation under the reviewed scope, confirming source location at step 1 and the autostart/logo gates as specified.

