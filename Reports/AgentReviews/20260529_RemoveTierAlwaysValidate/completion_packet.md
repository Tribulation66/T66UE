# Completion Packet: Remove Tier Routing And Always Validate

## Outcome

Removed the Tier 0 / Tier 1 routing concept from the live process docs and
replaced it with always-on Operator/Validator validation whose depth scales with
risk. The user-facing footer now reports token counts only; it no longer
reports a Tier line.

## Files Changed

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AgentReviews/20260529_RemoveTierAlwaysValidate/codex_operator_approval.md`
- `Reports/AgentReviews/20260529_RemoveTierAlwaysValidate/claude_operator_prompt.md`
- `Reports/AgentReviews/20260529_RemoveTierAlwaysValidate/claude_operator_continuation_prompt.md`
- `Reports/AgentReviews/20260529_RemoveTierAlwaysValidate/validator_check.md`
- `Reports/AgentReviews/20260529_RemoveTierAlwaysValidate/completion_packet.md`

## What Changed

- Kept the prompt-native task contract, but removed the Tier field.
- Replaced Tier routing with validation depth: quick validation for simple
  low-risk work, full validation when risk or process requirements warrant it.
- Required every substantive Operator answer/result to be validated by the
  Validator before it is sent to the user, unless the user explicitly opts out.
- Preserved the no-native-goal rule unless the user explicitly asks for a goal.
- Preserved Claude-as-Operator wrapper behavior: Codex receives the request,
  forwards the task contract and repo rules to Claude, approves or rejects
  scope, validates the real Operator output, and finishes the user-facing
  report.
- Removed the final `Tier` footer line from process templates.

## Wrapper Test Result

Claude was invoked twice through `Scripts\Invoke-ClaudeDirectRead.ps1` with
`-ToolProfile FullOperator` and a Codex approval artifact. Claude performed part
of the live process-doc edit by updating `AGENTS.md`, proving the write-enabled
wrapper path can reach the repo and make approved changes.

Both Claude runs ended with `error_max_turns` before producing a completion
packet or manifest. Codex Validator/Finisher completed the remaining bounded
`OPERATOR_VALIDATOR_PROTOCOL.md` cleanup, wrote this packet, and validated the
final state. This is an important process caveat: the wrapper works, but
operator prompts need to stay narrow or the helper needs continuation handling
that can complete before max-turn exit.

## Verification

Commands run:

```powershell
rg -n "Tier 0|Tier 1|Tier:|Tier N|tier|Working task and tier|token spend and tier|Approved Goal|working goal|Goal Clarification|function-created" AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md Reports/AGENTS.md Scripts/Invoke-ClaudeDirectRead.ps1 Scripts/Invoke-ClaudePlanReview.ps1
git diff --check -- AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md Reports/AGENTS.md Scripts/Invoke-ClaudeDirectRead.ps1 Scripts/Invoke-ClaudePlanReview.ps1 Reports/AgentReviews/20260529_RemoveTierAlwaysValidate
```

Expected result after completion: no stale Tier/goal-routing matches in live
process files; no whitespace errors except any pre-existing line-ending warning.

## Token Ledger

AuthoringTokens: Claude=1,454,366, Codex=Unavailable
ReviewTokens: Codex=Unavailable
FinishTokens: Codex=Unavailable
OperatorIsAuthoring: PARTIAL
PerModel: Claude=1,454,366, Codex=Unavailable
TargetMet: PARTIAL
Notes: Claude token count is from two failed helper stdout JSON usage objects because failed runs did not write manifest files. Codex host token count was not exposed without a goal tool.

## Caveats

- No native goal tool was used.
- No Unreal/runtime verification was run because this was process-doc-only.
- Claude Full Operator worked as a write path, but both attempts exceeded the
  helper max-turn cap before completion. Future tests should use smaller
  operator prompts or add resume/continuation support around failed helper runs.
