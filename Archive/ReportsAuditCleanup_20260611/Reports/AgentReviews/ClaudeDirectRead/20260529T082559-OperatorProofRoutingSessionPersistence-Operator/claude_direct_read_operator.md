All three approved fixes are implemented and verified. Operator work artifact complete — Codex validates before reporting to the user.

## Summary

**Files changed (4):**
- `Scripts\Invoke-ClaudeDirectRead.ps1` — session persistence now opt-in by max-turn cap
- `Scripts\Invoke-ClaudeReadOnlyOperator.ps1` — `-SessionPersistence` switch parity + help
- `OPERATOR_VALIDATOR_PROTOCOL.md` — proof-bearing routing + Codex final-proof-owner sections
- `AGENTS.md` — one router bullet pointing to the protocol

**Fix 1 — Session persistence:** Effective persistence is now `false` when `MaxTurns <= 0`, `true` when `MaxTurns > 0`. `-NoSessionPersistence` forces off; new `-SessionPersistence` forces on (the explicit opt-in for unbounded runs); the two conflict. Preflight and both manifests report `SessionPersistence` + `SessionPersistenceSource` truthfully, and max-turn auto-continuation stays disabled when effective persistence is off.

**Fix 2 — Proof routing:** New protocol section defines proof-bearing tasks and requires the implementation/proof phase to be a Codex-approved FullOperator run; read-only packets can plan only.

**Fix 3 — Codex final proof owner:** New section codifies Claude-produced proof as evidence (not acceptance) and Codex as validator/final reporter.

**Verification (all PASS):** both scripts parse-check clean; FullOperator preflight matrix (default→False, MaxTurns 10→True, NoSessionPersistence→False, MaxTurns 10+NoSessionPersistence→False, SessionPersistence→True, conflict→throws); ReadOnly wrapper matrix incl. forwarded `-SessionPersistence`; doc wording search; `git diff --check` clean on the four files. No live Claude run (preflight-only, per approval).

Completion packet written to `Reports\AgentReviews\20260529_OperatorProofRoutingSessionPersistence\claude_completion_packet.md`. No git commit/push made.
