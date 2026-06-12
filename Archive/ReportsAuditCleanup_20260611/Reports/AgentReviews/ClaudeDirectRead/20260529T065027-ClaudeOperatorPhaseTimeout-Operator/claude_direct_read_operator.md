Operator work complete. Summary of what changed and verification:

## Files Changed

**`Scripts/Invoke-ClaudeDirectRead.ps1`**
- FullOperator default timeout: `1200s` → `0` (unbounded). Read-only stays `180s`.
- Updated the `-TimeoutSeconds` param comment, the `.DESCRIPTION` synopsis, and added an inline comment so all helper text is now truthful about the unbounded default.

**`OPERATOR_VALIDATOR_PROTOCOL.md`** (detailed authority)
- Rewrote the Helper Timeout bullet: FullOperator defaults unbounded; explicit `-TimeoutSeconds N` only for an intentional timebox; read-only and timeboxed probes keep the short guard.
- Added a new **Phase-Bounded Operator Tasks** section: defines what counts as broad (multi-system, Unreal/editor asset work, runtime+assets, docs+runtime+proof, build/capture harness plus implementation, multi-proof-class verification), requires a phase plan before approval, approves phases one at a time, gives a non-rigid 5-phase recommended shape, and preserves Claude-authors / Codex-validates.
- Added a Codex Approval Gate paragraph requiring per-phase approval for broad tasks.

**`AGENTS.md`** (kept as short router)
- One bullet pointing to the new phase-bounded rule and unbounded timeout default, with detail delegated to the protocol file.

## Verification (all PASS)
- PowerShell parse-check of the edited script: `PARSE OK`.
- FullOperator preflight: `TimeoutPolicy: Unbounded (no wall-clock guard)`.
- Read-only preflight: `TimeoutPolicy: 180 s per attempt` (preserved).
- Explicit FullOperator `-TimeoutSeconds 600` preflight: `600 s per attempt` (timebox still honored).
- `git diff --check` on the three touched files: clean (only a benign LF→CRLF warning).

Completion packet written to `Reports/AgentReviews/20260529_ClaudeOperatorPhaseTimeout/claude_completion_packet.md`.

**Unverified:** No live FullOperator run was launched (preflight only), so end-to-end unbounded-run behavior wasn't exercised. No helper manifest was produced — this Operator ran directly, and preflight runs don't write manifests. Token counts were not exposed to this run (marked `Unavailable` in the ledger for Codex to populate). This is an Operator artifact, not a greenlight — Codex should validate the diff and write the final report.
