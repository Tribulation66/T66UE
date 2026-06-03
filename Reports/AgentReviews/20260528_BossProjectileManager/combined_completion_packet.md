# Boss Projectile Manager Combined Packet

Date: 2026-05-28

## Working Goal

Migrate boss projectile firing from the `AT66BossProjectile` actor path into `UT66ProjectileManagerSubsystem` while keeping bosses as rich `AT66BossBase` actors. Preserve boss firing patterns, render managed boss projectiles through HISM, keep damage attribution as `SourceID=<BossID>` / `Delivery=BossProjectile`, and validate with staged standalone smoke.

## Review Gate

- Plan packet: `C:\UE\T66\Reports\AgentReviews\20260528_BossProjectileManager\plan_packet.md`
- Claude review pass 4: `C:\UE\T66\Reports\AgentReviews\20260528_BossProjectileManager\20260528T045016-pass4\claude_review_pass4.md`
- Verdict: `APPROVE`
- User go-ahead: approved after the greenlight.

Accepted caveats from the reviewed packet:

- Boss actors remain rich actors; only projectile storage, movement, collision, damage dispatch, and rendering move to the manager.
- `AT66BossProjectile` files stay in source as deprecated compatibility code.
- Unique Debuff projectiles remain actor-owned and out of scope.
- The previous placed-miniboss pass still had a runtime proof gap for floors 3 and 4. This boss projectile smoke uses the production tower boss-floor entry path but does not newly walk floors 2, 3, and 4 by killing each placed miniboss.

## Implementation Summary

### Projectile Manager

Files:

- `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.h`
- `C:\UE\T66\Source\T66\Gameplay\T66ProjectileManagerSubsystem.cpp`

Changes:

- Raised manager projectile capacity from `256` to `512` for dense boss patterns and Four Horsemen multi-boss.
- Added structured `FT66ManagedProjectileFireParams`.
- Added `ET66ManagedProjectileDelivery::BossProjectile`.
- Added `FireBossProjectile(const FT66ManagedProjectileFireParams&)`.
- Added boss projectile state to flat-array records:
  - delivery type
  - boss attack profile
  - boss projectile tint colors
  - visual scale multiplier
  - manager-owned trail and impact Niagara references
- Added boss HISM visual buckets keyed by attack profile and quantized color.
- Bounded exact boss visual buckets to 32, then fallback to per-profile overflow buckets.
- Added diagnostics:
  - `DroppedInvalidSource`
  - `VisualBucketOverflow`
  - `ApplyDamageReturnedFalse`
  - manager tick and HISM update timing
- Boss projectiles pass through peer enemies/bosses, ignore the firing boss owner for sweep collision, collide with hero/world, and apply hero damage through the existing run-state damage path.
- Boss projectile VFX reuse the existing boss trail/impact Niagara assets, with manager-side budget reads from existing CVars:
  - `T66.VFX.BossProjectileMaxPerFrame`
  - `T66.VFX.BossProjectileUseEffectsScalability`

### Boss Firing

File:

- `C:\UE\T66\Source\T66\Gameplay\T66BossBase.cpp`

Changes:

- `SpawnProjectileInDirection()` now builds `FT66ManagedProjectileFireParams` and calls `UT66ProjectileManagerSubsystem::FireBossProjectile`.
- `SpawnScaledProjectileInDirection()` does the same, preserving the existing scale/profile/tint pattern parameters.
- Boss pattern scheduling remains in `AT66BossBase`; spread, radial, lobe/mouth, and timed helpers still decide when and where to fire.
- Fire audio remains in the boss helper and plays only when manager fire succeeds.
- No production boss firing path calls `SpawnActor<AT66BossProjectile>`.

### Deprecated Actor Path

File:

- `C:\UE\T66\Source\T66\Gameplay\T66BossProjectile.h`

Changes:

- Added `// DEPRECATED` header comment.
- Marked constructor with `UE_DEPRECATED(5.7, "Boss projectiles are managed by UT66ProjectileManagerSubsystem.")`.
- Files remain for stale asset/reference compatibility.

### Smoke Harness

Files:

- `C:\UE\T66\Source\T66\Gameplay\T66GameMode.h`
- `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Tower.cpp`
- `C:\UE\T66\Source\T66\Gameplay\T66PlayerController_Overlays.cpp`

Changes:

- Added non-shipping wrappers used only by boss projectile manager smoke.
- Added `T66GameplayAutoCapture=bossprojectilemanager`.
- Smoke supports:
  - normal stage boss
  - Stage 17 Four Horsemen multi-boss
  - optional `T66BossProjectileSmokeKillMidFlight`
- Smoke applies automation-only hero HP override, disables boss ground AOE for projectile-specific proof, stages Four Horsemen with `SelectedDifficulty=Impossible`, and emits terminal `ProjectileManagerSummary`.
- Smoke now snapshots registry boss arrays before destroying/configuring/reporting bosses to avoid live-array mutation during ranged-for iteration.
- Fixed boss-floor entry awaken guard in tower descent handling so dormant bosses can awaken when entered through the tower boss-floor path. Dormant bosses intentionally begin with `CurrentHP=0`; `ForceAwaken()` restores `MaxHP`.

### Documentation

Files:

- `C:\UE\T66\Gameplay\Combat\MASTER_COMBAT.md`
- `C:\UE\T66\PerformanceSystem\Miniboss_Special_Boss_Spawn_and_Integration_Audit.md`
- `C:\UE\T66\Source\T66\Gameplay\pending_issues_Gameplay.md`

Updates:

- Combat docs now state basic enemy projectiles and boss projectiles flow through `UT66ProjectileManagerSubsystem`.
- Audit now closes the boss half of the prior projectile-manager gap; Unique Debuff remains open/out of scope.
- Gameplay pending issues note that boss projectiles share the manager but this does not itself close B.10 basic-mob acceptance.

## Build And Stage Verification

Focused build command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' T66 Win64 Development -Project='C:\UE\T66\T66.uproject' -WaitMutex
```

Result:

- Exit code: `0`
- A first retry after the final harness patch exited early with no compiler diagnostic; the UBT log had no error lines. A second focused build regenerated a stale makefile and succeeded.
- Known pre-existing warning remained:
  - `C:\UE\T66\Source\T66Mini\T66Mini.Build.cs` references missing `Public\UI\Components`.

Staged standalone command:

```powershell
powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\StageStandaloneBuild.ps1
```

Result:

- Exit code: `0`
- Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Final staged SHA256: `CD4F9388FF6643749B8FF3B88DD1E640D416EF5BAAC6BB319F3800D540BD1EFC`
- Shortcut target: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Shortcut args: `-abslog="C:\UE\T66\Saved\StandaloneLogs\T66_Standalone.log" -forcelogflush`
- Process hygiene before and after smoke: no lingering `RunUAT`, `UnrealEditor-Cmd`, `T66`, or `git-lfs` processes.

## Static Reference Verification

Command:

```powershell
rg -n "SpawnActor<AT66BossProjectile>|AT66BossProjectile::StaticClass\(" Source/T66/Gameplay
```

Result:

- No production boss firing path spawns `AT66BossProjectile`.
- Only remaining match:
  - `C:\UE\T66\Source\T66\Gameplay\GameMode\T66GameMode_Backrooms.cpp:152`
  - This is a cleanup filter, not a firing path.

Command:

```powershell
rg --files Content | Select-String -Pattern "BossProjectile|T66BossProjectile"
```

Result:

- No content-path filename matches.

## Staged Smoke Matrix

All smoke rows ran on staged hash `CD4F9388FF6643749B8FF3B88DD1E640D416EF5BAAC6BB319F3800D540BD1EFC`.

| Smoke | Exit | Screenshots | Log | Key result |
| --- | ---: | ---: | --- | --- |
| Normal boss | 0 | 18 | `C:\UE\T66\Saved\StandaloneLogs\T66_BossProjectileManager_Smoke_Clean.log` | `Dungeon_SewerSlimeKing` awakened through tower boss-floor entry and fired managed boss projectiles. |
| Four Horsemen | 0 | 18 | `C:\UE\T66\Saved\StandaloneLogs\T66_BossProjectileManager_FourHorsemen_Clean.log` | Four bosses fired 280 managed projectiles, active peak 39, no drops/overflow, boss projectile damage applied to hero. |
| Kill mid-flight | 0 | 18 | `C:\UE\T66\Saved\StandaloneLogs\T66_BossProjectileManager_KillMidFlight_Clean.log` | Conquest killed during the smoke (`AliveAfter=0`), remaining bosses continued firing; no killed-source damage was observed after the kill. |

Screenshot folders:

- `C:\UE\T66\Saved\Codex\Gameplay\BossProjectileManager\normal_clean`
- `C:\UE\T66\Saved\Codex\Gameplay\BossProjectileManager\four_horsemen_clean`
- `C:\UE\T66\Saved\Codex\Gameplay\BossProjectileManager\kill_midflight_clean`

Log hygiene check:

- No `Error:`
- No `Fatal`
- No `ensure`
- No `Array has changed during ranged-for iteration`
- No duplicate console-object warnings for the boss projectile VFX CVars.

### Normal Boss Summary

Setup evidence:

- `BossGatePathArmed BossCount=1 FourHorsemen=0`
- `[MAP] Tower boss-floor entry activated via descent hole.`
- `BossState BossID=Dungeon_SewerSlimeKing Awakened=1 Alive=1 HP=1250`

Projectile summary:

```text
Fired=34 ActivePeak=18 HitHero=0 Expired=26 HitWorld=0 DroppedFires=0 DroppedInvalidSource=0 VisualBucketOverflow=0 ApplyDamageReturnedFalse=0 VisualBuckets=3 ManagerTickMaxUs=459.5 HISMUpdateMaxUs=422.9
```

Interpretation:

- The normal Dungeon boss fired through the manager and stayed alive/awakened.
- This harness placement did not produce hero hits for Sewer Slime King during the 18-second window. Hit/damage attribution is proven by the Four Horsemen smokes below.

### Four Horsemen Summary

Setup evidence:

- `Stage17FourHorsemenSetup SelectedDifficulty=Impossible CurrentStage=17`
- `BossGatePathArmed BossCount=4 FourHorsemen=1`
- `[MAP] Tower boss-floor entry activated via descent hole.`

Damage evidence:

- 17 `Delivery=BossProjectile` combat-damage lines.
- Example source IDs observed:
  - `Hell_Horseman_Conquest`
  - `Hell_Horseman_Death`
  - `Hell_Horseman_Famine`

Projectile summary:

```text
Fired=280 ActivePeak=39 HitHero=143 Expired=0 HitWorld=123 DroppedFires=0 DroppedInvalidSource=0 VisualBucketOverflow=0 ApplyDamageReturnedFalse=126 VisualBuckets=8 ManagerTickMaxUs=1668.5 HISMUpdateMaxUs=650.8
```

Interpretation:

- Multi-boss capacity held with no dropped fires and no visual bucket overflow.
- HISM/manager timings stayed under 2 ms max in this smoke.
- `ApplyDamageReturnedFalse=126` is expected under repeated collision attempts and run-state damage gating; real hero damage still applied and logged repeatedly.

### Kill Mid-Flight Summary

Setup evidence:

- `Stage17FourHorsemenSetup SelectedDifficulty=Impossible CurrentStage=17`
- `BossGatePathArmed BossCount=4 FourHorsemen=1`
- `[MAP] Tower boss-floor entry activated via descent hole.`
- `KillMidFlightApplied BossID=Hell_Horseman_Conquest Damage=584800 AliveAfter=0`
- Final state: `Hell_Horseman_Conquest Awakened=1 Alive=0 HP=0`.

Damage evidence:

- 15 `Delivery=BossProjectile` combat-damage lines.
- After Conquest is killed, subsequent damage lines come from other live bosses, primarily `Hell_Horseman_Death`.

Projectile summary:

```text
Fired=232 ActivePeak=39 HitHero=98 Expired=0 HitWorld=120 DroppedFires=0 DroppedInvalidSource=0 VisualBucketOverflow=0 ApplyDamageReturnedFalse=83 VisualBuckets=8 ManagerTickMaxUs=1558.6 HISMUpdateMaxUs=1327.2
```

Interpretation:

- The killed boss did not continue applying projectile damage after death in this smoke.
- `DroppedInvalidSource=0` means this run did not catch an active Conquest projectile after Conquest was killed; it proves no killed-source damage leaked, but not a positive invalid-source-drop event.

## Acceptance Status

- Boss projectiles fire through `UT66ProjectileManagerSubsystem`: PASS
- Boss actor remains rich `AT66BossBase`: PASS
- Boss firing patterns remain boss-owned: PASS
- HISM boss projectile bodies render in staged screenshots: PASS
- Boss projectile trail/impact VFX render in staged screenshots: PASS
- Boss projectile damage applies with `SourceID=<BossID>` and `Delivery=BossProjectile`: PASS
- Four Horsemen multi-boss capacity holds: PASS
- No production path spawns `AT66BossProjectile`: PASS
- `AT66BossProjectile` deprecated but retained: PASS
- Staged standalone and shortcut refresh: PASS
- Final smoke logs clean of handled ensures/errors/fatals: PASS
- Floors 3/4 placed-miniboss runtime walk-through in this pass: NOT RUN

## Remaining Caveats

- `AT66BossProjectile.cpp` remains compiled compatibility code and still owns the old actor implementation. That is intentional for this pass; future cleanup can remove the class after confirming no asset/reference consumers need it.
- `T66GameMode_Backrooms.cpp` still references `AT66BossProjectile::StaticClass()` in an actor cleanup filter. This is not a production firing path.
- Unique Debuff projectiles remain actor/projectile-movement based and out of scope.
- The boss projectile smoke jumps to the boss-floor entry path; it does not newly exercise all placed-miniboss floor transitions. The prior miniboss packet already documents the floor-3/floor-4 runtime proof gap and recommends a small dedicated non-shipping traversal smoke if full automated proof is desired.
- The normal Sewer Slime King smoke fired managed projectiles but did not hit the stationary hero in the harness. Four Horsemen provided the positive hero-damage proof.

## Next Follow-Up

Recommended next pass options:

1. Add a small dedicated placed-miniboss traversal smoke if the team wants runtime proof for floors 3 and 4 without manual play.
2. Continue to the planned basic-mob optimization layers after the remaining B.10 acceptance contract is settled:
   - B.11 VAT into manager
   - B.12 per-actor tick disable on `AT66MobBase`
   - B.13 mob HISM rendering with VAT
