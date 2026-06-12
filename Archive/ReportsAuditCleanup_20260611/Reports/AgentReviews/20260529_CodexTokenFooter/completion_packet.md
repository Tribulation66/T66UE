# Completion Packet: Non-Goal Codex Token Footer Support

Date: 2026-05-29
Operator: Claude (full-access T66 Operator)
Validator: Codex
Approval: `Reports/AgentReviews/20260529_CodexTokenFooter/codex_operator_approval.md`

## Summary

Added a read-only helper that reports the latest completed Codex turn token
usage from Codex rollout JSONL (a non-goal source), and updated process docs so
agents populate `Codex Token Spent` from it before final answers instead of
defaulting to `Unavailable`.

## Changed Files

1. `Scripts/Get-CodexTokenUsage.ps1` (new)
   - Read-only; does not write/mutate; uses no native goal API
     (`/goal`, `create_goal`, `get_goal`, etc.).
   - Resolves Codex home from `$env:CODEX_HOME`, else `C:\Users\DoPra\.codex`.
   - Finds recent `rollout-*.jsonl` under `<CodexHome>\sessions`, newest by
     `LastWriteTime` (configurable `-MaxFiles`, default 5).
   - Scans each file from the end for the latest `event_msg` line with
     `payload.type == token_count` and `payload.info.last_token_usage.total_tokens`.
   - Returns object with: `Available`, `CodexTokenSpent`, `Label`,
     `TotalSessionTokens`, `InputTokens`, `CachedInputTokens`, `OutputTokens`,
     `ReasoningOutputTokens`, `PrimaryUsedPercent`, `SecondaryUsedPercent`,
     `RolloutPath`, `Timestamp`, `Caveat`, `UnavailableReason`.
   - Supports `-Json`. Normal "no data" returns `Available = $false`; only
     actual script/parse errors throw.

2. `AGENTS.md` (token footer rules)
   - Added instruction to run `Scripts\Get-CodexTokenUsage.ps1` before final
     answers and populate `Codex Token Spent` from `CodexTokenSpent`.
   - Clarified the figure is the latest completed Codex turn before the final
     answer (final answer tokens flush after send).
   - `Unavailable` only when helper unavailable / `Available = $false`.

3. `OPERATOR_VALIDATOR_PROTOCOL.md` (token ledger / footer)
   - `Validator/Finisher Tokens` now names the helper as the canonical non-goal
     source and forbids defaulting to `Unavailable` merely because no goal
     function exists.
   - `Final User-Facing Footer` references the helper and the latest-completed-
     turn labeling.

## Validation

- PowerShell parser check: `PowerShell parse OK`.
- `Get-CodexTokenUsage.ps1` (live data) returned `Available = True`,
  `CodexTokenSpent = 172534`, `TotalSessionTokens = 121913234`,
  `PrimaryUsedPercent = 14`, `SecondaryUsedPercent = 90`, with `RolloutPath`
  and `Timestamp` populated from
  `...\sessions\2026\05\28\rollout-2026-05-28T11-24-20-019e6ef8-...jsonl`.
- `-Json` emitted valid JSON with the same fields.
- Unavailable path (`-CodexHome` pointing at a missing dir) returned
  `Available = false` with a clear `UnavailableReason`, no throw.
- Doc anchors confirmed via:
  `rg -n "Get-CodexTokenUsage|latest completed Codex turn|Codex Token Spent" AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md`
  — matches in both files.

## Caveats

- `CodexTokenSpent` reflects the latest completed Codex turn before the final
  answer. The final answer's own tokens are not counted until that turn flushes
  its `token_count` event after the answer is sent — this is documented in the
  helper's `Caveat` field and in both docs.
- The helper reads the newest rollout file by `LastWriteTime`; if multiple Codex
  sessions are active, it reflects the most recently written one.
- No git operations were performed (out of scope). Changes are staged in the
  working tree for Codex validation.

## Token Ledger

AuthoringTokens: Claude=742,311
ReviewTokens: Codex=latest completed rollout value at validation time
FinishTokens: Codex=latest completed rollout value at finalization time
OperatorIsAuthoring: YES
PerModel: Claude=742,311, Codex=dynamic latest completed Codex turn from `Scripts/Get-CodexTokenUsage.ps1`
TargetMet: YES
Notes: Claude token count came from the Claude helper manifest. Codex token
figures are intentionally dynamic because the new helper reads the latest
rollout `token_count` event; final-answer tokens flush only after the answer is
sent.
