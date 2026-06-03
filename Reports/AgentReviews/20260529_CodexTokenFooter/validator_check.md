Verdict: APPROVE

## Packet Completeness Gate

Working task and validation depth: PASS
Roles and tool profile: PASS
User constraints and out-of-scope: PASS
Applicable instructions read: PASS
Evidence and live findings anchored: PASS
PPF/process gates addressed or exempted: PASS
Proposed patch approach: PASS
Verification plan: PASS
Token routing: PASS
Operator position and open decisions: PASS
Anti-lookalike discriminator when required: N/A

## Anchor Spot Checks

- `Scripts/Get-CodexTokenUsage.ps1` is read-only and does not call native goal tools.
- `AGENTS.md` instructs agents to run `Scripts\Get-CodexTokenUsage.ps1` before final user-facing answers.
- `OPERATOR_VALIDATOR_PROTOCOL.md` names the helper as the canonical non-goal Codex token source.
- Claude Operator ran through `Scripts\Invoke-ClaudeDirectRead.ps1` with FullOperator profile and Codex approval.

## Validation

- PowerShell parser check passed for `Scripts/Get-CodexTokenUsage.ps1`.
- Helper live run returned `Available = True` with `CodexTokenSpent`.
- Helper `-Json` output returned valid JSON.
- Missing Codex home returned `Available = False` with `UnavailableReason`, not an unhandled exception.
- `git diff --check` passed for touched files, with only the existing `AGENTS.md` CRLF warning.

## Caveats

- `CodexTokenSpent` is the latest completed Codex turn before final answer. The final answer's own tokens flush after the answer is sent.
- If multiple Codex sessions are actively writing rollout files, the helper follows the newest rollout by `LastWriteTime`.
