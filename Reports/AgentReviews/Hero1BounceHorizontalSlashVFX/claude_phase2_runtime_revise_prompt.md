# Claude FullOperator Phase 2 Revision: Bounce Per-Link Target Handle

You are Claude Operator. Codex returned Phase 2 as `Verdict: REVISE` in:

`Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/validator_check_phase2_revise.md`

The original approval remains:

`Reports/AgentReviews/Hero1BounceHorizontalSlashVFX/codex_operator_approval_phase2_runtime.md`

Stay within the same approved runtime scope.

Required fix:

- In `Source/T66/Gameplay/T66CombatComponent.cpp`, `PublishBounceLink` currently creates one per-link `FT66CombatImpactContext` but sets `LinkContext.PrimaryTargetHandle = PrimaryHandle` for every link.
- Change that so each per-link context's primary/impact target handle is the link target: `LinkHandle`.
- Preserve damage authority, chain index, impact point, hit target list, `TrySpawnBoundWeaponBaseSlashVFX` call, and no-binding behavior.
- Do not edit assets, CSV/DataTable, scripts, capture harness, Git, Mini/minigame, imagegen, credentials, or unrelated files.

Verification:

- Run the focused editor build if possible:
  `& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66Editor Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex -NoHotReloadFromIDE`
- Report changed files, exact build result, and the source line changed.
