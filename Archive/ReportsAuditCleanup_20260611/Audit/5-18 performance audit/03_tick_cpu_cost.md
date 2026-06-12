# Section 3 - Tick and CPU Cost

## Direct Actor Ticks

The following `AActor` subclasses in `Source/T66` set `PrimaryActorTick.bCanEverTick = true`:

- `AT66ArthurUltimateSword`
- `AT66BossGroundAOE`
- `AT66BoostInteractable`
- `AT66BossGate`
- `AT66BossBase`
- `AT66ChestMimicEnemy`
- `AT66EnemyBase`
- `AT66CompanionBase`
- `AT66GameMode`
- `AT66HeroPlagueCloud`
- `AT66HeroOneAttackVFX`
- `AT66HouseNPCBase`
- `AT66HeroBase`
- `AT66HeroProjectile`
- `AT66LavaPatch`
- `AT66LoanShark`
- `AT66LootWheelInteractable`
- `AT66MiasmaManager`
- `AT66PilotableTractor`
- `AT66StartGate`
- `AT66TutorialManager`
- `AT66TutorialGuideCompanion`
- `AT66UniqueDebuffProjectile`
- `AT66FloorSpikePatchTrap`
- `AT66FloorFlameTrap`
- `AT66TrapArrowProjectile`

Tick interval details found:

- `AT66HouseNPCBase` sets `TickInterval = 0.05f` and can later switch to `0.15f` or disable tick.
- `AT66LavaPatch` ticks at `1/12s` and recalculates from `AnimationFPS`.
- Flame and spike traps can tick but start disabled.
- `AT66HeroOneAttackVFX` uses `TG_PostUpdateWork`.

No `PrimaryComponentTick.bCanEverTick = true` hits were found in `Source/T66`.

## Manager Tick Patterns

Enemy spawning:

- `AT66EnemyDirector` disables actor tick.
- It binds to stage timer changes, schedules runtime waves with timers, and processes pending spawn batches.
- Actual enemy movement remains per-enemy tick in `AT66EnemyBase::Tick`.
- `AT66EnemyAIController` disables tick and documents that direct movement replaced nav timer work.
- Pooled enemies have actor and movement component ticks disabled on release.

Projectiles:

- Enemy and boss projectiles generally disable actor tick and use `UProjectileMovementComponent`, overlap, and lifespan.
- Hero, unique debuff, and trap-arrow projectiles tick directly.

Pickups/interactables:

- `AT66LootBagPickup` disables actor tick and uses overlap/registry paths.
- `AT66BoostInteractable` and `AT66LootWheelInteractable` tick for visual/lifetime behavior.

VFX:

- Pixel/combat/boss VFX paths have subsystem budgets and cached references.
- Spawned Niagara systems still need runtime profiling for actual cost.

## Tick Ordering

No usage found for:

- `FTickFunction`
- `AddTickPrerequisiteActor`
- `AddTickPrerequisiteComponent`

Special ordering found:

- `AT66HeroOneAttackVFX` sets `PrimaryActorTick.TickGroup = TG_PostUpdateWork`.
- `UT66ToonOutlineViewSubsystem` is a tickable subsystem and declares a stat id.

## Parallelism

No hits found for:

- `ParallelFor`
- `TFuture`
- `FAsyncTask`
- `FNonAbandonableTask`
- `TGraphTask`
- `UE::Tasks`
- bare `Async(...)`

Limited async use:

- `AsyncTask(ENamedThreads::GameThread, ...)` appears in UI rebuild deferral and web image cache completion paths.

Most runtime CPU work appears single-threaded on the game thread.

## Obvious Scaling Risks

Loot bag spawn clearance:

- Does up to six iterations.
- Loops NPCs, world interactables, other loot bags, and altars.
- Drop bursts can become cumulative against existing loot bags.

EnemyDirector pending spawn batches:

- Uses front removal from `PendingSpawns`.
- This is O(n) per removal and can become wave-size sensitive when batch size is small.

Combat targeting:

- Generally avoids full world scans by using overlap lists and actor registry.
- Still has action-time scans over registry arrays in some target acquisition and ultimate paths.

Mini battle screen:

- Contains widget-simulation loops over projectiles/enemies.
- Needs separate profiling if Mini remains a target for low-end performance work.

## CPU Profiling Gaps

No current Unreal Insights trace exists in the repo. Static analysis can identify likely risks, but CPU priority order needs:

- `stat unit`
- `stat game`
- Unreal Insights CPU trace
- custom `T66.Perf.Dump`
- representative gameplay scenes with enemy counts, projectiles, VFX, and UI visible

