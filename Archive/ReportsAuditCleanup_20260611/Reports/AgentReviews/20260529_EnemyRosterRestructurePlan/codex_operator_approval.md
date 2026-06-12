Codex Approval: APPROVE

## Approved Task

Investigate the current T66 code/data state for Pablo's locked enemy-roster restructure decisions and produce one consolidated implementation plan document for review.

## Approved Scope

Read-only investigation plus report-only writes.

Approved write paths:
- `C:\UE\T66\Reports\RosterReview\enemy_roster_restructure_implementation_plan.md`
- `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructurePlan\operator_completion.md`
- Additional small report artifacts under `C:\UE\T66\Reports\AgentReviews\20260529_EnemyRosterRestructurePlan\` only if needed for validation.

Approved read paths:
- Root/process docs named in the prompt.
- Prior roster report.
- Enemy/boss/stage/shop/casino/backrooms/unique-enemy data files needed for the investigation.
- Enemy, boss, special, shop/vendor/steal, Backrooms, tower/descent-gate, data type, RNG tuning, and spawn/routing code under `Source\T66`, excluding Mini/minigame paths.
- Relevant pending issue/docs named in the prompt.

## Approved Tool Surface

Claude FullOperator through `Scripts\Invoke-ClaudeDirectRead.ps1`, constrained by this approval to targeted read/search commands and report-file writes only.

## Required Process Rules

- Follow `AGENTS.md`, `OPERATOR_VALIDATOR_PROTOCOL.md`, `Reports\AGENTS.md`, `Gameplay\GAMEPLAY_AGENTS.md`, and `Gameplay\README.md`.
- Keep Mini/minigame paths excluded.
- Use live repo data/code as authoritative.
- Do not invent missing foundations. Loan Shark must be reported as found or not found with evidence.
- Produce one consolidated plan for Pablo review, not implementation.

## Explicitly Excluded Actions

- No source, config, content, save, CSV, uasset, DataTable, runtime behavior, build, stage, or Git changes.
- No cleanup/deletion.
- No implementation of any roster restructure item.
- No casino interactable changes.
- No status-effect mob work.
- No real model work.
- No B.13 sandbox deletion or deprecated rich-mob/CVar/projectile-class deletion.
- No broad Git/LFS scans.
- No Mini/minigame inspection or edits.

## Verification Required After Operator Run

Codex will validate:
- the plan exists and covers all numbered items 1-11;
- the Loan Shark search/foundation conclusion is evidence-backed;
- shop steal trigger, Backrooms/Stalker, Gambler/Vendor, and placed guardian claims are supported by code/data anchors;
- proposed affected files/data are plausible and no implementation occurred;
- changed files stay inside approved report paths.

## Approval Rationale

The task is planning and investigation only. The approved write scope is confined to report artifacts, and the prompt explicitly excludes implementation, cleanup, and data/content/source changes.
