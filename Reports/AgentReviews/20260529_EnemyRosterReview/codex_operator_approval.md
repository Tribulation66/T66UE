Codex Approval: APPROVE

## Approved Task

Create a plain-language enemy roster review document for Pablo covering bosses, hidden bosses, minibosses, specials, basic mobs, theme/stage coverage, and gap flags.

## Approved Scope

Read-only inspection of enemy roster data/code and report-only writes.

Approved write paths:
- `C:\UE\T66\Reports\RosterReview\enemy_roster_review.md`
- `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterReview\operator_completion.md`
- Additional small artifacts under `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterReview\` only if needed for validation.

Approved read paths:
- Root/process docs required by the prompt.
- `Content\Data` roster CSVs named in the prompt.
- Enemy/boss/special/spawn/routing code under `Source\T66`, excluding Mini/minigame modules and paths.
- Relevant pending issues listed in the prompt.
- Related enemy/boss data tables only when referenced by the named data files.

## Approved Tool Surface

Claude FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1`, limited by this approval to read/search commands and report-file writes. No mutating runtime/data/content/config/source edits.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Reports\AGENTS.md`, `Gameplay\GAMEPLAY_AGENTS.md`, and `Gameplay\README.md`.
- Keep Mini/minigame paths excluded.
- Use live repo data/code as authoritative.
- Keep the main report designer-readable; put technical traceability in separate columns/appendix.
- Treat gaps as flags only; do not act on them.

## Explicitly Excluded Actions

- No source, config, content, save, CSV, uasset, DataTable, runtime behavior, build, stage, or Git changes.
- No cleanup/deletion.
- No Pass D or Pass E work.
- No NPC/item/hero/companion/projectile-system audit except when a projectile note is unavoidable to explain an enemy behavior at high level.
- No Mini/minigame inspection or edits.
- No broad Git/LFS scans.

## Verification Required After Operator Run

Codex will validate:
- the report exists and has all requested sections;
- counts are traceable to the named data sources;
- boss/special/reachable claims are supported by code/data anchors;
- gap flags are phrased as review flags, not fixes;
- no out-of-scope files were changed.

## Approval Rationale

The task is broad discovery but read/report-only. The approved write scope is confined to `Reports/`, and the prompt explicitly excludes source/data/content/config/save changes.
