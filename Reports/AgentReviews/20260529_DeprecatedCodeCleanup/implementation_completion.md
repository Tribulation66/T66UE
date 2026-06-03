# Deprecated Code Cleanup — Implementation Completion

Date: 2026-05-29
Scope: Codex-approved deprecated-code cleanup (Divergences A2 / B / C) on T66 (UE 5.7).
Build config: Development. Project version: alpha-0.8.

## Outcome

All approved deletions implemented; production behavior preserved. Editor build, staged
standalone build, and a full-resolution `enemywaveperf` smoke all pass. Hero survives the
full post-capture window, screenshot captured, all routing/spawn/projectile/perf signals
healthy.

No git staging, commit, push, tag, reset, or clean performed (Codex approval required).

## What was removed

### Divergence A2 — full clean removal of 5 deprecated CVars + all live plumbing
- `T66.Mob.UseLightweight`
- `T66.Mob.Diagnostics.RouteRushLightweight`
- `T66.Mob.Diagnostics.RouteFlyingLightweight`
- `T66.Mob.Diagnostics.RouteRangedLightweight`
- `T66.Mob.Diagnostics.UseTouchDamageOverlap`

Preserved `enemywaveperf` behavior: basic mobs unconditionally lightweight;
minibosses/specials/bosses rich. Route attribution diagnostics retained.

### Divergence B — full GamblerToken legacy deletion (13 code sites)
Enum value, item-ID alias (`Item_GamblersToken`), serialized legacy save fields
(renamed to VendorToken equivalents — no migration, accepted break), branch cases,
backend alias handling, localization cases, normalizer block. `VendorToken` is the sole
surviving token identity. Historical resolution notes in `pending_issues_RunState.md`
retained intentionally as history.

### Divergence C — orphaned non-git directory deletion
`C:\UE\T66_B13_Worktree` removed with path assertion + preserved-evidence confirmation.

### AT66BossProjectile actor class
Deleted (scan clean). The manager-driven boss-projectile path is preserved:
`FT66BossProjectileVisualKey` struct and `T66BossProjectileSmoke=` harness remain.
`AT66EnemyProjectileBase`, `AT66SpitProjectile`, overlay spit spawning, and
`ProjectileClass` fields all retained per scope.

### CoreRedirects (DefaultEngine.ini)
Removed 5 redirects (GameplayFloorsPerStage, InitialEnemiesPerGameplayFloor,
InitialTowerEnemiesPerGameplayFloor, ApplyGamblersTokenPickup,
GetActiveGamblersTokenLevel) + 2 orphaned comments, after targeted text+binary scans
confirmed the old names are unreferenced. All other redirects retained.

## Verification

### Grep-clean proof (Source/)
- 5 deprecated CVars in live code: **0**. (Only `pending_issues_Gameplay.md` historical
  notes mention the names — intentional history, not code.)
- `GamblerToken` / `GamblersToken` / `Item_GamblersToken` (excl. pending_issues history): **0**.
- `AT66BossProjectile` actor class: **0**. (10 remaining hits are the preserved
  `FT66BossProjectileVisualKey` struct + `T66BossProjectileSmoke=` cmdline params in the
  manager path — not the deleted actor.)

### Builds
- Focused T66Editor build: exit 0.
- Staged standalone build (RunUAT BuildCookRun): BUILD SUCCESSFUL — real compile
  (47 actions, all Module.T66.*.cpp + link, both targets), exit 0.
  Log: `staged_build.log`.

### Staged executable
- `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe`
- SHA256: `B7BD3B30D12A520ABCC919F1200023D3AB277060F9154E7C7C2880C63B784B6D`

### Full-resolution enemywaveperf smoke (1920x1080)
Command added `-T66AutoCaptureHeroHPOverride=20000` (standard for the stationary
auto-capture hero, matching prior accepted Miniboss/B13 runs — without it the standing
hero is shot down by ranged mobs before the capture window).

Log: `Saved/StandaloneLogs/T66_DeprecatedCodeCleanup_EnemyWavePerf2.log`
Screenshot: `enemywaveperf.png` (captured; mobs of all 4 families + projectiles on-screen).

Signals (terminal RouteAttributionSummary / ProjectileManagerSummary @ WorldTime=32.52):
- **Hero survived:** clean exit `RequestExitWithStatus(0, 0)`, no `T66EnemyWavePerfHeroDied`.
  HeroHP 19780/20000 at quit (240 HP from 12 projectile hits).
- **4 families spawn + route lightweight basic:** Melee 33 (32 LW + 1 boss non-director),
  Rush 24 (24 LW), Flying 15 (15 LW), Ranged 19 (19 LW). TotalObservedSpawns=91.
  `BossOrGuardianObserved=1` (rich boss correctly NOT lightweight).
- **Routing clean:** all families `FallbackBranch=0`, `FamilyLookupFailed=0` — A2 cleanup
  preserved correct attribution.
- **Projectiles fire/hit via manager:** Fired=12, HitHero=12, DroppedFires=0,
  VisualBucketOverflow=0, ActivePeak=2.
- **No overhead/clearance rejects:** RichSpawnFailed=0, LightweightSpawnFailed=0.
- **Healthy perf:** ManagerTickAvgUs=37.4 (Max 580.4), HISMUpdateAvgUs=3.0.
  PerformanceSystem session: AverageFps=174.6, 1%LowFps=101.1, FailedWrites=0.

## Notes
- Save-field renames (GamblersToken*→VendorToken*) change serialized tags; per Codex
  ruling no migration is provided (accepted break).
- First smoke attempt (no HP override) ended at WorldTime=8.54s via ranged projectile
  fire on the stationary hero — a capture-harness artifact, not a touch-overlap regression.
  Re-run with the standard override produced the clean capture documented above.
