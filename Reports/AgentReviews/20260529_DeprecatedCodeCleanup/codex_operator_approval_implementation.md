Codex Approval: APPROVE

Approved scope:
- Implement the deprecated-code cleanup described in `operator_plan_packet.md` with the binding corrections in `plan_validator_check.md`.
- Remove dead rich-basic-mob routing remnants while preserving rich miniboss/special/boss paths.
- Remove the five inert route/touch CVars if still diagnostic-only/inert.
- Delete `AT66BossProjectile` only after fresh reference scan confirms it is safe; remove its stale Backrooms cleanup-filter reference if deleted.
- Retain `AT66EnemyProjectileBase` and related `AT66SpitProjectile` path in this pass because current plan evidence shows it is load-bearing.
- Delete GamblerToken legacy enum/save fields/item alias per user's no-save-preservation decision.
- Conditionally remove CoreRedirects only when text + binary asset scans prove old names are unreferenced.
- Delete `C:\UE\T66_B13_Worktree` after exact path verification and preserved-evidence confirmation.
- Run targeted grep/scans, build, stage, SHA, and staged full-resolution `enemywaveperf` smoke.
- Write logs and completion packet under `C:\UE\T66\Reports\AgentReviews\20260529_DeprecatedCodeCleanup\`.

Not approved:
- No deletion of `AT66EnemyProjectileBase`, `AT66SpitProjectile`, overlay spit spawn, or `ProjectileClass` property this pass.
- No production behavior changes.
- No rich miniboss/special/boss cleanup.
- No Mini/minigame work.
- No broad Git/LFS scans.
- No git stage/commit/revert/clean.
