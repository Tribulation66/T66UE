Codex Approval: APPROVE

## Approved Task

Claude may perform Phase 2 import/build/runtime verification for the validated Phase 1 enemy-roster restructure, and may apply scoped fixups required by those verification gates.

## Approved Scope

Approved edits/actions:
- DataTable/uasset rebuilds for roster-related source data: Enemies, Stages, Items, PlayerExperience.
- Focused source/build fixes caused by the roster restructure.
- Non-shipping verification hooks only if necessary to prove the required smoke gates.
- Completion artifact under `Reports\AgentReviews\20260529_EnemyRosterRestructureImplementation`.

## Approved Tool Surface

Full Claude Operator profile via `Scripts\Invoke-ClaudeDirectRead.ps1` with `-ToolProfile FullOperator`, `-PermissionMode bypassPermissions`, and this approval artifact.

## Required Process Rules

- Use current verification; do not rely on prior logs as substitutes.
- Use Unreal-owned capture/log/automation routes, not desktop screenshots.
- Refresh staged standalone if runtime-facing build verification succeeds and staging is feasible.
- Preserve user-owned dirty work.
- Avoid broad Git/LFS scans.
- Respect Mini/minigame exclusion except explicitly required casino gambling surface.

## Explicitly Excluded Actions

- No Git commit/stage/push/tag/reset/clean/checkout.
- No unrelated cleanup.
- No real model/art asset generation.
- No B.13 sandbox deletion.
- No broad casino redesign.

## Verification Required After Operator Run

Claude must write `phase2_completion.md` with import/build/smoke commands, result markers, logs, fixups, and skipped verification rationale.

## Approval Rationale

Phase 1 source/data changes now pass targeted Codex validation. Phase 2 is required by repo process because the task touched runtime gameplay and data-authored Unreal assets.
