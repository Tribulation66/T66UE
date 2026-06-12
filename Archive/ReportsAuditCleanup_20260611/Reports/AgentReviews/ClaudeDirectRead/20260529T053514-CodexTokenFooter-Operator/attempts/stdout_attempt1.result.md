All four required changes are done and validated within the approved scope. No git operations performed.

**Changed files**
- `Scripts/Get-CodexTokenUsage.ps1` (new) — read-only, no goal API; resolves `$env:CODEX_HOME`→`C:\Users\DoPra\.codex`, finds newest `rollout-*.jsonl` under `\sessions`, scans from the end for the latest `event_msg`/`token_count`/`last_token_usage.total_tokens`, returns all required fields + `-Json`; returns `Available=$false` on no-data rather than throwing.
- `AGENTS.md` — footer rules now direct agents to run the helper and source `Codex Token Spent` from `CodexTokenSpent`, labeled as the latest completed Codex turn before the final answer; `Unavailable` only when helper/data is absent.
- `OPERATOR_VALIDATOR_PROTOCOL.md` — `Validator/Finisher Tokens` and `Final User-Facing Footer` updated to name the helper as the canonical non-goal source.
- `Reports/AgentReviews/20260529_CodexTokenFooter/completion_packet.md` (new).

**Validation**
- Parser: `PowerShell parse OK`.
- Live run: `Available=True`, `CodexTokenSpent=172534`, `TotalSessionTokens=121913234`, primary/secondary 14/90, rollout path + timestamp populated.
- `-Json`: valid JSON, same fields.
- Missing-home path: `Available=false` with clear `UnavailableReason`, no throw.
- `rg` anchors present in both docs.

**Caveat**: `CodexTokenSpent` is the latest *completed* Codex turn — the final answer's own tokens flush only after it's sent (documented in the helper and both docs).

This is an Operator work artifact, not a greenlight — Codex should validate the changes and write the final user-facing report.

**Codex Token Spent:**
172,534 (latest completed Codex turn before final answer, via `Scripts\Get-CodexTokenUsage.ps1`)

**Claude Tokens Spent:**
Unavailable
