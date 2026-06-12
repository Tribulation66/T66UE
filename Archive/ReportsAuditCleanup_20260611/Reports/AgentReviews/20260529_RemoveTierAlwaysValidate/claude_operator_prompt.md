# Claude Operator Prompt: Remove Tier Routing And Always Validate Operator Output

You are Claude Code acting as Operator for `C:\UE\T66`.

Codex has approved this run through:

`C:\UE\T66\Reports\AgentReviews\20260529_RemoveTierAlwaysValidate\codex_operator_approval.md`

Use the current repo files directly. Do not rely on stale memory.

## User Request

The user wants Codex to function as a wrapper when Claude is Operator. They also wants the Tier concept removed entirely because token reporting showed `Unavailable`/`0` while still saying Tier 1. They wants the Validator to always validate the Operator's answer before the answer is sent to the user. Simple tasks should still be validated, but validation can be quick.

## Task Contract

Update the process docs so:

1. There is no Tier 0 / Tier 1 routing concept.
2. The prompt-native task contract remains, but it does not include a Tier field.
3. Operator/Validator routing is always active for substantive answers/results. The Operator's answer/result is not sent to the user until the Validator validates it.
4. Claude-as-Operator remains the intended heavy-work path when `.t66\operator-state.json` names Claude as Operator: Codex acts as wrapper/router, approval gate, and Validator/Finisher.
5. Native goal functions remain disabled unless the user explicitly asks for one.
6. Final user-facing answers no longer include `**Tier: ...**`.
7. Token reporting remains useful without Tier. Use available host/helper token counts; if unavailable, say `Unavailable`. Do not imply a Tier is required for token reporting.

## Approved Edit Scope

- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\Reports\AGENTS.md` only if needed for consistency
- `C:\UE\T66\Scripts\Invoke-ClaudeDirectRead.ps1` only if comments/operator prompt text need wording updates
- `C:\UE\T66\Reports\AgentReviews\20260529_RemoveTierAlwaysValidate\completion_packet.md`

Do not edit runtime gameplay code, Unreal assets, Blender assets, Niagara assets, packaged builds, or tray/widget code.

## Required Output

Make the approved changes. Then write a concise completion packet at:

`C:\UE\T66\Reports\AgentReviews\20260529_RemoveTierAlwaysValidate\completion_packet.md`

Include:

- What changed.
- What files were edited.
- Search/validation evidence.
- Any caveats.
- Claude token count from your helper manifest/output if available.

Keep changes scoped and do not touch unrelated AGENTS sections except where the Tier/removal and always-validate routing requires it.
