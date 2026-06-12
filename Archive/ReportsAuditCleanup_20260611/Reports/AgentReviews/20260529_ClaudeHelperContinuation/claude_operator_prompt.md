# Claude Operator Prompt: Fix Helper Max-Turn And Continuation Behavior

You are Claude Code acting as Operator for `C:\UE\T66`.

Codex has approved this run through:

`C:\UE\T66\Reports\AgentReviews\20260529_ClaudeHelperContinuation\codex_operator_approval.md`

Use the live repo files directly. Do not rely on stale memory.

## User Concern

The previous answer about helper max-turns was produced by Codex, not Claude, even though `.t66\operator-state.json` names Claude as Operator. The user wants the wrapper behavior enforced so Claude does the Operator thinking/work, and Codex mainly forwards, approves scope, validates, and reports.

The user also agreed to:

- Remove the helper max-turn problem.
- Stop using `--no-session-persistence` by default.
- Add automatic continuation.
- Fix token reporting so `Claude Tokens Spent: 0` is not shown when Claude was expected but not run. `0` should mean Claude truly was not invoked; failed/incomplete Claude runs should still report usage when available, otherwise `Unavailable`.

## Required Changes

Primary file:

- `C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1`

Allowed docs if needed:

- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`

Required behavior:

1. Remove the helper's default `--max-turns 10` behavior.
   - Prefer no `--max-turns` flag by default.
   - Keep an optional parameter so callers can explicitly set a cap.
2. Stop using `--no-session-persistence` by default.
   - Add an explicit opt-in switch/parameter if non-persistent sessions are needed.
3. Add automatic continuation when Claude returns JSON with `subtype=error_max_turns` or `terminal_reason=max_turns`.
   - Use the returned `session_id` with Claude CLI resume/continue support.
   - Keep continuations bounded by a parameter such as `-MaxTurnContinuations`.
4. Write a manifest even when the helper ultimately fails.
   - Include FailureKind.
   - Include attempt stdout/stderr paths.
   - Include token usage when parseable from stdout JSON.
   - Include whether max-turn continuation was attempted.
5. Keep all current safety gates:
   - No Anthropic API key billing unless explicitly allowed.
   - FullOperator requires Codex approval.
   - FullOperator must not use Claude plan mode.

## Validation

Run:

```powershell
$tokens=$null; $errors=$null; [System.Management.Automation.Language.Parser]::ParseFile('C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1',[ref]$tokens,[ref]$errors) > $null; if ($errors.Count) { $errors | ForEach-Object { $_.Message }; exit 1 } else { 'PowerShell parse OK' }
rg -n "--no-session-persistence|--max-turns|MaxTurnContinuations|session_id|--resume|manifest" C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1
```

If a tiny smoke test is feasible without spending large tokens, run it. Otherwise state why skipped.

## Completion Packet

Write:

`C:\UE\T66\Reports\AgentReviews\20260529_ClaudeHelperContinuation\completion_packet.md`

Include:

- What changed.
- Files edited.
- Validation performed.
- Caveats.
- Claude token count if available.
