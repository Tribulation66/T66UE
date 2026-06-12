Codex Approval: APPROVE

Approved operator: Claude
Validator: Codex
Task contract:
- Make the Claude helper enforce the intended Claude-Operator wrapper behavior by removing the default helper max-turn cap, allowing session persistence by default, adding automatic continuation support for max-turn failures, and writing useful manifests/token data even when Claude fails.
- Update process docs only where needed to say the helper should not report Claude Tokens Spent as 0 when Claude was not run; it should report 0 only when Claude truly was not invoked, and otherwise use manifest/stdout usage or Unavailable.

Approved edit scope:
- Scripts/Invoke-ClaudeDirectRead.ps1
- AGENTS.md only if the final token-reporting rule needs clarification
- OPERATOR_VALIDATOR_PROTOCOL.md only if helper behavior or token-ledger wording needs clarification
- Reports/AgentReviews/20260529_ClaudeHelperContinuation/

Out of scope:
- Runtime gameplay code.
- Unreal assets, Blender assets, Niagara assets, packaged builds, tray/widget code.
- Native goal tooling.
- Broad repo cleanup or unrelated process rewrites.

Required implementation:
- Do not use Claude plan mode.
- Remove the helper's default `--max-turns 10` behavior. Prefer no `--max-turns` flag by default; keep an optional parameter so a caller can set a cap when desired.
- Stop using `--no-session-persistence` by default. If a caller needs non-persistent sessions, provide an explicit opt-in switch/parameter.
- Add automatic continuation for Claude `error_max_turns` / `terminal_reason=max_turns` results using the returned `session_id` and Claude CLI resume/continue support.
- Ensure failure runs still write a manifest with FailureKind, stdout/stderr paths, token usage when parseable, and whether max-turn continuation was attempted.
- Keep API-key billing safety checks intact.
- Keep Codex approval checks intact for FullOperator.

Validation expected from Operator:
- PowerShell parser check for `Scripts/Invoke-ClaudeDirectRead.ps1`.
- A narrow grep showing no unconditional `--no-session-persistence` or unconditional `--max-turns` remains.
- A minimal helper smoke test if feasible without spending large tokens; otherwise explain why skipped.
