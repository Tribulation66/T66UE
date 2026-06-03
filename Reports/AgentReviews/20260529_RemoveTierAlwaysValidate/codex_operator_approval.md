Codex Approval: APPROVE

Approved operator: Claude
Validator: Codex
Task contract:
- Remove the Tier 0 / Tier 1 routing concept from the T66 process docs.
- Keep the prompt-native task contract, but remove the Tier field.
- Require every Operator answer/result to be validated by the Validator before the user-facing answer is sent.
- Preserve the current "no native goal function unless the user explicitly asks" rule.
- Preserve the Claude-as-Operator wrapper model: Codex receives the user request, forwards the task contract and repo rules to Claude, approves or rejects scope, then validates the Operator result.
- Remove user-facing final `Tier:` reporting and any token-report shape that depends on Tier.

Approved edit scope:
- AGENTS.md
- OPERATOR_VALIDATOR_PROTOCOL.md
- Reports/AGENTS.md only if needed for consistency
- Scripts/Invoke-ClaudeDirectRead.ps1 only if comments/prompt text need updated wording
- A completion packet under Reports/AgentReviews/20260529_RemoveTierAlwaysValidate/

Out of scope:
- Gameplay/runtime source changes.
- Unreal assets, Blender assets, Niagara assets, packaged builds, or tray/widget code.
- Native goal tooling or host app changes.
- Broad repo rewrites outside the approved process-doc/helper scope.

Validation expected from Operator:
- Search evidence showing stale Tier routing/final-reporting language has been removed from live process files.
- Whitespace check or equivalent for edited files.
- PowerShell parse check if the helper script is edited.
