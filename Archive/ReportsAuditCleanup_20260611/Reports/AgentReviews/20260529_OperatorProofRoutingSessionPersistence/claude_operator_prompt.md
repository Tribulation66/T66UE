# Claude Operator Prompt: Session Persistence And Proof Routing

## Working Task

The user approved implementation of these three fixes:

1. Make session persistence opt-in for Operator runs when `MaxTurns = 0`. The current FullOperator baseline is `MaxTurns = 0` and unbounded timeout. Persistence mainly exists for max-turn resume, and a resume failed once with an API error about modified `thinking` / `redacted_thinking` blocks. Default persistence should be off when it is not useful.
2. Route proof-bearing work to FullOperator earlier. If a task requires builds, commandlets, captures, gameplay proof, or visual judgment, a read-only packet can be planning only; the implementation/proof phase must be FullOperator after Codex approval.
3. Keep Codex as final proof owner. Claude can run/build/capture in FullOperator mode, but Codex validates the actual evidence and remains the user-facing integrator/final reporter.

Implement these changes directly in the approved scope.

## Current Role State

- Operator: Claude
- Validator/Finisher: Codex
- Repo: `C:\UE\T66`
- Approval artifact: `C:\UE\T66\Reports\AgentReviews\20260529_OperatorProofRoutingSessionPersistence\codex_operator_approval.md`

## Files To Inspect

- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1`
- `C:\UE\T66\Scripts\Invoke-ClaudeReadOnlyOperator.ps1`
- `C:\UE\T66\Reports\AGENTS.md`

## Required Implementation Details

### Session Persistence

Update `Invoke-ClaudeDirectRead.ps1` so the effective default is:

- If `MaxTurns <= 0`, session persistence is off by default.
- If `MaxTurns > 0`, session persistence may stay on by default because max-turn auto-continuation can use `--resume`.
- `-NoSessionPersistence` still forces session persistence off.
- If you need an explicit opt-in switch for persistence with `MaxTurns <= 0`, add one with clear help/preflight wording. Prefer backwards-safe behavior and clear manifest fields over broad redesign.
- Preflight and manifests must report the effective session persistence truthfully.
- Keep max-turn continuation disabled when effective session persistence is off.
- If you add a new switch, update the read-only wrapper forwarding only if needed.

### Proof-Bearing Work Routing

Update the protocol/docs so future agents understand:

- A proof-bearing task is any task whose acceptance requires produced build logs, commandlet markers, runtime/editor captures, gameplay proof, visual judgment, or multiple proof classes.
- Read-only Operator packets can plan these tasks but cannot be the implementation/proof phase.
- After Codex approval, the proof-bearing implementation/proof phase should run as FullOperator so Claude performs the expensive build/commandlet/capture/proof production work.

### Codex Final Proof Owner

Update the protocol/docs so future agents understand:

- Claude-produced proof is evidence, not final acceptance.
- Codex validates existence, freshness, scope, and adequacy of the actual evidence.
- Codex remains the final user-facing integrator/reporter unless the user explicitly changes that.

## Constraints

- Do not use native goal functions.
- Do not use Claude plan mode.
- Do not edit unrelated sections.
- Do not touch runtime/gameplay/content/assets/build outputs.
- Do not run Unreal, Blender, Niagara, staged builds, git commit/push/tag/reset/clean, or broad Git/LFS scans.
- Preserve unrelated user changes.

## Verification To Run

- Parse-check any edited PowerShell scripts.
- Run FullOperator preflight with defaults and confirm `SessionPersistence: False` when `MaxTurns = 0`.
- Run FullOperator preflight with `-MaxTurns 10` and confirm session persistence behavior is truthful and intentional.
- Run FullOperator preflight with `-NoSessionPersistence` and confirm `SessionPersistence: False`.
- Run ReadOnly preflight and confirm behavior remains sensible.
- Narrow search for proof-bearing routing and Codex final proof owner wording.
- Run narrow `git diff --check` only on touched files.

## Required Final Operator Artifact

Write a completion packet at:

`C:\UE\T66\Reports\AgentReviews\20260529_OperatorProofRoutingSessionPersistence\claude_completion_packet.md`

It must list:

- Files changed.
- Exact changes made.
- Commands run and pass/fail results.
- Any unverified areas.
- Remaining blockers/follow-ups.

Your helper response should summarize the same. Codex will validate before reporting to the user.
