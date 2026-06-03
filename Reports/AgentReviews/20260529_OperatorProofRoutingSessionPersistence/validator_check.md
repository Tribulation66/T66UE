Verdict: APPROVE

## Scope Validated

Claude Operator implemented the three approved fixes:

- Session persistence is effectively opt-in for uncapped Operator runs.
- Proof-bearing work routes to FullOperator for implementation/proof phases.
- Codex remains the final proof owner and user-facing reporter.

## Files Validated

- `Scripts/Invoke-ClaudeDirectRead.ps1`
- `Scripts/Invoke-ClaudeReadOnlyOperator.ps1`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `AGENTS.md`
- `Reports/AgentReviews/20260529_OperatorProofRoutingSessionPersistence/claude_completion_packet.md`
- `Reports/AgentReviews/ClaudeDirectRead/20260529T082559-OperatorProofRoutingSessionPersistence-Operator/manifest.json`

## Helper Behavior Checks

- PowerShell parse-check for `Invoke-ClaudeDirectRead.ps1` — PASS.
- PowerShell parse-check for `Invoke-ClaudeReadOnlyOperator.ps1` — PASS.
- FullOperator default preflight (`MaxTurns = 0`) — PASS: `SessionPersistence: False`, source `default off (MaxTurns <= 0 has no max-turn resume to support)`.
- FullOperator with `-MaxTurns 10` — PASS: `SessionPersistence: True`, source `default on (MaxTurns > 0 can --resume on max-turn)`.
- FullOperator with `-NoSessionPersistence` — PASS: `SessionPersistence: False`, source `forced off by -NoSessionPersistence`.
- FullOperator with `-SessionPersistence` — PASS: `SessionPersistence: True`, source `forced on by -SessionPersistence`.
- FullOperator with both `-SessionPersistence` and `-NoSessionPersistence` — PASS: throws mutual-exclusion error.
- FullOperator with `-MaxTurns 10 -NoSessionPersistence` — PASS: forced-off overrides the turn-cap default.
- ReadOnly wrapper default preflight — PASS: `SessionPersistence: False`, `ToolProfile: ReadOnly`, `MutatingCapability: False`.
- ReadOnly wrapper with `-MaxTurns 5` — PASS: `SessionPersistence: True`.
- ReadOnly wrapper with `-SessionPersistence` — PASS: wrapper forwards the new switch.

## Documentation Checks

- `AGENTS.md` now routes proof-bearing work to FullOperator and states Claude-produced proof is evidence, not final acceptance.
- `OPERATOR_VALIDATOR_PROTOCOL.md` now has `Proof-Bearing Work Routing` and `Codex As Final Proof Owner` sections.
- Protocol wording defines proof-bearing tasks as tasks requiring produced build/compile logs, commandlet markers, runtime/editor captures, gameplay proof, visual judgment, or multiple proof classes.
- Protocol wording says read-only packets may plan proof-bearing work but cannot be the implementation/proof phase.
- Protocol wording says Codex validates proof existence, freshness, scope, and adequacy, and remains the final reporter unless the user explicitly changes that.

## Diff Hygiene

- `git diff --check -- AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md Scripts/Invoke-ClaudeDirectRead.ps1 Scripts/Invoke-ClaudeReadOnlyOperator.ps1 Reports/AgentReviews/20260529_OperatorProofRoutingSessionPersistence` — PASS. Git emitted only the existing `AGENTS.md` LF-to-CRLF warning.

## Notes

The Claude helper manifest reports:

- `ClaudeTokensSpent = 2593776`
- `TimeoutSeconds = 0`
- `MaxTurns = 0`
- `NoSessionPersistence = true` for the Operator run itself, because this run intentionally avoided the old persistence behavior while implementing the fix.
