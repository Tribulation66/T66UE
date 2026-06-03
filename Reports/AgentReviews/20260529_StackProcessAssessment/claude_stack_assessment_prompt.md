# Claude Stack Assessment Prompt

## Working Goal

Assess the current T66 AI collaboration stack and repo process, provide Claude's independent rating out of 10, and recommend changes that improve accuracy, reduce overcomplication, and lower unnecessary token use. Do not edit files.

## User Request

Pablo wants to know if there is room for improvement in the current AI stack/process for accomplishing tasks in `C:\UE\T66`. He wants:

- One independent rating out of 10 from Claude.
- One independent rating out of 10 from Codex.
- What should be changed, removed, added, or adjusted to improve accuracy and usefulness.
- What hurts the stack, overcomplicates it, or wastes unnecessary tokens.

## Context To Inspect

Please inspect the live repo process docs and helper scripts directly. At minimum, inspect:

- `AGENTS.md`
- `Scripts/README.md`
- `Scripts/Invoke-ClaudePlanReview.ps1`
- `Scripts/Invoke-ClaudeDirectRead.ps1`
- `Scripts/Invoke-CodexPlanReview.ps1`
- `Reports/AGENTS.md`
- `Scripts/pending_issues_Scripts.md`

Do not inspect Mini/minigame content unless needed for this process assessment; the default scope excludes Mini.

## Evaluation Criteria

Rate the stack/process on:

- Accuracy and hallucination resistance.
- Ability to use live repo state instead of stale assumptions.
- Cross-model validation quality.
- Safety around writes, Unreal assets, editor automation, billing/API keys, and LFS.
- Practicality for day-to-day development.
- Token efficiency and artifact overhead.
- Clarity for future agents.
- Ability to switch Claude/Codex operator roles.

## Output Required

Provide:

1. `Claude rating: X/10`
2. One paragraph justifying the rating.
3. `Keep`
4. `Change`
5. `Remove or soften`
6. `Add`
7. The single highest-leverage improvement.

Be direct. Do not flatter the process. Call out over-engineering where it exists.
