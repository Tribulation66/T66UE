# Claude Operator Prompt: Hard No Native Goal Tool Rule

## Working Task

The user wants the T66 process docs changed from "goal tools are telemetry unless explicitly requested" to a hard no-native-goal-tool rule.

Implement this wording change:

```text
Do not call native goal tools for T66 work. Do not create, update, complete, block, or query goals as part of repo process. If the host app displays or injects goal context anyway, treat it as non-authoritative telemetry only and ignore it for task control.
```

## Current Role State

- Operator: Claude
- Validator/Finisher: Codex
- Repo: `C:\UE\T66`
- Approval artifact: `C:\UE\T66\Reports\AgentReviews\20260529_NoNativeGoalToolRule\codex_operator_approval.md`

## Files To Inspect

- `C:\UE\T66\AGENTS.md`
- `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`
- `C:\UE\T66\Reports\AGENTS.md`

Use narrow search for goal-tool wording if needed.

## Required Changes

- In `AGENTS.md`, remove the existing exception that allows native goal functions when explicitly requested.
- Replace it with a hard rule that agents must not call `/goal`, `set_goal`, `create_goal`, `update_goal`, `get_goal`, or equivalent native goal APIs for T66 repo work.
- Preserve the useful host-state clarification: if a host-created/exposed goal appears anyway, it is telemetry only and must not control scope, stop conditions, continuations, blocking, or completion.
- Update `OPERATOR_VALIDATOR_PROTOCOL.md` only where it directly conflicts or would confuse future agents about native goal tools.
- Keep unrelated process rules unchanged.

## Constraints

- Do not use native goal functions.
- Do not use Claude plan mode.
- Do not edit unrelated sections.
- Do not touch runtime/gameplay/content/assets/build outputs.
- Do not run Unreal, Blender, Niagara, staged builds, git commit/push/tag/reset/clean, or broad Git/LFS scans.
- Preserve unrelated user changes.

## Verification To Run

- Narrow search for `unless the user explicitly asks`, `create_goal`, `set_goal`, `update_goal`, `get_goal`, `/goal`, and `host goal` in `AGENTS.md` and `OPERATOR_VALIDATOR_PROTOCOL.md`.
- Run narrow `git diff --check -- AGENTS.md OPERATOR_VALIDATOR_PROTOCOL.md`.

## Required Final Operator Artifact

At the end, write a concise completion packet at:

`C:\UE\T66\Reports\AgentReviews\20260529_NoNativeGoalToolRule\claude_completion_packet.md`

It must list:

- Files changed.
- Exact wording changed.
- Commands run and pass/fail results.
- Any unverified areas.
- Any remaining blockers or follow-ups.

Your helper response should summarize the same. Codex will validate before the user sees the final answer.
