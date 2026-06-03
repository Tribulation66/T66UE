Working task:
Implement the validated deprecated-code cleanup pass, then verify with grep, build, stage, SHA, and staged full-resolution enemywaveperf smoke.

Operator:
Claude (`claude-opus-4-8`, FullOperator)

Validator:
Codex

Scope:
Dead-code cleanup only. No intended production behavior change.

Source plan:
Use `C:\UE\T66\Reports\AgentReviews\20260529_DeprecatedCodeCleanup\operator_plan_packet.md`.

Binding Codex validation corrections:
1. Retain `AT66EnemyProjectileBase` in this pass. It is live via `AT66SpitProjectile`, a live overlay spawn, and `T66MobBase` projectile class properties. Do not delete it; report it as retained because it was load-bearing.
2. Delete `AT66BossProjectile` only if your fresh implementation-phase reference scan confirms no live production spawn/subclass/Blueprint references remain. Remove its stale Backrooms cleanup-filter reference if deleting it.
3. Remove all five inert route/touch CVars if all remain diagnostic-only/inert:
   - `T66.Mob.UseLightweight`
   - `T66.Mob.Diagnostics.RouteRushLightweight`
   - `T66.Mob.Diagnostics.RouteFlyingLightweight`
   - `T66.Mob.Diagnostics.RouteRangedLightweight`
   - `T66.Mob.Diagnostics.UseTouchDamageOverlap`
4. Delete GamblerToken legacy enum/fields/item alias per user decision. No save migration.
5. CoreRedirect removal is conditional:
   - scan old names in source/config/data and relevant binary assets/uassets
   - remove only clean redirect lines
   - retain and report any redirect with hits or inconclusive verification
6. Delete `C:\UE\T66_B13_Worktree` only after:
   - resolving the absolute path and confirming it equals exactly `C:\UE\T66_B13_Worktree`
   - confirming preserved B.13 evidence exists under the live repo reports
   - if it is a registered git worktree, use appropriate git worktree removal semantics or report unexpected state before destructive deletion

Approved actions:
- Edit/delete source/config files in the validated scope.
- Delete `C:\UE\T66_B13_Worktree` using the safe procedure above.
- Run targeted grep/scans, Unreal commandlets if needed for redirect verification, build, stage, and staged enemywaveperf.
- Write logs and completion packet under `C:\UE\T66\Reports\AgentReviews\20260529_DeprecatedCodeCleanup\`.

Not approved:
- Do not delete `AT66EnemyProjectileBase`, `AT66SpitProjectile`, the overlay spit spawn, or the `ProjectileClass` property in this pass.
- Do not alter rich miniboss/special/boss behavior.
- Do not touch Mini/minigame systems.
- Do not run broad Git/LFS scans.
- Do not git stage/commit/revert/clean.

Verification required:
- Grep-clean report for all deleted identifiers and retained load-bearing exceptions.
- `T66Editor Win64 Development` build succeeds.
- Staged standalone build succeeds; record SHA256 of staged `T66.exe`.
- Run a full-resolution staged `enemywaveperf` capture. Confirm basic mobs still spawn/behave across all four families, projectiles fire/hit through the manager, and FPS is healthy. Record the terminal summary and log path.
- Completion packet: `C:\UE\T66\Reports\AgentReviews\20260529_DeprecatedCodeCleanup\implementation_completion.md`.

If a foundation differs or a redirect cannot be safely removed, do not guess. Retain the item and report exact evidence.
