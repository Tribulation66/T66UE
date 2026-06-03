# T66 Lightweight Actor Plan

Status: ACTIVE ARCHITECTURE PLAN.

Source evaluation: `PerformanceSystem/2026-05-23_T66_LightweightActor_Evaluation.md`.

This plan supersedes the Mass migration implementation path for T66 regular mobs. The Mass plan remains historical reference only; regular-mob performance work now follows an additive Lightweight Actor path.

## Baseline And Measurement Rule

The Mass rollback variance pass showed that single-capture FPS numbers are too noisy for acceptance decisions in this codebase.

- Fixed-instrumentation historical comparator: 161.60 average FPS in the `>= 80` saturated band.
- Five-capture rollback characterization: mean 151.74 average FPS, median 151.37, sample stdev 6.53, range 144.39-160.89.
- Working Pass B comparator: five-capture median, anchored to 151.37 average FPS unless a later five-run baseline supersedes it.
- Standing rule: future performance acceptance must report five captures, median, mean, stdev, min, and max. Single-capture comparisons are not sufficient.

## Architectural Decision

T66 will introduce a new lightweight basic-mob actor path alongside the existing rich enemy path.

- `AT66MobBase : AActor` is the new lightweight foundation for regular/basic mobs.
- `AT66EnemyBase : ACharacter` remains the rich path for specials, mini-bosses, bosses, scripted/tutorial cases, lab actors, tower guardians, chest mimic, goblin thief, and unique debuff enemies.
- No inheritance swap happens until lightweight parity is proven across movement, combat, debug visibility, visual animation, pooling, and spawn routing.
- The first usable implementation keeps actor visuals before moving to ISM/HISM. ISM rendering is a later optimization after behavior parity is stable.

This avoids forcing special enemies through an unproven lightweight contract and keeps every build checkpoint attributable.

## Pass B Sequence

| Pass | Evaluation step ownership | Scope | Build-green gate | Performance gate |
| --- | --- | --- | --- | --- |
| B.1 | Step 2 and Step 5 foundation | Add unused `AT66MobBase`, unused `UT66MobManagerSubsystem`, and this plan. No gameplay wiring. | Development standalone build and staged refresh pass. | Five-capture median within 5% of working baseline; no visible mob stub logs in captures. |
| B.2 | Step 2 plus manager tick foundation | Hook manager into one central tick source, add explicit register/unregister wiring for future lightweight mobs, and add one controlled test spawn path that creates a dormant melee mob doing nothing. | Build after manager tick hook, build after test-spawn hook. | Five-capture median within 5%; one-mob smoke proves no director/combat side effects. |
| B.3 | Step 3 and Step 6 | Add central direct-vector movement and status timer update for one lightweight melee mob. No director routing yet. | Build after movement state adapter, build after status update. | One-mob gameplay smoke plus five-capture median. |
| B.4 | Step 6 and combat bridge foundation | Combat plumbing only: `AT66MobBase` becomes a first-class damageable target. Registry widening, hit-zone damage taking, touch damage to hero, death flow. No director changes. Tested with console-spawned mobs. | Build after registry widening, build after combat dispatch, smoke one mob through hit, touch, death, and status. | Five-capture no-mob regression check. |
| B.5 | Step 5 and Step 6 data binding | `AT66MobBase` reads `Enemies.csv` identity/family data, applies the same stage/difficulty combat scaling used by rich enemies, and applies `CharacterVisuals.csv` / `MobVertexAnimations.csv` visuals with AActor capsule-bottom alignment. Console tests spawn fully configured mobs. No director changes. | Build after configure path, build after visual alignment, smoke configured `Slime` and 10-row Dungeon roster. | Five-capture no-mob regression check. |
| B.6 | Step 6 and Step 7 | Add feature-gated director routing for Melee family only. Existing `AT66EnemyBase` path remains default. | Build with CVar default off, then CVar on smoke. | Five-capture off-path regression check; on-path melee parity capture. |
| B.7 | Step 7 integration closure slice | Add lightweight mob pool integration plus HUD/minimap widening. Loot, score, and debug bridge decisions land here, still Melee-only. | Build after pool bridge, build after HUD/minimap bridge. | Five-capture median and targeted map/debug smoke. |
| B.8 | Step 6 and Step 7 | Migrate Rush family behind the same feature gate. | Build after Rush behavior parity. | Five-capture Rush parity and saturated median checks. |
| B.9 | Step 6 and Step 7 | Migrate Flying family behind the same feature gate. | Build after Flying behavior parity. | Five-capture Flying parity and saturated median checks. |
| B.10 | Step 7 | Migrate Ranged family, including the enemy projectile fire bridge. Enemy projectiles remain actor-based until measured otherwise. | Build after Ranged movement, build after projectile bridge. | Five-capture Ranged parity and projectile-load checks. |
| B.11 | Step 8 | Move VAT frame/update ownership out of per-actor tick and into the manager while still using per-actor mesh components. | Build and visual smoke for each migrated family. | Five-capture median and visual capture. |
| B.12 | Step 9 | Confirm per-basic-mob actor tick is disabled for migrated lightweight families and rely on manager updates only. | Build and live smoke with migrated families. | Five-capture median must improve or remain neutral. |
| B.13 | Step 10 | CLOSED - NO-LAND (2026-05-29). Attempted ISM/HISM render swap; every variant regressed full-resolution FPS vs the per-mob static-mesh renderer. Per-mob static mesh is the chosen renderer. See `PerformanceSystem/B13_MobInstancedRendering_Audit.md`. | n/a - not landed. | De-risk gate satisfied: no instanced renderer entered live source. |
| B.14 | Final cleanup | Retain `AT66EnemyBase` only for rich/special paths, remove obsolete feature gates once proven, and update master combat/performance docs. | Full Development build/stage. | Five-capture median and gameplay parity checklist. |

Evaluation "Build-Green Order" mapping:

1. Mass rollback: complete before this plan.
2. Add lightweight manager/subsystem with no behavior changes: B.1 and B.2.
3. Extract movement/status/family calculations behind adapters while current enemies still work: B.3.
4. Preserve rich special path: architectural constraint across all passes.
5. Add explicit collision/visual components and compatibility accessors: B.1.
6. Replace CMC movement conveniences with central manager integration: B.3-B.10.
7. Fix projectile and pool assumptions: B.7 and B.10.
8. Move VAT frame ticking to the manager: B.11.
9. Disable per-basic-enemy actor tick: B.12.
10. Add ISM/HISM rendering after parity: B.13.

## Pass B.1 Additive Foundation

### `AT66MobBase`

Location:

- `Source/T66/Gameplay/T66MobBase.h`
- `Source/T66/Gameplay/T66MobBase.cpp`

Shape:

- Inherits `AActor`, not `ACharacter` and not `APawn`.
- `PrimaryActorTick.bCanEverTick = false`.
- Does not spawn anywhere in gameplay in B.1.
- Exists so later passes can route lightweight targets without changing `AT66EnemyBase`.

Components:

- `UCapsuleComponent` root named `MobCapsule`.
  - Default radius 42 uu.
  - Default half-height 88 uu, matching the current enemy director spawn-height assumption.
  - Query/physics collision, object type `ECC_Pawn`, blocking `WorldStatic`, `WorldDynamic`, `Pawn`, and `Visibility`, matching the current enemy capsule intent.
- `UStaticMeshComponent VisualMesh` attached to root.
  - Collision disabled.
  - Grounded with the existing `Z=-38` primitive offset used by `AT66EnemyBase`.
  - Uses the same default red sphere placeholder shape through `FT66VisualUtil`.
- `UT66CombatHitZoneComponent BodyHitZone` attached to root.
  - Body zone at `Z=64`, radius 42.
- `UT66CombatHitZoneComponent HeadHitZone` attached to root.
  - Head zone at `Z=124`, radius 24.
- `UWidgetComponent LockIndicatorWidget` attached to root.
  - Screen-space widget, `52x52`, `UT66EnemyLockWidget`, hidden by default.

Data members:

- `ET66EnemyFamily EnemyFamily`
- `FName MobID`
- `float CurrentHP`
- `float MaxHP`
- `FVector StoredVelocity`
- Status placeholders for stun, root, freeze, slow, and knockback timers/durations.
- Lock-on state.
- Lifecycle state: `Active`, `Dying`, `Pooled`.

API surface:

- `GetCurrentHP`
- `GetMaxHP`
- `GetEnemyFamily`
- `GetMobID`
- `TakeDamageFromHeroHitZone`
- `ApplyStun`
- `ApplyRoot`
- `ApplyFreeze`
- `ApplySlow`
- `ApplyAutoAttackKnockback`
- `ApplyPullTowards`
- `ApplyPushAwayFrom`
- `ShowLockIndicator`
- `HideLockIndicator`
- `ResetForReuse`

All B.1 behavior methods are stubs with `VeryVerbose` logs. They are not connected to gameplay.

### `UT66MobManagerSubsystem`

Location:

- `Source/T66/Gameplay/T66MobManagerSubsystem.h`
- `Source/T66/Gameplay/T66MobManagerSubsystem.cpp`

Shape:

- Inherits `UWorldSubsystem`.
- Maintains `TArray<TWeakObjectPtr<AT66MobBase>> ActiveMobs`.
- Provides `RegisterMob`, `UnregisterMob`, `GetActiveMobs`, and a no-op `Tick(float DeltaSeconds)`.
- `Tick` is not hooked into `FTickableGameObject`, world delegates, actor tick, timers, or any runtime loop in B.1.
- Lifecycle and API methods log at `VeryVerbose` only.

Planned B.2 tick strategy:

- Add one explicit manager-owned update entry point.
- Keep the update source centralized so lightweight mobs never regain per-actor tick.
- Register/unregister only through controlled spawn/pool paths.
- Avoid registry/HUD/combat widening until the first lightweight mob can exist safely in a test path.

## Isolation Rules

B.1 must not modify:

- `AT66EnemyBase`
- `AT66EnemyDirector`
- `UT66EnemyPoolSubsystem`
- `UT66CombatComponent`
- `UT66ActorRegistrySubsystem`
- `UT66CharacterVisualSubsystem`
- Existing enemy family classes
- Existing projectile classes
- HUD/minimap or debug target scans

Any of those changes belongs to B.2 or later.

## Open Questions

### VAT On ISM Per-Instance Data

Current regular mobs are friendly to static/VAT rendering, but the exact ISM strategy still needs proof:

- which material parameters become per-instance custom data
- how frame range, play rate, current frame, and clip selection map into instance data
- whether one ISM component per family/clip/material is cleaner than one large mixed component

### ACharacter Capsule-Bottom Alignment Fix

The current enemy primitive visual uses a manual `Z=-38` offset against an 88 uu half-height capsule. Later lightweight and ISM passes should preserve apparent floor contact before changing visuals. If VAT assets expose different bounds, the alignment rule needs a deterministic helper instead of hand-tuned offsets.

### Pool Integration Shape

The current pool is typed around `AT66EnemyBase`. Lightweight mobs should probably use a separate pool or manager-owned free list, not a widened rich enemy pool, so regular mobs can avoid `ACharacter` assumptions entirely.

### Registry Widening Strategy

`UT66ActorRegistrySubsystem` stores `AT66EnemyBase` enemy pointers and, as of B.4, a separate lightweight mob collection. Keep the collections separate so existing enemy-only callers preserve current HUD/minimap and director behavior.

- `GetEnemies()` remains enemy-only.
- `GetActiveMobs()` remains lightweight-only.
- `GetAllDamageableTargets()` is the temporary combat bridge that combines regular enemy actors and lightweight mobs.
- HUD/minimap marker widening is deferred to B.6.

### Combat Target Interface

Current combat target handles already carry `AActor`, which is favorable. B.4 uses explicit parallel casts for `AT66EnemyBase` and `AT66MobBase` in the existing combat dispatch path instead of introducing a broad interface refactor mid-pass. A future `UT66DamageableTargetInterface` or equivalent remains a cleanup candidate once Melee, Rush, Flying, and Ranged all prove the same contract.

### Specials Stay Rich

Goblin thief, unique debuff, chest mimic, mini-bosses, bosses, tutorial/scripted enemies, lab actors, and tower guardians remain on `AT66EnemyBase` indefinitely unless a later pass proves a specific class is safe to migrate.

## Pass B.1 Verification

Completed 2026-05-24.

Build and staging:

- Development standalone build succeeded with `Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject`.
- Staged standalone refreshed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`.
- Root and taskbar `T66 Standalone.lnk` shortcuts both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Symbol scan of the staged PDB found `AT66MobBase`, `UT66MobManagerSubsystem`, `LogT66MobBase`, and `LogT66MobManager`.
- Accepted capture logs contained no `T66MobBase` or `T66MobManager` runtime log lines, confirming no gameplay path invoked the new stubs at default verbosity.

Performance artifact root:

- `C:\UE\T66\Saved\Codex\Performance\LightweightActorB1`

Accepted five-capture set at 90 cap, saturated band `LiveRegularEnemies >= 80`:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T072729Z_vEK1I0eXqa9CJven42QHxw` | 155.98 | 84.54 | 34.17 | 90 | 2 | 7 | 0 us |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T073259Z_pj7W2kPsVkPdoWqlMw6fIw` | 147.67 | 81.65 | 33.71 | 90 | 2 | 6 | 0 us |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T073615Z_2Ehm-krm3w8hsWOy-ETAhg` | 157.35 | 86.56 | 36.60 | 90 | 0 | 6 | 0 us |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T074247Z_EIs0zU0oQz9aCayvgxzHTA` | 172.10 | 89.61 | 36.56 | 90 | 0 | 5 | 0 us |
| Run06 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T074643Z_OWWFuEfvlqoL-EGCA8tiSw` | 154.41 | 68.47 | 20.42 | 90 | 0 | 12 | 0 us |

Computed values:

- Avg FPS min/max: `147.67 / 172.10`
- Avg FPS median: `155.98`
- Avg FPS mean: `157.50`
- Avg FPS sample stdev: `8.97`
- Comparator baseline: median `151.37`, mean `151.74`
- Median delta from baseline median: `+3.05%`
- Acceptance result: pass, within the 5% median gate.
- PerformanceSystem overhead max median: `0 us` because no `PerformanceSystemOverhead/FrameworkBudgetExceeded` events were emitted in the accepted runs.

Discarded capture:

- Run04 wrote a valid PerformanceSystem session but the process returned exit `-1073740791`.
- Its log tail showed normal `RequestExit`, `PreExit`, and `Exiting` lines, but it was excluded from the accepted five-run performance set so the verification table only uses clean process exits.

## Pass B.2 Verification

Completed 2026-05-24.

Implemented scope:

- `UT66MobManagerSubsystem` now inherits `FTickableGameObject`.
- `IsTickable()` returns false while `ActiveMobs` is empty, so normal gameplay pays no manager tick cost before lightweight mobs exist.
- `AT66MobBase::BeginPlay` registers with the manager; `EndPlay` unregisters idempotently.
- Development-only console commands were added:
  - `T66.Mob.SpawnTest`
  - `T66.Mob.DespawnAllTest`

Smoke test artifacts:

- Screenshot: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB2\spawn_test.png`
- Primary smoke log: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB2\spawn_test_final.log`
- Open-floor visual smoke log: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB2\spawn_test_visual_final.log`

Smoke log excerpts:

```text
LogT66MobManager: VeryVerbose: RegisterMob mob=T66MobBase_2147479731 MobID=TestMob_SpawnTest ActiveMobs.Num()=1
LogT66MobManager: Display: T66.Mob.SpawnTest spawned mob=T66MobBase_2147479731 MobID=TestMob_SpawnTest location=V(X=300.00, Y=3575.00, Z=1202.15) ActiveMobs.Num()=1
LogT66MobManager: VeryVerbose: Tick delta=0.0051 ActiveMobs.Num()=1
LogT66MobManager: VeryVerbose: UnregisterMob mob=T66MobBase_2147479731 MobID=TestMob_SpawnTest ActiveMobs.Num()=0
LogT66MobManager: Display: T66.Mob.DespawnAllTest destroyed 1 test mob(s). ActiveMobs.Num()=0
```

The open-floor visual smoke used the existing `enemywaveperf` camera setup so the Unreal-owned screenshot could show the spawned diagnostic actor in a gameplay floor context. That run returned `-1073740791` after a normal `RequestExit` log sequence and was not used for performance acceptance.

Performance artifact root:

- `C:\UE\T66\Saved\Codex\Performance\LightweightActorB2`

Accepted five-capture set at 90 cap, saturated band `LiveRegularEnemies >= 80`, with no `AT66MobBase` spawned:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T081801Z_5v0680v3y7MhD1OTQnQdgQ` | 154.97 | 72.71 | 28.83 | 90 | 0 | 11 | 0 us |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T082117Z_K4qu8EjuhkJTm4mr4NxKeg` | 150.34 | 84.95 | 34.99 | 90 | 2 | 5 | 0 us |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T082430Z_7yLqhUCIRR9lcEG1qrjSaQ` | 155.49 | 82.84 | 32.45 | 90 | 0 | 5 | 0 us |
| Run04 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T082744Z_BCoVLknKkkvvVoie4ksH4w` | 157.06 | 86.88 | 36.69 | 90 | 2 | 5 | 0 us |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T083057Z_DFkYHUXtMsc0c6utTei-SA` | 152.45 | 74.88 | 34.16 | 90 | 0 | 5 | 0 us |

Computed values:

- Avg FPS min/max: `150.34 / 157.06`
- Avg FPS median: `154.97`
- Avg FPS mean: `154.06`
- Avg FPS sample stdev: `2.66`
- Comparator baseline median: `151.37`
- Pass B.1 median: `155.98`
- Median delta from baseline median: `+2.38%`
- Median delta from B.1 median: `-0.65%`
- Acceptance result: pass, within the 5% median gate.
- PerformanceSystem overhead max median: `0 us` because no `PerformanceSystemOverhead/FrameworkBudgetExceeded` events were emitted in the accepted runs.
- Discarded performance captures: none.
- Capture-log isolation check: no `T66MobBase`, `T66MobManager`, `T66.Mob`, `RegisterMob`, `UnregisterMob`, `TestMob`, `SpawnTest`, or `DespawnAllTest` lines appeared in the five accepted performance logs.

## Pass B.3 Verification

Completed 2026-05-24.

Implemented scope:

- `UT66MobManagerSubsystem::Tick` now owns lightweight mob movement and status timer updates.
- Active test mobs chase the local `AT66HeroBase` by direct vector movement at the Melee default speed of `350 uu/s`.
- The manager preserves mob Z, uses no NavMesh, wall avoidance, sweeps, or collision response, and leaves `AT66MobBase` actor tick disabled.
- Stun, root, freeze, slow, and knockback timer placeholders are decremented centrally in the manager tick.
- Slow applies `ChaseSpeed *= 1.0 - SlowStrength`, clamped to `[0, 1]`.
- Development-only status console commands were added:
  - `T66.Mob.TestStun [Seconds=2.0]`
  - `T66.Mob.TestSlow [Seconds=3.0] [Strength=0.5]`
  - `T66.Mob.TestRoot [Seconds=2.0]`
  - `T66.Mob.TestFreeze [Seconds=2.0]`

Build and staging:

- Development standalone build succeeded with `Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject`.
- Staged standalone refreshed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`.
- Root and taskbar `T66 Standalone.lnk` shortcuts both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Smoke test artifacts:

- Status smoke log: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3\chase_smoke\b3_chase_smoke.log`
- Visual sequence smoke log: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3\chase_smoke\b3_chase_visual_sequence.log`
- Unreal-authored screenshot sequence:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3\chase_smoke\chase_01.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3\chase_smoke\chase_02.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3\chase_smoke\chase_03.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3\chase_smoke\chase_04.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3\chase_smoke\chase_05.png`

Smoke log excerpts:

```text
LogT66MobManager: VeryVerbose: RegisterMob mob=T66MobBase_2147479526 MobID=TestMob_SpawnTest ActiveMobs.Num()=1
LogT66MobManager: Display: T66.Mob.SpawnTest spawned mob=T66MobBase_2147479526 MobID=TestMob_SpawnTest location=V(X=300.00, Y=3575.00, Z=1202.15) ActiveMobs.Num()=1
LogT66MobManager: VeryVerbose: Tick delta=0.0054 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=182.67, Y=3575.00, Z=1202.15) velocity=V(X=-350.00) status=None
LogT66MobManager: VeryVerbose: Tick delta=0.0054 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=98.34, Y=3575.00, Z=1202.15) velocity=V(0) status=None
LogT66MobManager: VeryVerbose: Tick delta=0.0091 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=66.00, Y=3606.97, Z=1202.15) velocity=V(X=-154.72, Y=313.94) status=None
LogT66MobManager: VeryVerbose: Tick delta=0.0113 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=43.29, Y=3653.06, Z=1202.15) velocity=V(0) status=None
LogT66MobBase: VeryVerbose: ApplyStun mob=T66MobBase_2147479490 duration=3.000
LogT66MobManager: Display: T66.Mob.TestStun seconds=3.00 affected=1
LogT66MobManager: VeryVerbose: Tick delta=0.0053 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=98.14, Y=3575.00, Z=1202.15) velocity=V(0) status=Stun=2.83
LogT66MobBase: VeryVerbose: ApplySlow mob=T66MobBase_2147479490 strength=0.700 duration=5.000
LogT66MobManager: Display: T66.Mob.TestSlow seconds=5.00 strength=0.70 affected=1
LogT66MobManager: VeryVerbose: Tick delta=0.0052 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=98.14, Y=3575.00, Z=1202.15) velocity=V(0) status=Slow=4.78/0.70
LogT66MobBase: VeryVerbose: ApplyRoot mob=T66MobBase_2147479490 duration=2.000
LogT66MobManager: Display: T66.Mob.TestRoot seconds=2.00 affected=1
LogT66MobManager: VeryVerbose: Tick delta=0.0052 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=98.14, Y=3575.00, Z=1202.15) velocity=V(0) status=Root=1.87|Slow=1.15/0.70
LogT66MobBase: VeryVerbose: ApplyFreeze mob=T66MobBase_2147479490 duration=2.000
LogT66MobManager: Display: T66.Mob.TestFreeze seconds=2.00 affected=1
LogT66MobManager: VeryVerbose: Tick delta=0.0041 ActiveMobs.Num()=1 sampleMob=TestMob_SpawnTest loc=V(X=98.14, Y=3575.00, Z=1202.15) velocity=V(0) status=Freeze=1.73
LogT66MobManager: VeryVerbose: UnregisterMob mob=T66MobBase_2147479526 MobID=TestMob_SpawnTest ActiveMobs.Num()=0
LogT66MobManager: Display: T66.Mob.DespawnAllTest destroyed 1 test mob(s). ActiveMobs.Num()=0
```

The screenshot sequence shows the diagnostic sphere visible in the staged gameplay viewport. The position/velocity log lines are the stronger chase proof because the test mob reaches the 100 uu arrival band quickly at `350 uu/s`.

Performance artifact root:

- `C:\UE\T66\Saved\Codex\Performance\LightweightActorB3`

Accepted five-capture set at 90 cap, saturated band `LiveRegularEnemies >= 80`, with no `AT66MobBase` spawned:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T091237Z_fzEMYUwdX567s9ezXBgiIQ` | 159.55 | 89.50 | 42.48 | 90 | 4 | 4 | 8143.0 us | 0 |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T091551Z_FFUoTU0gSd3di2iQQHaX3g` | 163.13 | 94.56 | 46.69 | 90 | 0 | 4 | 939.3 us | 0 |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T091903Z_AjPHZkFrrE37maW8InbRTA` | 154.29 | 88.71 | 38.69 | 90 | 0 | 4 | 10081.7 us | 0 |
| Run04 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T092215Z_g6BEA0Sj7LDukBamWUZh9w` | 151.32 | 92.87 | 44.90 | 90 | 0 | 5 | 1049.5 us | 0 |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T092527Z_ezZLBkIIy50AKlGsVgKKjg` | 157.81 | 89.63 | 40.83 | 90 | 2 | 5 | 1251.9 us | 0 |

Computed values:

- Avg FPS min/max: `151.32 / 163.13`
- Avg FPS median: `157.81`
- Avg FPS mean: `157.22`
- Avg FPS sample stdev: `4.59`
- Comparator baseline median: `151.37`
- Pass B.1 median: `155.98`
- Pass B.2 median: `154.97`
- Median delta from baseline median: `+4.26%`
- Median delta from B.2 median: `+1.83%`
- Acceptance result: pass, within the 5% median gate.
- PerformanceSystem overhead max median: `1251.9 us`, under the 2 ms median gate.
- PerformanceSystem overhead largest individual spike: `10081.7 us`; this keeps the intermittent framework-overhead pending issue relevant, but the pass gate used median overhead as requested.
- Discarded performance captures: none.
- B.3 performance non-zero exits: `0`.
- Crash watch running total across B.1-B.3: `2` known `-1073740791` exits before B.3 (`1` B.1 performance discard, `1` B.2 smoke). B.3 added no new non-zero exits.
- Capture-log isolation check: no `T66MobBase`, `T66MobManager`, `TestMob`, `SpawnTest`, `TestStun`, `TestSlow`, `TestRoot`, `TestFreeze`, `RegisterMob`, or `UnregisterMob` lines appeared in the five accepted performance logs.

## Pass B.4 Verification

Completed 2026-05-24.

Implemented scope:

- `UT66ActorRegistrySubsystem` now stores lightweight mobs separately from rich enemies and exposes `GetAllDamageableTargets()` for combat scans that should see both `AT66EnemyBase` and `AT66MobBase`.
- `AT66MobBase::BeginPlay` and `EndPlay` register/unregister with both `UT66MobManagerSubsystem` and `UT66ActorRegistrySubsystem`.
- `AT66MobBase::TakeDamageFromHeroHitZone` now applies body/head multipliers, logs damage, emits floating combat text, applies knockback setup, transitions to `Dying`, notifies the manager, and defers `Destroy()` on lethal damage.
- `UT66MobManagerSubsystem` now handles lightweight mob dying state, touch damage to the hero, touch cooldown decrementing, and knockback displacement during the active knockback window.
- `UT66CombatComponent` now widens damageable target scans and dispatch paths with explicit parallel casts for `AT66EnemyBase` and `AT66MobBase`. The future interface extraction remains deferred.
- Director routing, pool integration, HUD/minimap widening, loot, score, death VFX, lock-on, ISM/VAT work, and family migration remain out of B.4.

Build, staging, and shortcut verification:

- Development standalone build succeeded with `Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject`.
- Staged standalone refreshed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`.
- Root and taskbar `T66 Standalone.lnk` shortcuts both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Smoke test artifacts:

- Smoke log: `C:\UE\T66\Saved\StandaloneLogs\T66_PhaseB4_CombatSmoke.log`
- Unreal-authored screenshot sequence:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB4\combat_smoke\b4combat_0000.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB4\combat_smoke\b4combat_0001.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB4\combat_smoke\b4combat_0002.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB4\combat_smoke\b4combat_0003.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB4\combat_smoke\b4combat_0004.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB4\combat_smoke\b4combat_0005.png`

Smoke log excerpts:

```text
LogTemp: Display: [MobCombatSmoke] Entered gameplay floor=2 location=V(Y=-1300.00) with director paused.
LogT66MobManager: VeryVerbose: RegisterMob mob=T66MobBase_2147479679 MobID=TestMob_B4Kill ActiveMobs.Num()=1
LogTemp: Display: [MobCombatSmoke] Spawned TestMob_B4Kill hp=45.0 location=V(X=240.00, Y=-1300.00)
LogT66MobBase: Display: MobHitDamage mob=T66MobBase_2147479679 MobID=TestMob_B4Kill damage=10044 hp=0.0/45.0 hitZone=1 source=Ultimate event=None
LogT66MobManager: Display: NotifyMobDying mob=T66MobBase_2147479679 MobID=TestMob_B4Kill finalLocation=V(X=98.35, Y=-1300.00)
LogTemp: Display: [MobCombatSmoke] Fired hero combat component scoped shot through T66MobBase_2147479679.
LogT66MobManager: VeryVerbose: UnregisterMob mob=T66MobBase_2147479679 MobID=TestMob_B4Kill ActiveMobs.Num()=0
LogT66MobManager: VeryVerbose: RegisterMob mob=T66MobBase_2147479504 MobID=TestMob_B4StunnedTouch ActiveMobs.Num()=1
LogTemp: Display: [MobCombatSmoke] Spawned TestMob_B4StunnedTouch hp=300.0 location=V(X=48.00, Y=-1300.00, Z=-21.85)
LogTemp: Display: [MobCombatSmoke] Applied stun to T66MobBase_2147479504 for touch-damage parity check.
LogT66MobManager: Display: MobTouchDamage mob=T66MobBase_2147479504 MobID=TestMob_B4StunnedTouch damageHP=20 hero=BP_HeroBase_C_2147479852 cooldown=0.50
LogT66MobManager: Display: MobTouchDamage mob=T66MobBase_2147479504 MobID=TestMob_B4StunnedTouch damageHP=20 hero=BP_HeroBase_C_2147479852 cooldown=0.50
LogT66MobManager: VeryVerbose: UnregisterMob mob=T66MobBase_2147479504 MobID=TestMob_B4StunnedTouch ActiveMobs.Num()=0
```

The B.4 smoke path uses a non-shipping `mobcombatsmoke` automation hook. It fires the hero combat component's scoped piercing path through the lightweight mob to prove the real `UT66CombatComponent -> AT66MobBase::TakeDamageFromHeroHitZone` dispatch and death path without changing director routing or requiring weapon altar setup. Touch damage is verified separately by a stunned test mob, matching current rich-enemy behavior where stun does not suppress touch damage.

Performance artifact root:

- `C:\UE\T66\Saved\Codex\Performance\LightweightActorB4`

Accepted five-capture set at 90 cap, saturated band `LiveRegularEnemies >= 80`, with no `AT66MobBase` spawned:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T105703Z_VBbghEAF-xcKYBu0MR4mqw` | 157.13 | 100.09 | 51.38 | 90 | 2 | 9 | 1126.4 us | 0 |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T105933Z_XmkOZkYMCmnwKhC3wsoBog` | 164.66 | 101.03 | 55.98 | 90 | 0 | 6 | 1092.7 us | 0 |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T110206Z_MyNbCkZ-N3MyIcqntZNrKA` | 169.73 | 111.31 | 65.26 | 90 | 2 | 5 | 824.0 us | 0 |
| Run04 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T110438Z_FvQL7U5Nl4VRcrK0GUTEDg` | 159.11 | 105.05 | 71.93 | 90 | 0 | 5 | 808.3 us | 0 |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T110711Z_63Wfckfw5fqgnnufXnrg4g` | 149.95 | 73.94 | 26.47 | 90 | 2 | 13 | 1061.8 us | 0 |

Computed values:

- Avg FPS min/max: `149.95 / 169.73`
- Avg FPS median: `159.11`
- Avg FPS mean: `160.12`
- Avg FPS sample stdev: `6.73`
- Comparator baseline median: `151.37`
- Pass B.1 median: `155.98`
- Pass B.2 median: `154.97`
- Pass B.3 median: `157.81`
- Median delta from baseline median: `+5.11%`
- Median delta from B.3 median: `+0.82%`
- Acceptance result: pass, above the 5% regression gate.
- PerformanceSystem overhead max median: `1061.8 us`, under the 2 ms median gate.
- PerformanceSystem overhead largest individual spike: `1126.4 us`; no B.4 accepted capture reproduced the earlier multi-millisecond framework-overhead spike.
- Discarded valid performance captures: none.
- B.4 performance non-zero exits: `0`.
- Crash watch running total across B.1-B.4: `2` known `-1073740791` exits before B.3 (`1` B.1 performance discard, `1` B.2 smoke). B.3 and B.4 performance sets added no new non-zero exits.
- Capture-log isolation check: no `T66MobBase`, `T66MobManager`, `TestMob`, `SpawnTest`, `TestStun`, `TestSlow`, `TestRoot`, `TestFreeze`, `RegisterMob`, `UnregisterMob`, `MobHitDamage`, `MobTouchDamage`, or `NotifyMobDying` lines appeared in the five accepted performance logs.

## Pass B.5 Verification

Completed 2026-05-24.

Implemented scope:

- `AT66MobBase::ConfigureAsMob` now follows the existing rich-enemy data path for `Enemies.csv` identity, family, archetype, and visual identity, then applies the same current stage/difficulty/finale combat scaling formulas used by rich enemies for HP and touch damage.
- `AT66MobBase` now stores configured `ChaseSpeed`, `TouchDamageHearts`, visual identity, archetype, and basic VAT playback state for the manager-owned update path.
- `UT66CharacterVisualSubsystem` now supports AActor-owned lightweight mobs for both static mesh and VAT visual application by resolving the capsule from either `ACharacter` or `AT66MobBase`.
- `T66.Mob.SpawnTest [MobID=Slime]` now spawns a configured lightweight mob instead of a placeholder-only test actor.
- `T66.Mob.SpawnTestRoster` spawns the 10 Dungeon VAT visual rows used for B.5 visual parity checks.
- No director routing, pool routing, HUD/minimap routing, family behavior parity, ISM rendering, or spawn animation restoration landed in B.5.

Build, staging, and shortcut verification:

- Development standalone build succeeded with `Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject`.
- Staged standalone refreshed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`.
- The stage script updated the root and taskbar `T66 Standalone.lnk` shortcuts to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- During the first build attempt, Unreal unity compilation exposed a private-symbol collision between two anonymous namespace constants named `T66AtmosphereSparedTag`. The fix was a local rename in `T66TowerLighting.cpp`; no gameplay behavior changed.

Smoke test artifacts:

- Smoke log: `C:\UE\T66\Saved\StandaloneLogs\T66_PhaseB5_DataBindingSmoke.log`
- Unreal-authored screenshot sequence:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB5\data_binding_smoke\b5databinding_0000.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB5\data_binding_smoke\b5databinding_0001.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB5\data_binding_smoke\b5databinding_0002.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB5\data_binding_smoke\b5databinding_0003.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB5\data_binding_smoke\b5databinding_0004.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB5\data_binding_smoke\b5databinding_0005.png`

Smoke log excerpts:

```text
LogTemp: Display: [MobDataBindingSmoke] Entered gameplay floor=2 location=V(Y=-1300.00) with director paused.
LogT66CharacterVisuals: Verbose: [MOB_VAT] Applied VisualID=Slime Mesh=/Game/Characters/MobsVAT/Slime/SM_EasyMobVAT_Slime.SM_EasyMobVAT_Slime PositionTexture=/Game/Characters/MobsVAT/Slime/TX_EasyMobVAT_Slime_Position.TX_EasyMobVAT_Slime_Position RowsPerFrame=4 NumFrames=195
LogT66MobBase: Display: ConfigureAsMob mob=T66MobBase_2147479593 MobID=Slime family=0 archetype=Melee stage=1 hp=50.0/50.0 touchHearts=1 chaseSpeed=350.0 visual=VAT
LogTemp: Display: [MobDataBindingSmoke] Spawned configured MobID=Slime hp=50.0 speed=350.0 touch=1 location=V(X=240.00, Y=-1560.00)
LogT66CharacterVisuals: Verbose: [MOB_VAT] Applied VisualID=CaveBat Mesh=/Game/Characters/MobsVAT/CaveBat/SM_EasyMobVAT_CaveBat.SM_EasyMobVAT_CaveBat PositionTexture=/Game/Characters/MobsVAT/CaveBat/TX_EasyMobVAT_CaveBat_Position.TX_EasyMobVAT_CaveBat_Position RowsPerFrame=4 NumFrames=195
LogT66MobBase: Display: ConfigureAsMob mob=T66MobBase_2147479578 MobID=CaveBat family=1 archetype=Flying stage=1 hp=50.0/50.0 touchHearts=1 chaseSpeed=430.0 visual=VAT
LogT66MobManager: Display: MobTouchDamage mob=T66MobBase_2147479586 MobID=Slime damageHP=20 hero=BP_HeroBase_C_2147479772 cooldown=0.50
LogT66MobBase: Display: MobHitDamage mob=T66MobBase_2147479593 MobID=Slime damage=10049 hp=0.0/50.0 hitZone=1 source=Ultimate event=None
LogT66MobManager: Display: NotifyMobDying mob=T66MobBase_2147479593 MobID=Slime finalLocation=V(X=80.99, Y=-1387.74)
LogTemp: Display: [MobDataBindingSmoke] Fired hero combat component scoped shot through configured T66MobBase_2147479593.
LogTemp: Display: [MobDataBindingSmoke] Destroyed touch-damage Slime after verification window.
```

The B.5 smoke path uses a non-shipping `mobdatabindingsmoke` automation hook so the verification can prove data binding, VAT visual application, AActor capsule-bottom alignment, hit damage, touch damage, and death flow without touching `AT66EnemyDirector`. The roster branch suppresses repeated roster touch damage inside the automation after visual setup so the run can complete without the hero entering a death state; touch damage itself is verified by the separate configured Slime touch test.

Known B.5 visual regression accepted for this pass:

- `AT66MobBase` does not yet implement rich-enemy rise-from-ground or wall-emerge spawn presentation. Configured lightweight mobs appear immediately. This remains acceptable for B.5 data binding and B.6 director routing parity, and should be restored in a later lightweight visual/spawn pass.

Performance artifact root:

- `C:\UE\T66\Saved\Codex\Performance\LightweightActorB5`

Accepted five-capture set at 90 cap, saturated band `LiveRegularEnemies >= 80`, with no `AT66MobBase` spawned:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T120308Z_TwyO4k5Gxuq4rI63-IPA-w` | 168.89 | 112.91 | 69.79 | 90 | 2 | 6 | 894.5 us | 0 |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T120527Z_LxyrxUTmXiqGvzmWW5uO5A` | 147.06 | 101.04 | 68.42 | 90 | 0 | 6 | 1321.0 us | 0 |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T120745Z_EmblxEkw4D13cX2VK0L1bQ` | 173.44 | 105.83 | 57.05 | 90 | 0 | 5 | 1232.5 us | 0 |
| Run04 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T121002Z_Kgt--UJYYA_ki3ixF7HE9g` | 169.16 | 108.82 | 72.70 | 90 | 2 | 5 | 929.3 us | 0 |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T121220Z_DiDi6EgWezUOsNiuwna9VQ` | 157.57 | 100.39 | 59.67 | 90 | 2 | 6 | 1317.5 us | 0 |

Computed values:

- Avg FPS min/max: `147.06 / 173.44`
- Avg FPS median: `168.89`
- Avg FPS mean: `163.22`
- Avg FPS sample stdev: `10.78`
- Comparator baseline median: `151.37`
- Pass B.1 median: `155.98`
- Pass B.2 median: `154.97`
- Pass B.3 median: `157.81`
- Pass B.4 median: `159.11`
- Median delta from baseline median: `+11.58%`
- Median delta from B.4 median: `+6.15%`
- Acceptance result: pass, above the 5% regression gate.
- PerformanceSystem overhead max median: `1232.5 us`, under the 2 ms median gate.
- PerformanceSystem overhead largest individual spike in accepted runs: `1321.0 us`.
- Discarded valid performance captures: none.
- B.5 performance non-zero exits: `0`.
- Crash watch running total across B.1-B.5: `2` known `-1073740791` exits before B.3 (`1` B.1 performance discard, `1` B.2 smoke). B.3, B.4, and B.5 performance sets added no new non-zero exits.
- Capture-log isolation check: no `T66MobBase`, `T66MobManager`, `TestMob`, `SpawnTest`, `RegisterMob`, `UnregisterMob`, `MobHitDamage`, `MobTouchDamage`, or `NotifyMobDying` lines appeared in the five accepted performance logs.

Methodology note:

- A first local B.5 performance attempt used `-T66GameplayAutoScreenshotDelay=125`, which delayed the `enemywaveperf` preparation itself and produced five underfilled sessions peaking at only 13 live enemies. Those sessions were not used for acceptance. The accepted table above uses the correct ordering: prepare at `1` second and capture/quit after a `125` second post-capture delay.

## Pass B.6 Verification

Completed 2026-05-24.

Implemented scope:

- Added non-shipping CVar `T66.Mob.UseLightweight`, default `0`. When enabled, `AT66EnemyDirector` routes Melee-family, non-mini-boss, non-special spawns to `AT66MobBase`; all other families and special paths continue through `AT66EnemyBase`.
- Added director-side `AT66MobBase` spawn branching in initial population and runtime staggered spawns using the same configured `MobID`, family, archetype, stage scaling, and spawn collision handling as the rich enemy path.
- Added functional live-count widening for spawn/wave progression: `AT66EnemyDirector::GetAliveEnemyCount()` now includes rich enemies plus lightweight mobs, while rich-only and lightweight-only accessors remain available for diagnostics.
- Extended `UT66ActorRegistrySubsystem`, `UT66LagTrackerSubsystem`, and `UT66PerformanceSubsystem` reporting with rich/lightweight split counters in schema version 4.
- Added `-T66MobUseLightweight=0/1` support to the `enemywaveperf` automation path so captures can force the CVar before director setup.

Build, staging, and shortcut verification:

- Development standalone build succeeded with `Build.bat T66 Win64 Development -Project=C:\UE\T66\T66.uproject`.
- Staged standalone refreshed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development`.
- The stage script updated the root and taskbar `T66 Standalone.lnk` shortcuts to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Smoke test artifacts:

- Smoke log: `C:\UE\T66\Saved\StandaloneLogs\T66_PhaseB6_DirectorRoutingSmoke.log`
- Unreal-authored screenshot sequence:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\director_routing_smoke\b6director_0000.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\director_routing_smoke\b6director_0001.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\director_routing_smoke\b6director_0002.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\director_routing_smoke\b6director_0003.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\director_routing_smoke\b6director_0004.png`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\director_routing_smoke\b6director_0005.png`

Smoke log excerpts:

```text
LogTemp: Display: [MobDirectorRoutingSmoke] T66.Mob.UseLightweight set to 1 before director routing.
LogT66EnemyDirector: Warning: [LightweightMob] Wave progression/live-count widened: richAlive=2 lightweightAlive=2 combinedAlive=4.
LogT66MobManager: VeryVerbose: RegisterMob mob=T66MobBase_2147479430 MobID=TombSpider ActiveMobs.Num()=1
LogT66ActorRegistry: Verbose: [GOLD] ActorRegistry: registered lightweight mob T66MobBase_2147479430 (total: 1)
LogT66EnemyDirector: Display: [LightweightMob] Routed spawn MobID=BoneWalker family=0 channel=1 richAlive=3 lightweightAlive=3 totalAlive=6 location=V(X=-14200.42, Y=14258.65, Z=-36.00)
LogT66MobBase: Display: MobHitDamage mob=T66MobBase_2147479430 MobID=TombSpider damage=50 hp=0.0/50.0 hitZone=1 source=B6DirectorRoutingSmoke event=None
LogT66EnemyDirector: Verbose: [LightweightMob] Mob died MobID=TombSpider richAlive=17 lightweightAlive=19 totalAlive=36
LogT66MobManager: Display: NotifyMobDying mob=T66MobBase_2147479430 MobID=TombSpider finalLocation=V(X=98.63, Y=-1297.20, Z=-36.00)
LogT66MobManager: VeryVerbose: UnregisterMob mob=T66MobBase_2147479430 MobID=TombSpider ActiveMobs.Num()=19
LogT66ActorRegistry: Verbose: [GOLD] ActorRegistry: unregistered lightweight mob T66MobBase_2147479430 (total: 19)
LogT66MobManager: Display: MobTouchDamage mob=T66MobBase_2147479347 MobID=TombSpider damageHP=20 hero=BP_HeroBase_C_2147479596 cooldown=0.50
LogTemp: Display: [MobDirectorRoutingSmoke] T66.Mob.UseLightweight set to 0 after routing sample.
```

The smoke test passed the functional B.6 routing checks: CVar toggled on, production Melee allocations entered the `AT66MobBase` path, mixed rich/lightweight live counts reached the director's combined count path, lightweight mobs registered with the manager and registry, hit damage killed a lightweight mob, death notified the director/manager/registry, touch damage applied to the hero, and the CVar toggled back off.

Performance methodology:

- Capture mode: `-T66Entry=Run:Tower -T66GameplayAutoCapture=enemywaveperf -T66GameplayAutoScreenshotDelay=1 -T66GameplayAutoPostCaptureScreenshotDelay=125`.
- Metrics below use schema v4 `board_saturation_samples.jsonl` and the saturated band `LiveRegularEnemies >= 80`.
- `Avg FPS` is computed as `1000 / average(FrameTimeMs)` across saturated samples. This is intentionally stricter than averaging instantaneous FPS samples because it preserves hitch cost.

CVar-OFF accepted five-capture set:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Lightweight Share | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T230844Z_rhGdRU74Lyc2aUCTEsF-7g` | 123.73 | 51.81 | 20.30 | 90 | 90 | 0 | 0.00% | 4 | 8 | 1203.2 us | 0 |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T231108Z_a9A0gUkXO94GTe-VarekHA` | 119.78 | 49.54 | 18.32 | 90 | 90 | 0 | 0.00% | 0 | 5 | 2226.4 us | 0 |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T231331Z_5eZcBEWXltBLSielF6-BCg` | 117.12 | 52.58 | 21.27 | 90 | 90 | 0 | 0.00% | 2 | 6 | 2787.8 us | 0 |
| Run04 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T231557Z_QSfBNkF0uEkaEzage80z-A` | 117.26 | 53.04 | 20.76 | 90 | 90 | 0 | 0.00% | 0 | 8 | 1153.7 us | 0 |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T231830Z_y5BWnUXlFdM6CqWKu68FMg` | 109.52 | 41.74 | 13.22 | 90 | 90 | 0 | 0.00% | 2 | 12 | 21268.8 us | 0 |

Computed CVar-OFF values:

- Avg FPS min/max: `109.52 / 123.73`
- Avg FPS median: `117.26`
- Avg FPS mean: `117.48`
- Avg FPS sample stdev: `5.19`
- Comparator baseline median: `151.37`
- Median delta from baseline median: `-22.54%`
- Acceptance result: fail. The CVar-off path did not stay within the 5% baseline gate.
- PerformanceSystem overhead max median: `2226.4 us`, above the 2 ms median target.
- PerformanceSystem overhead largest individual spike: `21268.8 us`.
- CVar-off non-zero exits: `0`.

CVar-ON partial parity set:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Lightweight Share | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T232216Z_JNIHdEZwzNnyt0ms8m_wbQ` | 108.71 | 48.47 | 17.02 | 90 | 52 | 38 | 42.21% | 2 | 7 | 3219.1 us | 0 |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260524T232446Z_3nTHfU_wiHgZZdm_usr1vg` | 125.20 | 53.67 | 18.92 | 90 | 54 | 36 | 40.00% | 2 | 10 | 1812.8 us | 0 |

The CVar-ON set halted after attempts 3 and 4 both timed out and were killed by the harness (`ExitCode=-999`). Per the B.6 instructions, no further replacement captures were run after two non-zero exits.

Discarded CVar-ON attempts:

| Attempt | Exit Code | Log | Observed Cause |
| --- | ---: | --- | --- |
| Attempt03 | -999 | `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\captures_on\b6_on_attempt03.log` | Hero died to `AT66MobBase` touch damage at game time `22.66s`, before the delayed screenshot/exit timer fired. |
| Attempt04 | -999 | `C:\UE\T66\Saved\Codex\Performance\LightweightActorB6\captures_on\b6_on_attempt04.log` | Hero died to `AT66MobBase` touch damage at game time `30.76s`, before the delayed screenshot/exit timer fired. |

Representative discarded-run log excerpt:

```text
LogT66DamageReceived: [CombatDamage] AppliedHP=20 RequestedHP=20 IncomingHP=20 SourceID=T66MobBase Delivery=EnemyTouch SourceActor=T66MobBase_2147479147 SourceClass=T66MobBase DamageCauser=T66MobBase_2147479147 CauserClass=T66MobBase HeroHP=20.0->0.0 MaxHP=100.0 HeroLoc=V(X=136.92, Y=-1326.04, Z=-21.85) SourceLoc=V(X=90.30, Y=-1407.08, Z=-36.00) CauserLoc=V(X=90.30, Y=-1407.08, Z=-36.00) SourceDist2D=93.5 SourceDist3D=94.6 CauserLOS=Blocked LOSBlocker=T66MobBase_2147479106/T66MobBase Stage=1 WorldTime=22.66
LogT66MobManager: Display: MobTouchDamage mob=T66MobBase_2147479147 MobID=Slime damageHP=20 hero=BP_HeroBase_C_2147479509 cooldown=0.50
```

CVar-ON partial observations:

- Accepted-run avg FPS range: `108.71 / 125.20`
- Accepted-run mean: `116.96`
- Peak lightweight population: `38`
- Saturated-band lightweight share: `40.00%` to `42.21%`
- CVar-ON parity result: inconclusive. The five-capture set did not complete.
- Crash watch result: fail/halt by instruction. There were `2` non-zero CVar-on exits across the B.6 capture set.

B.6 conclusion:

- Functional director routing passed in smoke.
- CVar-off regression check failed against the `151.37` baseline under the schema v4 saturated-band metric.
- CVar-on production parity cannot be accepted yet because the lightweight path can kill the stationary `enemywaveperf` hero before the capture exits.
- The immediate follow-up should not be B.7. First add a non-shipping `enemywaveperf` safety mode or park the perf hero outside touch range, then rerun the B.6 CVar-off and CVar-on five-capture sets. The pending gameplay issue is tracked in `Source/T66/Gameplay/pending_issues_Gameplay.md`.

## Pass B.6.1 Recovery

Completed 2026-05-24.

This section supersedes the B.6 conclusion above. B.6.1 did not add hero safety mode, did not park the `enemywaveperf` hero, and did not change `AT66EnemyBase` touch damage semantics.

Implemented recovery scope:

- Added non-allocating `UT66ActorRegistrySubsystem::ForEachDamageableTarget` and switched combat target scans away from repeated `GetAllDamageableTargets()` heap-return usage.
- Folded `UT66LagTrackerSubsystem` board saturation split counts through `AT66EnemyDirector` when available, avoiding separate registry counting for rich/lightweight live counts.
- Cached `T66.Mob.UseLightweight` once per director spawn decision batch instead of reading the CVar inside every lightweight-route predicate call.
- Changed `AT66MobBase` touch damage to entry-edge semantics: damage fires when a mob transitions from not in damage range to in damage range, remains disarmed while the hero stays in range, and rearms only after leaving range. This matches `AT66EnemyBase::OnCapsuleBeginOverlap`.
- Demoted routine lightweight production logs (`ConfigureAsMob`, routed spawn, hit damage, touch damage, death notification) from `Display` to `Verbose`. Console diagnostic command logs remain visible.

Diagnostic findings:

- Scoped timing on a CVar-off diagnostic capture did not find the B.6 off-path suspects to be large enough to explain the `117.26` median by themselves.
- `CaptureBoardSaturationFrameSample` measured around `11.8 us/frame` average, with sub-millisecond worst samples.
- `SampleBoardSaturation` measured around `4.6-5.1 us` on the once-per-second path.
- `GetAliveEnemyCountForSpawnBudget` measured around `0.03 us` average, `0.1 us` worst, across roughly 440 calls.
- `GetAllDamageableTargets()` was not active in the diagnostic capture, but the allocation-return pattern was still removed because it is unsafe for future combat scans.
- The large CVar-on regression after the touch fix was isolated to the lightweight production path running routine `Display` logs under `-forcelogflush`: before log demotion, CVar-on median was `127.21`; after log demotion, CVar-on median was `176.66`.

Touch damage semantic audit:

| Path | Fire trigger | Repeat behavior while stationary in range | Cooldown owner | Notes |
| --- | --- | --- | --- | --- |
| `AT66EnemyBase::OnCapsuleBeginOverlap` | `UCapsuleComponent::OnComponentBeginOverlap` with hero capsule and distance guard | Does not repeat until a new overlap-enter event happens | `AT66EnemyBase::LastTouchDamageTime` | Rejects vehicle, safe-zone, non-hero capsule, and far overlap events before damage. |
| Pre-B.6.1 `UT66MobManagerSubsystem::ApplyMobTouchDamageIfNeeded` | Per-frame 2D distance check | Repeated every cooldown while the hero stayed in range | `AT66MobBase::TouchDamageCooldownSeconds` | This made a surrounded stationary hero take much more damage than the rich path. |
| B.6.1 `UT66MobManagerSubsystem::ApplyMobTouchDamageIfNeeded` | 2D distance rising edge, `bIsTouchingHero=false -> true` | Does not repeat until the mob leaves range and re-enters | `AT66MobBase::TouchDamageCooldownSeconds` plus `bIsTouchingHero` | Preserves the current rich-enemy design without adding perf-capture hero safety. |

Build, staging, and shortcut verification:

- Development standalone build succeeded after the code changes.
- Staged standalone refreshed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild -SkipCook`.
- Root and taskbar shortcuts were refreshed to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

CVar-OFF final five-capture set:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T012659Z_T_ztNU8SVo9GRf-ksN8F0Q` | 161.42 | 88.84 | 31.23 | 90 | 90 | 0 | 0 | 6 | 841.4 us | 0 |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T012937Z_g0koyUfV_CGFU12ivuoCJA` | 155.70 | 81.91 | 29.74 | 90 | 90 | 0 | 2 | 6 | 856.5 us | 0 |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T013211Z_xS0-cUaLEWO8Q7WCOpSHxA` | 157.79 | 86.92 | 33.34 | 90 | 90 | 0 | 2 | 6 | 1891.3 us | 0 |
| Run04 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T013444Z_mnFRREjKh3QJIm2_IvaZ3w` | 148.77 | 83.99 | 31.75 | 90 | 90 | 0 | 2 | 7 | 858.7 us | 0 |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T013717Z_SmxEOkQ3fBuagsWsZL86Pg` | 140.88 | 74.49 | 27.42 | 90 | 90 | 0 | 2 | 7 | 1073.6 us | 0 |

Computed CVar-OFF values:

- Avg FPS min/max: `140.88 / 161.42`
- Avg FPS median: `155.70`
- Avg FPS mean: `152.91`
- Avg FPS sample stdev: `8.15`
- Comparator baseline median: `151.37`
- Median delta from baseline median: `+2.86%`
- Acceptance result: pass, within the 5% regression gate.
- Non-zero exits: `0`

CVar-ON final five-capture set after lightweight production log demotion:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Lightweight Share | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Run01 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T011130Z_0YadDEn3fZvCAlmDEFqMMQ` | 168.37 | 79.40 | 29.73 | 90 | 50 | 40 | 44.44% | 0 | 5 | 1045.3 us | 0 |
| Run02 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T011407Z__BA26URdB9Q-KiOCDuaiOA` | 178.86 | 80.19 | 29.04 | 90 | 43 | 47 | 52.22% | 2 | 8 | 1582.3 us | 0 |
| Run03 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T011641Z_-pr5JEsQiegSXOiMV4BHQA` | 176.66 | 81.49 | 29.44 | 90 | 53 | 37 | 41.11% | 0 | 8 | 789.5 us | 0 |
| Run04 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T011915Z_PaJHak3uNdvMAZGjsWGKnw` | 175.50 | 77.79 | 29.30 | 90 | 43 | 47 | 52.22% | 0 | 7 | 851.4 us | 0 |
| Run05 | `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\20260525T012150Z_Zl6nNkrNIN4nLdiTB4FubQ` | 182.63 | 81.22 | 30.85 | 90 | 52 | 38 | 42.22% | 0 | 6 | 1019.8 us | 0 |

Computed CVar-ON values:

- Avg FPS min/max: `168.37 / 182.63`
- Avg FPS median: `176.66`
- Avg FPS mean: `176.40`
- Avg FPS sample stdev: `5.25`
- CVar-OFF median comparator: `155.70`
- Median delta from CVar-OFF: `+13.46%`
- Acceptance result: pass. The CVar-on median is not more than 5% below the CVar-off median.
- Non-zero exits: `0`

Recovery conclusion:

- B.6.1 recovers the CVar-off regression against the working baseline.
- B.6.1 fixes the touch damage parity bug without changing autocapture behavior.
- B.6.1 recovers CVar-on parity after removing routine forced-flush logging from the lightweight production path.
- The old pending gameplay issue `Lightweight Mob Perf Capture Can Kill The Stationary Auto-Capture Hero` is resolved in `Source/T66/Gameplay/pending_issues_Gameplay.md`.
- B.7 can resume from a clean B.6 routing baseline.

## Pass B.7 Verification

Implemented B.7 scope:

- Added a lightweight-only pool to `UT66MobManagerSubsystem` with a 128 inactive-mob cap. The pool is intentionally separate from `UT66EnemyPoolSubsystem`, which remains typed around the rich `AT66EnemyBase` path.
- Routed lightweight Melee director spawns through `UT66MobManagerSubsystem::AcquireMob` and lightweight deaths through `ReleaseMob`.
- Preserved deferred actor spawning for fresh lightweight pool misses so `ConfigureAsMob` still runs before `BeginPlay`/registration, matching the B.6 production path timing.
- Implemented `AT66MobBase::ResetForReuse` for HP, status, velocity, touch state, lock indicator, lifecycle state, collision, and visibility reset.
- Added canonical combined live-count access through `UT66ActorRegistrySubsystem::GetCombinedLiveEnemyCount`.
- Widened the HUD/minimap map rebuild to include `AT66MobBase` markers while preserving the Phase A minimap 48-marker cap and dirty-flag refresh pattern.
- Added PerformanceSystem schema v5 pool counters for reuse acquires, releases, current inactive count, and peak inactive count.

Implementation findings:

- A first minimap widening attempt collected and sorted every enemy marker before applying the minimap cap. That regressed the CVar-off path. The final implementation restores the old capped behavior for minimap refreshes and interleaves rich/lightweight markers until the 48-marker cap is reached.
- Fresh pool misses initially used immediate `SpawnActor`, which changed lightweight registration timing relative to B.6. The final implementation uses deferred spawning for fresh actors and immediate reset for pooled actors.
- A later CVar-on set exposed touch-radius jitter that could re-arm lightweight touch damage while the hero and mob were still effectively in contact. The manager now uses a small rearm hysteresis so touch damage still matches rich overlap-enter semantics in practice.

CVar-OFF five-capture set, `T66.Mob.UseLightweight=0`:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260525T025742Z_OsyrOUhdBQN39lmUSWwwFg` | 159.53 | 81.55 | 27.84 | 90 | 90 | 0 | 0 | 7 | 954.4 us | 0 |
| 2 | `20260525T030015Z_tRlfJE1060_JnRKkA67zAA` | 151.23 | 80.08 | 32.68 | 90 | 90 | 0 | 0 | 6 | 1046.3 us | 0 |
| 3 | `20260525T030244Z_fW7icUWjywjG4Mq-G23KrQ` | 150.55 | 81.20 | 30.45 | 90 | 90 | 0 | 2 | 5 | 994.6 us | 0 |
| 4 | `20260525T030514Z_OKLj1E01fuFS8rO41SNacw` | 151.58 | 81.54 | 27.03 | 90 | 90 | 0 | 2 | 6 | 847.8 us | 0 |
| 5 | `20260525T030743Z_2VLHIkV_00-991m_Ok0gfw` | 124.09 | 38.01 | 11.47 | 90 | 90 | 0 | 2 | 16 | 1146.1 us | 0 |

CVar-OFF computed values:

- Avg FPS min/max: `124.09 / 159.53`
- Avg FPS median: `151.23`
- Avg FPS mean: `147.40`
- Avg FPS stdev: `12.10`
- Comparator: B.6.1 CVar-off median `155.70`
- Median delta from B.6.1 CVar-off: `-2.87%`
- Acceptance result: pass, within the 5% regression gate.
- Non-zero exits: `0`

CVar-ON confirm five-capture set, `T66.Mob.UseLightweight=1`:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Lightweight Share | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260525T040711Z_BGcFGkR9vK9UeLG-S-fFTQ` | 181.81 | 79.95 | 30.77 | 90 | 46 | 44 | 48.89% | 2 | 6 | 871.6 us | 0 |
| 2 | `20260525T040939Z_vNotV03kcerElRS099sWHQ` | 170.63 | 75.42 | 28.64 | 90 | 55 | 35 | 38.89% | 4 | 5 | 865.5 us | 0 |
| 3 | `20260525T041205Z_W3-9kU4elU9bvhmDS7qprw` | 178.58 | 77.83 | 29.86 | 90 | 52 | 38 | 42.22% | 2 | 7 | 8212.0 us | 0 |
| 4 | `20260525T041431Z_jQRSc0AlIpznJIaZkdrpTA` | 178.80 | 78.65 | 28.86 | 90 | 45 | 45 | 50.00% | 0 | 5 | 899.4 us | 0 |
| 5 | `20260525T041657Z_4k-T7k-eRDO4C-6tANnggQ` | 173.64 | 79.15 | 29.78 | 90 | 52 | 38 | 42.22% | 2 | 6 | 1084.9 us | 0 |

CVar-ON computed values:

- Avg FPS min/max: `170.63 / 181.81`
- Avg FPS median: `178.58`
- Avg FPS mean: `176.69`
- Avg FPS stdev: `4.01`
- Comparator: B.6.1 CVar-on median `176.66`
- Median delta from B.6.1 CVar-on: `+1.09%`
- Median delta from B.7 CVar-off: `+18.09%`
- Acceptance result: pass. The CVar-on median is not more than 5% below the B.6.1 CVar-on median and remains above the B.7 CVar-off median.
- Non-zero exits: `0`

Pool/HUD/minimap smoke:

- Screenshot: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB7\pool_and_hud_smoke\b7poolhud_final_afterfix.png`
- Log: `C:\UE\T66\Saved\StandaloneLogs\T66_PhaseB7_PoolAndHudSmoke_AfterFix.log`
- The smoke used `-T66GameplayAutoCapture=lightweightactorb7`, enabled `T66.Mob.UseLightweight=1`, force-killed lightweight mobs in three bursts to exercise pool release/reuse, then toggled the CVar back to `0`.
- Key excerpts:
  - `t10s combined=64 registryLightweight=26 directorRich=35 directorLightweight=26 activeMobs=26 inactiveMobs=0 reuse=0 releases=0 inactivePeak=0`
  - `Force-killed 10 lightweight mob(s) to exercise pool release/reuse.`
  - `t25s combined=93 registryLightweight=37 directorRich=53 directorLightweight=37 activeMobs=37 inactiveMobs=0 reuse=10 releases=10 inactivePeak=10`
  - `t40s combined=93 registryLightweight=31 directorRich=59 directorLightweight=31 activeMobs=31 inactiveMobs=6 reuse=14 releases=20 inactivePeak=10`
  - `T66.Mob.UseLightweight set to 0; subsequent spawns use rich path.`
  - `t55s combined=93 registryLightweight=28 directorRich=62 directorLightweight=28 activeMobs=28 inactiveMobs=9 reuse=21 releases=30 inactivePeak=16`
- Standard `enemywaveperf` captures do not naturally kill enough lightweight mobs to exercise pool reuse, so the pool counters in those captures remain `0`. The dedicated B.7 smoke validated reuse at saturated combined counts.

Hot-path log audit:

- New B.7 pool acquire/release/reset and lightweight spawn routing logs are `VeryVerbose`.
- B.7 smoke automation milestones use normal logging because they are test lifecycle events, not production hot paths.
- Routine `LogT66TrapProjectile` projectile-fire output still appears in some gameplay runs. That is unrelated to B.7 and is now tracked in `Source/T66/Gameplay/pending_issues_Gameplay.md` as a separate hot-path telemetry cleanup issue.

Build, staging, and shortcut verification:

- `git diff --check` passed for the modified tracked files and new schema files.
- Development standalone build completed successfully.
- Staged standalone was refreshed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild -SkipCook`.
- Root and taskbar `T66 Standalone.lnk` shortcuts were refreshed to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

B.7 conclusion:

- CVar-off path remains within the accepted regression gate.
- CVar-on path remains performance-positive against both B.6.1 CVar-on and B.7 CVar-off medians.
- HUD live count and minimap marker widening are functional with mixed rich/lightweight populations.
- Lightweight mob pool release/reuse is verified in staged gameplay at saturated combined counts.
- B.8 can proceed to Rush-family migration behind the existing lightweight feature gate.

## Pass B.8 Verification

Implemented B.8 scope:

- Added Rush state to `AT66MobBase`: active rush flag, planar snapshot rush direction, active rush timer, and cooldown timer.
- Mirrored rich `AT66RushEnemy` timing and speed values in the lightweight path: `MaxWalkSpeed=330`, initial cooldown `0.8s`, interval `2.75s`, duration `0.45s`, speed multiplier `3.5`, and trigger distance `1700uu`.
- Extended `AT66MobBase::ConfigureAsMob` and `ResetForReuse` for Rush-family setup/reset.
- Extended `UT66MobManagerSubsystem::Tick` with manager-owned Rush behavior using a frozen rush direction during the active burst.
- Widened `AT66EnemyDirector` lightweight routing from Melee-only to Melee-or-Rush under the existing `T66.Mob.UseLightweight` gate. Mini-boss and special-spawn guards remain unchanged.
- Added PerformanceSystem schema v6 split counters for `LiveLightweightMeleeMobs` and `LiveLightweightRushMobs`.
- Added staged Unreal-owned B.8 smoke automation at `-T66GameplayAutoCapture=lightweightactorb8`.

Rush behavior parity notes:

| Behavior | Rich `AT66RushEnemy` | Lightweight `AT66MobBase` Rush |
| --- | --- | --- |
| Normal movement | `AT66EnemyBase` chase input through `CharacterMovementComponent` | Manager direct-vector chase at configured `ChaseSpeed` |
| Rush trigger | Cooldown elapsed and target within `1700uu` | Same |
| Direction | Snapshot planar direction to target at rush start | Same |
| Rush movement | Temporary `MaxWalkSpeed = BaseSpeed * 3.5` | Direct movement at `ChaseSpeed * 3.5` |
| Duration/cooldown | `0.45s` active, then `2.75s` cooldown | Same |
| Status effects | Base enemy tick skips family behavior under hard status, effectively pausing rush behavior | Manager status gate runs before Rush logic, so rush timers/movement pause under hard status |
| Contact behavior | CMC blocking prevents tunneling through the stationary hero | B.8 added a Rush-only contact clamp so the direct-vector burst stops at the hero contact threshold instead of crossing through and re-arming touch damage |

Smoke verification:

- Build used staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Screenshot sequence: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB8\rush_smoke\rush_0000.png` through `rush_0004.png`.
- Log: `C:\UE\T66\Saved\StandaloneLogs\T66_PhaseB8_RushSmoke_AfterClamp.log`.
- Result: pass. The smoke exited with code `0`, spawned a configured `RatPack` Rush mob, showed normal chase, showed active Rush velocity at `1155uu/s`, stopped at the hero contact threshold, applied a `3.0s` stun, and confirmed naturally spawned Rush-lightweight counts from the director.
- Key excerpts:
  - `T66.Mob.UseLightweight set to 1 before Rush smoke.`
  - `t=0.95s MobID=RatPack loc=V(X=464.99, Y=-1300.00) velocity=V(X=-1155.00) rushing=1 rushRemaining=0.45 rushCooldown=0.00`
  - `t=1.85s MobID=RatPack loc=V(X=98.60, Y=-1300.00) velocity=V(0) rushing=0 rushRemaining=0.00 rushCooldown=2.27`
  - `Applied 3.0s stun to Rush mob; rush timers should pause while status blocks chase.`
  - `directorCounts rich=26 lightweight=22 lightweightMelee=18 lightweightRush=4.`

CVar-OFF five-capture set, `T66.Mob.UseLightweight=0`:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Lightweight Melee | Peak Lightweight Rush | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260525T062211Z_hNtzTUymGe3U_P-gvClnfQ` | 151.85 | 75.68 | 25.36 | 90 | 90 | 0 | 0 | 0 | 2 | 6 | 1064.1 us | 0 |
| 2 | `20260525T062432Z_WdXE8k-fyWrU16mzCGHUGQ` | 143.87 | 58.69 | 14.57 | 90 | 90 | 0 | 0 | 0 | 2 | 6 | 750660.9 us | 0 |
| 3 | `20260525T062652Z_zJnBEE0aYkxrq1-M-dC8gQ` | 144.45 | 86.85 | 36.69 | 90 | 90 | 0 | 0 | 0 | 2 | 7 | 1251.8 us | 0 |
| 4 | `20260525T062911Z_8ZMQLk09NmnPXQu0igMpJA` | 154.85 | 73.18 | 19.21 | 90 | 90 | 0 | 0 | 0 | 0 | 6 | 537765.3 us | 0 |
| 5 | `20260525T063130Z_YM70JU1swSJEcMWHFqZgHA` | 149.78 | 84.66 | 38.02 | 90 | 90 | 0 | 0 | 0 | 2 | 6 | 947.6 us | 0 |

CVar-OFF computed values:

- Avg FPS min/max: `143.87 / 154.85`
- Avg FPS median: `149.78`
- Avg FPS mean: `148.96`
- Avg FPS stdev: `4.24`
- Comparator: B.7 CVar-off median `151.23`
- Median delta from B.7 CVar-off: `-0.96%`
- Acceptance result: pass, within the 5% regression gate.
- Non-zero exits: `0`

CVar-ON first five-capture set, `T66.Mob.UseLightweight=1`:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Lightweight Melee | Peak Lightweight Rush | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260525T063555Z_4VJ6OkLbXFCsvPajetdRUA` | 170.14 | 70.66 | 27.86 | 90 | 36 | 54 | 41 | 13 | 2 | 10 | 65551.3 us | 0 |
| 2 | `20260525T063817Z_AWqyyEvaLi54eIqZ8ekGxA` | 165.92 | 76.43 | 35.68 | 90 | 47 | 45 | 39 | 6 | 1 | 5 | 2005.5 us | 0 |
| 3 | `20260525T064037Z_sqGE8UR_nsLVSbesYZKygw` | 163.48 | 69.33 | 25.31 | 90 | 31 | 59 | 42 | 17 | 0 | 5 | 225244.1 us | 0 |
| 4 | `20260525T064257Z_EJPFRUgTwHSfRwGZAPWyZg` | 162.53 | 82.33 | 38.61 | 90 | 32 | 58 | 43 | 15 | 0 | 6 | 1075.1 us | 0 |
| 5 | `20260525T064519Z_lLXj2UQDjVIdkyaVO0Ky8g` | 159.53 | 62.59 | 20.61 | 90 | 41 | 49 | 38 | 11 | 0 | 7 | 1299.9 us | 0 |

First CVar-ON computed values:

- Avg FPS min/max: `159.53 / 170.14`
- Avg FPS median: `163.48`
- Avg FPS mean: `164.32`
- Avg FPS stdev: `3.56`
- Comparator: B.7 CVar-on median `178.58`
- Required floor: `169.65` (5% below B.7 CVar-on)
- Acceptance result: fail. The CVar-on median is `8.45%` below B.7 CVar-on.
- Non-zero exits: `0`

CVar-ON confirmation set, `T66.Mob.UseLightweight=1`:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Lightweight Melee | Peak Lightweight Rush | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max | Exit Code |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260525T065149Z_Mkg9f2ixp2aaA9fQgAsGkA` | 171.42 | 62.45 | 17.11 | 90 | 45 | 45 | 34 | 11 | 0 | 9 | 558361.0 us | 0 |
| 2 | `20260525T065412Z_eCio1WRogxF45zQkhlNBOQ` | 168.75 | 67.03 | 24.31 | 90 | 42 | 48 | 33 | 15 | 2 | 6 | 996.0 us | 0 |
| 3 | `20260525T065634Z_J476uXsmrg1qMogt4w7wog` | 152.93 | 59.32 | 35.89 | 90 | 37 | 53 | 32 | 21 | 0 | 4 | 4102.3 us | 0 |
| 4 | `20260525T065855Z_VRo1Mzqnv3RznObYW2z1Dw` | 167.21 | 66.53 | 25.33 | 90 | 34 | 56 | 43 | 13 | 0 | 6 | 65385.2 us | 0 |
| 5 | `20260525T070116Z_078cxMzbp97uRAY9bF7pGQ` | 178.79 | 81.90 | 38.64 | 90 | 31 | 59 | 46 | 13 | 2 | 6 | 1777.7 us | 0 |

Confirm CVar-ON computed values:

- Avg FPS min/max: `152.93 / 178.79`
- Avg FPS median: `168.75`
- Avg FPS mean: `167.82`
- Avg FPS stdev: `8.44`
- Comparator: B.7 CVar-on median `178.58`
- Required floor: `169.65`
- Acceptance result: fail. The confirm median is `5.50%` below B.7 CVar-on.
- Non-zero exits: `0`

B.8 conclusion:

- Functional Rush migration is in place and smoke-tested.
- CVar-off default path remains within the accepted regression gate.
- CVar-on Rush path is stable and still beats the current CVar-off median by `+12.67%` in the confirm set, but it fails the explicit B.8 acceptance comparator against B.7 CVar-on.
- The performance shortfall reproduced across two clean CVar-on five-capture sets, so this is not a crash or timeout artifact.
- The next pass should be a focused B.8.1 recovery/triage before Flying migration: compare Melee-only lightweight routing against Melee+Rush under the same binary, profile `UT66MobManagerSubsystem::Tick` cost by family, and decide whether Rush needs a dedicated lightweight movement optimization or should remain rich until later ISM/VAT-manager work.

Hot-path log audit:

- New Rush tick, trigger, duration, and cooldown paths do not emit `Log` or `Display` telemetry.
- B.8 smoke automation uses `Display` logs only for test lifecycle milestones.
- Existing trap projectile normal-level telemetry remains a separate pending issue in `Source/T66/Gameplay/pending_issues_Gameplay.md`.

Build, staging, and shortcut verification:

- Development standalone build completed successfully after the B.8 implementation.
- Staged standalone was refreshed after the Rush contact-clamp change.
- Root and taskbar `T66 Standalone.lnk` shortcuts target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

## Pass B.8.1 Diagnostic

Completed 2026-05-25.

Implemented diagnostic scope:

- Added non-shipping `T66.Mob.ManagerTickProfileEnabled`, default `0`. When enabled, `UT66MobManagerSubsystem` records grouped Melee/Rush/pool timing and emits one `VeryVerbose` `[MobManagerProfile]` line every 60 frames.
- Added non-shipping `T66.Mob.Diagnostics.RouteRushLightweight`, default `1`, so the same binary can compare current Melee+Rush routing against Melee-only routing. Default `1` preserves the production B.8 state.
- Added `enemywaveperf` command-line parsing for `-T66MobManagerTickProfileEnabled=` and `-T66MobRouteRushLightweight=`.

Set A, CVar off, `T66.Mob.UseLightweight=0`, `T66.Mob.ManagerTickProfileEnabled=0`:

| Run | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Lightweight Melee | Peak Lightweight Rush | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 86.93 | 25.45 | 15.54 | 90 | 90 | 0 | 0 | 0 | 0 | 12 | 1270.7 us |
| 2 | 85.73 | 49.23 | 25.75 | 90 | 90 | 0 | 0 | 0 | 0 | 5 | 2198.6 us |
| 3 | 87.41 | 49.09 | 25.80 | 90 | 90 | 0 | 0 | 0 | 0 | 5 | 1099.2 us |
| 4 | 90.82 | 51.28 | 27.00 | 90 | 90 | 0 | 0 | 0 | 0 | 7 | 1340.1 us |
| 5 | 92.10 | 50.25 | 25.89 | 90 | 90 | 0 | 0 | 0 | 2 | 7 | 1849.7 us |
| 6 | 83.10 | 25.96 | 10.65 | 90 | 90 | 0 | 0 | 0 | 2 | 31 | 1232.7 us |
| 7 | 89.95 | 48.66 | 24.80 | 90 | 90 | 0 | 0 | 0 | 2 | 8 | 1610.8 us |
| 8 | 122.74 | 59.18 | 34.28 | 90 | 90 | 0 | 0 | 0 | 0 | 8 | 1124.5 us |
| 9 | 130.95 | 63.29 | 35.45 | 90 | 90 | 0 | 0 | 0 | 2 | 6 | 907.3 us |
| 10 | 129.13 | 59.92 | 33.97 | 90 | 90 | 0 | 0 | 0 | 0 | 6 | 1001.7 us |

Set A computed values:

- Avg FPS min/max: `83.10 / 130.95`
- Avg FPS median: `90.38`
- Avg FPS mean: `99.89`
- Avg FPS stdev: `18.41`
- Non-zero exits: `0`
- Note: this off-path set showed an environmental warm-up/throttle pattern and is not used to classify the Rush-specific question. It is kept here because it was part of the requested three-set diagnostic.

Set B, CVar on with Melee+Rush routing, `T66.Mob.UseLightweight=1`, `T66.Mob.Diagnostics.RouteRushLightweight=1`, `T66.Mob.ManagerTickProfileEnabled=0`:

| Run | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Lightweight Melee | Peak Lightweight Rush | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 182.58 | 84.01 | 38.74 | 90 | 33 | 57 | 49 | 8 | 2 | 7 | 964.8 us |
| 2 | 175.48 | 72.75 | 32.88 | 90 | 37 | 53 | 38 | 15 | 2 | 6 | 909.6 us |
| 3 | 180.18 | 84.55 | 37.71 | 90 | 38 | 52 | 35 | 17 | 0 | 6 | 14076.5 us |
| 4 | 170.70 | 82.12 | 39.44 | 90 | 36 | 54 | 38 | 16 | 4 | 4 | 928.0 us |
| 5 | 181.47 | 71.72 | 35.62 | 90 | 35 | 55 | 44 | 11 | 2 | 4 | 895.9 us |
| 6 | 179.84 | 82.82 | 37.58 | 90 | 38 | 52 | 40 | 12 | 0 | 7 | 1070.5 us |
| 7 | 177.81 | 86.92 | 40.51 | 90 | 48 | 42 | 29 | 13 | 2 | 5 | 893.9 us |
| 8 | 172.74 | 78.99 | 38.01 | 90 | 36 | 54 | 45 | 9 | 0 | 4 | 838.4 us |
| 9 | 174.01 | 76.17 | 35.41 | 90 | 40 | 50 | 39 | 11 | 2 | 4 | 967.2 us |
| 10 | 182.85 | 90.97 | 42.15 | 90 | 42 | 48 | 34 | 14 | 0 | 4 | 927.6 us |

Set B computed values:

- Avg FPS min/max: `170.70 / 182.85`
- Avg FPS median: `178.82`
- Avg FPS mean: `177.77`
- Avg FPS stdev: `4.09`
- Non-zero exits: `0`

Set C, CVar on with Melee-only routing, `T66.Mob.UseLightweight=1`, `T66.Mob.Diagnostics.RouteRushLightweight=0`, `T66.Mob.ManagerTickProfileEnabled=0`:

| Run | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Lightweight Melee | Peak Lightweight Rush | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 175.69 | 85.18 | 35.61 | 90 | 58 | 32 | 32 | 0 | 4 | 6 | 879.3 us |
| 2 | 179.04 | 85.95 | 38.45 | 90 | 48 | 42 | 42 | 0 | 2 | 6 | 4748.5 us |
| 3 | 180.60 | 85.27 | 36.55 | 90 | 52 | 38 | 38 | 0 | 0 | 4 | 854.3 us |
| 4 | 177.08 | 95.36 | 42.81 | 90 | 55 | 35 | 35 | 0 | 2 | 4 | 1093.2 us |
| 5 | 180.83 | 89.34 | 38.86 | 90 | 55 | 35 | 35 | 0 | 2 | 4 | 883.5 us |
| 6 | 180.71 | 86.57 | 39.46 | 90 | 50 | 40 | 40 | 0 | 2 | 4 | 1015.8 us |
| 7 | 179.40 | 93.90 | 39.65 | 90 | 55 | 35 | 35 | 0 | 2 | 5 | 1019.3 us |
| 8 | 184.46 | 88.74 | 42.45 | 90 | 44 | 46 | 46 | 0 | 0 | 4 | 973.3 us |
| 9 | 175.74 | 88.87 | 39.44 | 90 | 48 | 42 | 42 | 0 | 0 | 4 | 751.9 us |
| 10 | 175.34 | 88.11 | 40.67 | 90 | 54 | 36 | 36 | 0 | 0 | 5 | 1104.4 us |

Set C computed values:

- Avg FPS min/max: `175.34 / 184.46`
- Avg FPS median: `179.22`
- Avg FPS mean: `178.89`
- Avg FPS stdev: `2.78`
- Non-zero exits: `0`

Family-cost attribution set, CVar on with Melee+Rush routing and `T66.Mob.ManagerTickProfileEnabled=1`:

| Run | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Lightweight Melee | Peak Lightweight Rush | Projectile Peak |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 185.03 | 91.29 | 39.16 | 90 | 52 | 38 | 29 | 9 | 0 |
| 2 | 186.77 | 86.80 | 39.09 | 90 | 36 | 54 | 39 | 15 | 0 |
| 3 | 179.10 | 89.25 | 40.49 | 90 | 41 | 49 | 34 | 15 | 2 |
| 4 | 190.08 | 96.71 | 42.57 | 90 | 36 | 54 | 40 | 14 | 0 |
| 5 | 185.28 | 93.49 | 40.07 | 90 | 42 | 48 | 36 | 12 | 2 |

Family-cost timing summary from `1955` `[MobManagerProfile]` windows:

- Median Melee per-mob cost: `0.854713 us`
- Median Rush per-mob cost: `1.803750 us`
- Rush:Melee per-mob cost ratio: `2.11x`
- Median pool acquire per-call cost: `291.4 us`
- Median pool release per-call cost: `0.0 us` in this set because the standard `enemywaveperf` run still does not naturally kill enough lightweight mobs to exercise releases.

Statistical comparison:

- Set B Melee+Rush median: `178.82`
- Set C Melee-only median: `179.22`
- Median delta B-C: `-0.40 FPS` (`-0.22%`)
- Larger set stdev: `4.09`
- 2x larger stdev threshold: `8.18 FPS`
- Result: `0.40 < 8.18`, so Set B and Set C overlap heavily. The diagnostic cannot distinguish Melee+Rush from Melee-only performance in this environment.

B.8.1 outcome:

- Outcome Z. The B.8 missed gate was measurement variance, not a statistically distinguishable Rush regression.
- Rush per-mob work is higher than Melee in the diagnostic profiler, but the difference does not produce a measurable production-FPS loss at the current 90-cap scenario.
- Recommendation: accept B.8 as production-valid and proceed to B.9 Flying. Keep the grouped timing hook disabled by default for future diagnostics.

Production-state verification:

- `T66.Mob.UseLightweight` still defaults to `0`.
- `T66.Mob.Diagnostics.RouteRushLightweight` defaults to `1`, so Melee+Rush routing is intact when lightweight routing is enabled.
- `T66.Mob.ManagerTickProfileEnabled` defaults to `0`, so production captures do not pay cycle-count sampling cost unless explicitly enabled.

## Pass B.9 Verification

Status as of 2026-05-25: implementation and smoke validation complete; final FPS acceptance failed the CVar-on parity gate after a clean rerun. B.9 should move to a focused B.9.1 diagnostic before Ranged migration.

Implemented scope:

- Added `AT66MobBase` Flying state: `HoverAnchorZ`, `HoverBobTime`, `HoverBobFrequency`, `HoverBobAmplitude`, and `HoverHeight`.
- `AT66MobBase::ConfigureAsMob` now configures Flying mobs with the rich Flying defaults: `430 uu/s` chase speed, `180 uu` hover height, `35 uu` bob amplitude, `2.2` bob frequency, and randomized per-mob bob phase.
- `UT66MobManagerSubsystem` now handles Flying movement: planar chase plus `HoverAnchorZ + sin(HoverBobTime * HoverBobFrequency) * HoverBobAmplitude`, interpolated with `FInterpTo(..., 6.0f)` to match `AT66FlyingEnemy`.
- Status gating matches the rich actor path: stun/root/freeze skip family behavior, so Flying mobs stop planar chase and hover bob while hard-disabled.
- Director routing now includes Melee, Rush, and Flying when `T66.Mob.UseLightweight=1`; Ranged and all special/mini-boss paths remain rich actors.
- PerformanceSystem schema bumped to v7 with `LiveLightweightFlyingMobs` in board saturation samples and event attributions.

Flying behavior comparison:

| Behavior | `AT66FlyingEnemy` rich actor | `AT66MobBase` Flying |
| --- | --- | --- |
| Base speed | CharacterMovement `MaxWalkSpeed=430` | `ChaseSpeed=430` |
| Movement mode | CMC `MOVE_Flying` | Manager-set transform, `bSweep=false` |
| Horizontal movement | `AddMovementInput` toward hero | Planar chase toward hero |
| Hover anchor | Spawn Z + `HoverHeight` | Spawn Z + `HoverHeight` |
| Hover bob | `sin(Time * Frequency) * Amplitude`, `FInterpTo(..., 6.0f)` | Same formula and interpolation |
| Bob phase | Starts at actor state reset | Randomized per reuse/spawn so swarms do not sync |
| Hard status effects | Base tick skips family behavior | Manager skips Flying behavior |
| Touch damage | Capsule overlap event on hero capsule | Capsule overlap-state rising edge; no distance-only synthesized hit |

Smoke validation:

- Staged standalone log: `C:\UE\T66\Saved\StandaloneLogs\T66_PhaseB9_FlyingSmoke.log`
- Unreal-owned screenshot sequence: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB9\flying_smoke\frame_0000.png` through `frame_0004.png`
- Video assembled by the capture script: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB9\flying_smoke.mp4`
- Key log evidence:
  - `[FlyingSmoke] T66.Mob.UseLightweight set to 1 before Flying smoke.`
  - `[FlyingSmoke] t=0.35s MobID=CaveBat loc=V(X=664.98, Y=-1300.00, Z=136.31) velocity=V(X=-430.00) hoverAnchorZ=180.0 hoverBobTime=2.04 status=Active`
  - `[FlyingSmoke] t=2.05s MobID=CaveBat loc=V(X=97.88, Y=-1300.00, Z=212.14) velocity=V(X=0.00) hoverAnchorZ=180.0 hoverBobTime=3.74 status=Active`
  - `[FlyingSmoke] Applied 3.0s stun to Flying mob; hover and chase should pause while status blocks movement.`
  - `[FlyingSmoke] Applied automation lethal damage to Flying mob; death path should release it.`
  - `[FlyingSmoke] directorCounts rich=12 lightweight=41 lightweightMelee=27 lightweightRush=3 lightweightFlying=11.`
  - `[FlyingSmoke] Completed B.9 Flying smoke automation.`

Touch-damage correction found during B.9:

- The first CVar-on performance probe exposed a remaining lightweight touch-parity mismatch: `AT66MobBase` still synthesized contact from a distance threshold, while `AT66EnemyBase` only fires after the enemy capsule actually begins overlapping the hero capsule.
- `UT66MobManagerSubsystem::ApplyMobTouchDamageIfNeeded` now uses `MobCapsule->IsOverlappingComponent(HeroCapsule)` as the touch-state source. This keeps the B.6.1 rising-edge semantics but removes distance-only false positives.
- A staged CVar-on probe after this correction completed cleanly without the previous death/hang:
  - Session: `20260525T103149Z_jmleFkh7SWTGrryZ4_p_jQ`
  - Peak live: `90`; peak rich: `30`; peak lightweight: `60`; peak lightweight Flying: `10`

Final performance acceptance blocker:

- After the correction and staged rebuild, the final CVar-off acceptance set was invalid because an unrelated active `dota2.exe` process was consuming roughly `16.38 CPU-seconds` over a `5s` sample, about `3.3` full CPU cores.
- The invalid CVar-off set landed at `79.00` median FPS with `0` lightweight mobs active, while earlier same-pass CVar-off captures before the external load were in the expected `140-154` FPS range. That makes the low final set an environment contamination, not evidence about Flying routing.
- Final B.9 FPS acceptance still needs to be rerun after closing the external load:
  - CVar off: 5 captures, gate `>= 143.67` and `<= 158.79` around the B.7 off comparator `151.23`.
  - CVar on: 5 captures, gate `>= 169.88` against the B.8.1 Melee+Rush comparator `178.82`.
  - Apply the standing 10-capture tiebreaker if either 5-capture median lands within `2x stdev` of its gate boundary.

Hot-path log audit:

- No new Log/Display-level telemetry was added to production per-frame/per-mob Flying movement paths.
- B.9 smoke automation uses Display logs only inside the explicit automation mode.
- A new pending gameplay issue tracks mixed rich/lightweight `LogCharacterMovement` stuck spam when rich `AT66RangedEnemy` actors collide with `AT66MobBase` capsules.

Final clean rerun after external load removed:

- External load check: `dota2.exe` was closed and the CPU sample showed no sustained non-T66 load before rerun.
- Build under test: staged standalone at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- All clean rerun captures exited with code `0`.
- The earlier invalid post-stage set remains documented above as environmental contamination and is not used for acceptance.

CVar off, `T66.Mob.UseLightweight=0`, final 10-capture tiebreaker:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Flying | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| off_clean01 | `20260525T230254Z_5oC6d0f4csJubS6uAj2bzw` | 161.96 | 114.23 | 68.98 | 90 | 90 | 0 | 0 | 2 | 5 | 10809.7 us |
| off_clean02 | `20260525T230519Z_to0PK0roYP1ZHK--Z-acTA` | 163.68 | 120.12 | 86.41 | 90 | 90 | 0 | 0 | 0 | 5 | 1207.6 us |
| off_clean03 | `20260525T230744Z_0BNJEkpeFcuRfkukpoCwuA` | 159.85 | 120.62 | 79.58 | 90 | 90 | 0 | 0 | 2 | 5 | 843.7 us |
| off_clean04 | `20260525T231008Z_weWrtU7DYohAUqOj7-xECg` | 146.08 | 113.97 | 92.94 | 90 | 90 | 0 | 0 | 2 | 4 | 873.2 us |
| off_clean05 | `20260525T231232Z_L2ScF04-C49GmmaN4rP4dQ` | 161.26 | 126.42 | 92.75 | 90 | 90 | 0 | 0 | 0 | 7 | 872.3 us |
| off_clean06 | `20260525T231632Z_p2TzK0ZUu4yNIp6cbR9LqQ` | 160.26 | 119.70 | 92.70 | 90 | 90 | 0 | 0 | 2 | 5 | 890.6 us |
| off_clean07 | `20260525T231857Z_Ln4tZEt06fd9z4umBuralw` | 149.48 | 110.24 | 81.92 | 90 | 90 | 0 | 0 | 4 | 4 | 825.0 us |
| off_clean08 | `20260525T232121Z_0pv-ZES_5JLr4QeQYED5Jw` | 147.03 | 106.84 | 74.26 | 90 | 90 | 0 | 0 | 4 | 5 | 505246.8 us |
| off_clean09 | `20260525T232348Z_PKeUZU7r-OkMZ9iws2dnnQ` | 153.39 | 117.18 | 79.91 | 90 | 90 | 0 | 0 | 0 | 6 | 924.0 us |
| off_clean10 | `20260525T232613Z_41QaIELfG_186eqJo8GKcA` | 146.57 | 110.56 | 64.45 | 90 | 90 | 0 | 0 | 2 | 6 | 1137.4 us |

CVar off computed values:

- Avg FPS min/max: `146.08 / 163.68`
- Avg FPS median: `156.62`
- Avg FPS mean: `154.96`
- Avg FPS stdev: `6.79`
- Acceptance gate around B.7 off comparator `151.23`: `143.67 - 158.79`
- Result: pass. The combined tiebreaker median is inside the required band and all lightweight counters stayed at `0`.

CVar on, `T66.Mob.UseLightweight=1`, Melee + Rush + Flying routed to `AT66MobBase`, final 10-capture tiebreaker:

| Run | Session | Avg FPS | 1% Low | 0.1% Low | Peak Live | Peak Rich | Peak Lightweight | Peak Melee | Peak Rush | Peak Flying | Projectile Peak | SingleFrameHitch Count | PerfSystem Overhead Max |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| on_clean01 | `20260525T233018Z_irCnX0W_5ZzEoa-nJG8ORw` | 168.77 | 123.82 | 62.73 | 90 | 31 | 59 | 34 | 13 | 12 | 0 | 5 | 2009.4 us |
| on_clean02 | `20260525T233245Z_ccZNZEn0BNL47Z6bTSkU4Q` | 161.09 | 107.48 | 57.44 | 90 | 32 | 58 | 35 | 11 | 12 | 2 | 6 | 999.3 us |
| on_clean03 | `20260525T233509Z_gFphBkOg7raoHFGD-YnKAQ` | 161.27 | 116.03 | 62.10 | 90 | 30 | 60 | 31 | 16 | 13 | 2 | 6 | 1011.1 us |
| on_clean04 | `20260525T233733Z_tDVNzEDtDHtNVFKGrOgVpA` | 158.31 | 112.06 | 54.42 | 90 | 28 | 62 | 42 | 8 | 12 | 0 | 5 | 987.2 us |
| on_clean05 | `20260525T233957Z_VwfQTkjg2kGrGpKQEUMKGw` | 154.30 | 103.08 | 54.30 | 90 | 21 | 69 | 39 | 16 | 14 | 2 | 6 | 21614.4 us |
| on_clean06 | `20260525T234325Z_lShWu08YJIEwMzW7NhYIeQ` | 164.33 | 117.35 | 59.72 | 90 | 30 | 60 | 37 | 14 | 9 | 2 | 6 | 1129.9 us |
| on_clean07 | `20260525T234550Z_-Be0XEXSYMlH2_CMEjm5aA` | 161.01 | 113.32 | 55.58 | 90 | 24 | 66 | 45 | 12 | 9 | 2 | 4 | 917.5 us |
| on_clean08 | `20260525T234813Z_s7u--EDEHx2dUlybKH_gdw` | 164.58 | 119.08 | 54.58 | 90 | 24 | 66 | 39 | 13 | 14 | 2 | 5 | 912.8 us |
| on_clean09 | `20260525T235039Z_nihzsEuulTnKSaC5lQh8XQ` | 164.59 | 118.79 | 59.54 | 90 | 32 | 58 | 38 | 14 | 6 | 2 | 7 | 1168.9 us |
| on_clean10 | `20260525T235303Z_SxyP2UEQc_VG7sOP_EJtdQ` | 166.56 | 110.22 | 51.33 | 90 | 24 | 66 | 46 | 7 | 13 | 0 | 5 | 1102.6 us |

CVar on computed values:

- Avg FPS min/max: `154.30 / 168.77`
- Avg FPS median: `162.80`
- Avg FPS mean: `162.48`
- Avg FPS stdev: `3.98`
- Acceptance gate against B.8.1 Melee+Rush comparator `178.82`: `>= 169.88`
- Result: fail. The first five-capture set missed within `2x stdev`, so the required second five-capture set was run. The combined ten-capture median still missed the floor by `7.08 FPS`.
- Production comparison against the same clean CVar-off set is still positive: `162.80` CVar-on median vs `156.62` CVar-off median. The failure is specifically against the B.8.1 Melee+Rush comparator, meaning Flying routing reduced the previous lightweight-path gain rather than regressing below rich-only gameplay.

B.9 outcome:

- Functional criteria passed: Flying mobs spawn as `AT66MobBase`, hover/chase, pause under stun, die/release, update HUD/minimap and PerformanceSystem counters, and no autocapture hero death/hang remains.
- Performance criteria failed: CVar-on median is below the B.8.1 comparator gate after the 10-capture tiebreaker.
- Recommended next pass: B.9.1 diagnostic. Keep Flying routing in code, but do not proceed to Ranged migration until grouped timing or an equivalent same-binary comparison identifies whether the miss is Flying movement cost, higher total lightweight population cost, collision/body-blocking cost, or another capture-path effect.

## Pass B.9.1 Diagnostic

Status as of 2026-05-26: diagnostic complete. The B.9 CVar-on median miss did not reproduce in the same-binary isolation matrix. Production state remains current B.9 behavior: Flying routing enabled by default when `T66.Mob.UseLightweight=1`, overlap-state touch damage enabled by default, and manager tick profiling disabled by default.

Implemented diagnostic scope:

- Added `T66.Mob.Diagnostics.RouteFlyingLightweight`, default `1`, so Flying routing can be isolated without changing Melee/Rush routing.
- Added `T66.Mob.Diagnostics.UseTouchDamageOverlap`, default `1`, so the B.9 overlap-state touch path can be compared with the pre-B.9 distance/rising-edge path.
- Added command-line automation hooks for `-T66MobRouteFlyingLightweight=` and `-T66MobUseTouchDamageOverlap=`.
- Kept both diagnostic CVars non-shipping and runtime-toggleable. No production behavior changes from B.9 defaults.
- Incidental build fix: moved the `NiagaraParameterMapHistory.h` include in `Source/T66/Gameplay/T66Hero1AxeAOEVFXCommandlet.cpp` behind the existing `WITH_EDITOR` guard so the Development game target compiles cleanly.

Build and staging verification:

- Development Win64 build completed successfully with `Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex -MaxParallelActions=8`.
- Staged standalone refresh completed with `Scripts\StageStandaloneBuild.ps1`.
- Root and taskbar `T66 Standalone.lnk` shortcuts both target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Capture matrix artifact: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB9_1\capture_results.json`.
- Capture runner/evidence script: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB9_1\run_b91_captures.ps1`.
- Logs: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB9_1\`.
- Sessions: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Saved\PerformanceSystem\Sessions\`.

Set A, CVar off baseline, `T66.Mob.UseLightweight=0`:

| Run | Session | Avg | 1% | 0.1% | Rich | LW | M | R | F | Hitches | Overhead us | MoveLogs |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260526T005...` | 142.63 | 107.89 | 67.15 | 90 | 0 | 0 | 0 | 0 | 6 | 2882.7 | 576 |
| 2 | `20260526T010...` | 159.44 | 116.58 | 68.50 | 90 | 0 | 0 | 0 | 0 | 5 | 7494.8 | 482 |
| 3 | `20260526T010...` | 162.51 | 121.34 | 88.44 | 90 | 0 | 0 | 0 | 0 | 5 | 952.0 | 628 |
| 4 | `20260526T010...` | 167.05 | 124.70 | 94.24 | 90 | 0 | 0 | 0 | 0 | 5 | 834.6 | 614 |
| 5 | `20260526T010...` | 157.72 | 104.46 | 64.16 | 90 | 0 | 0 | 0 | 0 | 5 | 1236.7 | 497 |
| 6 | `20260526T011...` | 156.23 | 119.06 | 60.44 | 90 | 0 | 0 | 0 | 0 | 5 | 2402.7 | 779 |
| 7 | `20260526T011...` | 144.57 | 90.39 | 44.26 | 90 | 0 | 0 | 0 | 0 | 8 | 13924.2 | 956 |
| 8 | `20260526T011...` | 151.59 | 100.27 | 66.19 | 90 | 0 | 0 | 0 | 0 | 4 | 907.7 | 851 |
| 9 | `20260526T011...` | 161.12 | 117.41 | 73.08 | 90 | 0 | 0 | 0 | 0 | 6 | 1796.7 | 621 |
| 10 | `20260526T012...` | 160.64 | 124.85 | 82.93 | 90 | 0 | 0 | 0 | 0 | 5 | 776.3 | 863 |

Set A computed values: min `142.63`, max `167.05`, median `158.58`, mean `156.35`, stdev `7.85`.

Set B, CVar on, Melee+Rush only, pre-B.9 distance/rising-edge touch path:

| Run | Session | Avg | 1% | 0.1% | Rich | LW | M | R | F | Hitches | Overhead us | MoveLogs |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260526T012...` | 175.18 | 128.08 | 61.80 | 38 | 52 | 42 | 10 | 0 | 6 | 833.8 | 218 |
| 2 | `20260526T012...` | 183.38 | 128.22 | 60.93 | 37 | 53 | 41 | 12 | 0 | 6 | 396142.5 | 250 |
| 3 | `20260526T012...` | 188.99 | 138.85 | 68.55 | 43 | 47 | 37 | 10 | 0 | 5 | 858.6 | 730 |
| 4 | `20260526T013...` | 179.90 | 122.50 | 58.36 | 36 | 54 | 35 | 19 | 0 | 5 | 828.5 | 369 |
| 5 | `20260526T013...` | 184.39 | 132.09 | 64.15 | 45 | 45 | 38 | 7 | 0 | 5 | 1061.9 | 433 |
| 6 | `20260526T013...` | 171.03 | 86.18 | 52.51 | 36 | 54 | 41 | 13 | 0 | 5 | 1391.0 | 255 |
| 7 | `20260526T013...` | 169.80 | 88.42 | 55.00 | 31 | 59 | 51 | 8 | 0 | 5 | 895.9 | 159 |
| 8 | `20260526T014...` | 179.81 | 121.85 | 53.10 | 43 | 47 | 36 | 11 | 0 | 8 | 873.0 | 531 |
| 9 | `20260526T014...` | 164.40 | 83.36 | 36.16 | 34 | 56 | 46 | 10 | 0 | 20 | 1007.8 | 401 |
| 10 | `20260526T014...` | 184.47 | 125.08 | 59.89 | 33 | 57 | 42 | 15 | 0 | 4 | 778.4 | 324 |

Set B computed values: min `164.40`, max `188.99`, median `179.85`, mean `178.13`, stdev `7.80`.

Set C, CVar on, Melee+Rush only, B.9 overlap-state touch path:

| Run | Session | Avg | 1% | 0.1% | Rich | LW | M | R | F | Hitches | Overhead us | MoveLogs |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260526T014...` | 113.62 | 55.39 | 10.78 | 39 | 51 | 38 | 13 | 0 | 33 | 2381.3 | 282 |
| 2 | `20260526T015...` | 171.56 | 101.21 | 61.79 | 36 | 54 | 39 | 15 | 0 | 5 | 955.6 | 543 |
| 3 | `20260526T015...` | 178.11 | 127.18 | 61.06 | 41 | 49 | 37 | 12 | 0 | 5 | 790.6 | 593 |
| 4 | `20260526T015...` | 184.16 | 133.97 | 62.90 | 32 | 58 | 45 | 13 | 0 | 4 | 788.4 | 217 |
| 5 | `20260526T020...` | 186.26 | 129.91 | 59.82 | 35 | 55 | 42 | 13 | 0 | 5 | 955.7 | 535 |
| 6 | `20260526T020...` | 169.25 | 118.61 | 60.14 | 38 | 52 | 40 | 12 | 0 | 5 | 847.0 | 179 |
| 7 | `20260526T020...` | 172.58 | 116.75 | 66.81 | 39 | 51 | 40 | 11 | 0 | 4 | 872.5 | 460 |
| 8 | `20260526T020...` | 180.82 | 120.75 | 64.78 | 34 | 56 | 42 | 14 | 0 | 5 | 761.5 | 675 |
| 9 | `20260526T021...` | 179.87 | 129.34 | 65.38 | 46 | 44 | 34 | 10 | 0 | 6 | 967.9 | 424 |
| 10 | `20260526T021...` | 183.53 | 136.62 | 65.72 | 43 | 47 | 39 | 8 | 0 | 5 | 1383.5 | 591 |

Set C computed values: min `113.62`, max `186.26`, median `178.99`, mean `171.98`, stdev `21.29`. One non-zero exit occurred on run 2 attempt 1 with exit code `-1`; the accepted row above is the clean replacement. Run 1 is a severe low outlier and explains the large stdev, but the median remains aligned with Set B.

Set D, CVar on, full B.9 state, Flying routed lightweight and overlap-state touch enabled:

| Run | Session | Avg | 1% | 0.1% | Rich | LW | M | R | F | Hitches | Overhead us | MoveLogs |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260526T021...` | 182.57 | 128.90 | 67.07 | 33 | 57 | 34 | 14 | 9 | 5 | 884.4 | 8 |
| 2 | `20260526T021...` | 174.45 | 127.50 | 62.18 | 25 | 65 | 43 | 12 | 10 | 5 | 808.5 | 10 |
| 3 | `20260526T022...` | 167.13 | 116.93 | 39.68 | 26 | 64 | 39 | 12 | 13 | 14 | 1061.6 | 10 |
| 4 | `20260526T022...` | 160.60 | 68.44 | 53.82 | 27 | 63 | 32 | 12 | 19 | 7 | 1258.9 | 14 |
| 5 | `20260526T022...` | 174.30 | 121.31 | 54.70 | 24 | 66 | 40 | 13 | 13 | 5 | 1028.3 | 6 |
| 6 | `20260526T022...` | 175.82 | 121.82 | 64.85 | 26 | 64 | 35 | 13 | 16 | 5 | 900.1 | 5 |
| 7 | `20260526T023...` | 177.70 | 121.46 | 65.62 | 26 | 64 | 36 | 14 | 14 | 5 | 1971.7 | 12 |
| 8 | `20260526T023...` | 180.30 | 128.97 | 62.28 | 37 | 53 | 32 | 13 | 8 | 5 | 994.1 | 100 |
| 9 | `20260526T023...` | 175.79 | 119.50 | 59.26 | 21 | 69 | 45 | 13 | 11 | 6 | 882.9 | 2 |
| 10 | `20260526T023...` | 176.71 | 114.11 | 58.68 | 29 | 61 | 28 | 17 | 16 | 6 | 1170.3 | 61 |

Set D computed values: min `160.60`, max `182.57`, median `175.81`, mean `174.54`, stdev `6.37`.

Set E, full B.9 state with `-LogCmds="LogCharacterMovement Off"`:

| Run | Session | Avg | 1% | 0.1% | Rich | LW | M | R | F | Hitches | Overhead us | MoveLogs |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260526T024...` | 177.64 | 123.76 | 60.22 | 31 | 59 | 39 | 8 | 12 | 4 | 877.8 | 0 |
| 2 | `20260526T024...` | 183.86 | 130.44 | 61.14 | 14 | 76 | 47 | 14 | 15 | 5 | 980.1 | 0 |
| 3 | `20260526T024...` | 177.61 | 122.76 | 59.85 | 17 | 73 | 46 | 15 | 12 | 4 | 879.4 | 0 |
| 4 | `20260526T024...` | 182.03 | 125.61 | 59.05 | 20 | 70 | 51 | 6 | 13 | 6 | 870.4 | 0 |
| 5 | `20260526T025...` | 185.72 | 125.10 | 63.95 | 30 | 60 | 34 | 13 | 13 | 5 | 861.8 | 0 |
| 6 | `20260526T025...` | 178.62 | 132.08 | 56.24 | 29 | 61 | 38 | 11 | 12 | 4 | 892.9 | 0 |
| 7 | `20260526T025...` | 178.24 | 122.21 | 61.92 | 23 | 67 | 44 | 13 | 10 | 6 | 74944.2 | 0 |
| 8 | `20260526T025...` | 180.23 | 123.89 | 67.33 | 26 | 64 | 39 | 17 | 8 | 4 | 895.5 | 0 |
| 9 | `20260526T030...` | 180.21 | 123.92 | 64.78 | 29 | 61 | 31 | 16 | 14 | 4 | 844.2 | 0 |
| 10 | `20260526T030...` | 182.86 | 126.16 | 60.01 | 28 | 62 | 37 | 8 | 17 | 5 | 948.8 | 0 |

Set E computed values: min `177.61`, max `185.72`, median `180.22`, mean `180.70`, stdev `2.82`.

Family-cost attribution set, full B.9 with `T66.Mob.ManagerTickProfileEnabled=1`:

| Run | Session | Avg | 1% | 0.1% | Rich | LW | M | R | F | Hitches | Overhead us | MoveLogs |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `20260526T030...` | 177.64 | 131.09 | 62.43 | 19 | 71 | 38 | 18 | 15 | 4 | 843.5 | 4 |
| 2 | `20260526T030...` | 184.62 | 129.35 | 60.58 | 39 | 51 | 30 | 13 | 8 | 5 | 994.4 | 10 |
| 3 | `20260526T031...` | 186.00 | 135.73 | 62.41 | 25 | 65 | 30 | 19 | 16 | 5 | 871.1 | 1 |
| 4 | `20260526T031...` | 174.66 | 123.69 | 64.67 | 18 | 72 | 44 | 8 | 20 | 4 | 872.4 | 7 |
| 5 | `20260526T031...` | 177.37 | 122.02 | 64.13 | 27 | 63 | 42 | 7 | 14 | 4 | 790.8 | 3 |

Profile set computed values: min `174.66`, max `186.00`, median `177.64`, mean `180.06`, stdev `4.96`.

Parsed `[MobManagerProfile]` timing summary:

- Parsed windows: `1900`; saturated-like windows with `ActiveMobs >= 50`: `1730`.
- Median per-mob cost across saturated-like windows:
  - Melee: `0.833 us`
  - Rush: `1.416 us`
  - Flying: `29.225 us`
  - Total per lightweight mob: `8.683 us`
- Flying:Melee per-mob ratio: `35.09x`.
- Rush:Melee per-mob ratio: `1.70x`.
- Saturated-like weighted total costs across the profiling set:
  - Melee: `16.47 s` across `3791063` samples.
  - Rush: `6.24 s` across `1349686` samples.
  - Flying: `40.41 s` across `1504951` samples.
  - Total measured mob manager family work: `63.12 s`.
- Interpretation: Flying is the expensive family inside the manager, but the measured population-weighted cost is still roughly sub-millisecond per frame in the 90-cap capture. It is worth optimizing before any 300-cap experiment, but it did not produce a statistically significant same-binary FPS regression in this pass.

Statistical attribution:

| Comparison | Isolated variable | Median delta | 2x max stdev threshold | Significant |
| --- | --- | ---: | ---: | --- |
| Set B -> Set C | B.9 overlap-state touch damage, no Flying | `-0.86 FPS` | `42.57 FPS` | No |
| Set C -> Set D | Flying routing, overlap touch held constant | `-3.18 FPS` | `42.57 FPS` | No |
| Set D -> Set E | `LogCharacterMovement` suppression | `+4.41 FPS` | `12.74 FPS` | No |
| Set B -> Set D | Total same-binary B.8-equivalent to full B.9 | `-4.05 FPS` | `15.60 FPS` | No |

Diagnostic conclusion:

- The old B.9 full-state median of `162.80` did not reproduce. Same-binary Set D full B.9 landed at `175.81`, above the B.9 acceptance floor of `169.88`.
- Set B reproduced the expected B.8.1 Melee+Rush level (`179.85` vs prior `178.82`), so the diagnostic binary and capture path are consistent with prior accepted data.
- Touch overlap is not a measurable FPS cost in this matrix.
- Flying manager work is much more expensive per mob than Melee/Rush, but the Set C -> Set D median delta is not statistically distinguishable under the requested 2x-stdev rule.
- `LogCharacterMovement Off` removed all stuck-log lines and improved the median by `4.41 FPS`, but that delta is also below the significance threshold. The mixed rich/lightweight collision log issue remains a hygiene/perf-risk item, not the dominant B.9 regression cause.
- Outcome: the B.9 acceptance miss is classified as measurement/system-state variance rather than a deterministic Flying regression. No B.9.2 optimization pass is required before B.10 Ranged.

PerformanceSystem overhead spike notes:

- New B.9.1 spikes appeared in multiple configurations:
  - Set B run 2 reported `396142.5 us`, but the session directory had already rotated out by the time of inspection.
  - Set E run 7 reported `74944.2 us`; the retained session is `20260526T025650Z_j9S6TUHPwxXzvKiou5wGcg`.
- In the retained spike, `PerformanceSystemOverhead/FrameworkBudgetExceeded` fired at `GameTimeSeconds=108.441` with `FrameworkCostUs=74944.202`, followed by a `SingleFrameHitch` at `GameTimeSeconds=108.523` with `FrameTimeMs=81.647`.
- Board state at the hitch: `90` live enemies, `23` rich, `67` lightweight, `44` Melee, `13` Rush, `10` Flying, `0` pending spawns, `0` active enemy projectiles.
- The overhead spike occurs on the PerformanceSystem tick path that records frame samples, captures a board saturation frame sample, runs cadence detectors, and writes the periodic snapshot every `5.0s`. The spike timing is not at final report write, and it also occurred with `LogCharacterMovement` fully suppressed.
- No single definitive cause was isolated inside the 30-minute time box. The most likely next instrumentation point is `WritePeriodicSnapshot` and its `FFileHelper::SaveStringToFile` call, plus event JSONL append cost around overhead-event emission. The pending PerformanceSystem issue remains open.

Recommendation:

- Proceed to B.10 Ranged with B.9 behavior intact.
- Keep `T66.Mob.Diagnostics.RouteFlyingLightweight` and `T66.Mob.Diagnostics.UseTouchDamageOverlap` until final B.14 cleanup; they are useful isolation switches and default to production behavior.
- Keep Flying manager cost in mind for future 300-cap experiments, but do not optimize it at the 50-90 design ceiling unless it resurfaces as a repeatable 10-capture regression.
- Keep the mixed rich/lightweight `LogCharacterMovement` issue queued as log/collision hygiene, because it can still distort forced-flush captures even though it did not explain this B.9 miss.

## Pass B.10 Verification

Status: implemented, staged, smoke-passed, but CVar-on performance acceptance is blocked by real Ranged projectile pressure killing the stationary `enemywaveperf` hero before the 90-second window completes.

Implementation summary:

- `AT66MobBase` now supports `Ranged` configuration, fire cooldown state, distance-band movement, projectile spawn height, projectile class storage, and `FireProjectile` / `TryFireProjectileAtHero` using `Owner=this` and `Instigator=nullptr`.
- `UT66MobManagerSubsystem` now ticks Ranged mobs through the same rich behavior shape: flee below `DesiredMinRange`, chase beyond `DesiredMaxRange`, stop inside the band, and attempt fire only when cooldown is ready, the hero is not in a safe zone, range is within `DesiredMaxRange + FireRangeGrace`, and line of sight is clear. LOS is checked only on the fire attempt; no speculative LOS cache fields were added.
- `AT66EnemyProjectileBase` now ignores overlaps with non-Pawn owners only. This lets `AT66MobBase` projectiles avoid self-destroying on the owning capsule while preserving the rich `APawn`-owned projectile path for CVar-off parity.
- `UT66RunStateSubsystem` damage-source attribution now resolves `AT66MobBase::MobID`, so combat logs report `HexSlinger` / `StoneSentinel` instead of generic `T66MobBase`.
- `AT66EnemyDirector` can route Ranged-family non-mini-boss non-special spawns through `AT66MobBase` when `T66.Mob.UseLightweight=1` and `T66.Mob.Diagnostics.RouteRangedLightweight=1`.
- PerformanceSystem schema was bumped to v8 with `LiveLightweightRangedMobs` in board samples and reports.

Smoke test:

- Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Smoke log: `C:\UE\T66\Saved\StandaloneLogs\T66_PhaseB10_RangedSmoke.log`
- Screenshot sequence: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10\ranged_smoke\ranged_0000.png` through `ranged_0004.png`
- Result: PASS.
- Key assertions:
  - `ProjectileTravelAssertion spawnHeight=80.0 ownedProjectiles=1 maxTravel=489.9 result=PASS`
  - `HeroDamageAssertion initialHP=100.0 currentHP=80.0 result=PASS`
  - Projectile hit log showed `OwnerClass=T66MobBase`.
  - Damage log showed `SourceID=HexSlinger Delivery=EnemyProjectile SourceActor=T66MobBase_*`.
  - Director count sample showed Ranged lightweight population: `lightweightRanged=8`.

CVar-off five accepted captures:

| Run | Accepted attempt | Session | Avg FPS | PerfSystem overhead max us | Peak lightweight Ranged |
| ---: | ---: | --- | ---: | ---: | ---: |
| 1 | 3 | `20260526T064147Z_zm7F40zOgtS2xKqtgQsJIw` | 142.53 | 921.2 | 0 |
| 2 | 1 | `20260526T064413Z_XVB2uE1NGJ_JzTWBwYZstA` | 153.31 | 1102.9 | 0 |
| 3 | 1 | `20260526T064638Z_e8RoXEwjGyCkJEayuUt-0w` | 143.41 | 1106.3 | 0 |
| 4 | 1 | `20260526T064902Z_Ea2Uxkkjs-QkM_Sk9wTP_A` | 156.47 | 1078.0 | 0 |
| 5 | 1 | `20260526T065129Z_BUUaw0KENw78trKX4rqL7g` | 145.65 | 871.2 | 0 |

CVar-off computed values:

- Min: `142.53`
- Max: `156.47`
- Median: `145.65`
- Mean: `148.27`
- Stdev: `6.25`
- Comparison: median is `3.69%` below the B.7 CVar-off comparator `151.23`, so the off path remains within the 5% gate.
- Replacement notes: run 1 attempt 1 exited `-1`; run 1 attempt 2 exited cleanly but was rejected because `PerformanceSystemOverheadMaxUs=23216.803`, above the requested `10000 us` rejection threshold.

CVar-on acceptance attempt:

| Attempt | Result | Session | Notes |
| ---: | --- | --- | --- |
| 1 | Exit `-1` | `20260526T065359Z_OKHYOkR_N-AYZPSOjmMxAg` | Stationary hero died to repeated lightweight `EnemyProjectile` hits. Final fatal damage: `HeroHP=20.0->0.0`, `SourceID=StoneSentinel`, `SourceClass=T66MobBase`, `WorldTime=17.58`. |
| 2 | Rejected overhead | `20260526T070955Z_z3TKYUD590EiAhu-fCyq9w` | Clean exit, but `PerformanceSystemOverheadMaxUs=268567.0`, above the requested `10000 us` rejection threshold. Metrics before rejection: Avg `125.10`, 1% `53.65`, 0.1% `16.98`, peak live `90`, peak lightweight `90`, peak lightweight Ranged `25`, peak projectiles `1`, hitches `28`. |
| 3 | Exit `-1` | `20260526T071304Z_wkEPHEQlWz0bKLCF_G_GmA` | Stationary hero died to repeated lightweight `EnemyProjectile` hits. Final fatal damage: `HeroHP=20.0->0.0`, `SourceID=StoneSentinel`, `SourceClass=T66MobBase`, `WorldTime=28.56`. |

CVar-on conclusion:

- The CVar-on capture set halted per the locked methodology after two non-zero exits in the set.
- This is not a shutdown crash and not a process artifact. The logs show real combat damage from lightweight Ranged projectiles killing the stationary autocapture hero.
- The implementation should not hide this with a hero safety mode or parked hero. The decision needed before acceptance rerun is whether B.10 should:
  - tune lightweight Ranged projectile pressure to preserve the current `enemywaveperf` survival contract,
  - add a parity explanation and revise the capture contract for all-Ranged-enabled lightweight runs,
  - or run a focused same-binary comparison between rich Ranged and lightweight Ranged projectile pressure before making a design change.

Build and staging:

- `Build.bat T66 Win64 Development` succeeded after the final projectile owner-ignore narrowing.
- `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipCook` succeeded after stopping a stale staged `T66.exe` process that held the staging directory lock.
- Root and taskbar `T66 Standalone.lnk` shortcuts were refreshed to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Schema v8:

- `PerformanceSystem/schema/SCHEMA_CHANGELOG.md` records v8: added `LiveLightweightRangedMobs` for B.10 Ranged migration validation.
- `board_saturation_frame_sample`, `performance_event`, and `performance_session_report` schemas now include the Ranged lightweight split.

Hot-path log audit:

- New B.10 Ranged manager tick, LOS, and projectile fire logs are `VeryVerbose`.
- Existing normal-level `LogT66EnemyProjectile` hero-hit and `LogT66TrapProjectile` fire/impact telemetry remains a known log-hygiene risk from earlier passes; B.10 did not broaden those normal-level hot-path logs.

## Pass B.10.1 Diagnostic

Status: diagnostic instrumentation landed, packaged build/stage passed, RA/RB diagnostic did not complete the requested 10x10 matrix because RA hit the overhead-rejection halt rule at 6 captures. The partial diagnostic is still directionally clear: rich Ranged produced no hero projectile hits across six route-valid RA captures, while full lightweight Ranged produced hero projectile hits and deaths in the post-patch attribution rerun.

Artifacts:

- Reviewed plan packet: `C:\UE\T66\Saved\AgentReviews\B10_1_DiagnosticOverheadFixAcceptancePlan.md`
- Claude review: `C:\UE\T66\Saved\AgentReviews\20260526T055620-pass10\claude_review_pass10.md`
- Capture runner: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\run_b101_diagnostic_captures.ps1`
- RA partial rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\ra_partial_rows_20260526_072211.jsonl`
- RB post-patch preflight row: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\rb_preflight_rows_20260526_074809.jsonl`
- Pre-patch attribution rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\attribution_partial_rows_20260526_074110.jsonl`
- Post-patch attribution rerun rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\capture_rows.jsonl`
- Current result JSON: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\capture_results.json`

Instrumentation added:

- `UT66MobManagerSubsystem` now owns aggregate Ranged-pressure diagnostics reset at subsystem init and `enemywaveperf` start.
- Rich Ranged, lightweight Ranged, and enemy projectiles record route, fire-attempt, LOS, projectile lifecycle, hit, damage, and terminal summary counters.
- `AT66PlayerController::HandleGameplayAutomationQuit` emits clean terminal summaries; `AT66PlayerController::OnPlayerDied` emits death summaries.
- `enemywaveperf` automation now exits after a death summary instead of parking on the game-over pause. The runner classifies deaths from the terminal summary because Windows still reported process exit code `0` for these graceful automation exits.
- `UT66PerformanceSubsystem` has default-off substep attribution behind `-T66PerfSubstepAttribution=1`.

Build and stage verification:

- `Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex -MaxParallelActions=8`: PASS.
- `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`: PASS after repairing cooked metadata from an interrupted stray staging process.
- Root and taskbar standalone shortcuts both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

RA partial diagnostic: `UseLightweight=1`, Rush/Flying lightweight, `RouteRangedLightweight=0` (Ranged rich). All rows were route-valid (`RichSpawns>0`, `LightweightSpawns=0`) and the hero survived with no projectile hits. The set halted at run 6 because run 4 and run 6 exceeded the `10000 us` PerformanceSystem overhead rejection threshold.

| Run | Terminal | Hero HP | Hero projectile hits | Rich spawns | Lightweight spawns | Rich attempts | Rich projectiles | Avg FPS | Perf overhead us | Rejected |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 1 | Completed | 100 | 0 | 21 | 0 | 0 | 0 | 171.98 | 894.0 | No |
| 2 | Completed | 100 | 0 | 35 | 0 | 895 | 0 | 174.15 | 903.8 | No |
| 3 | Completed | 100 | 0 | 23 | 0 | 420 | 0 | 175.24 | 875.2 | No |
| 4 | Completed | 100 | 0 | 33 | 0 | 983 | 0 | 163.86 | 105216.0 | Yes |
| 5 | Completed | 100 | 0 | 23 | 0 | 2505 | 0 | 159.30 | 1010.3 | No |
| 6 | Completed | 100 | 0 | 19 | 0 | 0 | 0 | 154.25 | 12583.9 | Yes |

RB post-patch preflight: `UseLightweight=1`, all four families lightweight. The single row was route-valid (`LightweightSpawns=27`, `RouteRanged=1`), hero survived at 80 HP, and lightweight Ranged landed 1 projectile hit for 20 HP damage.

| Run | Terminal | Hero HP | Hero projectile hits | Lightweight spawns | Lightweight attempts | Lightweight projectiles | Avg FPS | Perf overhead us | Rejected |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 1 | Completed | 80 | 1 | 27 | 8728 | 12 | 178.40 | 787.9 | No |

Post-patch attribution rerun: `UseLightweight=1`, all four families lightweight, `-T66PerfSubstepAttribution=1`. This was not a primary RA/RB parity matrix because the PerformanceSystem probes were enabled, but it proves death sessions now serialize and shows full-lightweight Ranged pressure reliably reaches the stationary hero.

| Run | Terminal | Hero HP | Hero projectile hits | Damage HP | First hit time | Last hit time | Lightweight spawns | Lightweight attempts | Lightweight projectiles | Perf overhead us | Snapshot peak us | Event append peak us |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | HeroDied | 0 | 6 | 120 | 22.32 | 28.31 | 31 | 50 | 7 | 1131.7 | 861.6 | 645.0 |
| 2 | HeroDied | 0 | 5 | 100 | 6.19 | 11.81 | 24 | 5 | 5 | 878.5 | 892.9 | 537.3 |
| 3 | Completed | 60 | 2 | 40 | 19.90 | 21.44 | 33 | 10900 | 32 | 2115.9 | 2091.8 | 582.7 |
| 4 | Completed | 60 | 3 | 60 | 39.67 | 41.37 | 24 | 8092 | 10 | 1530.7 | 1500.2 | 522.0 |
| 5 | HeroDied | 0 | 5 | 100 | 27.56 | 33.94 | 28 | 222 | 6 | 985.6 | 604.6 | 822.4 |

Attribution findings:

- A pre-patch attribution attempt while a stray staging/cook process was competing for disk showed the suspected file-I/O sensitivity very clearly: `PerfSystemOverheadMaxUs=3317357.4`, `SubstepSummary_EventJsonAppendPeakUs=10086628.3`, `SubstepSummary_PeriodicSnapshotPeakUs=6477247.0`, and `FrameworkTotalPeakUs=10765523.0`.
- The clean post-patch attribution rerun had no overhead rejects, with max framework overhead `2115.9 us`. This does not close the pending issue because no I/O fix landed; it only shows the substep probes themselves are usable and low-cost when the disk is not contended.
- The actionable risk remains synchronous file I/O on the game thread under forced flush / disk contention: event JSONL append and periodic snapshot writes are the dominant observed stall owners when the spike reproduces.

Diagnostic branch:

- The formal 10x10 RA/RB matrix is incomplete because RA hit the two-overhead-reject halt rule.
- The available route-valid evidence strongly favors the "lightweight Ranged is more aggressive than rich Ranged in practice" branch: RA rich Ranged survived 6/6 with zero hero hits; full-lightweight Ranged produced projectile hits in the RB preflight and killed the hero in 3/5 attribution rerun captures.
- This is not enough for B.10 acceptance. It is enough to stop and write a concrete follow-up packet for two reviewed fixes:
  - PerformanceSystem: move/batch periodic snapshot and event JSONL file writes off the game thread or otherwise prevent disk stalls from rejecting captures.
  - Gameplay: drill and fix Ranged parity around movement efficiency, LOS/fire cadence, projectile spawn failure, and rich-vs-lightweight projectile pressure.

B.10 acceptance re-attempt:

- Not run in B.10.1.
- Blockers remaining: incomplete RA/RB matrix, lightweight Ranged pressure gap, and no reviewed PerformanceSystem I/O fix yet.

## Pass B.10.1B PerformanceSystem I/O Fix And Clean RA/RB Diagnostic

Status: targeted PerformanceSystem write-queue mitigation landed and clean B.10.1B validation completed. B.10 acceptance was not re-attempted under this packet. Ranged parity behavior was not changed.

Artifacts:

- Reviewed follow-up packet: `C:\UE\T66\Saved\AgentReviews\B10_1B_PerfIoAndRangedParityFixPlan.md`
- Claude review: `C:\UE\T66\Saved\AgentReviews\20260526T081557-pass15\claude_review_pass15.md`
- Combined Claude handoff packet: `C:\UE\T66\Saved\AgentReviews\B10_1_Combined_ClaudePacket.md`
- Queue ordering self-test log: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1\T66_B101B_WriteQueueSelfTest_AfterFix.log`
- Probe-on validation rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\b101b_attribution_rows_20260526_092030.jsonl`
- RA rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\b101b_ra_rows_20260526_094659.jsonl`
- RB rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\b101b_rb_rows_20260526_100523.jsonl`
- Current result JSON after RB: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1\capture_results.json`

Implementation:

- Audited producer call sites before queue code. `EmitPerformanceEvent` and `WritePeriodicSnapshot` are private `UT66PerformanceSubsystem` paths driven by `Initialize`, `TickPerformanceSystem`, detector checks, GC delegates, and `RecordMeasuredOperation`. The only public ingress is `UT66LagTrackerSubsystem::RecordOperation`, whose current scoped call sites are game/UI/ticker paths. No off-thread producer was found.
- Added Development-visible `ensureMsgf` guards to `RecordMeasuredOperation`, `EmitPerformanceEvent`, and `WritePeriodicSnapshot`. Validation logs did not contain the guard text, ensure failures, assertions, or fatal errors.
- Added one dedicated `UT66PerformanceSubsystem` write worker with a fixed normal capacity of `4096` commands and a bounded `10s` shutdown flush policy.
- Moved `events.jsonl` appends and non-forced `snapshot.current.json` replacements off the game thread as owned `FString` payloads. Forced snapshots flush the queue before rotating `snapshot.current.json` to `snapshot.previous.json`; final reports flush before synchronous summary writes.
- Added queue counters to running snapshots and final session summaries: attempted, queued, completed, failed, fallback, abandoned, max queue depth, worker write peak, fallback write peak, and accounting balance.
- Added a non-shipping ordering self-test path used only for validation. The normal queue capacity remains `4096` in all real captures.

Build and stage verification:

- `Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex -MaxParallelActions=8`: PASS.
- `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`: PASS.
- After the ordering-race fix: focused rebuild PASS; `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild -SkipCook`: PASS.
- Root and taskbar shortcuts both point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Queue ordering self-test:

- First self-test intentionally forced queue-full fallback and exposed an ordering race: `EventIds=1,3,2,5,4,7,6,9,8,10`. The worker was decrementing queue depth before marking the dequeued command active, so `FlushAndWait` could see a false idle window.
- Fixed the race by marking worker writes active before decrementing queue depth and before the developer-only worker delay.
- Post-fix self-test session: `20260526T121124Z_D6HLFE5lnQdhcACKfLUwYA`.
- Post-fix result: `EventIds=1,2,3,4,5,6,7,8,9,10`, `QueueFullFallbackWrites=2`, `FallbackWrites=2`, `AttemptedWrites=10`, `CompletedWrites=10`, `FailedWrites=0`, `AbandonedWrites=0`, `AccountingBalanced=true`, `MaxQueueDepth=1`.
- `events.jsonl` invalid/torn line count: `0`.
- Teardown timeout is fixed for this pass at `10.0` seconds through `StopAndJoin(T66PerformanceWriteQueueShutdownTimeoutSeconds)`. Normal validation exercised shutdown with `FailedWrites=0`, `AbandonedWrites=0`, and balanced queue accounting; the timeout-overrun abandon branch was not forcibly exercised.

Clean-environment controls:

- Before probe-on validation: no `RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`, or `git-lfs.exe` processes after waiting out one transient `git-lfs.exe`.
- Before RA: clean.
- Before RB: clean.
- After RB: clean.

Probe-on overhead validation: `UseLightweight=1`, all four migrated families lightweight, `-T66PerfSubstepAttribution=1`.

| Run | Terminal | Hero HP | Hero hits | Damage HP | Perf overhead us | Event append peak us | Snapshot peak us | Queue writes | Failed | Fallback | Max queue depth | Accounting |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 1 | HeroDied | 0 | 5 | 100 | 0.0 | 9.9 | 0.0 | 15/15 | 0 | 0 | 1 | Balanced |
| 2 | Completed | 40 | 4 | 80 | 890.9 | 10.1 | 0.0 | 38/38 | 0 | 0 | 1 | Balanced |
| 3 | Completed | 60 | 3 | 60 | 680.8 | 8.7 | 0.0 | 38/38 | 0 | 0 | 1 | Balanced |
| 4 | HeroDied | 0 | 7 | 140 | 523.5 | 9.6 | 0.0 | 13/13 | 0 | 0 | 1 | Balanced |
| 5 | HeroDied | 0 | 9 | 180 | 0.0 | 11.7 | 0.0 | 15/15 | 0 | 0 | 1 | Balanced |

Validation summary: 5/5 route-valid, 0 overhead rejects, max framework overhead `890.9 us`, max queue depth `1`, no failed/fallback/abandoned writes, and all queue accounting balanced.

Clean RA diagnostic: `UseLightweight=1`, Rush/Flying lightweight, `RouteRangedLightweight=0` (Ranged rich), probes off.

| Run | Terminal | Hero HP | Hero hits | Damage HP | Perf overhead us | Queue writes | Failed | Fallback | Max queue depth | Worker write peak us |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | Completed | 100 | 0 | 0 | 1312.5 | 40/40 | 0 | 0 | 1 | 2709.6 |
| 2 | Completed | 100 | 0 | 0 | 910.4 | 39/39 | 0 | 0 | 1 | 2876.5 |
| 3 | Completed | 100 | 0 | 0 | 902.4 | 40/40 | 0 | 0 | 1 | 3951.2 |
| 4 | Completed | 100 | 0 | 0 | 724.5 | 40/40 | 0 | 0 | 1 | 2287.0 |
| 5 | Completed | 100 | 0 | 0 | 962.0 | 40/40 | 0 | 0 | 1 | 2774.5 |
| 6 | Completed | 100 | 0 | 0 | 799.7 | 38/38 | 0 | 0 | 1 | 2737.6 |
| 7 | Completed | 100 | 0 | 0 | 757.1 | 40/40 | 0 | 0 | 1 | 2773.0 |
| 8 | Completed | 100 | 0 | 0 | 772.7 | 38/38 | 0 | 0 | 1 | 2315.2 |
| 9 | Completed | 100 | 0 | 0 | 842.3 | 36/36 | 0 | 0 | 1 | 2476.2 |
| 10 | Completed | 100 | 0 | 0 | 919.9 | 40/40 | 0 | 0 | 1 | 2897.9 |

RA summary: 10/10 route-valid, 10/10 completed, 0 hero deaths, 0 projectile hits, 0 projectile damage, 0 overhead rejects, max framework overhead `1312.5 us`, median FPS `175.62`, mean FPS `175.98`, stdev `5.38`.

Clean RB diagnostic: `UseLightweight=1`, all four migrated families lightweight, probes off.

| Run | Terminal | Hero HP | Hero hits | Damage HP | First hit | Last hit | Perf overhead us | Queue writes | Failed | Fallback | Max queue depth | Worker write peak us |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | Completed | 100 | 0 | 0 | -1.00 | -1.00 | 697.5 | 38/38 | 0 | 0 | 1 | 2993.0 |
| 2 | Completed | 80 | 1 | 20 | 36.28 | 36.28 | 903.4 | 42/42 | 0 | 0 | 1 | 2458.3 |
| 3 | HeroDied | 0 | 5 | 100 | 1.84 | 8.33 | 0.0 | 10/10 | 0 | 0 | 1 | 2110.1 |
| 4 | HeroDied | 0 | 5 | 100 | 2.03 | 8.44 | 0.0 | 9/9 | 0 | 0 | 1 | 2266.7 |
| 5 | HeroDied | 0 | 5 | 100 | 2.15 | 8.56 | 0.0 | 10/10 | 0 | 0 | 1 | 2666.1 |
| 6 | Completed | 40 | 3 | 60 | 20.87 | 24.29 | 952.3 | 39/39 | 0 | 0 | 1 | 2345.4 |
| 7 | HeroDied | 0 | 5 | 100 | 16.74 | 23.13 | 0.0 | 13/13 | 0 | 0 | 1 | 2491.3 |
| 8 | Completed | 100 | 0 | 0 | -1.00 | -1.00 | 1049.6 | 40/40 | 0 | 0 | 1 | 27822.7 |
| 9 | Completed | 20 | 5 | 100 | 15.36 | 21.53 | 705.0 | 39/39 | 0 | 0 | 2 | 2734.9 |
| 10 | Completed | 60 | 2 | 40 | 18.31 | 24.44 | 978.3 | 39/39 | 0 | 0 | 1 | 2905.4 |

RB summary: 10/10 route-valid, 6 completed, 4 hero deaths, 31 projectile hits, 620 HP projectile damage, 0 overhead rejects, max framework overhead `1049.6 us`, median FPS `177.27`, mean FPS `177.73`, stdev `3.79`.

Snapshot and file validity:

- All validation, RA, and RB `events.jsonl` and `session_summary.json` files parsed successfully.
- `snapshot.current.json`: valid JSON, session `20260526T130139Z_0fRjK0ShMjAvznOO36eX6A`, includes `PerformanceWriteQueue`, accounting balanced.
- `snapshot.previous.json`: valid JSON, session `20260526T125910Z_rWWKtENE7-F6lzKrMTgqJw`, includes `PerformanceWriteQueue`, accounting balanced.

PerformanceSystem fix attribution:

- The queue did not merely hide data: every normal-capacity capture reported `AttemptedWrites == CompletedWrites + FailedWrites + AbandonedWrites`, with `FailedWrites=0`, `FallbackWrites=0`, and `AbandonedWrites=0`.
- The fix materially changes the game-thread measurement surface for PerformanceSystem I/O: `EventJsonAppendPeakUs` now measures enqueue cost rather than disk append latency, as intended by the reviewed packet.
- The post-race-fix staged executable was fresh for validation: `Source/T66/PerformanceSystem/T66PerformanceSubsystem.cpp` timestamp `2026-05-26 09:09:29`, source `Binaries/Win64/T66.exe` timestamp `2026-05-26 09:10:00`, and staged `Saved/StagedBuilds/Windows/T66/Binaries/Win64/T66.exe` timestamp `2026-05-26 09:10:00`.
- RB's 4 hero-death runs ended early, so RB overhead is rejection/queue-integrity evidence, not a representative full-duration overhead distribution. Probe-on validation is the primary full-run overhead validation evidence; RB run 8 is the clearest off-thread absorption evidence because the worker observed a `27822.7 us` write while game-thread framework overhead stayed `1049.6 us`.
- The original pre-mitigation RA rejects were also zero-hit/zero-damage RA captures (`HeroProjectileHits=0`, `ProjectileDamageHP=0`, `RichProjectiles=0` on the `105216.0 us` and `12583.9 us` rejected rows), so the post-mitigation RA reject removal was not caused by a new drop in RA projectile workload. RA remains unsuitable as a representative Ranged-load comparator until the next packet explains the rich-Ranged zero projectile activity.
- Would the queue alone have cleared the original RA overhead rejects (`105216.0 us` and `12583.9 us`)? For PerformanceSystem-owned event/snapshot file-write stalls, yes: the post-fix captures kept game-thread framework overhead under `1312.5 us` even when the worker itself observed a `27822.7 us` write in RB run 8. That proves at least 27.8 ms of write latency can be moved out of the game-thread framework budget. The earlier 105 ms/12.6 ms RA rejects happened under disk contention, so the queue cannot be credited with eliminating non-PerformanceSystem disk contention or other game-thread stalls. The correct conclusion is: queue + clean-environment control made the infrastructure trustworthy; the queue is the architectural fix for PerformanceSystem file-write stalls, while the clean-environment check remains required to exclude unrelated disk load from captures.

Ranged diagnostic decision:

- The completed clean matrix shows a route-dependent split in observed Ranged outcomes without PerformanceSystem overhead rejects. RA rich Ranged produced `0` hits and `0` damage across 10 route-valid captures. RB full-lightweight Ranged produced `31` hits, `620` HP damage, and `4` hero deaths across 10 route-valid captures.
- This supports keeping B.10 blocked on Ranged parity rather than changing the measurement contract in B.10.1B. The next reviewed gameplay packet should explain why RA rich Ranged has zero projectile activity, then make lightweight Ranged match rich behavior in the same discipline as B.6.1 touch-damage parity. No Ranged parity behavior was changed in B.10.1B.

Comparator handling:

- This pass changed the measurement surface for PerformanceSystem event/snapshot I/O, but only for captures that would otherwise pay file-write latency inside the game-thread framework budget. Normal framework overhead stayed in the same sub-2 ms range seen in prior clean captures, and the prior comparator policy already rejected large overhead spikes.
- Existing B.10 comparators can stand for the next parity-fix/acceptance packet if that packet keeps the same rejection rule and reports queue counters. A comparator refresh is not required solely because of this I/O mitigation. If future captures show a broader normal-overhead distribution shift rather than rejects-only behavior, refresh comparators before judging B.10 acceptance.

B.10 acceptance re-attempt:

- Not run in B.10.1B.
- Remaining blocker: lightweight Ranged parity. The PerformanceSystem overhead blocker is cleared for the current capture methodology.

## Pass B.10.1C-Freeze Diagnostic

Status: investigation-only freeze diagnostic completed. No production behavior fix, revert, Ranged parity fix, or B.10 acceptance reattempt was performed.

Artifacts:

- Reviewed plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_freeze_diagnostic\plan_packet.md`
- Claude review greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T004207-pass5\claude_review_pass5.md`
- Matrix runner: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\run_b101c_freeze_matrix_simple.ps1`
- Matrix rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\freeze_matrix_rows_simple.jsonl`
- Matrix summary: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\freeze_matrix_summary_simple.json`
- Provenance: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\provenance_simple.json`
- Logs: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1C_Freeze\`
- Process samples: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\process_samples\`
- Tail files: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Freeze\log_tails\`

Reviewed-method deviation used for this investigation: freeze evidence was retained even if it would normally violate acceptance-capture hygiene. In practice, none of the matrix runs exceeded `PerformanceSystemOverheadMaxUs > 10000`, and none hit the 180s wall cap. One ambiguous rerun was allowed per cell; Config 1 and Config 3 each reran once after an early no-HP hero death.

Staged-binary provenance:

- Final matrix executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- Size: `311056896`
- Timestamp: `2026-05-27T04:26:21.5788180Z`
- SHA256: `C4E6053B00CFFF722CFEB2FC7E976EEC0504CD7B9D1B21608A6335A1305752E5`
- Source timestamps were earlier than the staged executable: `T66MobManagerSubsystem.cpp` `2026-05-27T01:11:54Z`, `T66MobBase.cpp` `2026-05-27T01:12:04Z`, `T66RangedEnemy.cpp` `2026-05-27T01:12:04Z`, `T66PlayerController_Overlays.cpp` `2026-05-27T01:17:34Z`.
- Binary string scan was inconclusive: it did not find `Stage=CooldownBlocked`, `LogT66RangedDiagnostics`, or `RangedFireDecision`. The logs are the authoritative proof that this staged binary honored `T66.Ranged.DiagnosticLogging` and used `LogT66RangedDiagnostics`.
- Broader observation: staged executable provenance drifted during the investigation window. Earlier preflight saw a staged exe timestamp `2026-05-27T02:57:27Z` and SHA `631972E6135572D8A4484B094017DF9828528C66AE08E1C49C523AB44190E452`; an aborted runner later saw timestamp `2026-05-27T04:01:28Z` and SHA `C16431BAE717DFE3DA160E26AF8A904AE91B8CA9F7E65F5BFE0E8AC84CB752CC`; the final matrix used the `2026-05-27T04:26:21Z` binary above. The final matrix is internally consistent and provenance-recorded, but cross-run comparisons should use the final binary only.

Save-state isolation:

- The staged `SaveGames` directory was snapshotted and restored before each config.
- Final staged save fingerprint matched baseline: `985FF2C366B3D3781469AEF94AB2DD4430DAE88A7CAA256E50B79F3E36EAFE91`.
- No matrix run mutated the staged save state after restore.

Matrix: full RB routing (`UseLightweight=1`, Rush/Flying/Ranged lightweight), 30.0s intended game time, 180s wall cap.

| Config | Attempt | HP override | Diag logging | Exit | Wall s | World s | Game/wall | Log KB | Decisions | Hero hits | Damage lines | Perf overhead us | Process state | Save |
| --- | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- |
| Config1 | 1 | 0 | 0 | Early death | 40.13 | 24.68 | 0.615 | 98.4 | 0 | 5 | 5 | 2211.0 | Responding, CPU active | Restored |
| Config1 | 2 | 0 | 0 | Natural exit | 44.08 | 30.03 | 0.681 | 102.5 | 0 | 2 | 2 | 2677.8 | Responding, CPU active | Restored |
| Config2 | 1 | 500 | 0 | Natural exit | 43.94 | 30.04 | 0.684 | 105.6 | 0 | 4 | 4 | 802.2 | Responding, CPU active | Restored |
| Config3 | 1 | 0 | 1 | Early death | 42.05 | 26.84 | 0.638 | 362.8 | 1467 | 5 | 5 | 2039.5 | Responding, CPU active | Restored |
| Config3 | 2 | 0 | 1 | Natural exit | 49.35 | 30.02 | 0.608 | 457.9 | 2012 | 2 | 2 | 891.4 | CPU active; one transient not-responding sample near exit | Restored |
| Config4 | 1 | 500 | 1 | Natural exit | 53.82 | 30.02 | 0.558 | 548.5 | 2506 | 0 | 0 | 3273.3 | CPU active; transient not-responding sample observed during run | Restored |

Toggle confirmation:

- Every config logged `MobRoutingFlags UseLightweight=1 RouteRush=1 RouteFlying=1 RouteRanged=1 UseTouchDamageOverlap=1 ManagerTickProfile=0`.
- Diagnostic-off configs logged `RangedDiagnosticLogging=0` and emitted `0` `[RangedFireDecision]` lines.
- Diagnostic-on configs logged `RangedDiagnosticLogging=1` and emitted `[RangedFireDecision]` lines through `LogT66RangedDiagnostics`.
- Override-on configs logged `AutoCaptureHeroHPOverride AppliedHP=500.0 RequestedHP=500.0 MaxHP=500.0 CurrentHP=500.0 CurrentHearts=25 MaxHearts=25`.
- Override-off configs logged `HeroHPOverride=0.0` and no override application.

Process-level symptoms:

- No matrix run deadlocked or hit the 180s wall cap.
- T66 consumed CPU throughout the samples; this was slow-but-live behavior, not an idle deadlock.
- Disk writes were not elevated enough to implicate the PerformanceSystem queue. Peak observed process write rate in the matrix was about `395 KB/s` in Config4.
- PerformanceSystem queue accounting remained clean in all six matrix rows: `FailedWrites=0`, `FallbackWrites=0`, `AbandonedWrites=0`, `AccountingBalanced=true`; max queue depth was `1` or `2`.
- Worker write peaks were below `7.3 ms` in the matrix (`7212.9 us` max), while framework overhead stayed below the `10 ms` rejection threshold (`3273.3 us` max).
- No stack/dump was taken in this pass. The process evidence does not support a mutex/file-write deadlock; if future full-length diagnostics hard-freeze in the post-mitigation binary, a reviewed dump-based packet would be needed to prove the exact wait owner.

Isolation finding:

- HP override is not the freeze cause. Config1 attempt 2 and Config2 attempt 1 had nearly identical game/wall ratios (`0.681` vs `0.684`) and similar wall time (`44.08s` vs `43.94s`).
- Diagnostic logging is the current slowdown cause. With HP override enabled, diagnostic-on Config4 took `53.82s` wall for the same `30.02s` world time where diagnostic-off Config2 took `43.94s`, a `+22.5%` wall-time increase and an `18.4%` lower game/wall ratio.
- The short matrix did not reproduce the user-visible hard freeze on the final staged binary. It reproduced a slow-but-live diagnostic overhead.
- The original hard-freeze signature is explained by earlier pre-final B.10.1C diagnostic logs. `T66_B101C_Override_DiagOn_RB_rerun.log` was `8.23 MB` with `46415` `[RangedFireDecision]` lines, `45470` of them `Stage=CooldownBlocked`, all through legacy `LogT66MobManager: VeryVerbose`, and no terminal `RangedPressureSummary`. `T66_B101C_Override_DiagOn_RB_20260526_215506.log` was `11.58 MB` with `65556` decision lines, `64135` `CooldownBlocked`, again no terminal summary. Their tails are continuous per-frame `CooldownBlocked` output. That log flood under `-forcelogflush` matches the frozen-window symptom.
- The later B.10.1C gate logs prove the mitigation shape: `T66_B101C_Override_DiagOn_RB_Gate_20260526_222022.log` was `0.22 MB`, had `768` decision lines, `0` `CooldownBlocked` log lines, used `LogT66RangedDiagnostics`, and completed with a terminal summary.

Broader B.10.1C observations:

| Severity | Observation | What fixing would entail |
| --- | --- | --- |
| Major | Pre-final diagnostic logging enabled legacy `LogT66MobManager VeryVerbose` and emitted per-frame `CooldownBlocked` lines under `-forcelogflush`, producing 46k to 65k decision lines in short smokes and no terminal summary. | Keep cooldown-blocked data aggregate-only; never enable broad manager VeryVerbose for capture diagnostics; use sampled/throttled decision traces if full logs are required. |
| Major | Current post-mitigation `LogT66RangedDiagnostics VeryVerbose` still slows short captures materially, even though it no longer hard-freezes: Config4 was `53.82s` wall for `30.02s` world time. | Convert long diagnostic captures to counters-first output with bounded examples, or add a reviewed throttle/sample CVar for `[RangedFireDecision]` lines. |
| Major | No-HP full-lightweight 30s captures can still kill the stationary hero. Config1 attempt 1 died at `24.68s` after 5 projectile hits and 100 HP damage; Config3 attempt 1 died at `26.84s`. | Use the explicit HP override for ranged-active diagnostics, and keep acceptance captures separate from diagnostic survivability captures. |
| Major | Staged executable provenance drifted during the investigation window while unrelated tooling was observed earlier. The final matrix is internally valid, but the changing binary means older partial runs are not comparable without provenance. | Capture workflows should record exe hash/timestamp before every matrix and block if any build/stage process appears or if the staged exe changes mid-pass. |
| Major | The first local freeze-matrix runner had investigation-harness bugs: `$Pid` conflicted with PowerShell's read-only `$PID`, and a restore attempt copied `.sav` files into staged `Saved` root before correction. The final simple runner restored the baseline and verified final save fingerprint equality. | Keep the simple scalar-output runner or promote a hardened master capture harness; include save-root assertions and post-run root `.sav` checks. |
| Minor | Binary string scanning did not find diagnostic strings despite logs proving the category was active. | Treat staged exe string scans as corroborating only; prefer source/stage timestamps and log signatures. |
| Minor | Staged logs repeated existing non-blocking warnings: Steam unavailable, missing audio packages (`/Game/Audio/SC_Music`, `/Game/Audio/SC_SFX`, theme assets), invalid community item rows (`Item_Alchemy`), and the already-tracked `LogT66TrapProjectile` hot-path fire/impact logs. | Audit data/package references in a separate cleanup pass; trap projectile log demotion is already tracked. |

Root cause:

- The original B.10.1C visible freeze was caused by diagnostic log volume, specifically pre-final per-frame `CooldownBlocked` `[RangedFireDecision]` output through `LogT66MobManager VeryVerbose` under `-forcelogflush`.
- The HP override did not cause the freeze.
- PerformanceSystem write queue/I/O did not cause the freeze in the post-mitigation matrix.
- Current diagnostic logging is improved but still not cheap enough to treat as performance-neutral in long captures.

Proposed next-packet fix scope:

- Do not touch Ranged parity yet unless the next packet is explicitly the B.10.1D rich-Ranged fix.
- Add a reviewed Ranged diagnostic hygiene pass before any long diagnostic matrix that needs full decision traces:
  - Keep all hot-path counters.
  - Keep cooldown-blocked as counters only, not per-event logs.
  - Gate `[RangedFireDecision]` lines behind a bounded sample/throttle such as first N per stage/path per capture or one-per-N-seconds per stage/path.
  - Keep the dedicated `LogT66RangedDiagnostics` category; do not enable broad `LogT66MobManager VeryVerbose` in capture scripts.
  - Add a short smoke proving diagnostic-on capture reaches terminal summary, keeps overhead below `10 ms`, and does not produce unbounded logs.
- Separately, the B.10.1D packet can proceed with rich-Ranged firing diagnosis/fix using HP override, counters, and bounded logs rather than full unthrottled traces.

## Pass B.10.1C-Rerun

Status: diagnostic rerun completed with aggregate Ranged decision counters. No rich Ranged fix, lightweight behavior change, or B.10 acceptance reattempt was performed.

Artifacts:

- Combined packet: `C:\UE\T66\PerformanceSystem\B10_1C_Rerun_ConsolidatedPacket.md`
- Aggregate diagnostics plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_aggregate_diagnostics\plan_packet.md`
- Aggregate diagnostics Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_aggregate_diagnostics\20260527T022620-pass2\claude_review_pass2.md`
- Git/LFS provenance plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_option2_gitlfs_provenance\plan_packet.md`
- Git/LFS provenance Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T_b101c_rerun_option2_gitlfs_provenance\20260527T055211-pass2\claude_review_pass2.md`
- RA-D rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\capture_rows_RA-D_accepted_gitlfs_provenance_6212558B.jsonl`
- RB-D rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1C_Rerun\capture_rows_RB-D_accepted_6212558B.jsonl`

Implementation:

- Replaced B.10.1C per-frame/per-mob Ranged fire logs with in-memory aggregate counters and one terminal `[RangedDecisionSummary]` line per capture.
- `T66.Ranged.DiagnosticLogging=1` now tracks counters and emits the summary; `0` leaves tracking disabled.
- Added runner field `PostCaptureGitContaminated`. Post-capture Git/LFS activity is recorded as diagnostic provenance, not a hard reject, for this counter-only pass. Pre-capture Git/LFS waits and executable hash checks remain active.

Binary provenance:

- Staged executable: `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`
- SHA256 for accepted RA-D and RB-D evidence: `6212558B842942338D3071098D094703542CB473C9DAADDCF5B96C49FFC71ACA`
- RA-D pass-start, per-capture, and pass-end hashes matched.

RA-D: `UseLightweight=1`, Rush/Flying lightweight, `RouteRangedLightweight=0`, `HeroHPOverride=500`, `Ranged.DiagnosticLogging=1`.

| Run | HP | Hits | Overhead us | Git contaminated | Rich attempts | Dist pass | LOS pass | Dispatch | Spawned | Spawn fail |
| ---: | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 500 | 0 | 1184.7 | True | 385834 | 862 | 33 | 33 | 0 | 33 |
| 2 | 500 | 0 | 941.7 | False | 536236 | 430 | 3 | 3 | 0 | 3 |
| 3 | 500 | 0 | 952.5 | False | 633083 | 3236 | 0 | 0 | 0 | 0 |
| 4 | 500 | 0 | 920.6 | True | 566440 | 2382 | 0 | 0 | 0 | 0 |
| 5 | 500 | 0 | 928.8 | False | 582892 | 804 | 0 | 0 | 0 | 0 |
| 6 | 500 | 0 | 1102.8 | False | 518431 | 444 | 30 | 30 | 0 | 30 |
| 7 | 500 | 0 | 997.5 | False | 493499 | 497 | 83 | 83 | 0 | 83 |
| 8 | 500 | 0 | 926.4 | False | 549417 | 1631 | 72 | 72 | 0 | 72 |
| 9 | 500 | 0 | 698.4 | True | 460189 | 0 | 0 | 0 | 0 | 0 |
| 10 | 500 | 0 | 1297.0 | False | 668993 | 1493 | 0 | 0 | 0 | 0 |

RA-D summary: 10/10 accepted, 10/10 hero survived, 0 hero hits, max framework overhead `1297.0 us`, 3 rows marked `PostCaptureGitContaminated=true`. Rich totals: `5,395,014` attempts, `11,779` distance-passed, `221` LOS-passed, `221` dispatched, `0` spawned, `221` spawn failed, `0` hero hits.

RB-D: `UseLightweight=1`, Rush/Flying/Ranged lightweight, `HeroHPOverride=500`, `Ranged.DiagnosticLogging=1`.

| Run | HP | Hits | Overhead us | Lightweight attempts | Dist pass | LOS pass | Dispatch | Spawned | Spawn fail |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 240 | 13 | 1036.3 | 570705 | 9571 | 17 | 17 | 17 | 0 |
| 2 | 420 | 5 | 929.3 | 522802 | 8536 | 6 | 6 | 6 | 0 |
| 3 | 480 | 1 | 999.5 | 508574 | 7777 | 8 | 8 | 8 | 0 |
| 4 | 180 | 17 | 1119.1 | 481888 | 7607 | 27 | 27 | 27 | 0 |
| 5 | 500 | 0 | 916.0 | 490160 | 7501 | 1 | 1 | 1 | 0 |
| 6 | 320 | 17 | 891.0 | 578362 | 9373 | 27 | 27 | 27 | 0 |
| 7 | 340 | 8 | 925.7 | 682861 | 10928 | 14 | 14 | 14 | 0 |
| 8 | 380 | 6 | 941.2 | 568122 | 9092 | 56 | 56 | 8 | 48 |
| 9 | 460 | 2 | 1022.2 | 535754 | 8045 | 9 | 9 | 9 | 0 |
| 10 | 440 | 4 | 1228.2 | 534360 | 8552 | 18 | 18 | 13 | 5 |

RB-D summary: 10/10 accepted, 10/10 hero survived under HP override, 73 hero hits, 1460 HP projectile damage, max framework overhead `1228.2 us`. Lightweight totals: `5,473,588` attempts, `86,982` distance-passed, `183` LOS-passed, `183` dispatched, `130` spawned, `53` spawn failed, `73` hero hits.

Root cause:

- Rich Ranged reaches the fire-dispatch path. It is not blocked by a missing tick, permanent cooldown, status gating, distance only, or LOS only.
- The break is at projectile spawn/dispatch: RA-D produced `RichLOSPassed=221`, `RichProjectilesDispatched=221`, `RichProjectilesSpawned=0`, and `RichSpawnFailed=221`.
- Lightweight Ranged on the same binary spawned and hit from the same aggregate pipeline shape: `LightweightProjectilesDispatched=183`, `LightweightProjectilesSpawned=130`, `LightweightProjectilesHitHero=73`.
- B.10.1D should fix rich `AT66RangedEnemy` projectile spawn failure in staged standalone, likely around projectile class resolution/cook availability, spawn transform, owner/instigator, collision handling, or class/type constraints in `FireProjectileAtPlayer`.

Measurement caveat:

- RA-D rows 1, 4, and 9 observed `git-lfs.exe` after `T66.exe` exited and are marked `PostCaptureGitContaminated=true`. Those rows are valid for in-process counter evidence because `[RangedDecisionSummary]` is emitted before process exit, but their FPS values are diagnostic-only and not B.10 acceptance-grade.
- Existing B.10 acceptance methodology still requires clean capture conditions and should not reuse this Git/LFS relaxation for FPS gates without a separate review.

Proposed B.10.1D scope:

- Fix rich Ranged projectile spawn failure without changing lightweight Ranged pressure.
- Add focused proof that rich Ranged can spawn projectiles and hit the HP-overridden autocapture hero.
- Re-run B.10 acceptance only after the rich fix lands.

## Pass B.10.1D Projectile Manager and HISM Rendering

Status: implementation and smoke proof completed; B.10.1D acceptance is blocked by ranged-active measurement/gameplay variability under the fixed HP500 contract. No B.10 acceptance claim is made.

Review and packet artifacts:

- Plan packet: `C:\UE\T66\Saved\AgentReviews\20260527T_b101d_projectile_manager_hism\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527T_b101d_projectile_manager_hism\20260527T071750-pass8\claude_review_pass8.md`
- Combined packet: `C:\UE\T66\PerformanceSystem\B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`

Implementation summary:

- Added `UT66ProjectileManagerSubsystem` as a world subsystem with a fixed 256-slot flat array of enemy projectile records.
- Enemy projectiles are rendered through a manager-owned `UHierarchicalInstancedStaticMeshComponent` instead of per-projectile actors.
- Rich `AT66RangedEnemy` and lightweight `AT66MobBase` Ranged both fire through `UT66ProjectileManagerSubsystem::FireProjectile`.
- `AT66EnemyProjectileBase` remains in the codebase for asset/data compatibility and is marked deprecated for a future cleanup pass.
- The manager records terminal `[ProjectileManagerSummary]` counters: fired, active peak, hero hits, world hits, dropped fires, apply-damage-false returns, manager tick timing, HISM update timing, render bounds, and last world impact actor.
- PerformanceSystem projectile board samples now include manager-owned active projectiles.
- During smoke validation, the first non-hero sweep implementation over-counted trigger volumes such as `T66TowerDescentHole.TriggerBox`; the final manager filters non-hero hits by component response to the projectile object channel before treating them as blocking impacts.

Build and stage verification:

- Focused Development build passed with `C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex`.
- Known pre-existing warning remained: `Source\T66Mini\T66Mini.Build.cs` references missing `Public\UI\Components`.
- Staged standalone refresh passed with `C:\UE\T66\Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`.
- `C:\UE\T66\T66 Standalone.lnk` and the pinned taskbar shortcut were refreshed to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.

Smoke proof:

| Smoke | Log | Screenshot | Result | Key evidence |
| --- | --- | --- | --- | --- |
| Rich isolated | `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1D\T66_B101D_RichRangedSmoke_Isolated_Final.log` | `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\projectile_manager_smoke\isolated_rich_final.png` | Passed | `ProjectileTravelAssertion managerFired=2 active=2 result=PASS`; `HeroDamageAssertion initialHP=100.0 currentHP=80.0 result=PASS`; `[CombatDamage] SourceID=HexSlinger Delivery=EnemyProjectile SourceClass=T66RangedEnemy`; summary `Fired=4 HitHero=4 HitWorld=0 DroppedFires=0 ManagerTickMaxUs=342.7 HISMUpdateMaxUs=60.3`. |
| Lightweight isolated | `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1D\T66_B101D_LightweightRangedSmoke_Isolated_Final.log` | `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\projectile_manager_smoke\isolated_lightweight_final.png` | Passed | `ProjectileTravelAssertion managerFired=1 active=1 result=PASS`; `HeroDamageAssertion initialHP=100.0 currentHP=80.0 result=PASS`; `[CombatDamage] SourceID=HexSlinger Delivery=EnemyProjectile SourceClass=T66MobBase`; summary `Fired=2 HitHero=2 HitWorld=0 DroppedFires=0 ManagerTickMaxUs=447.4 HISMUpdateMaxUs=83.0`. |

Preliminary acceptance attempt before Git-status abort:

- Staged executable SHA256: `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B`
- CVar-off completed 3/3 accepted before the later CVar-on run aborted on two long-lived `git status --porcelain` processes. Because the whole matrix did not finish, this is historical evidence only, not acceptance.

| Set | Run | Terminal | Hero HP | Hero hits | Fired | Hit hero | Avg FPS | Overhead us | Rejected |
| --- | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| CVarOff | 1 | Completed | 240 | 13 | 23 | 13 | 150.77 | 908.0 | No |
| CVarOff | 2 | Completed | 300 | 12 | 26 | 12 | 151.58 | 1186.8 | No |
| CVarOff | 3 | Completed | 200 | 19 | 21 | 19 | 171.52 | 741.2 | No |
| CVarOn | 1 | Completed | 400 | 6 | 9 | 6 | 172.25 | 714.0 | No |

Validation-runner infrastructure correction:

- The first acceptance attempt stopped after CVar-on capture 2 because two `git.exe -c core.hooksPath=NUL -c core.fsmonitor=false status --porcelain` processes stayed alive beyond the runner's `300s` transient Git wait.
- The validation runner was adjusted so status-only Git waits are recorded as non-fatal after a short wait; `git-lfs.exe` activity is still waited and recorded.
- The two stale status processes were then explicitly terminated. They were both created at `2026-05-27 08:36:58` and had command line `git.exe -c core.hooksPath=NUL -c core.fsmonitor=false status --porcelain`.

Final clean acceptance rerun:

- Staged executable SHA256 remained stable across pass-start, per-capture, and post-capture checks: `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B`
- No `git-lfs.exe`, `git status`, `RunUAT`, `UnrealEditor-Cmd`, or staged `T66.exe` conflict was recorded during the final clean rerun.
- CVar-off halted after two rejected captures, per standing methodology. CVar-on was not run because no accepted CVar-off baseline was established.

| Set | Run | Terminal | Reject reason | Hero HP | Hero hits | Fired | Hit hero | Rich LOS passed | Rich LOS blocked | Avg FPS | Overhead us | Manager tick max us | HISM max us |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOff | 1 | HeroDied | HeroDeath | 0 | 34 | 41 | 33 | 41 | 85 | 85.77 | 1529.7 | 709.6 | 630.0 |
| CVarOff | 2 | Completed | None | 460 | 2 | 7 | 2 | 7 | 1532 | 87.10 | 1434.7 | 360.5 | 91.7 |
| CVarOff | 3 | Completed | NoProjectilesFired | 500 | 0 | 0 | 0 | 0 | 3840 | 92.37 | 1318.2 | 0.0 | 0.0 |

Acceptance outcome:

- Projectile manager capacity and rendering gates passed in every completed clean row: `DroppedFires=0`, `ManagerTickMaxUs<=709.6 us`, `HISMUpdateMaxUs<=630.0 us`, `HISMUpdateAvgUs<=4.6 us`.
- PerformanceSystem overhead remained below the `10000 us` rejection threshold in all clean rows.
- The implementation proves rich and lightweight Ranged can fire, render, and damage through the manager in focused smoke.
- The acceptance matrix did not pass because the fixed HP500 stationary-hero contract is no longer robust once rich Ranged projectiles are functional, and rich Ranged still has high-variance LOS outcomes in saturated CVar-off captures.
- The old CVar-off comparators remain superseded conceptually because they measured broken rich projectile spawning, but no new authoritative CVar-off baseline can be adopted from this blocked pass.

Open follow-up:

- A reviewed follow-up packet should decide the measurement contract before another acceptance run. The immediate options are to raise the autocapture HP override for ranged-active performance captures, revise the saturated stationary-hero scenario, and/or fix rich Ranged LOS/positioning variance so CVar-off cannot produce `NoProjectilesFired` captures.
- B.10.1D did retire the actor `SpawnActor` projectile path for enemy projectiles; do not re-open the rich `SpawnActor` nullptr issue as a spawn-path patch unless the manager architecture is reverted.
- Projectile HISM rendering is complete for enemy projectiles; B.11 and B.12 remain mob-manager work, and B.13 remains mob VAT/ISM work.

### B.10.1D Resume - HP2000 Measurement Contract Attempt

Status: halted. The reviewed continuation packet approved one no-source-change acceptance rerun using the existing runner with `-HeroHPOverride 2000`, while preserving `NoProjectilesFired` as a hard reject.

Artifacts:

- Continuation packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume_measurement_contract\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume_measurement_contract\20260527T091338-pass1\claude_review_pass1.md`
- Combined packet update: `C:\UE\T66\PerformanceSystem\B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`
- Captured row/log: `C:\UE\T66\Saved\StandaloneLogs\LightweightActorB10_1D\T66_B101D_CVarOff_01.log`

Preflight and command:

- Preflight found `git-lfs.exe` plus a stale `git status --porcelain` worker. `git-lfs.exe` drained; the status worker PID `46904` persisted through a five-minute wait and was terminated before Unreal launch.
- Staged executable SHA256 before launch: `CF70BC921BA930115837775F96F2277040A3A78D09BC6C44FBBB6A730D62F20B`
- Command:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\run_b101d_projectile_manager_validation.ps1 -Mode Acceptance -AcceptanceCount 3 -HeroHPOverride 2000`

Captured CVar-off row:

| Run | Terminal | Reject reason | Requested HP | Applied HP | Hero hits | Fired | Rich dist pass | Rich LOS pass | Rich LOS blocked | Avg FPS | Overhead us |
| ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | Completed | NoProjectilesFired | 2000 | 1000 | 0 | 0 | 103 | 0 | 103 | 157.93 | 689.0 |

Findings:

- The command-line override was parsed and recorded as `2000.0`, but `UT66RunStateSubsystem::ApplyAutomationHeroHPOverride` capped the applied value at `1000.0`.
- The first CVar-off row reproduced the zero-fire path: `RichDistancePassed=103`, `RichLOSBlocked=103`, `RichLOSPassed=0`, `RichProjectilesDispatched=0`, and `ProjectileManagerSummary Fired=0`.
- The runner then stopped before capture 2 because `UnrealEditor-Cmd.exe` appeared in the clean-environment gate. The process was gone by inspection, but the gate correctly prevented continuing the acceptance matrix under contamination.
- PerformanceSystem overhead remained healthy (`689.0 us`); the halt was measurement/LOS plus environment hygiene, not framework overhead.

Next required packet:

- Fix the automation HP cap so the reviewed ranged-active measurement HP can actually apply, without affecting normal gameplay.
- Add bounded aggregate LOS blocker attribution, not per-frame logs.
- Fix rich Ranged LOS/occlusion behavior enough that CVar-off cannot produce `NoProjectilesFired` saturated captures.
- Rebuild/stage, preserve binary hash provenance, rerun smoke and B.10.1D acceptance.

### B.10.1D Resume2 - Rich Ranged LOS Blocker Attribution Probe

Status: completed as a diagnostics-only pass. No HP-cap change, LOS/collision behavior fix, projectile sweep change, or B.10.1D acceptance rerun was performed.

Artifacts:

- Review packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_hp_cap_los_fix\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_hp_cap_los_fix\20260527T095018-pass7\claude_review_pass7.md`
- Combined packet: `C:\UE\T66\PerformanceSystem\B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`
- Probe rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows.jsonl`

Implementation:

- Added bounded LOS blocker buckets to `RangedDecisionSummary` for both rich and lightweight firing paths:
  - `WorldStatic`, `WorldDynamic`, `RichEnemy`, `LightweightMob`, `OtherPawn`, and `Unknown`.
- The LOS helpers reuse the existing visibility trace hit result; no extra LOS trace was added.
- Attribution remains gated by `T66.Ranged.DiagnosticLogging=1`.
- The validation runner now waits up to `120s` for transient capture blockers (`RunUAT`, `UnrealEditor-Cmd`, staged `T66.exe`) before failing the clean-environment gate.
- A harness-only `CVarOffProbe` mode was added so the reviewed CVar-off probe could run without launching the full B.10.1D acceptance matrix.

Build/stage/provenance:

- Focused Development build passed.
- Stage refresh passed with `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild`.
- Standalone shortcuts point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Staged executable SHA256 stayed stable across smoke and probes: `5FB9EECF1C2B25B79DDFC5CA07D221AA1C4BE986636FA732409AD011B7A13589`.

Diagnostic-off guard:

- `T66.Ranged.DiagnosticLogging=0` emitted no `RangedDecisionSummary`, no `RangedFireDecision`, and no `RangedPressureDiagnostic` lines.
- The only `LOSBlocker` strings in that log were pre-existing `[CombatDamage] ... LOSBlocker=None` fields.

Smoke after attribution build:

| Set | Hero HP | Hero hits | Fired | Hit hero | Hit world | Rich spawns | Lightweight spawns | Avg FPS | Overhead us |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 640 | 19 | 33 | 19 | 14 | 35 | 0 | 145.14 | 515.6 |
| CVarOnSmoke | 940 | 3 | 4 | 3 | 1 | 0 | 17 | 172.39 | 561.3 |

Smoke blocker attribution:

| Set | LOS blocked | RichEnemy blockers | LightweightMob blockers | LOS passed |
| --- | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 256 | 256 | 0 | 33 |
| CVarOnSmoke | 801 | 0 | 801 | 4 |

CVar-off probe:

| Run | Rejected | Reject reason | Hero HP | Rich spawns | Rich attempts | Distance passed | LOS blocked | WorldStatic blockers | RichEnemy blockers | LOS passed | Fired | Hit hero | Hit world | Avg FPS | Overhead us |
| ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | No | None | 820 | 33 | 206343 | 457 | 403 | 12 | 391 | 54 | 54 | 16 | 37 | 139.65 | 526.5 |
| 2 | No | None | 880 | 19 | 125162 | 42 | 28 | 0 | 28 | 14 | 14 | 6 | 8 | 144.43 | 1827.7 |
| 3 | Yes | NoProjectilesFired | 1000 | 25 | 157205 | 815 | 815 | 0 | 815 | 0 | 0 | 0 | 0 | 144.88 | 0.0 |

Finding:

- The intermittent `NoProjectilesFired` mode is now attributed to peer rich enemy capsule self-occlusion on the visibility LOS trace.
- Probe run 3 reached the distance-passed firing decision `815` times, and all `815` LOS failures were classified as `RichEnemy`.
- No PerformanceSystem overhead issue, HP issue, projectile manager capacity issue, or spawn failure explains this specific no-fire row.
- Accepted probe rows show the same dominant pattern even when some projectiles fire: run 1 had `391/403` LOS blocks from rich enemies, and run 2 had `28/28`.

Next reviewed scope:

- Keep `UT66ProjectileManagerSubsystem` and HISM rendering.
- Keep bounded aggregate diagnostics.
- Review and fix the measurement HP cap so the requested ranged-active HP value can apply.
- Review and fix rich Ranged LOS/collision behavior so peer rich enemies do not starve CVar-off LOS checks in saturated captures.
- Rerun smoke and B.10.1D acceptance only after those fixes land.

### B.10.1D Resume3 - HP2000 Cap, Rich LOS Ignore, Projectile Peer Filtering

Status: implemented and smoke-passed, but acceptance halted in CVar-off on hero deaths before the CVar-on set could run.

Artifacts:

- Review packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume3_hp_cap_rich_los_acceptance\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume3_hp_cap_rich_los_acceptance\20260527T105609-pass7\claude_review_pass7.md`
- Combined packet: `C:\UE\T66\PerformanceSystem\B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`

Implementation:

- Raised the automation-only hero HP override cap to `2000.f`; logs confirmed `AppliedHP=2000.0 RequestedHP=2000.0 MaxHP=2000.0`.
- Rich Ranged projectile LOS now ignores registered rich enemy actors so peer rich bodies cannot starve saturated CVar-off LOS checks.
- Enemy projectiles now pass through rich and lightweight peer enemy bodies while preserving hero and world collision.

Verification:

- Focused Development build passed.
- Standalone stage passed and both standalone shortcuts target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Stable staged SHA256: `782F98AAA4D5269D450A94FBA7345E54B335C91F53BFDD61D324258569962C9D`.
- Smoke passed:

| Set | Exit | Hero HP | Hero hits | Fired | Hit hero | Rich LOS passed | Rich LOS blocked | Avg FPS | Overhead us |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 0 | 1640 | 18 | 18 | 18 | 18 | 0 | 166.47 | 0.0 |
| CVarOnSmoke | 0 | 1740 | 19 | 19 | 19 | 0 | 0 | 208.96 | 0.0 |

CVar-off acceptance rows before halt:

| Run | Terminal | Rejected | Hero HP | Hero hits | Fired | Hit hero | Rich LOS passed | Rich LOS blocked | Avg FPS | Overhead us |
| ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | Completed | No | 800 | 118 | 119 | 118 | 119 | 0 | 178.36 | 754.2 |
| 2 | Completed | No | 800 | 118 | 119 | 118 | 119 | 0 | 180.47 | 888.8 |
| 3 | HeroDied | Yes | 0 | 194 | 194 | 193 | 194 | 0 | 163.85 | 686.4 |
| 4 | Completed | No | 760 | 62 | 62 | 62 | 62 | 0 | 173.52 | 845.5 |
| 5 | Completed | No | 1600 | 39 | 40 | 39 | 40 | 0 | 172.97 | 936.6 |
| 6 | HeroDied | Yes | 0 | 262 | 264 | 261 | 264 | 0 | 166.20 | 531.6 |

Finding:

- The rich LOS self-occlusion blocker is fixed in this build: every CVar-off row had `RichLOSBlocked=0` and `RichLOSBlockerRichEnemy=0`.
- Projectile manager capacity, HISM update cost, PerformanceSystem overhead, binary provenance, and Git/LFS hygiene were all clean in the acceptance attempt.
- B.10.1D acceptance remains blocked because HP2000 is not enough for a stationary saturated CVar-off ranged-active capture once rich Ranged fires freely. CVar-on was not run after the halt, so no new authoritative baseline exists.

Next reviewed scope:

- Decide the ranged-active measurement contract and/or rich saturated projectile pressure target before rerunning B.10.1D acceptance.
- Update the validation harness for this pass family so a `HeroDeath` reject halts immediately when the reviewed packet says to stop on first hero death.

### B.10.1D Resume4 - Measurement HP Contract and HeroDeath Harness Halt

Status: HP survival fixed and smoke passed. B.10.1D acceptance remains blocked after the 10-capture escalation because CVar-on intermittently violates route validity by reporting rich spawns/fire attempts under `UseLightweight=1`.

Artifacts:

- Review packet: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume4_measurement_contract_acceptance\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Saved\AgentReviews\20260527_B10_1D_resume4_measurement_contract_acceptance\20260527T223640-pass2\claude_review_pass2.md`
- Combined packet: `C:\UE\T66\PerformanceSystem\B10_1D_ProjectileManager_HISM_ConsolidatedPacket.md`

Implementation:

- Raised `UT66RunStateSubsystem::ApplyAutomationHeroHPOverride` cap to `50000.f` and documented that the high cap is automation-only for stationary performance captures.
- Updated the B.10.1D validation runner so rejected rows with `TerminalStatus=HeroDied`, `RejectReasons=HeroDeath`, or `RejectReasons=HeroDied` halt immediately.
- Preserved the existing second-reject halt behavior for non-HeroDeath rejects.

Build/stage/provenance:

- Focused Development build passed.
- Stage refresh passed.
- Both standalone shortcuts target `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Stable staged SHA256: `0A0AC836F224B898353CD7FA59B5A58ECC24D7676F6903DFD765CA9A3D9252EB`.

Smoke with HP20000:

| Set | Hero HP | Hero hits | Damage HP | Fired | Hit hero | Avg FPS | Overhead us |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOffSmoke | 19260 | 112 | 2240 | 112 | 112 | 153.64 | 513.3 |
| CVarOnSmoke | 19780 | 25 | 500 | 25 | 25 | 171.86 | 0.0 |

The logs confirmed `AppliedHP=RequestedHP=MaxHP=CurrentHP=20000.0`.

3+3 attempts:

- Attempt 1: CVar-off had only 2 accepted rows and 1 `NoProjectilesFired` reject; CVar-on had 3 accepted rows.
- Attempt 2: CVar-off again had only 2 accepted rows and 1 `NoProjectilesFired` reject; CVar-on had 3 accepted rows but high stdev triggered the reviewed 10-capture escalation.

10-capture escalation:

- Artifact: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows_acceptance_hp20000_attempt3_escalated_halted_routevalidity.jsonl`
- CVar-off completed 10/10 accepted rows:

| Metric | Value |
| --- | ---: |
| CVar-off accepted rows | 10 |
| Hero deaths | 0 |
| Median FPS | 157.68 |
| Mean FPS | 156.62 |
| Stdev FPS | 7.65 |
| Max overhead | 805.9 us |
| Total fired | 2519 |
| Total hit hero | 2506 |
| Dropped fires | 0 |

- CVar-on halted after 5 rows:

| Metric | Value |
| --- | ---: |
| CVar-on rows before halt | 5 |
| Accepted rows | 3 |
| Rejected rows | 2 |
| Reject reason | RouteValidity |
| Hero deaths | 0 |
| Max overhead | 988.8 us |
| Dropped fires | 0 |

Finding:

- The HP measurement contract is fixed for the tested pressure range; no HP20000 smoke or acceptance row killed the hero.
- PerformanceSystem overhead, Git/LFS, HISM, dropped fires, and binary provenance were clean in the escalated set.
- Acceptance remains blocked because the CVar-on route contract is not stable: rejected rows had `UseLightweight=1`, `RouteRanged=1`, `LightweightSpawns>0`, but also `RichSpawns=1` and nonzero rich fire attempts.
- The runner also still exposes a replacement-semantics problem: fixed-count 3-capture sets can finish with fewer than 3 accepted rows after a single non-HeroDeath reject.

Next reviewed scope:

- Investigate and fix the intermittent CVar-on rich-spawn leakage under lightweight routing.
- Decide whether the acceptance harness should collect until the requested accepted-row count is reached, while preserving immediate halt for `HeroDeath` and second-reject halt for other reject classes.
### B.10.1D Resume5 - Route Leakage Diagnostic

Status: diagnostic complete. No routing fix, Ranged behavior change, or B.10 acceptance reattempt was performed.

Artifacts:

- Original plan packet: `C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\plan_packet.md`
- Claude greenlight: `C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\20260528T002135-pass2\claude_review_pass2.md`
- Gate amendment packet: `C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\gate_amendment_packet.md`
- Gate amendment greenlight: `C:\UE\T66\Reports\AgentReviews\20260528_B10_1D_resume5_route_leakage_diagnostic\20260528T004840-pass5\claude_review_pass5.md`
- Final rows: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_rows.jsonl`
- Final results: `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\capture_results.json`
- Intermediate control/diagnostic rows archived under:
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\aborted_control_runs\`
  - `C:\UE\T66\Saved\Codex\Performance\LightweightActorB10_1D\intermediate_route_diagnostic_runs\`

Implementation:

- Added aggregate `RouteAttributionSummary` diagnostics to `UT66MobManagerSubsystem`, gated by `T66.Ranged.DiagnosticLogging=1`.
- Instrumented the director rich/lightweight routing decision points for initial population and runtime trickle spawns.
- Instrumented known non-director rich spawn paths as aggregate `RoutedRich_NonDirectorPath` evidence: tower guardian, lab spawn, test room spawn, tutorial spawn, and gameplay automation helper spawns.
- Updated the B.10.1D validation runner with `RouteDiagnostic` mode.
- Route validity failures are evidence, not hard rejects, in CVar-on route diagnostic rows; hard hygiene rejects still apply for non-zero exit, `HeroDeath`, PerformanceSystem overhead over `10000 us`, dropped fires, missing summaries, route-counter mismatch, and binary hash drift.
- Added opt-in `-AllowHighFpsControlAdvisory` after the CVar-off control row exceeded the previous Resume4 high bound while all structural counters were clean. Default runner behavior remains strict.
- Added `-RunRouteControlGateSelfTest`; it passed:
  - high FPS blocks when the advisory switch is absent
  - high FPS is allowed when the switch is present and structural gates are clean
  - low FPS still blocks even when the switch is present

Build, stage, and provenance:

- `Scripts\StageStandaloneBuild.ps1 -ClientConfig Development` passed.
- Existing build warnings only:
  - `Source\T66Mini\T66Mini.Build.cs` references missing `Public\UI\Components`.
  - `Source\T66\Gameplay\T66Hero1AxeAOEVFXLabActor.cpp` uses deprecated Niagara `IsReadyToRun`.
- Both standalone shortcuts were refreshed to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Stable staged SHA256 for the final diagnostic: `D1E3235ED789C2596626BF6748F3DE49018B883D99F941B6160D860C535192FF`.

Control-gate observations:

| Row | Hygiene | Avg FPS | Fired | Hit hero | Ranged total | Overhead us | Action |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| Discarded control | stale `git status --porcelain` active at launch | 172.04 | 189 | 187 | 36 | 803.7 | discarded as hygiene-unclean |
| Clean strict-control retry | clean | 168.93 | 135 | 135 | 24 | 724.8 | halted by old high-FPS gate |
| Final opt-in advisory control | clean | 170.63 | 118 | 117 | 27 | 714.1 | accepted with `RouteControlHighFpsAdvisory` |

The high-FPS drift is documented as a measurement-surface concern, not acceptance evidence. The final control remained structurally valid (`RouteValid=true`, `CounterMismatch=0`, one summary line per subsystem, no dropped fires, no overhead reject), and the pass objective was categorical route attribution rather than FPS acceptance.

Final route diagnostic matrix:

| Set | Run | Route leak observed | Rich ranged spawns | Lightweight ranged spawns | Mini-boss rich slots | Special rich slots | Ranged rich fallback | Ranged rich mini-boss | Fired | Hit hero | Avg FPS | Overhead us |
| --- | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| CVarOffRouteControl | 1 | No | 27 | 0 | 0 | 0 | 0 | 0 | 118 | 117 | 170.63 | 714.1 |
| CVarOnRouteDiagnostic | 1 | No | 0 | 20 | 0 | 2 | 0 | 0 | 2 | 2 | 179.38 | 1075.8 |
| CVarOnRouteDiagnostic | 2 | Yes | 0 | 29 | 1 | 3 | 0 | 0 | 9 | 9 | 175.24 | 970.5 |
| CVarOnRouteDiagnostic | 3 | No | 0 | 25 | 0 | 2 | 0 | 0 | 9 | 9 | 177.59 | 984.7 |
| CVarOnRouteDiagnostic | 4 | No | 0 | 19 | 0 | 2 | 0 | 0 | 18 | 18 | 160.09 | 745.4 |
| CVarOnRouteDiagnostic | 5 | No | 0 | 34 | 0 | 0 | 0 | 0 | 15 | 15 | 161.94 | 782.7 |
| CVarOnRouteDiagnosticExtension | 1 | No | 0 | 27 | 0 | 0 | 0 | 0 | 18 | 18 | 172.17 | 729.1 |
| CVarOnRouteDiagnosticExtension | 2 | No | 0 | 23 | 0 | 0 | 0 | 0 | 17 | 17 | 162.75 | 824.4 |
| CVarOnRouteDiagnosticExtension | 3 | No | 0 | 19 | 0 | 2 | 0 | 0 | 8 | 8 | 158.15 | 742.5 |
| CVarOnRouteDiagnosticExtension | 4 | No | 0 | 37 | 0 | 0 | 0 | 0 | 22 | 22 | 173.52 | 798.0 |
| CVarOnRouteDiagnosticExtension | 5 | No | 0 | 26 | 0 | 0 | 0 | 0 | 22 | 22 | 163.95 | 794.3 |

CVar-on aggregate route attribution across 10 rows:

| Family | Total observed | Routed lightweight basic | Rich special/mini-boss | Rich mini-boss promotion | Rich fallback | Rich non-director |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Melee | 372 | 372 | 0 | 0 | 0 | 0 |
| Rush | 119 | 119 | 0 | 0 | 0 | 0 |
| Flying | 139 | 138 | 0 | 1 | 0 | 0 |
| Ranged | 259 | 259 | 0 | 0 | 0 | 0 |
| SpecialUnknown | 11 | 0 | 11 | 0 | 0 | 0 |

Findings:

- The specific Ranged rich-route leak did not reproduce in the final 10 CVar-on diagnostic captures: `RangedRoutedLightweightBasic=259`, all Ranged rich reason buckets were `0`, `RichSpawns=0`, and rich fire attempts were `0` in every CVar-on row.
- The route-attribution leak that did reproduce was one planned mini-boss promotion, and it landed on the Flying family in CVar-on row 2 (`FlyingRoutedRichMiniBossPromotion=1`).
- Source audit explains how this can also produce the earlier Resume4 Ranged `RichSpawns=1` rejects:
  - `AT66EnemyDirector::ShouldRouteSpawnToLightweightMob` explicitly returns false for `bIsMiniBoss`.
  - Runtime waves choose `MiniBossIndex` from regular mob slots before the final `MobID`/family is rolled.
  - Therefore mini-boss promotion is family-neutral and can land on Melee, Rush, Flying, or Ranged.
  - If that promotion lands on a Ranged `MobID`, the row correctly records a rich Ranged spawn/fire attempt under `UseLightweight=1`, which the previous route-validity gate treated as an illegal leak.
- Special wave spawns are also expected rich routes: `SpecialUnknownRoutedRichSpecialOrMiniBoss=11` across 10 rows. They came through the director (`NonDirectorObservedSpawns=0`) and are not a second spawn path.
- No evidence was found for a per-instance routing race, family lookup failure, fallback branch leak, lightweight acquire failure, or non-director spawner causing the Resume4 route-validity failures.

Spawn-path audit:

- Director initial population: respects `T66.Mob.UseLightweight` and records `InitialPopulation`.
- Director runtime trickle: respects `T66.Mob.UseLightweight`; planned special spawns and mini-boss promotions stay rich by design and record their reason.
- Tower gate guardian: non-director special/guardian rich route; not present in standard `enemywaveperf` rows.
- Lab/test-room/tutorial/gameplay automation helper spawns: direct rich debug/tutorial paths now record `NonDirector`; not present in the final `enemywaveperf` route diagnostic (`NonDirectorObservedSpawns=0`).

Root-cause conclusion:

- The observed CVar-on route-validity rejects are most likely the acceptance harness treating planned rich routes as illegal leakage.
- For the specific `RichSpawns=1` Ranged rejects, the likely source is family-neutral mini-boss promotion landing on a Ranged `MobID`, not a random routing race or second spawn path.
- The fix should be a reviewed measurement/route-validity decision, not an unreviewed behavior patch:
  1. For pure lightweight-family acceptance, disable mini-boss/special wave spawns in `enemywaveperf`, or
  2. Teach the route-validity gate to allow planned rich special/mini-boss routes while still rejecting fallback, lookup-failed, non-director, or basic-family rich leakage, or
  3. Change the gameplay routing contract so migrated-family mini-boss promotions also route lightweight. This is a design decision because current code intentionally keeps mini-boss/special routes rich.

Next reviewed scope:

- Decide the B.10.1D acceptance contract for special and mini-boss routes.
- Update route-validity gating accordingly.
- Rerun B.10.1D acceptance after the measurement contract is explicit.

## Pass B.11/B.12 Lightweight-Only Divorce and VAT State Ownership

Date: 2026-05-28

Review and process:

- One consolidated plan packet was reviewed at `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/20260528T105634-pass4/claude_review_pass4.md`.
- Claude returned `Verdict: APPROVE`; under the updated `AGENTS.md` workflow, a valid Claude approval authorizes implementation for the reviewed scope without a second manual approval prompt.
- Implementation and FPS capture were done in isolated source tree `C:\UE\T66_B11B12_Worktree` to avoid unrelated dirty runtime/content changes in `C:\UE\T66`.
- The verified source edits were then copied back to live `C:\UE\T66` only after targeted files matched the isolated pre-change manifest or were clean. Copy manifest: `Reports/AgentReviews/20260528_B11_B12_MultiWorkstream/isolated_to_live_copy_manifest.csv`.

Source-state decision:

- The live tree contained unrelated runtime/content work, so Stage 0a was not measured from the whole dirty tree.
- Stage 0a used the isolated baseline source state, including the expected Lightweight Actor/enemy infrastructure and Hero_2 Chad AnimatedToonStyle content completeness sentinels.
- The missing Hero_2 staged-content concern was checked during staging; no Hero_2_Chad missing-content cook spam blocked the pass.

File ownership map:

| Workstream | Owner | Files |
| --- | --- | --- |
| Basic-mob rich divorce | Dirac | `Source/T66/Gameplay/T66EnemyDirector.cpp` |
| VAT state ownership and touch-overlap CVar neutralization | Main | `Source/T66/Gameplay/T66MobBase.h/.cpp`, `Source/T66/Gameplay/T66MobManagerSubsystem.h/.cpp` |
| Deferred proof hooks | Faraday + Main exit fix | `Source/T66/Gameplay/GameMode/T66GameMode_Tower.cpp`, `Source/T66/Gameplay/T66TowerDescentHole.h/.cpp`, `Source/T66/Gameplay/T66ProjectileManagerSubsystem.h/.cpp` |
| Minor cleanup | Herschel + Main Gambler edit | `Source/T66/Core/T66ActorRegistrySubsystem.h/.cpp`, `Source/T66/Gameplay/Traps/T66TrapArrowProjectile.h/.cpp`, `Source/T66/UI/Gambler/T66CasinoGamblerTabWidget_Economy.cpp` |

Phase 1 lightweight baseline:

| Metric | Value |
| --- | ---: |
| Accepted captures | 3/3 |
| Median FPS | 192.80 |
| Mean FPS | 193.71 |
| Stdev FPS | 5.80 |
| Max PerformanceSystem overhead | 1011.4 us |
| Staged SHA256 | `86EDE7D6F2533614D9E0525305230BC06CD468F1547642A47C6A2B5C1613C9F5` |
| Zero non-zero exits / HeroDeath / overhead rejects | Yes |

Phase 1 row summary:

| Run | Avg FPS | Overhead us | Fired | Hit hero | Hero HP | Peak lightweight | Peak rich | Ranged lightweight basic |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 199.91 | 837.0 | 45 | 45 | 19560 | 90 | 0 | 26 |
| 2 | 192.80 | 1011.4 | 26 | 26 | 19580 | 90 | 0 | 25 |
| 3 | 188.41 | 920.2 | 63 | 63 | 19400 | 90 | 0 | 22 |

Notes:

- `RouteBossOrGuardianObserved=1` in each Phase 1 row is expected for the placed guardian/non-director actor and is not basic-mob rich leakage.
- Row 3 observed Git/LFS before launch and after exit, but the runner waited before starting Unreal and after exit. The executable hash stayed stable and no overhead reject occurred.
- This median is the authoritative lightweight baseline for the post-projectile-manager, post-placed-miniboss source state and closes B.10 on the lightweight-only measurement contract.

Implementation summary:

- Basic mobs now route lightweight unconditionally. Melee, Rush, Flying, and Ranged basic-family spawns no longer consult `T66.Mob.UseLightweight`, `RouteFlyingLightweight`, or `RouteRangedLightweight` for routing. Those CVars and the rich-basic routing branch are marked deprecated for the later cleanup pass.
- Minibosses, specials, guardians, and bosses still route rich intentionally.
- VAT runtime state moved out of `AT66MobBase` into manager-owned flat per-mob state (`FT66MobVertexAnimationRuntimeState`), with custom-data fields laid out for the future B.13 HISM/per-instance-custom-data pass.
- Dynamic material instance application still happens this pass; B.13 owns the render-swap/custom-data application.
- The actor-resident VAT fields and actor-side VAT mutation were removed. Manager tick owns VAT advancement.
- `T66.Mob.Diagnostics.UseTouchDamageOverlap` is neutralized/deprecated; the chosen touch-damage path is now always used.
- Bosses are included in damageable-target registry iteration and boss registration/unregistration now broadcasts `EnemiesChanged`.
- Gambler boss spawn ownership is centralized through `AT66PlayerController::TriggerCasinoBossIfAngry`; the Gambler UI no longer owns a fallback `SpawnActor` path.
- Routine trap arrow projectile fire/impact telemetry is demoted to `VeryVerbose`; warnings/errors remain at normal levels.

Runtime proof hooks:

| Proof | Result | Evidence |
| --- | --- | --- |
| Tick/VAT runtime proof | Pass | `ActorTickEnabled=0`, `ComponentTickEnabled=0`, `ComponentTickRegistered=0`, `FamiliesPresent=4`, `ClipSamples=16`, `ClipSamplesWithFrameChange=16`, `PoolReuseResetPasses=4` |
| Floors 2/3/4 miniboss traversal | Pass | Each floor guardian spawned, blocked while alive, unblocked after death, and allowed interaction after death |
| Boss projectile kill-mid-flight | Pass | `FireBossProjectile=1`, `SourceDestroyed=1`, `DroppedInvalidSourceDelta=1`, `PostDeathHitDelta=0`, `PostDeathDamage=0` |

The tick/VAT proof initially failed because lightweight mob components still had ticking enabled even though actor tick was off. The pass fixed `AT66MobBase` component tick state for capsule, visual mesh, lock indicator, body hit zone, and head hit zone, then reran the proof successfully.

Phase 3 final lightweight health check:

| Metric | Value |
| --- | ---: |
| Accepted captures | 3/3 |
| Median FPS | 200.99 |
| Mean FPS | 192.04 |
| Stdev FPS | 16.89 |
| Max PerformanceSystem overhead | 911.8 us |
| Total projectiles fired / hit hero | 22 / 22 |
| Hero deaths / rejected captures / overhead rejects | 0 / 0 / 0 |
| Staged SHA256 | `BD1F3BFB6AE27000684F0980FB5EA4FB356D2B94C3941278F57CAA43826B6E33` |

Phase 3 row summary:

| Run | Avg FPS | Overhead us | Fired | Hit hero | Hero HP | Peak lightweight | Peak rich | Ranged lightweight basic | Route validity |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 1 | 172.56 | 716.7 | 11 | 11 | 19800 | 88 | 2 | 20 | Pass |
| 2 | 202.58 | 805.9 | 9 | 9 | 19820 | 88 | 2 | 23 | Pass |
| 3 | 200.99 | 911.8 | 2 | 2 | 19960 | 90 | 0 | 26 | Pass |

Findings:

- Phase 3 median improved by `+8.19 FPS` vs Phase 1 (`200.99` vs `192.80`) and stayed inside expected noise given the Phase 3 stdev (`16.89`).
- No rich Ranged or rich basic-mob route appeared in Phase 3. Ranged routed lightweight basic in all rows.
- `RouteLeaksObserved=3` in the runner summary is the expected placed guardian/non-director rich actor reason bucket, not a basic-family rich route. `RouteValid=true`, `CounterMismatch=0`, and `LightweightAcquireFailed=0` in every row.
- Git/LFS or git-status activity was observed before or after some rows, but not as accepted Unreal-process overlap; executable hash stayed stable from pass start through pass end.

Build/stage observations:

- Live editor build passed after copying verified changes back to `C:\UE\T66`.
- Existing build warning remains: `Source/T66/Gameplay/T66Hero1AxeAOEVFXLabActor.cpp` uses deprecated Niagara `IsReadyToRun`.
- Existing cook/stage warning remains from previous runs: missing `/Game/World/Tower/Textures/T_TowerDescentGate_Closed`.

Closure:

- B.10 is closed on the lightweight-only contract: basic Ranged is reliable through the lightweight/projectile-manager path, basic mobs no longer depend on the intermittent rich CMC path, and the final lightweight-only health check is accepted.
- B.11 VAT state ownership is complete: VAT state is manager-owned and laid out for B.13 custom data.
- B.12 tick disable is runtime-proven: no lightweight actor tick and no lightweight component tick were registered while mobs were live.
- B.13 (mob HISM/ISM rendering with per-instance custom data) is CLOSED as a no-land; the per-mob static-mesh renderer is the chosen renderer. See `PerformanceSystem/B13_MobInstancedRendering_Audit.md`. The GPU-driven crowd renderer is the deferred escape hatch.

## RetroFX Full-Resolution Baseline Note

The RetroFX off-by-default fix on 2026-05-29 restored the honest full-resolution rendering contract before B.13. Earlier lightweight captures near 192/200 FPS were likely inflated by real-low-resolution scaling because `r.ScreenPercentage` could be driven below full resolution by default-on RetroFX state.

Corrected full-resolution sanity read, staged SHA `BB56594D3142EE2C35FC6740A0ECEB2F198E62137132FB052CB213A92FAEDAA2`:

| Capture | Resolution | Avg FPS | 1% low | 0.1% low | Peak live regular | Overhead max us | Hero HP | Projectiles fired | Hero hits |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| RetroFX-off full-res lightweight sanity | 1920x1080, `r.ScreenPercentage=100` | 146.30 | 72.33 | 50.12 | 90 | 878.3 | 18860 | 111 | 109 |

Result JSON: `Saved/Codex/Performance/RetroFXOffByDefaultFix/full_res_enemywaveperf_result.json`.

B.13 should use full-resolution captures as its before/after contract. This note updates the absolute FPS reference only; it does not change the CPU-side lightweight architecture conclusions from the earlier relative passes.

## B.13 Mob HISM Rendering With VAT - De-Risk Result

Status: CLOSED - NO-LAND (2026-05-29). Authoritative consolidation:
`PerformanceSystem/B13_MobInstancedRendering_Audit.md`. The per-mob
static-mesh-component renderer is the chosen, deliberate live renderer for basic
mobs; instanced rendering (HISM and ISM) empirically regressed full-resolution
FPS for constantly-moving VAT mobs in UE 5.7 and was not landed. A GPU-driven
crowd renderer is the deferred escape hatch. The sections below remain as
historical evidence; the audit is the single source of truth for the decision.

B.13 was reviewed and attempted in an isolated implementation tree (`C:\UE\T66_B13_Worktree`) instead of the dirty live repo. The pass created an instanced VAT material variant for lightweight basic mobs, kept rich actors on the original MID-driven material path, and proved that VAT frame selection can be driven by per-instance custom data.

Live runtime source was not updated because every dynamic-instancing candidate failed the full-resolution performance gate.

Before baseline, staged SHA `B5C226E1870168430F1FDFCAC91D135C77735999D18666B110DA75B626E78BF8`:

| Captures | Median FPS | 1% low | 0.1% low | Max overhead |
| ---: | ---: | ---: | ---: | ---: |
| 3/3 accepted | 189.65 | 156.16 | 72.03 | 1237.8 us |

Validated correctness:

| Proof | Result |
| --- | --- |
| Stage 1 Slime-only HISM proof | Pass: `Buckets=1`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0` |
| Full HISM rollout proof | Pass: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3` |

Candidate performance:

| Candidate | Median FPS | 1% low | 0.1% low | Result |
| --- | ---: | ---: | ---: | --- |
| HISM with four-slot world-offset custom data | 137.43 | 77.02 | 41.98 | Rejected |
| HISM world transforms with offset-capable material | 116.89 | 73.76 | 64.73 | Rejected |
| HISM frame-only material with per-frame custom writes | 161.39 | 113.61 | 53.88 | Rejected |
| HISM frame-only material with sparse custom writes | 172.64 | 139.61 | 68.98 | Rejected |
| ISM feasibility variant with sparse custom writes | 176.42 | 151.14 | 67.12 | Better than HISM, still below before |

Finding:

- The VAT custom-data technique works functionally.
- HISM is not a win for these dynamic moving mobs. The per-frame instance transform update path and hierarchy/tree management cost more than the draw-call reduction saves.
- Plain ISM removes some hierarchy overhead, but still did not beat the existing per-mob visual mesh path.
- The material-offset strategy was worse because it added per-vertex custom-data reads and defeated useful instance spatial behavior.

Decision:

- B.13 is not landed.
- The current per-mob visual mesh path remains the authoritative live renderer.
- A revised B.13R plan is required before attempting another basic-mob render swap.

Full packet: `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\combined_packet.md`.

### B.13 Continuation Verification

Additional isolated candidates were run after the initial de-risk packet to rule out the remaining plausible HISM-specific causes.

Engine audit finding: UE 5.7's HISM batch transform path still iterates through `UpdateInstanceTransform`, sets the component out of date, and calls `BuildTreeIfOutdated` for location-changing instances. Disabling `bAutoRebuildTreeOnInstanceChanges` is not enough to remove per-frame hierarchy/update cost for moving mobs.

| Candidate | SHA256 | Captures | Median FPS | 1% low | 0.1% low | Result |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| HISM plus VisualMesh unregister | `F561071FE925911B911124BF5F68DA969E4C834C8D97A91B26BF88AED4A98C2F` | 0/1 | n/a | n/a | n/a | Rejected: crashed in scene relevance worker (`FPrimitiveSceneInfo::UpdateComponentLastRenderTime`). |
| HISM plus safe VisualMesh render suppression | `FD6A427D442BF015E2F7556A565D84EA749E012ABE9226621B497A2BAABC55A4` | 3/3 | 176.83 | 150.36 | 75.33 | Stable but below the 189.65 before baseline. |
| HISM hidden pool at origin | `8D0C9FAC2A978BD1E1E3736858420314B3B46EFE8AC46D067A73FE04F8A928B8` | 3/3 | 169.16 | 143.11 | 56.47 | Worse than offscreen hidden pool. |
| HISM transform-change cache | `604642337D71835F44990C3BFB94A2647939FFB1A7A552538D75E446CB860841` | 3/3 | 171.53 | 119.51 | 54.62 | Worse; duplicate same-transform submissions were not the core cost. |

Final B.13 decision remains unchanged: do not land the HISM renderer. The VAT custom-data material path is correct, but the UE component-instance transform path is the wrong performance primitive for constantly moving lightweight mobs in this setup. B.13R requires a new reviewed packet, likely exploring a different renderer architecture rather than more HISM tuning.

### B.13R Spatial-Cell Continuation

Claude reviewed a follow-up HISM spatial-cell candidate in `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\plan_packet_b13r_cell_bucket.md`; review artifact `C:\UE\T66\Reports\AgentReviews\20260529_B13_MobHISM_VAT\20260529T031018-pass2\claude_review_pass2.md` returned `Verdict: APPROVE`.

The spatial-cell candidate kept HISM rendering and per-instance VAT frame custom data, but changed exact per-frame HISM transforms into coarse cell transforms. Fine movement inside each cell was applied in the instanced material through custom data slots `OffsetX/Y/Z`.

Correctness proof passed in the isolated tree:

| Proof | Evidence |
| --- | --- |
| Slime-only spatial-cell proof | `T66_B13_Stage1Slime_HISMProof_cellbucket.log`: `Buckets=1`, `ActiveHISMStates=10`, `FallbackVATStates=50`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `CustomDataFloats=4`, `CellSize=2000.0`, `Pass=1` |
| All-family spatial-cell proof | `T66_B13_Full_HISMProof_cellbucket.log`: `Buckets=7`, `ActiveHISMStates=60`, `FallbackVATStates=0`, `IndependentFrameValues=3`, `VisibleVisualMeshesWhileHISM=0`, `CustomDataFloats=4`, `CellSize=2000.0`, `Pass=1` |
| Saturated visual sanity screenshot | `C:\UE\T66_B13_Worktree\Saved\Codex\Performance\B13_MobHISM\screenshots\T66_B13_After_HISMCellBucket_02.png` showed HISM mobs visible and co-located during capture. |

Performance still failed. The rows below ran the same full-resolution `enemywaveperf` scene/saturation contract as the `189.65` pre-B.13 baseline; staged hashes differ because each row rebuilt the isolated renderer candidate under test. The `1/1` rows are rejection probes only, not positive baselines.

| Candidate | Hash | Accepted | Median FPS | Median 1% low | Median 0.1% low | Finding |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| HISM spatial-cell, `2000` uu cells | `4CAE3738260A946E33A4B4ECFECBADCCDE54A6E8E17BCDBF361F4B8805345292` | 3/3 | 80.42 | 44.67 | 38.98 | Correct but much slower; large offsets likely hurt culling/bounds and material cost. |
| HISM spatial-cell, `500` uu cells | `C0D8E542693EF644F3BEE89593E186E11CA657CC0B82D5638CA9C4AD42E95F53` | 1/1 probe | 167.21 | 94.64 | 62.28 | Better than 2000-cell but still below baseline. |
| HISM spatial-cell `500` uu, shadows/decals disabled | `FD487A2CB24AEFE1C874F212F2377BA8C6D28F3783F8F423A255C55A6101C6DA` | 1/1 probe | 177.31 | 142.78 | 68.46 | Render flags helped but did not close the median gap. |
| HISM spatial-cell `500` uu, no shadows/decals, lazy pooled allocation | `479760F29BE932D2C86413B078E01CD1F92FDCF46452F3FA6BA7B777132BD39F` | 1/1 probe | 176.01 | 138.49 | 66.08 | Reducing hidden preallocation did not recover the gap. |
| HISM spatial-cell `500` uu, no shadows/decals, lazy pool, actor mesh nulled on takeover | `949DD53D44D697DF6BDF5381031D6B7B84991F533C55611AB3F83F54B8068B32` | 1/1 probe | 179.36 | 157.71 | 139.87 | Best final probe for lows; median still regressed by `10.30` FPS versus the `189.65` baseline. |

Final decision: B.13 remains a no-land. The reviewed de-risk stage worked as intended: it proved VAT per-instance custom data and HISM visual correctness before preventing a slower renderer from entering live runtime source. The next renderer attempt should be a new reviewed B.13R architecture rather than more HISM tuning.
