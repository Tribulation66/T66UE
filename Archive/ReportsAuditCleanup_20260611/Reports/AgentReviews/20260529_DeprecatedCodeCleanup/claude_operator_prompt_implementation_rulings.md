# Deprecated Code Cleanup — Implementation Rulings And Continue Prompt

You are Claude acting as FullOperator for `C:\UE\T66`. Continue the approved deprecated-code cleanup pass after your Phase 0 findings in:

`Reports/AgentReviews/20260529_DeprecatedCodeCleanup/operator_phase0_findings.md`

## Task Contract

Working task: delete deprecated code accumulated across the lightweight-mob and roster-restructure work, preserving production behavior.
Operator: Claude (`claude-opus-4-8`, FullOperator).
Validator: Codex.
Scope: implementation + verification inside the approved cleanup bounds below.
Stop condition: completion report with grep-clean/build/stage/runtime smoke evidence, or a new load-bearing dependency that contradicts the approved cleanup target.

## Codex Rulings On Your Phase 0 Stops

### Divergence A — choose A2 full clean removal

Proceed with full clean removal of the deprecated routing/touch CVar footprint.

Remove the deprecated CVar definitions and all live string/plumbing references for:

- `T66.Mob.UseLightweight`
- `T66.Mob.Diagnostics.RouteRushLightweight`
- `T66.Mob.Diagnostics.RouteFlyingLightweight`
- `T66.Mob.Diagnostics.RouteRangedLightweight`
- `T66.Mob.Diagnostics.UseTouchDamageOverlap`

This includes:

- `T66EnemyDirector.cpp` definitions, helpers, no-op routing plumbing, and unreachable/dead route-attribution remnants.
- `T66MobManagerSubsystem.cpp` touch-overlap helper/caller, route-attribution fields, and related no-op diagnostic fields.
- `T66PlayerController_Overlays.cpp` smoke/autocapture set-sites, command-line overrides, and summary fields that reference the deprecated CVars.

Reason: the user requested grep-clean removal of these deprecated CVars and dead branches. Leaving no-op harness references would fail the cleanup goal. Preserve the actual `enemywaveperf` behavior: basic mobs remain unconditionally lightweight; minibosses, specials, and bosses remain intentionally rich.

### Divergence B — proceed with full GamblerToken footprint

Proceed with the full GamblerToken legacy deletion footprint you found. The user explicitly approved deleting legacy save fields and no save migration is required. The larger file count is an implementation detail, not a blocker.

Delete the deprecated enum value, aliases, serialized legacy fields, branch cases, backend alias handling, and `Item_GamblersToken` alias as needed so `VendorToken` is the only surviving token identity. Preserve current VendorToken behavior.

### Divergence C — delete orphaned B13 directory

Proceed with raw deletion of the orphaned non-git directory `C:\UE\T66_B13_Worktree`, using a path assertion before deletion.

Required safety:

- Confirm `Resolve-Path -LiteralPath 'C:\UE\T66_B13_Worktree'` resolves exactly to that path.
- Confirm the preserved evidence directory exists in the live repo before deletion:
  `C:\UE\T66\Reports\AgentReviews\20260529_B13_NoLand_Closeout\preserved_worktree_evidence`
- Then delete the orphaned directory with PowerShell native `Remove-Item -LiteralPath <resolved path> -Recurse -Force`.
- After deletion, confirm `Test-Path 'C:\UE\T66_B13_Worktree'` is false.

Do not run broad binary/content diff scans to compare the orphan directory.

## Continue The Approved Cleanup

Continue from the existing plan and validation packet:

- `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/operator_plan_packet.md`
- `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/plan_validator_check.md`
- `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/codex_operator_approval_implementation.md`

Additional binding constraints:

- Retain `AT66EnemyProjectileBase`, `AT66SpitProjectile`, overlay spit spawning, and `ProjectileClass` fields. Your Phase 0 proved this path is still load-bearing.
- Delete `AT66BossProjectile` only if a fresh scan remains clean, then remove only the stale Backrooms cleanup-filter include/reference for that actor class.
- Remove CoreRedirects only if the old-name text and binary scans remain clean. If any old-name reference appears or the scan is inconclusive, leave those redirects and report why.
- Do not touch Mini/minigame systems.
- Do not perform git staging, committing, reverting, resetting, cleaning, or broad status/diff scans.

## Required Verification

After edits:

1. Grep-clean proof for every deleted identifier, excluding historical report artifacts under `Reports/AgentReviews/20260529_DeprecatedCodeCleanup` where appropriate.
2. Focused editor build.
3. Staged standalone build.
4. Record staged `T66.exe` SHA256.
5. Run a full-resolution staged `enemywaveperf` smoke. Confirm basic mobs spawn and behave across all four families, projectiles fire/hit via manager, no first HeroDeath, no overhead reject, and FPS is healthy.
6. Write a completion report to:
   `Reports/AgentReviews/20260529_DeprecatedCodeCleanup/implementation_completion.md`

The completion report must include:

- What was deleted and what was intentionally retained.
- Tier 2 CoreRedirect decision.
- B13 directory disposition.
- Grep-clean results.
- Build/stage logs and staged SHA.
- Runtime smoke log path and result.
- Any deviations or deferred items for Pablo.
