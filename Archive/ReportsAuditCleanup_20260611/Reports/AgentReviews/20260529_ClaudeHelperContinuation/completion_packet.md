# Completion Packet: Fix Helper Max-Turn And Continuation Behavior

Operator: Claude
Validator: Codex
Date: 2026-05-29
Approval: Reports/AgentReviews/20260529_ClaudeHelperContinuation/codex_operator_approval.md (first non-empty line `Codex Approval: APPROVE`)

This is an Operator work artifact, not a greenlight. Codex validates the actual changes and writes the final user-facing report.

## What Changed

`Scripts/Invoke-ClaudeDirectRead.ps1`:

1. **Removed default `--max-turns 10`.**
   - Parameter changed from `[ValidateRange(1,10)] [int] $MaxTurns = 10` to `[ValidateRange(0,100)] [int] $MaxTurns = 0`.
   - `0` (the new default) means no `--max-turns` flag is passed. The flag is only emitted when `$MaxTurns -gt 0`, so callers can still cap explicitly.

2. **Stopped passing `--no-session-persistence` by default.**
   - Added `[switch] $NoSessionPersistence`. The `--no-session-persistence` flag is only emitted when that switch is set. Sessions persist by default, which is what makes resume-based continuation possible.

3. **Added automatic max-turn continuation.**
   - New helpers: `Get-ClaudePayloadFromStdout`, `Test-ClaudeMaxTurnResult` (matches `subtype=error_max_turns` or `terminal_reason=max_turns`), `Get-ClaudeSessionId`.
   - The single CLI invocation logic was factored into `Invoke-ClaudeRun` (supports `--resume <session_id>`, conditional `--max-turns`, conditional `--no-session-persistence`).
   - `Invoke-ClaudeAttempt` now wraps `Invoke-ClaudeRun`: on a max-turn result it resumes the returned `session_id` via `--resume`, bounded by new parameter `[ValidateRange(0,10)] [int] $MaxTurnContinuations = 3`. Continuation is skipped when `-NoSessionPersistence` is set (resume needs a persisted session). A run that still reports max-turns after exhausting continuations is forced to `Success=$false`.
   - Continuation runs write distinct artifacts: `stdout_attempt{N}_cont{K}.{ext}` / `stderr_attempt{N}_cont{K}.txt`.

4. **Manifest is now written even on failure.**
   - The failure branch writes `manifest.json` with `ArtifactKind=ClaudeHelperFailed` before throwing, including: `FailureKind`, per-attempt `StdoutPath`/`StderrPath`/`TimedOut`/`ExitCode`/`Success`, `MaxTurnContinuationAttempted` (any attempt), per-attempt continuation counts, and `ClaudeTokensSpent` parsed from the last attempt whose stdout JSON exposes usage (otherwise `$null`, never a misleading `0`).
   - The success manifest gained `MaxTurns`, `MaxTurnContinuations`, `NoSessionPersistence`, and `MaxTurnContinuationAttempted`.

5. **Safety gates unchanged.**
   - `Assert-NoAnthropicApiKey` (API-key billing block unless `-AllowApiKeyBilling`), `Assert-CodexApproval` for FullOperator, and the `FullOperator` + plan-mode rejection are all intact and untouched.

`OPERATOR_VALIDATOR_PROTOCOL.md` (Operator Tokens section) and `AGENTS.md` (footer rule): clarified that `Claude Tokens Spent: 0` means Claude was genuinely not invoked; a failed/incomplete Claude run reports parseable usage from the manifest/stdout JSON, otherwise `Unavailable` — never `0`.

## Files Edited

- `Scripts/Invoke-ClaudeDirectRead.ps1`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `AGENTS.md`
- `Reports/AgentReviews/20260529_ClaudeHelperContinuation/completion_packet.md` (this file)

## Validation Performed

- **PowerShell parser check** on `Scripts/Invoke-ClaudeDirectRead.ps1` via `[System.Management.Automation.Language.Parser]::ParseFile` → `PowerShell parse OK` (0 errors), run after the final edit.
- **Grep** confirms no unconditional flags remain:
  - `--no-session-persistence` appears only inside `if ($NoSessionPersistence)`.
  - `--max-turns` appears only inside `if ($MaxTurns -gt 0)`.
  - `--resume` / `session_id` / `MaxTurnContinuations` / failure `manifest` all present as intended.
- **Token-free smoke test**: invoked the script with a non-existent `-PromptPath`. It threw `Prompt file not found: ...` — the prompt-existence guard runs before any Claude invocation, so this exercises new parameter binding (`MaxTurns=0` default, `MaxTurnContinuations`, `NoSessionPersistence`) without spending tokens or hitting the network.

## Caveats / Notes For Validator

- A live helper-managed max-turn to continuation cycle was not intentionally forced because that would spend additional Claude tokens. The continuation path, `--resume` argument shape, and the exact JSON field names (`subtype=error_max_turns`, `terminal_reason=max_turns`, `session_id`) are based on the actual failed helper payloads from the previous task and should be exercised naturally on the next max-turn event.
- Continuation passes a fixed prompt ("Please continue the task...") over the resumed session. This was a judgment call; not validated against live CLI resume semantics.
- `--resume` and `--no-session-persistence` are mutually exclusive by design; continuation is intentionally disabled when `-NoSessionPersistence` is set.
- Doc edits were kept minimal and confined to the token-reporting rule, per approved scope.

## Claude Token Count

1,696,274 from the two direct Claude CLI bootstrap stdout JSON `modelUsage` totals. The first direct attempt hit a temporary 529 overload before doing useful work; the second completed the Operator edit.
