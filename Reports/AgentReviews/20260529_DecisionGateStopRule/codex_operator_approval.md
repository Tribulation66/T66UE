Codex Approval: APPROVE

## Approved Goal

Tighten the T66 process docs so the first valid `NEEDS_HUMAN_DECISION` verdict or equivalent decision gate stops work immediately until the user answers.

## Approved Scope

Claude may edit only these process/report files if needed:

- `AGENTS.md`
- `OPERATOR_VALIDATOR_PROTOCOL.md`
- `Reports/AgentReviews/20260529_DecisionGateStopRule/completion_packet.md`

## Approved Tool Surface

Claude full Operator mode is allowed for this bounded docs/process change. Claude may read relevant docs and edit the approved text files. Shell use is allowed only for focused text searches and validation such as parser-free grep, `git diff --check` on the touched paths, and reading relevant files.

## Required Process Rules

- Do not use Claude plan mode.
- Preserve unrelated `AGENTS.md` wording.
- Do not touch runtime source, Unreal assets, staged builds, generated content, or Git state.
- Keep the rule compatible with native goal tools that may not allow marking `blocked` until a host-defined threshold. The process rule should still require work to stop immediately at the first valid decision gate.

## Explicitly Excluded Actions

- No runtime code edits.
- No broad Git/LFS scans.
- No Unreal, Blender, Niagara, staged build, or capture work.
- No commit, push, tag, reset, clean, or destructive operations.

## Verification Required After Operator Run

- Codex validates the actual docs diff.
- `git diff --check` on touched paths must pass except known line-ending warnings.
- Completion packet must state the B.13 failure mode: it should have stopped immediately after the first valid `NEEDS_HUMAN_DECISION` and only referenced the saved decision block on continuations.

## Approval Rationale

The user's example shows repeated work after a human decision gate. This is a process-rule defect, and the approved change is limited to clarifying stop behavior.
