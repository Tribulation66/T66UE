# T66 Mass Migration Plan

Status: SUPERSEDED. Mass migration was evaluated and replaced by the Lightweight Actor pattern. See PerformanceSystem/2026-05-23_T66_LightweightActor_Evaluation.md for rationale and recommended architecture.

Historical status before supersession: architecture and measurement plan only. No Mass plugin enablement, Mass code, enemy director refactor, or gameplay tuning was part of the original planning pass.

## Goal

Move regular bullet-heaven mobs toward a UE 5.7 Mass-based, data-oriented runtime while retaining the existing `ACharacter` combat path for elites, bosses, scripted enemies, and temporarily promoted basic mobs. The saturation validation showed the current actor model is comfortable at the current 90 live-enemy cap, so Mass is a long-term density and maintainability project rather than an emergency replacement.

## Current T66 Enemy Baseline

The live regular enemy stack is actor-centric:

- Spawn ownership: `AT66EnemyDirector` in `Source/T66/Gameplay/T66EnemyDirector.*`.
- Actor base: `AT66EnemyBase` in `Source/T66/Gameplay/T66EnemyBase.*`.
- Family subclasses:
  - `AT66MeleeEnemy`
  - `AT66RangedEnemy`
  - `AT66RushEnemy`
  - `AT66FlyingEnemy`
- Special actor path:
  - `AT66GoblinThiefEnemy`
  - `AT66UniqueDebuffEnemy`
  - bosses under `AT66BossBase`
- Family class resolution:
  - `T66ResolveStageMobIDs` reads `FStageData::EnemyA` through `EnemyJ`.
  - `T66ResolveStageEnemyClass` resolves `FT66EnemyData::FamilyID` to family actor class.
  - `FT66EnemyFamilyResolver` is still a fallback resolver.
- Data seams:
  - `Content/Data/Stages.csv` owns per-stage mob roster.
  - `Content/Data/Enemies.csv` owns `FamilyID`, `Archetype`, `Feeling`, `Rarity`, `StageTag`, and colors.
  - `FT66EnemyData` and `FStageData` live in `Source/T66/Data/T66DataTypes.h`.
- Combat target discovery:
  - `UT66CombatComponent` and `AT66PlayerController` scan `UT66ActorRegistrySubsystem::GetEnemies()`.
  - Actor targets expose `FT66CombatTargetHandle` through `AT66EnemyBase::ResolveCombatTargetHandle`.
- Debug:
  - `T66CombatDebugDraw` draws hit zones and damage volumes when `T66.Combat.DebugView` is enabled.
- Performance:
  - `UT66LagTrackerSubsystem` samples live enemies, pending spawns, and active enemy projectiles.
  - `UT66PerformanceSubsystem` writes session reports and board saturation frame samples.

## UE 5.7 Mass Plugins And Modules

Local UE 5.7 inspection shows `Engine/Plugins/Runtime/MassEntity/MassEntity.uplugin` is deprecated and says MassEntity code moved into the engine. The next implementation pass should treat Mass as module dependencies first, and only enable plugins that still contain runtime modules.

| Plugin or module | UE 5.7 local status | What it gives T66 | Needed now | Minimal usage shape |
|---|---|---|---|---|
| `MassEntity` module | Engine runtime module; plugin descriptor is deprecated | Entity manager, fragments, tags, archetypes, queries, deferred command buffer | Yes for any Mass code | Add module dependency when scaffolding starts; define T66 fragments/tags/processors against engine Mass APIs. Do not enable deprecated `MassEntity.uplugin`. |
| `MassGameplay` plugin/modules | Plugin disabled by default, contains `MassCommon`, `MassActors`, `MassSignals`, `MassSpawner`, `MassSimulation`, `MassLOD`, `MassMovement`, `MassRepresentation` | Runtime processors, LOD, movement helpers, representation, spawner, signal flow | Yes for Phase C | Enable `MassGameplay`; add only required module dependencies in `T66.Build.cs`. Start with `MassCommon`, `MassSimulation`, `MassLOD`, `MassMovement`, `MassRepresentation`, `MassSpawner`, `MassSignals`. |
| `MassRepresentation` module | In `MassGameplay` | ISM/HISM and actor representation LOD | Yes | Use for medium/low visual tiers, not for boss or elite actor path. |
| `MassLOD` module | In `MassGameplay` | Entity LOD/significance framework | Yes | Drive High/Medium/Low/Off tiers from player distance, lock-on state, and floor ownership. |
| `MassMovement` module | In `MassGameplay` | Movement fragments/processors | Yes | Use for basic 2D chase, flee, rush, and flying movement intent before deciding if ZoneGraph is needed. |
| `MassSpawner` module | In `MassGameplay` | Mass entity spawning and spawn data | Yes | Feed `AT66EnemyDirector` spawn plans into Mass archetype spawn requests. |
| `MassSignals` module | In `MassGameplay` | Low-cost deferred events/signals | Yes | Use for death, damage, fire-request, promotion, demotion, and debug refresh signals. |
| `MassAI` plugin/modules | Plugin disabled by default, contains `MassNavigation`, `MassNavMeshNavigation`, `MassZoneGraphNavigation`, `MassAIBehavior`, debug/replication modules | Navigation and AI behavior integration | Later, not first scaffold | Do not start with full behavior trees. Add only after basic movement parity proves insufficient. |
| `MassNavigation` module | In `MassAI` | Generic navigation fragments/processors | Maybe | Use if direct vector movement cannot handle wall-aware chase. |
| `MassZoneGraphNavigation` module | In `MassAI` | ZoneGraph lane/path following | Maybe | Use only if tower walkable floors get authored ZoneGraph lanes. T66 currently uses generated tower floor/wall data, not a ZoneGraph authoring workflow. |
| `ZoneGraph` plugin | Plugin disabled by default | Runtime ZoneGraph data structures | Maybe | Enable only with `MassZoneGraphNavigation` or if generated floors produce ZoneGraph data. |
| `MassCrowd` plugin | Separate AI plugin | Crowd avoidance/high-density crowds | Not first | Consider later only if basic avoidance/crowding becomes the next bottleneck. |
| `MassInsights` plugin | Tooling plugin | Mass profiling/insights | Optional dev-only | Enable only for editor/profiling machines, not required for packaged Development baseline. |

## Source Layout

Keep Mass code inside the existing `T66` runtime module until there is a proven reason for a separate plugin.

Proposed folders:

- `Source/T66/Gameplay/Mass/`
  - `T66MassEnemyFragments.h`
  - `T66MassEnemyTags.h`
  - `T66MassEnemyTypes.h`
  - `T66MassEnemyTraits.h`
  - `T66MassEnemyProcessors.h`
  - `T66MassEnemyProcessors.cpp`
  - `T66MassEnemyRepresentation.*`
  - `T66MassEnemyPromotionSubsystem.*`
  - `T66MassEnemySpawnBridge.*`
  - `T66MassEnemyCombatBridge.*`
  - `T66MassEnemyDebug.*`
- `Source/T66/PerformanceSystem/`
  - keep PerformanceSystem runtime counters here
  - add Mass counters/report fields here, not inside gameplay processors
- `PerformanceSystem/schema/`
  - version any new Mass performance JSON outputs

Naming conventions:

- Fragments: `FT66MassEnemy<Concept>Fragment`
- Shared fragments: `FT66MassEnemy<Concept>SharedFragment`
- Tags: `FT66MassEnemy<Concept>Tag`
- Processors: `UT66MassEnemy<Concept>Processor`
- Subsystems: `UT66MassEnemy<Concept>Subsystem`
- Traits: `UT66MassEnemy<Concept>Trait`
- Bridges: `UT66MassEnemy<ExistingSystem>Bridge`

Relationship to actors:

- `AT66EnemyBase` remains the rich actor reference behavior and promotion target.
- Family subclasses remain canonical for full actor parity and promoted entities.
- Mass entities model basic mobs only.
- The first Mass pass should not delete or bypass actor families. It should run one family behind a controlled feature gate, compare against actor baseline, then expand.

## Fragment And Tag Taxonomy

### Core Fragments

`FT66MassEnemyIdentityFragment`

- `FName MobID`
- `FName EnemyID`
- `FName FamilyID`
- `FName Archetype`
- `FName StageTag`
- `uint16 StageNum`
- `uint8 DifficultyTier`

Maps from `AT66EnemyBase::MobID`, `EnemyFamily`, `FStageData`, and `FT66EnemyData`.

`FT66MassEnemyTransformFragment`

- `FTransform Transform`
- `FVector Velocity`
- `FVector DesiredMoveDirection`
- `float DesiredSpeed`
- `float Radius`
- `float HalfHeight`
- `uint16 FloorNumber`

Replaces actor transform and enough capsule shape data for movement, hit testing, and HUD markers.

`FT66MassEnemyMovementFragment`

- `float BaseMoveSpeed`
- `float CurrentMoveSpeed`
- `float FarChaseSpeedMultiplier`
- `float FarChaseRampDistance`
- `float LeashMaxDistance`
- `float LeashCheckAccumSeconds`
- `float MoveSlowMultiplier`
- `float MoveSlowSecondsRemaining`

Maps from `BaseMaxWalkSpeed`, `LeashMaxDistance`, `FarChaseSpeedMultiplier`, `FarChaseRampDistance`, and move slow state.

`FT66MassEnemyTargetFragment`

- `FMassEntityHandle TargetEntity`
- `TWeakObjectPtr<APawn> TargetPawn`
- `FVector TargetLocation`
- `float TargetDistance2D`
- `float TargetRefreshCooldownSeconds`
- `bool bHasTarget`

Maps from `CachedPlayerPawn`, `PlayerPawnRefreshCooldownSeconds`, and target distance computations.

`FT66MassEnemyCombatFragment`

- `int32 MaxHP`
- `int32 CurrentHP`
- `int32 TouchDamageHearts`
- `float Armor`
- `float ArmorDebuffAmount`
- `float ArmorDebuffSecondsRemaining`
- `float BodyDamageMultiplier`
- `float HeadDamageMultiplier`
- `float LastTouchDamageTime`

Maps from the actor combat fields. Damage is applied through deferred Mass commands and bridged into existing run-state/floating-text/logging.

`FT66MassEnemyStatusFragment`

- `float ConfusionSecondsRemaining`
- `float ForcedRunAwaySecondsRemaining`
- `float StunSecondsRemaining`
- `float RootSecondsRemaining`
- `float FreezeSecondsRemaining`
- `float AutoAttackKnockbackSecondsRemaining`
- `FVector CachedWanderDir`
- `float WanderDirRefreshAccum`

Maps from status fields on `AT66EnemyBase`.

`FT66MassEnemySafeZoneFragment`

- `float SafeZoneCheckAccumSeconds`
- `bool bCachedInsideSafeZone`
- `FVector CachedSafeZoneEscapeDir`
- `FVector CachedSafeZoneCenter`
- `float CachedSafeZoneRadius`
- `FVector CachedSafeZoneLoiterDir`
- `float SafeZoneLoiterDirRefreshAccum`

Maps from `AT66EnemyBase` safe-zone state. The first implementation should batch safe-zone checks instead of reintroducing per-entity actor scans.

`FT66MassEnemySpawnLifecycleFragment`

- `float SpawnAgeSeconds`
- `float RiseAlpha`
- `float RiseTargetZ`
- `float RiseStartZ`
- `FVector WallEmergeStartLocation`
- `FVector WallEmergeTargetLocation`
- `float WallEmergeAlpha`
- `bool bRisingFromGround`
- `bool bEmergingFromWall`
- `bool bPendingDeath`
- `bool bPendingPoolReturn`

Maps from rise/wall-emerge state and death lifecycle.

`FT66MassEnemyVisualFragment`

- `FName CharacterVisualID`
- `FName MobVertexAnimationClip`
- `float MobVertexAnimationClipTime`
- `float MobVertexAnimationOverrideSecondsRemaining`
- `bool bUsingMobVertexAnimation`
- `uint8 LODTier`
- `int32 RepresentationHandle`

Maps from `CharacterVisualID`, VAT clip state, and MassRepresentation data.

`FT66MassEnemyLootScoreFragment`

- `int32 PointValue`
- `int32 XPValue`
- `int32 ResolvedScoreAward`
- `bool bDropsLoot`
- `float DifficultyScalarApplied`
- `float ProgressionEnemyScalarApplied`
- `float FinaleScalarApplied`

Maps from actor score/loot fields and scaling.

### Family Fragments

`FT66MassEnemyRangedFragment`

- `float DesiredMinRange`
- `float DesiredMaxRange`
- `float FireIntervalSeconds`
- `float FireCooldownRemaining`
- `float FireRangeGrace`
- `float ProjectileSpawnHeight`
- `TSubclassOf<AT66EnemyProjectileBase> ProjectileClass` for bridge-only requests

Maps from `AT66RangedEnemy`.

`FT66MassEnemyRushFragment`

- `float RushIntervalSeconds`
- `float RushCooldownRemaining`
- `float RushDurationSeconds`
- `float RushSecondsRemaining`
- `float RushSpeedMultiplier`
- `float RushTriggerDistance`
- `FVector RushDirection`

Maps from `AT66RushEnemy`.

`FT66MassEnemyFlyingFragment`

- `float HoverHeight`
- `float HoverBobAmplitude`
- `float HoverBobFrequency`
- `float HoverAnchorZ`
- `float HoverBobTime`

Maps from `AT66FlyingEnemy`.

### Tags

- `FT66MassEnemyAliveTag`
- `FT66MassEnemyDeadTag`
- `FT66MassEnemyPendingSpawnTag`
- `FT66MassEnemyMeleeTag`
- `FT66MassEnemyRangedTag`
- `FT66MassEnemyRushTag`
- `FT66MassEnemyFlyingTag`
- `FT66MassEnemyMiniBossTag`
- `FT66MassEnemyPromotedTag`
- `FT66MassEnemyPromotionRequestedTag`
- `FT66MassEnemyDemotionRequestedTag`
- `FT66MassEnemyHeroTargetedTag`
- `FT66MassEnemyLockedTargetTag`
- `FT66MassEnemyVisibleOnHudTag`
- `FT66MassEnemyDebugDrawableTag`
- `FT66MassEnemyOffFloorTag`

### Field Mapping From `AT66EnemyBase`

| Actor field or behavior | Mass destination | Actor side retained |
|---|---|---|
| `MaxHP`, `CurrentHP` | `FT66MassEnemyCombatFragment` | Promoted actors use native fields. |
| `TouchDamageHearts` | `FT66MassEnemyCombatFragment` | Actor touch damage for promoted entities. |
| `EnemyFamily` | family tags plus `FT66MassEnemyIdentityFragment::FamilyID` | Actor subclass remains canonical for promoted behavior. |
| `Armor`, `ArmorDebuffAmount`, `ArmorDebuffSecondsRemaining` | `FT66MassEnemyCombatFragment` | Actor armor fields for promoted entities. |
| `bIsConfused`, `ConfusionSecondsRemaining` | `FT66MassEnemyStatusFragment` | Actor status fields for promoted entities. |
| `PointValue`, `XPValue`, `ResolvedScoreAward` | `FT66MassEnemyLootScoreFragment` | Actor values for promoted/pool-return path. |
| `bDropsLoot` | `FT66MassEnemyLootScoreFragment` | Actor special cases. |
| `VisualMesh`, `GetMesh`, widget component | `FT66MassEnemyVisualFragment` plus MassRepresentation | Full `UWidgetComponent` lock indicator stays actor-only. |
| `CharacterVisualID` | `FT66MassEnemyVisualFragment` | Actor visual subsystem for promoted entities. |
| `LockIndicatorWidget` | `FT66MassEnemyLockedTargetTag` and promotion request | Widget stays actor-only; Mass locked target promotes or uses HUD overlay. |
| `bUsesCombatHitZones`, body/head multipliers | `FT66MassEnemyCombatFragment` and debug hit-zone provider | Actual `UT66CombatHitZoneComponent` stays actor-only. |
| `BodyHitZone`, `HeadHitZone` | procedural Mass target zones represented as center/radius data | Components only on promoted actors. |
| `OwningDirector` | spawn bridge owner handle or weak actor pointer | Actor side retained for promoted deaths. |
| `AutoAttackKnockbackSpeed`, `AutoAttackKnockbackStutterSeconds` | status/movement fragments | Actor movement state for promoted entities. |
| `bRunAwayFromPlayer` | movement/status fragments | Actor bool retained. |
| leash/far chase fields | movement fragment | Actor fields retained. |
| `MobID`, `bIsMiniBoss` | identity fragment and mini-boss tag | Mini-boss should likely remain actor/promoted. |
| rise/wall emerge state | spawn lifecycle fragment | Actor animation retained for promoted spawn. |
| safe-zone cache fields | safe-zone fragment | Actor cache retained. |
| VAT clip state | visual fragment | Actor VAT code retained. |
| `CachedPlayerPawn` | target fragment | Actor cached weak pointer retained. |

## Processor Architecture

Processors should run in deterministic order and write through deferred commands where possible.

### `UT66MassEnemySpawnProcessor`

- Reads: spawn requests from `UT66MassEnemySpawnBridge`, shared stage data.
- Writes: identity, transform, combat, movement, visual, lifecycle fragments.
- Frequency: spawn-request driven.
- Dependencies: MassSpawner, `AT66EnemyDirector` bridge, `UT66RunStateSubsystem`, `UT66StageProgressionSubsystem`.
- Notes: Do not ask the director to optimize spawn planning in this migration pass. Feed already resolved spawn slots into Mass first.

### `UT66MassEnemyTargetAcquisitionProcessor`

- Reads: transform, target fragment, floor number, alive tag.
- Writes: target fragment.
- Frequency: throttled, matching actor `PlayerPawnRefreshIntervalSeconds` plus jitter.
- Dependencies: player pawn lookup, actor registry/hero registry.
- Notes: Should not scan controllers per entity every frame. Cache active hero positions once per processor execution.

### `UT66MassEnemySafeZoneProcessor`

- Reads: transform, safe-zone fragment, floor number.
- Writes: safe-zone fragment, movement intent.
- Frequency: throttled around 1.0 second, matching current actor optimization.
- Dependencies: NPC/arcade safe-zone provider from registry or a new compact safe-zone cache.
- Notes: This is a key batching win over actor-side N x M checks.

### `UT66MassEnemyMovementIntentProcessor`

- Reads: identity tags, movement, target, status, safe-zone, family fragments.
- Writes: desired move direction, desired speed, family cooldowns.
- Frequency: every tick for High/Medium entities; throttled for Low; skipped for Off.
- Dependencies: target acquisition and safe-zone processors.
- Family logic:
  - Melee: chase or flee.
  - Ranged: maintain range, stop in fire band, request fire when off cooldown and line of sight is valid.
  - Rush: start rush when cooldown and trigger distance allow.
  - Flying: maintain hover and chase/flee in XY.

### `UT66MassEnemyMovementApplyProcessor`

- Reads: transform, movement intent, LOD tier.
- Writes: transform, velocity.
- Frequency: every tick for visible/near tiers; throttled for Low.
- Dependencies: MassMovement.
- Notes: Use direct 2D integration first. Add MassNavigation/ZoneGraph only if wall-aware movement needs it.

### `UT66MassEnemyRangedFireProcessor`

- Reads: ranged fragment, transform, target fragment, line-of-sight result.
- Writes: fire request signal or deferred command.
- Frequency: every tick for ranged High/Medium, throttled for Low.
- Dependencies: MassSignals, projectile bridge.
- Recommendation: keep `AT66EnemyProjectileBase` actors initially, with Mass emitting fire requests. This preserves visible red projectile shapes, collision, damage logs, and existing wall-hit behavior while Mass enemy movement is being validated. Pooling or Mass projectile visuals should be measured later, not bundled into first enemy migration.

### `UT66MassEnemyDamageApplicationProcessor`

- Reads: queued damage commands, combat fragment, hit-zone data.
- Writes: combat fragment, death tag, floating text/death signals.
- Frequency: signal/deferred command driven.
- Dependencies: `UT66CombatComponent`, `UT66DamageLogSubsystem`, `UT66FloatingCombatTextSubsystem`, `UT66RunStateSubsystem`.
- Notes: Hero combat must be able to damage Mass entities without pretending they are `AActor` pointers. Add a combat bridge target handle type instead of overloading raw actors.

### `UT66MassEnemyTouchDamageProcessor`

- Reads: transform/capsule radius, combat fragment, hero hurtbox cache.
- Writes: run-state damage requests and last touch time.
- Frequency: every tick for High/Medium; throttled for Low.
- Dependencies: hero hurtbox cache and run-state damage provenance.
- Notes: Must preserve the current fix that rejects far/false overlap damage.

### `UT66MassEnemyLifecycleProcessor`

- Reads: combat, lifecycle, loot/score, death tags.
- Writes: death commands, representation release, entity destruction/deferred recycle.
- Frequency: every tick or signal-driven.
- Dependencies: loot bag spawning, score budget, enemy director count bridge, actor pool only for promoted actors.

### `UT66MassEnemyLODProcessor`

- Reads: transform, target state, locked target tags, floor state.
- Writes: LOD tier, representation tags, promotion/demotion requests.
- Frequency: 4-10 Hz, not every tick.
- Dependencies: MassLOD, MassRepresentation, promotion subsystem.

### `UT66MassEnemyDebugDrawProcessor`

- Reads: debug cvars, transform, hit-zone data, damage data, LOD tier.
- Writes: draw calls only when enabled.
- Frequency: debug only; throttle or filter when many entities exist.
- Dependencies: `T66CombatDebugDraw`.
- Notes: Debug draw must remain off by default. When enabled, Mass entities need visible hitboxes and damage ranges equivalent to actor debug.

## MassRepresentation LOD Tiers

Initial LOD policy should use hero distance, floor ownership, lock state, and scripted priority.

| Tier | Conditions | Representation | Behavior rate |
|---|---|---|---|
| High | locked target, hero-targeted, under 1,200 uu, recently damaged, or promotion requested | promoted `ACharacter` or high-detail skeletal/actor representation | full behavior; full combat/debug |
| Medium | 1,200-3,500 uu, same floor, visible on minimap/full map | MassRepresentation ISM/HISM or static mesh representation | movement every tick or modest throttle; fire/touch checks active |
| Low | 3,500-6,000 uu, same floor but not a combat focus | coarse ISM/HISM or invisible gameplay entity with HUD marker | movement/target/safe-zone throttled |
| Off | different floor, beyond 6,000 uu, or culled by scenario | dormant Mass entity, no render | no movement/fire/touch; lifecycle only |

The distances are first-pass values for the current Dungeon floor scale. They must be validated against `T66GameplayAutoCapture=enemywaveperf` and a manual lock-on smoke after implementation.

## Promotion And Demotion Rules

Promote a Mass entity to full actor when:

- the player locks it or the attack-lock fallback selects it
- it is the current hero auto-attack primary target and needs rich hit-zone behavior
- it becomes a mini-boss, elite, scripted encounter, tutorial target, or special mob
- it is within 900-1,200 uu and has a behavior that is not yet Mass-parity complete
- a debug command explicitly asks for selected entity inspection

Demote a promoted actor when:

- it is not locked
- it has not been hit or targeted for at least 3 seconds
- it is farther than 1,600 uu from all heroes
- it is not a mini-boss, elite, scripted target, tutorial target, or boss
- the promotion subsystem is below churn budget

Promotion budget:

- hard cap promoted basic mobs: 12 at once in first implementation
- promote at most 2 entities per frame
- demote at most 4 entities per frame
- use hysteresis distances to avoid promote/demote flapping
- copy all mutable fragments into actor fields on promotion
- copy actor combat/status/lifecycle state back into fragments on demotion

## Bridges To Existing Systems

### Spawn Budget Bridge

Owner: `AT66EnemyDirector`.

The director should continue to decide wave cadence, stage roster, cap, floor, and spawn locations at first. Instead of always materializing `FPendingEnemySpawn` as actors, it should pass regular basic mobs into `UT66MassEnemySpawnBridge` while still spawning:

- mini-bosses
- goblin thieves until a Mass special path exists
- unique debuff enemies
- bosses
- explicitly promoted scripted/tutorial enemies

The bridge must preserve:

- `EnemiesPerWave`, `MaxAliveEnemies`, runtime trickle/stagger values
- `FStageData::EnemyA` through `EnemyJ`
- `FT66EnemyData::FamilyID` and `Archetype`
- per-floor restrictions from tower layout

### Combat Bridge

Current hero combat assumes `AActor` target handles. Mass migration needs a target abstraction that can represent:

- actor targets
- Mass entity targets
- actor-promoted Mass targets

Recommendation:

- Extend or wrap `FT66CombatTargetHandle` with a Mass entity handle and hit-zone primitive data.
- Add `UT66MassEnemyCombatBridge` for `ApplyDamageToTargetHandle`, target aim point, line-of-sight, and damage provenance.
- Do not force Mass entities into fake `AActor` shells just to satisfy combat scans.

### Projectile Bridge

Recommendation for first Mass migration:

- Enemy projectiles remain `AT66EnemyProjectileBase` actors.
- Mass ranged entities emit fire requests through `UT66MassEnemyProjectileBridge`.
- The bridge spawns or later pools the same visible red projectile actors.
- This keeps existing collision, line-of-sight, damage logs, and debug draw working while enemy movement/representation migrates.

Do not move projectiles to Mass in the first enemy-family migration. Projectile actor cost must be measured under fixed active-projectile counters before deciding.

### Loot And Death Bridge

Mass deaths must still route through:

- `UT66RunStateSubsystem::NotifyEnemyKilledByHero`
- damage source accounting
- floating combat text
- death burst VFX
- loot bag spawn rules in `AT66EnemyBase::OnDeath`
- score budget registration
- director live-count decrement

Implementation recommendation:

- Extract reusable loot/death helpers from `AT66EnemyBase::OnDeath` only when Mass death needs them.
- Do not duplicate loot rarity logic inside a processor.
- Use signals to queue game-thread actor spawns for loot bags and VFX.

### Hit Zone And Debug Bridge

Mass entities need logical body/head hit zones:

- body: center near current actor `BodyHitZone` relative offset, radius 42
- head: center near current actor `HeadHitZone` relative offset, radius 24
- multipliers from `BodyDamageMultiplier` and `HeadDamageMultiplier`

When `T66.Combat.DebugView` enables hitboxes or damage volumes, Mass debug must draw equivalent shapes. Invisible Mass damage is not acceptable.

### PerformanceSystem Bridge

Add to PerformanceSystem before or with Mass scaffolding:

- total Mass enemy count
- Mass count per family
- Mass count per LOD tier
- promoted actor count
- promotion/demotion per-second counts
- Mass processor costs by processor name
- Mass projectile fire requests and active projectile actor count

Extend:

- `board_saturation_samples.jsonl`
- `session_summary.json`
- `session_summary.md`
- schemas under `PerformanceSystem/schema/`

### Actor Registry And HUD Bridge

`UT66ActorRegistrySubsystem` currently owns actor arrays used by HUD and combat. It should not store fake actor pointers for Mass entities.

Recommendation:

- Add Mass-aware target/marker provider APIs rather than mixing entity handles into actor arrays.
- HUD map/minimap should read actor enemies plus Mass marker snapshots.
- Combat should query actor registry plus Mass target provider.
- Registry change events should fire when Mass marker data changes enough to dirty HUD state.

## Migration Order

### First Family: Melee

Melee should migrate first because it has the smallest behavior surface:

- chase/flee vector movement
- touch damage
- safe-zone avoidance
- health/armor/status/death
- no projectile fire path
- no hover/rush special state

Validation criteria:

- same spawn roster and count as actor baseline
- same approximate movement speed and chase/flee behavior
- same touch damage cooldown and source logging
- same hero auto-attack targetability
- same body/head hit-zone damage multipliers
- same loot/score/floating text outcome
- debug hitboxes and touch damage volume visible when enabled
- PerformanceSystem reports Mass counts, LOD distribution, and processor costs

### Second Family: Rush

Rush adds cooldown/duration state and burst speed. It should follow once melee proves target/damage/death parity.

Validation criteria:

- rush trigger distance and cooldown match actor behavior
- speed multiplier applies only during rush window
- demotion/promotion preserves rush cooldown and direction

### Third Family: Flying

Flying adds hover Z behavior and movement-mode differences.

Validation criteria:

- hover height/bob reads close to actor baseline
- touch/debug volumes follow the visible entity
- representation tier changes do not snap Z visibly

### Fourth Family: Ranged

Ranged should come after movement/death/debug foundations because it tests projectile decisions.

Validation criteria:

- desired min/max range behavior matches actor
- line of sight still blocks shots through walls
- projectile fire cooldown and visible red projectile actor spawn match actor baseline
- active projectile counters match visible projectiles
- projectile actor cost is measured before any Mass projectile rewrite

### Deferred Families And Archetypes

The roster contains archetypes without production classes: `Exploder`, `Strafer`, `Stutterer`, `Turret`, `Burrower`, and `Necromancer`. They currently map through fallback family behavior. Do not invent Mass-only mechanics for these until the actor-side design exists or Pablo explicitly approves Mass-first implementation.

Final desired state:

- basic mobs are Mass entities by default
- current family actors are retained for promoted basic mobs and parity reference
- special mobs, elites, bosses, tutorial/scripted targets, and active lock-on targets use actors
- Mass becomes the default path only after at least Melee, Rush, Flying, and Ranged parity are proven

## Validation Methodology

Use the fixed-instrumentation baseline from `T66GameplayAutoCapture=enemywaveperf` as the comparator.

For each family migration:

- run a 90+ gameplay-second enemywaveperf capture
- confirm cap reached or document why not
- record full-window average FPS, 1 percent low, and 0.1 percent low
- record saturated `>= 60` and `>= 80` windows
- compare `SingleFrameHitch` event count to raw frames above threshold
- record PerformanceSystem overhead max
- record active projectile counts when ranged is involved
- record Mass processor cost and LOD distribution
- run one debug capture with `T66.Combat.DebugView=3`
- manually verify visible hitboxes/damage volumes for Mass entities

Acceptance target for first Mass family:

- no behavior regressions against actor baseline
- no invisible damage
- no HUD/minimap disappearance
- no increase in PerformanceSystem overhead above 2 ms
- clear processor timing showing where Mass work happens

## Risks And Unknowns

- `MassEntity.uplugin` is deprecated in UE 5.7. The next pass must add module dependencies, not blindly enable the deprecated plugin.
- Current `AT66EnemyDirector` spawn planning remains actor-oriented and still has pending work around archetype-aware spawn selection. Mass should consume its output first, then refactor planning later.
- Hero combat target handles are actor-shaped. A Mass target handle bridge is required before Mass entities can be first-class hero targets.
- Actor registry is actor-shaped. HUD and combat need Mass-aware provider APIs.
- Ranged projectiles did not appear in the Phase A saturation sample despite ranged mobs existing in the Dungeon roster. Counter and scenario validation must be trusted before deciding projectile migration urgency.
- ZoneGraph may not fit generated tower floors unless T66 authors or generates ZoneGraph data.
- Mini-bosses use actor scaling, loot, and richer visibility. Keep them actor-side until promotion/demotion state transfer is proven.
- Existing special archetypes are fallback behavior only. Mass should not be the place where new mechanics are secretly invented.

## Resolved Phase C.1 Decisions

The Phase C.1 scaffold pass locked these decisions before implementation:

- First Mass milestone target is 300 live basic enemies, not 90-cap parity.
- Low LOD stays visible through ISM/HISM. No invisible-at-Low representation.
- Movement v1 is direct vector chase/flee. Line-trace wall avoidance is deferred until measured-needed.
- Lock-on always promotes a Mass entity to `ACharacter`.
- Promotion budget is 12 promoted hard cap, 8 warn threshold, 2 promotions per frame max, and 4 demotions per frame max.
- Special enemies remain actor-only indefinitely: goblin thief, unique debuff enemies, mini-bosses, bosses, scripted enemies, and tutorial enemies.

## Decisions Needed From Pablo

- Whether MassGameplay's transitive ZoneGraph/SmartObjects/StateTree plugin load is acceptable for the scaffold, or whether C.1 should be reworked to a lower-level module-only path before migration starts.
- Whether the Phase C.1 90-cap regression should block the next Mass implementation pass or be accepted as a known scaffold cost to measure against during the Melee migration.
- Whether the remaining PerformanceSystem framework overhead spikes above 2 ms should be handled before Mass migration logic, or tracked as Phase B instrumentation debt.

## Phase C.1 Scaffolding Verification

Phase C.1 landed the header-only Mass fragments/tags/types/traits, compile-clean processor/subsystem/bridge stubs, Mass PerformanceSystem counters, and the feature-gated spawn bridge shadow path. No live Mass entities are spawned in this pass.

Actor baseline at 300 cap:

- Optimization plan section: `PerformanceSystem/2026-05-23_EnemySpawnRate_Lag_Optimization_Plan.md`
- Session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T033922Z__dBCe0kd2O2c1_e1abfa8Q`
- Metrics artifact: `Saved/Codex/Performance/MassC1/actor300_baseline_metrics.json`
- Override approach: non-shipping `T66.EnemyDirector.MaxAliveOverride 300`
- Wave size bumped: no
- Peak live regular enemies: 300 at gameplay time 55.515 seconds
- `>= 250` saturated band: 56.30 average FPS, 43.91 1 percent low FPS, 36.22 0.1 percent low FPS
- Single-frame hitches: 11 events, matching 11 raw frames at or above the 50 ms threshold
- Worst frame: 1308.996 ms
- Max `PerformanceSystemOverhead`: 4,456.799 us
- Active hostile projectile peak: 2

Final 90-cap regression capture after Mass scaffolding:

- Session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T044150Z_NNqEOkqsKTCwdR25X1MwkA`
- Standalone log: `Saved/StandaloneLogs/T66_PhaseC1_90CapRegression_Final2.log`
- Unreal viewport screenshot: `Saved/Codex/Performance/MassC1/enemywaveperf_phase_c1_90cap_regression_final2.png`
- Metrics artifact: `Saved/Codex/Performance/MassC1/phase_c1_90cap_regression_final2_metrics.json`
- Full session summary: 138.17 average FPS, 96.67 1 percent low FPS, 74.33 0.1 percent low FPS
- Gameplay-only window: 135.85 average FPS, 96.02 1 percent low FPS, 55.17 0.1 percent low FPS
- `>= 80` saturated band: 134.46 average FPS, 96.31 1 percent low FPS, 66.49 0.1 percent low FPS
- Peak live regular enemies: 90 at gameplay time 19.264 seconds
- Single-frame hitches: 10 all-map events matching 10 raw all-map frames at or above 50 ms; 9 GameplayLevel events matching 9 raw GameplayLevel frames at or above 50 ms
- Max `PerformanceSystemOverhead`: 4,216.399 us
- Active hostile projectile peak: 0
- Mass counters: present in board samples and final summaries, all zero
- `T66.Mass.SpawnBridge.Enabled`: default off; no `Mass spawn requested` log lines in the default-off regression capture

Comparison against the fixed-instrumentation 90-cap baseline:

| Window | Prior baseline | Phase C.1 final | Delta |
| --- | ---: | ---: | ---: |
| `>= 80` average FPS | 161.60 | 134.46 | -16.8 percent |
| `>= 80` 1 percent low FPS | 107.57 | 96.31 | -10.5 percent |
| `>= 80` 0.1 percent low FPS | 70.27 | 66.49 | -5.4 percent |
| PerformanceSystem overhead max | 1,210.403 us | 4,216.399 us | +3,005.996 us |

Result: build, staging, schema, counters, and spawn-bridge toggle verification passed, but the 90-cap regression check did not meet the +/- 5 percent acceptance target. Runtime logs show the explicit `MassGameplay` plugin entry also mounts `ZoneGraph`, `ZoneGraphAnnotations`, `SmartObjects`, and `StateTree` as transitive engine plugin dependencies. The processors were made `UCLASS(Abstract)` so empty stub processors do not auto-register or emit Mass query warnings, but the final capture still regressed. Treat this as an open scaffold-performance issue before using C.1 numbers as a clean Mass migration baseline.

Spawn bridge enabled smoke:

- Log: `Saved/StandaloneLogs/T66_PhaseC1_SpawnBridgeEnabled.log`
- Session: `Saved/StagedBuilds/Windows/T66/Saved/PerformanceSystem/Sessions/20260524T044600Z_6kR7WU6KFXrFCuCFpY1E1Q`
- `T66.Mass.SpawnBridge.Enabled 1` produced `Mass spawn requested` log lines for basic Melee, Rush, Ranged, and Flying spawn slots while actors continued to spawn normally.

Build and staging:

- Development standalone build succeeded with the Mass module dependencies and stubs.
- `Scripts/StageStandaloneBuild.ps1 -ClientConfig Development -SkipBuild` refreshed the staged build.
- Both `C:\UE\T66\T66 Standalone.lnk` and the taskbar `T66 Standalone.lnk` point to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
