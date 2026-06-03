Codex Approval: APPROVE

Approved operator: Claude
Validator: Codex
Task contract:
- Make future T66 agent answers populate `Codex Token Spent` from the non-goal Codex rollout token source when available.
- Add a small read-only helper script that finds the latest active Codex rollout JSONL and returns the latest `token_count` event.
- Update process docs so agents use that helper before final answers instead of reporting `Unavailable` when rollout token data is available.

Approved edit scope:
- Scripts/Get-CodexTokenUsage.ps1
- AGENTS.md
- OPERATOR_VALIDATOR_PROTOCOL.md
- Reports/AgentReviews/20260529_CodexTokenFooter/

Out of scope:
- Native goal tooling.
- Runtime gameplay code.
- Unreal, Blender, Niagara, packaged build, or tray/widget changes.
- SQLite writes or DB-locking approaches.
- Git commit/push/tag/reset/clean.
- Broad scans over Unreal Content or binary asset folders.

Required implementation:
- Script must be read-only.
- Script must not use `/goal`, `create_goal`, `get_goal`, or equivalent native goal APIs.
- Prefer latest rollout file under `$env:CODEX_HOME\sessions` or `C:\Users\DoPra\.codex\sessions`.
- Scan from the end for the latest JSONL event whose payload type is `token_count` and whose payload info contains token usage.
- Return enough data for final answers: latest completed Codex turn tokens, cumulative session tokens, rate-limit percents, rollout path, timestamp, and a caveat that the final answer's own tokens are not included until after the turn flushes.
- If unavailable, return a clear unavailable result rather than throwing for normal "not found" cases.
- Update docs to say `Codex Token Spent` should use this helper and label it as latest completed Codex turn.

Validation expected from Operator:
- PowerShell parser check for the new script.
- Run the helper and show sample output.
- Grep or anchor checks proving docs reference the helper and no longer say Codex is unavailable when no goal function exists.
