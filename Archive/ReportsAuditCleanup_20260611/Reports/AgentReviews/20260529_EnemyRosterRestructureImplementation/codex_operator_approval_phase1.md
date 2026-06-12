Codex Approval: APPROVE

## Approved Task

Claude may implement Phase 1 of the enemy-roster restructure in `C:\UE\T66`, covering source/data changes for Sections A-E from the user prompt and the approved plan at `Reports\RosterReview\enemy_roster_restructure_implementation_plan.md`.

## Approved Scope

Approved edits:
- Relevant source under `Source\T66` for enemy removals, Vendor boss trigger/renaming, casino anger removal, mob-floor terminology, stage schema expansion, and tower guardian mega-mob assignment.
- Relevant source data under `Content\Data` for `Enemies.csv`, `Stages.csv`, boss/vendor/token rows, unique enemy rows, RNG tuning source/config rows if present.
- Project/source metadata only when required by source file deletion/renaming.
- Completion/report artifacts under `Reports\AgentReviews\20260529_EnemyRosterRestructureImplementation`.

Approved command surface:
- File reads, targeted grep/search, source/data edits.
- Focused static checks and lightweight scripts.
- Focused compile if needed as a blocker check.

## Approved Tool Surface

Full Claude Operator profile:
- `Scripts\Invoke-ClaudeDirectRead.ps1`
- `-Mode Operator`
- `-ToolProfile FullOperator`
- `-PermissionMode bypassPermissions`
- `-Model claude-opus-4-8`
- `-Effort high`
- `-AddDir C:\UE\T66`

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Gameplay\GAMEPLAY_AGENTS.md`, `UI\UI_AGENTS.md`, and `Reports\AGENTS.md`.
- Preserve user-owned dirty work. Do not revert unrelated changes.
- Avoid broad Git/LFS scans over Unreal binary asset folders.
- Respect Mini/minigame exclusion except for explicitly required casino gambling surface work.
- Sequence sections A-E in the order requested by the user.
- If a foundation differs or an item cannot be cleanly applied, stop or surface it in the completion packet rather than guessing.

## Explicitly Excluded Actions

- No Git commit, push, tag, staging, reset, checkout, clean, or destructive repository cleanup.
- No B.13 sandbox deletion.
- No unrelated rich-mob/CVar/projectile cleanup.
- No real model/art asset creation.
- No casino redesign beyond removing anger/boss-spawn coupling while keeping gambling functional.
- No Mini/minigame work.
- No broad editor/staged-runtime verification in Phase 1 unless needed only as a quick blocker check; Phase 2 owns import/build/runtime verification.

## Verification Required After Operator Run

Claude must write `Reports\AgentReviews\20260529_EnemyRosterRestructureImplementation\phase1_completion.md` with:
- Files changed/deleted.
- Section-by-section implementation result.
- Static grep checks for removed references.
- Deferred import/build/smoke items for Phase 2.
- Any blockers/deviations.

Codex will validate the changed files and completion packet before approving Phase 2.

## Approval Rationale

The user explicitly requested implementation and selected Claude as implementer. The approved roster plan exists and the user supplied locked decisions. This approval is phase-bounded to source/data implementation so the broad task remains controlled while preserving the user's requested one-pass outcome.
