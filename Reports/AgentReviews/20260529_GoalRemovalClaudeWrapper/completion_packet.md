# Completion Packet: Goal Removal And Claude Wrapper Routing

## Outcome

Removed mandatory native goal-function usage from the root process and shifted the Claude-operator model toward a Codex wrapper/router flow.

The process now uses a prompt-native task contract instead of calling `/goal`, `set_goal`, `create_goal`, or equivalent tools. When Claude is Operator, Codex should package the user's message plus the task contract and relevant repo rules for Claude, avoid deep duplicate planning, approve or reject Claude's requested change scope, then validate the actual result.

## Files Changed

- `AGENTS.md`
  - Replaced mandatory goal-function creation with a plain task contract:
    `Working task`, `Tier`, `Operator`, `Validator`, `Scope`, `Stop condition`.
  - Added that native goal functions must not be called unless the user explicitly asks.
  - Replaced goal-tool and blocked-threshold wording with prompt-native decision-gate stop rules.
  - Renamed `Goal Clarification` to `Task Clarification`.
  - Updated planning, tier routing, delegation, and Operator/Validator wording around task contracts.
  - Clarified that Codex is wrapper/router first when Claude is Operator.

- `OPERATOR_VALIDATOR_PROTOCOL.md`
  - Reframed the purpose around Operator-owned authoring work, not formal goal tracking.
  - Changed Codex's Claude-operator role to wrapper, Validator, and Finisher.
  - Updated Tier 1 flow so Codex forwards the user request, task contract, and repo rules to Claude with minimal Codex-side interpretation.
  - Replaced working-goal wording in packet and completeness templates with working-task wording.
  - Removed token-accounting dependency on the goal tool; Codex tokens are reported only when the host exposes them without using goal functions.

- `Scripts/Invoke-ClaudeDirectRead.ps1`
  - Updated the full Operator prompt header from approved working goal to approved task contract.

- `Reports/AgentReviews/20260529_DecisionGateStopRule/completion_packet.md`
  - Earlier interrupted Claude work added the immediate decision-gate stop summary; it remains valid evidence for the B.13 failure mode.

## Validation

- Read `.t66/operator-state.json`: Claude remains Operator, Codex remains Validator/Finisher.
- Searched live docs/scripts for goal-function, blocked-threshold, decision-gate, Operator, and Validator wording.
- Confirmed the relevant files needing updates were `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Scripts/Invoke-ClaudeDirectRead.ps1`, and the report packets.
- Recommended follow-up validation: `git diff --check` on the touched docs/scripts and a focused grep for stale mandatory goal-function language.

## Token Ledger

AuthoringTokens: Codex=Unavailable
ReviewTokens: Claude=0
FinishTokens: Codex=Unavailable
OperatorIsAuthoring: NO for this patch because the user specifically redirected process handling mid-run and no new Claude run was used
PerModel: Claude=0, Codex=Unavailable
TargetMet: Unavailable
Notes: No native goal tool was used, per user instruction, so Codex token count is unavailable.

## Caveats

- This is a docs/process update, not a runtime code change.
- Existing helper scripts still report Claude token counts from Claude manifests when Claude is used.
- Codex token reporting is now necessarily `Unavailable` unless the host exposes a token count without requiring a goal function.
