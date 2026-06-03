# Completion Packet: Full Claude Operator Permissions

## Outcome

Implemented the user's revised Operator model: Claude Operator can use a full Claude Code tool surface after Codex approval, and Claude Operator runs must not use Claude plan mode.

## Files Changed

- `Scripts/Invoke-ClaudeDirectRead.ps1`
  - Default permission mode changed from `plan` to `default`.
  - Added `-ToolProfile ReadOnly|FullOperator`.
  - Full Operator mode defaults to `bypassPermissions`, uses Claude `--tools default`, and refuses `plan`.
  - Full Operator mode requires `-CodexApprovalPath`.
  - Approval artifact must begin with `Codex Approval: APPROVE`.
  - Full Operator prompt allows edits, shell commands, and configured MCP/editor tools inside the approved scope.

- `Scripts/Invoke-ClaudePlanReview.ps1`
  - Review helper now uses Claude `default` permission mode instead of `plan`.

- `OPERATOR_VALIDATOR_PROTOCOL.md`
  - Redefined Operator around authoring work, not just planning.
  - Added the Codex Approval Gate.
  - Replaced Integrator language with Validator/Finisher.
  - Added full Claude Operator command profile and no-plan rule.
  - Updated token ledger to phase-attributed authoring/review/finish buckets.

- `AGENTS.md`
  - Routed Tier 1 Claude Operator implementation through Codex approval plus full Operator mode.
  - Clarified Codex validates actual changes and writes the final report.

- `Reports/AGENTS.md`
  - Added routing for Codex approval artifacts and full Claude Operator run folders.

## Verification

- Parsed `Invoke-ClaudeDirectRead.ps1` successfully with PowerShell parser.
- Parsed `Invoke-ClaudePlanReview.ps1` successfully with PowerShell parser.
- Ran a missing-approval smoke test against `Invoke-ClaudeDirectRead.ps1 -Mode Operator`; it failed before launching Claude with `FailureKind=MissingCodexApproval`, proving the approval gate is enforced.
- Ran a no-op full Operator smoke test with `ToolProfile: FullOperator`, `PermissionMode: bypassPermissions`, and a Codex approval artifact. Claude returned `Full Operator smoke OK.` and wrote manifest `Reports/AgentReviews/ClaudeDirectRead/20260529T035917-FullOperatorSmoke-Operator/manifest.json`.
- Checked process docs for stale `plan mode`, `Integrator`, and permission-routing language.

## Token Ledger

AuthoringTokens: Codex current-goal count
ReviewTokens: Claude 33027 for no-op full Operator smoke validation
FinishTokens: Codex current-goal count
OperatorIsAuthoring: NO for this implementation-only infrastructure edit; YES for future Claude full Operator tasks after Codex approval
PerModel: Claude=33027, Codex=<reported by goal tool>
TargetMet: NO for this turn, because Codex implemented the enabling infrastructure before the full Claude Operator mode existed

## Caveats

The helper now gives Claude the broad tool surface through Claude Code when Codex approval exists. Actual Unreal/Niagara access still depends on the configured Claude environment and MCP/editor routes available to Claude Code at run time. `claude mcp list` currently reports Blender connected.
