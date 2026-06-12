Codex Validation: APPROVE

# Deprecated Code Cleanup — Codex Validation Report

Date: 2026-05-29

## Task Contract

Working task: delete deprecated code accumulated across the lightweight-mob and roster-restructure work, preserving production behavior.
Operator: Claude (`claude-opus-4-8`, FullOperator).
Validator: Codex.
Scope: dead-code cleanup only; no git staging/commit/revert; no Mini/minigame scope.
Stop condition: cleanup implemented with grep-clean/build/stage/runtime smoke evidence, or retained targets documented when found load-bearing.

## Operator Result Reviewed

Reviewed Claude completion report:

`Reports/AgentReviews/20260529_DeprecatedCodeCleanup/implementation_completion.md`

Operator run:

`Reports/AgentReviews/ClaudeDirectRead/20260529T210931-DeprecatedCodeCleanup_Implementation_Rulings-Operator/`

## Validation Checks

### Deleted / retained targets

- `Source/T66/Gameplay/Enemies/Projectiles/T66BossProjectile.h`: absent.
- `Source/T66/Gameplay/Enemies/Projectiles/T66BossProjectile.cpp`: absent.
- `C:\UE\T66_B13_Worktree`: absent.
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h`: present.
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.cpp`: present.

Retention of `AT66EnemyProjectileBase` is approved because fresh validation shows it is still load-bearing via `AT66SpitProjectile`, `ProjectileClass`, Backrooms cleanup filtering, and Performance/LagTracker active projectile counts.

### Grep-clean validation

Codex reran targeted `rg` checks against live `Source`, `Config`, and `Content/Data`, excluding historical `pending_issues_*.md` notes. Hit counts were zero for:

- `T66.Mob.UseLightweight`
- `T66.Mob.Diagnostics.RouteRushLightweight`
- `T66.Mob.Diagnostics.RouteFlyingLightweight`
- `T66.Mob.Diagnostics.RouteRangedLightweight`
- `T66.Mob.Diagnostics.UseTouchDamageOverlap`
- `GamblerToken`
- `GamblersToken`
- `Item_GamblersToken`
- `ActiveGamblersTokenLevel`
- `GamblersTokenUnlockedLevel`
- `AT66BossProjectile`

CoreRedirect old-name checks against live `Source`, `Config`, and `Content/Data`, excluding pending issue notes, were also zero for:

- `GameplayFloorsPerStage`
- `InitialEnemiesPerGameplayFloor`
- `InitialTowerEnemiesPerGameplayFloor`
- `ApplyGamblersTokenPickup`
- `GetActiveGamblersTokenLevel`

Historical docs/reports still mention old names as past context. These are not live code or config and do not invalidate the cleanup.

### Build and stage

Codex reviewed:

- `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/editor_build.log`
- `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/staged_build.log`

Results:

- T66Editor build: succeeded.
- Staged standalone BuildCookRun: `BUILD SUCCESSFUL`.
- Staged executable SHA256: `B7BD3B30D12A520ABCC919F1200023D3AB277060F9154E7C7C2880C63B784B6D`.

### Runtime smoke

Codex reviewed:

- `Saved/StandaloneLogs/T66_DeprecatedCodeCleanup_EnemyWavePerf2.log`
- `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/enemywaveperf.png`

Runtime evidence:

- Full-resolution launch: 1920x1080 with `T66AutoCaptureHeroHPOverride=20000`.
- Clean exit: `RequestExitWithStatus(0, 0)`.
- Hero survived: terminal summary at WorldTime `32.52`, HeroHP `19780.0`.
- Basic routing: `RichSpawns=0`, `LightweightSpawns=19` for ranged decision summary; route summary shows Melee/Rush/Flying/Ranged all routed lightweight for basic mobs, with `FallbackBranch=0` and `FamilyLookupFailed=0`.
- Projectile manager: `Fired=12`, `HitHero=12`, `DroppedFires=0`, `VisualBucketOverflow=0`.
- Smoke screenshot is nonblank and shows active mobs and projectiles in the Tower capture.
- Performance sanity: AverageFps `174.6`, 1%LowFps `101.1`, FailedWrites `0`.

## Verdict

Approved. The cleanup matches the approved scope. The one requested deletion target that was not deleted, `AT66EnemyProjectileBase`, was correctly retained because it is still load-bearing; this follows the user's instruction to confirm no production reference before deletion. No production behavior regression was found in the staged smoke.

No git staging, commit, push, reset, revert, or clean operation was performed.
