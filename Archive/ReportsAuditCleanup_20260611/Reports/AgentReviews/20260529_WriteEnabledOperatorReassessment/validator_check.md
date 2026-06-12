# Validator Check: Write-Enabled Operator Reassessment

Verdict: NEEDS_HUMAN_DECISION

## Scope

Validate Claude's Operator recommendation for changing the T66 Operator role from read-only planning to controlled write-enabled authoring.

## Operator Artifact

- Prompt: `Reports/AgentReviews/20260529_WriteEnabledOperatorReassessment/claude_operator_prompt.md`
- First run: `Reports/AgentReviews/ClaudeDirectRead/20260529T034006-WriteEnabledOperatorReassessment-Operator/`
- First run result: plan-mode returned only an internal-plan pointer, not a usable packet.
- First run tokens: `552431`
- Second run: `Reports/AgentReviews/ClaudeDirectRead/20260529T034401-WriteEnabledOperatorReassessmentDirect-Operator/`
- Second run output: `Reports/AgentReviews/ClaudeDirectRead/20260529T034401-WriteEnabledOperatorReassessmentDirect-Operator/claude_direct_read_operator.md`
- Second run tokens: `116946`

## Validator Conclusion

Claude's criticism is correct. The current `OPERATOR_VALIDATOR_PROTOCOL.md` assigns planning/discovery to the Operator, but reserves edit application, integration, verification, and final reporting for Codex. For write-heavy docs/code/config tasks, that means the expensive authoring phase remains with Codex even when Claude is labeled Operator.

The current target of `70-80 percent of planning and analysis tokens` is too narrow. It should become phase-based: the Operator must own the authoring phase for text implementation tasks, and Codex should become Validator plus boundary finisher.

## Recommended Change

Adopt a controlled Claude write-Operator profile for Tier 1 text implementation tasks:

- Allow only text authoring tools: `Read,Grep,Glob,Edit,Write`.
- Use Claude Code subscription auth only; keep the `ANTHROPIC_API_KEY` refusal gate.
- Do not allow shell/build/test commands by default.
- Do not allow Unreal, Blender, Niagara, commandlets, staged builds, Git commit/push/tag, credentials, billing, or destructive commands.
- Deny direct writes to binary/content/generated-heavy paths including `Content/`, `SourceAssets/`, `Saved/StagedBuilds/`, `.uasset`, common image/video/model formats, and LFS-heavy outputs unless a later task creates a separate process-owned mode.
- Require a before/after changed-file snapshot, diff artifact, and manifest entry for actual Claude-edited files.
- Validator reviews the real diff, not only a proposed plan.

## Core Files To Change After Approval

- `Scripts/Invoke-ClaudeDirectRead.ps1`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `AGENTS.md`
- `Reports/AGENTS.md`

## User Decision Needed

Approve or reject this new default:

`Claude as Operator may directly edit repo text files for Tier 1 implementation tasks using a controlled text-write profile, while Codex validates the actual diff and handles verification/final reporting.`

Recommended answer: approve controlled text-write mode as the default for Tier 1 text implementation tasks.
