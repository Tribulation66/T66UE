# T66 Lightweight Actor Evaluation

Status: evaluation only. No Mass rollback, code rollback, gameplay tuning, or Lightweight Actor implementation is part of this document.

## Working Conclusion

I agree with the pivot away from the Mass Phase C.1 path for T66's actual scale. The current codebase shows regular mobs already use simple top-down direct movement, no active NavMesh pathing, static/VAT visuals for regular enemy rows, and actor-shaped combat handles. That makes a Lightweight Actor pattern a better fit than paying the fixed runtime cost of `MassGameplay` and its transitive plugin load.

The important caveat is that this should not be treated as a mechanical one-line inheritance swap. `AT66EnemyBase` currently inherits `ACharacter`, and many systems use `AT66EnemyBase` as the common type for regular mobs, mini-bosses, lab spawns, tutorial enemies, gate guardians, goblin thief, unique debuff enemies, HUD/minimap markers, traps, combat, and run-state effects. Most of those systems do not require `ACharacter`, but they do require the `AT66EnemyBase` API to remain stable.

My recommendation from the code is:

- Roll back Mass separately first and recapture the clean 90-cap baseline.
- For Lightweight Actor, either introduce a new lightweight basic-mob actor path first, or split the special `ACharacter` path out before changing the `AT66EnemyBase` inheritance.
- Keep bosses, mini-bosses, goblin thief, unique debuff enemies, chest mimic, scripted/tutorial special cases, and runtime-promoted/locked enemies on the richer actor path until the basic-mob path has proven parity.

## Evaluation Context

Relevant prior measurements:

- Fixed-instrumentation 90-cap actor baseline: `>= 80` saturated band was 161.60 average FPS, 107.57 1 percent low FPS, and 70.27 0.1 percent low FPS in `PerformanceSystem/2026-05-23_EnemySpawnRate_Lag_Optimization_Plan.md`.
- Actor baseline at 300 cap: `>= 250` saturated band was 56.30 average FPS, 43.91 1 percent low FPS, and 36.22 0.1 percent low FPS. Peak live regular enemies reached 300 at gameplay time 55.515 seconds.
- Phase C.1 Mass scaffold verification: `MassGameplay` was enabled in `T66.uproject`, Mass modules were added in `Source/T66/T66.Build.cs`, and the `>= 80` saturated 90-cap band regressed from 161.60 to 134.46 average FPS with zero live Mass entities. The Mass pending issue documents the transitive `ZoneGraph`, `ZoneGraphAnnotations`, `SmartObjects`, and `StateTree` load.

Mass scaffold files still present and read as context:

- `Source/T66/Gameplay/Mass/T66MassEnemyFragments.h`
- `Source/T66/Gameplay/Mass/T66MassEnemyTags.h`
- `Source/T66/Gameplay/Mass/T66MassEnemyTypes.h`
- `Source/T66/Gameplay/Mass/T66MassEnemyProcessors.h`
- `Source/T66/Gameplay/Mass/T66MassEnemySpawnBridge.*`
- `Source/T66/Gameplay/Mass/pending_issues_Mass.md`

## 1. `AT66EnemyBase` `ACharacter` Dependencies

`AT66EnemyBase` is declared as `class T66_API AT66EnemyBase : public ACharacter` in `Source/T66/Gameplay/T66EnemyBase.h`. The direct `ACharacter`/`APawn` dependency categories are below.

### Inheritance And Pawn Setup

Files and usages:

- `Source/T66/Gameplay/T66EnemyBase.h`
  - Includes `GameFramework/Character.h`.
  - `AT66EnemyBase` inherits `ACharacter`.
  - `GetDefaultMovementMode()` returns `EMovementMode` and defaults to `MOVE_Walking`.
- `Source/T66/Gameplay/T66EnemyBase.cpp`
  - Constructor sets `AIControllerClass = AT66EnemyAIController::StaticClass()`.
  - Constructor sets `AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned`.
  - Constructor sets `bUseControllerRotationYaw = false`.
- `Source/T66/Gameplay/T66EnemyAIController.cpp`
  - The AI controller no longer pathfinds or ticks; comments say direct movement is handled by `AT66EnemyBase::Tick()`.

No enemy-side usage was found for `FollowCamera`, `JumpMaxHoldTime`, `Jump`, `Crouch`, mantling, crouching hooks, or camera-specific character features.

### Character Movement Component

`AT66EnemyBase` uses `UCharacterMovementComponent` for speed, movement mode, orientation, velocity, and hard stops:

- Constructor:
  - `GetCharacterMovement()`
  - `Move->MaxWalkSpeed = 350.f`
  - `Move->bOrientRotationToMovement = true`
  - `Move->RotationRate = FRotator(0.f, 720.f, 0.f)`
- `BeginPlay()`:
  - `Move->SetMovementMode(GetDefaultMovementMode())`
  - Captures `BaseMaxWalkSpeed = Move->MaxWalkSpeed`
- `ResetForReuse()`:
  - Re-enables movement component ticking.
  - Restores `SetMovementMode(GetDefaultMovementMode())`.
- `Tick()`:
  - Requires `UCharacterMovementComponent* Move = GetCharacterMovement()`.
  - Updates `Move->MaxWalkSpeed` for safe-zone loiter, slow, freeze, and far-chase modifiers.
  - Calls `StopMovementImmediately()` for knockback end, stun, root, and freeze.
- `ApplyAutoAttackKnockback()`:
  - Calls `StopMovementImmediately()`.
  - Writes `Move->Velocity = AwayFromHit * KnockbackSpeed`.
- `ApplyStun()`, `ApplyRoot()`, `ApplyFreeze()`:
  - Call `StopMovementImmediately()`.
- `ApplyPullTowards()` and `ApplyPushAwayFrom()`:
  - Call helper `T66ApplyCharacterDisplacement(ACharacter* Character, ...)`, which requires an `ACharacter*` and stops CMC movement after a swept `SetActorLocation`.

No active navigation query usage was found in the regular enemy movement code. A repo search of enemy base, enemy family classes, enemy director, and the enemy AI controller only found a comment saying the old `SimpleMoveToActor` timer was removed.

### `ACharacter` Mesh And Capsule Helpers

Files and usages:

- `Source/T66/Gameplay/T66EnemyBase.cpp`
  - Constructor calls `GetCapsuleComponent()` to configure collision.
  - Constructor calls `GetMesh()` to hide and configure the inherited skeletal mesh.
  - `RefreshCombatHitZoneState()` checks capsule visibility.
  - `BeginPlay()` binds `GetCapsuleComponent()->OnComponentBeginOverlap` to `OnCapsuleBeginOverlap`.
  - `ConfigureAsMob()` and `BeginPlay()` call `ApplyCharacterVisual(CharacterVisualID, GetMesh(), VisualMesh, ...)`.
  - VAT success hides `GetMesh()`.
  - Debug draws `T66CombatDebugDraw::DrawDamageCapsule(GetCapsuleComponent(), ...)`.
  - Rise/wall-emerge temporarily modify capsule collision response.
  - Touch damage uses enemy and hero capsule radii.

These are mostly convenience dependencies. An `AActor` with an explicit `UCapsuleComponent` root and optional `USkeletalMeshComponent` can preserve the behavior, but call sites must stop assuming `GetCapsuleComponent()` and `GetMesh()` exist unless compatibility accessors are added.

### Movement Input

`AddMovementInput()` is used in the base and family behavior:

- `AT66EnemyBase::TickFamilyBehavior()`
- Safe-zone loiter in `AT66EnemyBase::Tick()`
- Confusion wander in `AT66EnemyBase::Tick()`
- `AT66RangedEnemy::TickFamilyBehavior()`
- `AT66RushEnemy::TickFamilyBehavior()`
- `AT66FlyingEnemy::TickFamilyBehavior()`

This is an `APawn`/`ACharacter` API. The Lightweight Actor manager must replace it with explicit velocity/integration, not just disable actor tick.

### Other Character-Coupled Details

- `TickMobVertexAnimationState()` uses `GetVelocity()` to choose idle vs move. `AActor::GetVelocity()` exists, but after custom manager movement it may return zero unless velocity is stored or exposed through a movement component. Lightweight movement should feed VAT animation from an explicit stored velocity.
- `AT66RangedEnemy::FireProjectileAtPlayer()` sets `FActorSpawnParameters::Instigator = this`. This compiles today because `AT66RangedEnemy` is an `APawn`. It will fail or need replacement when the enemy becomes an `AActor`.
- `UT66EnemyPoolSubsystem::Release()` is typed to `AT66EnemyBase*`, but it calls `Enemy->GetCharacterMovement()->StopMovementImmediately()` and disables the movement component tick.

## 2. Family Subclass `ACharacter` Dependencies

### `AT66MeleeEnemy`

File: `Source/T66/Gameplay/Enemies/T66MeleeEnemy.cpp`

- Constructor sets `EnemyFamily = ET66EnemyFamily::Melee`.
- Constructor sets `GetCharacterMovement()->MaxWalkSpeed = 350.f`.
- It does not override tick behavior. It uses base `AT66EnemyBase::TickFamilyBehavior()`, which directly chases or flees with `AddMovementInput()`.

Lightweight replacement: trivial 2D chase/flee vector plus speed.

### `AT66RangedEnemy`

File: `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp`

- Constructor sets `GetCharacterMovement()->MaxWalkSpeed = 320.f`.
- Uses `AddMovementInput()` to flee when too close, chase when too far, and run away under forced flee.
- Uses `GetCharacterMovement()->StopMovementImmediately()` when inside desired range.
- Fires projectiles from `GetActorLocation() + FVector(0,0,ProjectileSpawnHeight)`.
- Uses line trace for projectile line of sight.
- Sets `SpawnParams.Owner = this` and `SpawnParams.Instigator = this`.

Lightweight replacement: movement is simple, but projectile spawning must stop using the enemy actor as an `APawn` instigator. Owner can remain the enemy actor. Instigator should be `nullptr` or a compatible pawn only if some later system truly needs an `APawn`.

### `AT66RushEnemy`

File: `Source/T66/Gameplay/Enemies/T66RushEnemy.cpp`

- Constructor sets `GetCharacterMovement()->MaxWalkSpeed = 330.f`.
- Rush burst is implemented as:
  - `RushDirection = ToPlayer`
  - `RushSecondsRemaining = RushDurationSeconds`
  - `Move->StopMovementImmediately()`
  - temporary `Move->MaxWalkSpeed = GetBaseWalkSpeed() * RushSpeedMultiplier`
  - `AddMovementInput(RushDirection, 1.f)`
- There is no root motion and no custom velocity push.

Lightweight replacement: direct velocity or displacement along `RushDirection` for `RushDurationSeconds`.

### `AT66FlyingEnemy`

Files:

- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.h`
- `Source/T66/Gameplay/Enemies/T66FlyingEnemy.cpp`

Evidence:

- Constructor sets `Move->MaxWalkSpeed = 430.f`.
- Constructor sets `Move->SetMovementMode(MOVE_Flying)`.
- Header overrides `GetDefaultMovementMode()` to return `MOVE_Flying`.
- `TickFamilyBehavior()` manually computes a hover Z:
  - `HoverAnchorZ + sin(HoverBobTime * HoverBobFrequency) * HoverBobAmplitude`
  - `SetActorLocation(HoverLoc)`
- Horizontal chase still uses `AddMovementInput()`.

Answer: Flying currently uses CMC `MOVE_Flying` and manual Z bob. The actual airborne behavior is mostly manual, so custom movement is simpler than it would be for a CMC-driven flying/nav actor. The replacement needs a hover anchor, bob timer, and explicit planar velocity.

## 3. Visual Representation Per Enemy Type

### Regular Enemy Data

Data files:

- `Content/Data/Enemies.csv`
- `Content/Data/CharacterVisuals.csv`
- `Content/Data/MobVertexAnimations.csv`

Current counts from the CSVs:

- `Enemies.csv` has 50 regular enemy rows.
- `CharacterVisuals.csv` has matching rows for all 50 enemy IDs, and those matching rows use `StaticMesh`; none of the regular enemy rows have `SkeletalMesh` populated.
- `MobVertexAnimations.csv` has 10 enabled VAT rows, all Dungeon mobs:
  - `Slime`
  - `CaveBat`
  - `BoneWalker`
  - `RatPack`
  - `TombSpider`
  - `HexSlinger`
  - `StoneSentinel`
  - `MimicLure`
  - `BoneConjurer`
  - `CryptWraith`
- The other 40 regular enemy rows currently use static mesh character visuals, not VAT and not skeletal animation.

### Runtime Visual Path

Files:

- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Core/T66CharacterVisualSubsystem.cpp`

Current order:

1. `AT66EnemyBase::ConfigureAsMob()` sets placeholder mesh/color and `CharacterVisualID = MobID`.
2. `AT66EnemyBase::BeginPlay()` tries `TryApplyMobVertexAnimationVisual()` first.
3. If VAT applies, `VisualMesh` gets the VAT static mesh/material and inherited `GetMesh()` is hidden.
4. If VAT does not apply, `ApplyCharacterVisual()` uses the static mesh row from `CharacterVisuals.csv` or a skeletal mesh if one exists.

### ISM/HISM Migration Requirements

Regular mobs are favorable for ISM because they are currently static mesh or VAT static mesh. There is no regular skeletal mesh blocker in the current data.

Requirements:

- Group instances by visual identity: mesh, material, textures, outline needs, and possibly clip family.
- For the 10 VAT Dungeon rows, the current code creates a per-enemy `UMaterialInstanceDynamic` and drives scalar parameters like `Frame`, `StartFrame`, `EndFrame`, `SampleRate`, and `RowsPerFrame`.
- `UInstancedStaticMeshComponent` cannot use a unique MID per instance for frame state. VAT-on-ISM needs a different material strategy:
  - preferred: use per-instance custom data for current frame and clip values;
  - fallback: bucket instances by clip/frame into separate ISM/HISM components, which is more churn and probably worse.
- For the 40 non-VAT static mesh rows, ISM rendering is straightforward visually, but they will remain unanimated unless VAT rows are baked or another cheap animation strategy is added.
- `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual()` only applies capsule-bottom alignment when `TargetStaticMesh->GetOwner()` casts to `ACharacter`. If `AT66EnemyBase` becomes `AActor`, VAT relative Z alignment must be updated or the meshes may appear offset.

Skeletal enemies:

- No current regular enemy row is skeletal.
- Boss support is mixed: `GamblerBoss` has a skeletal mesh row, while the Dungeon boss visual rows are static QuadRetro meshes.
- Skeletal mesh enemies cannot use ISM directly. They should either stay individual actors or get VAT-baked later.

## 4. Collision And Hit Zone Components

Files:

- `Source/T66/Gameplay/T66EnemyBase.h`
- `Source/T66/Gameplay/T66EnemyBase.cpp`
- `Source/T66/Gameplay/T66CombatHitZoneComponent.cpp`

Current attachments:

- `VisualMesh` is created and attached to `RootComponent`.
- `LockIndicatorWidget` is created and attached to `RootComponent`.
- `BodyHitZone` is created and attached to `RootComponent`.
- `HeadHitZone` is created and attached to `RootComponent`.

Because `RootComponent` is currently the inherited character capsule, all of these are effectively capsule-root relative, not skeletal-mesh relative. They should reattach cleanly to an `AActor` root if the new root remains a `USceneComponent` or `UCapsuleComponent` with compatible origin semantics.

`UT66CombatHitZoneComponent` is a `USphereComponent`. It sets query collision, visibility blocking, world dynamic overlap, `SetHiddenInGame(true)`, and `SetCanEverAffectNavigation(false)`. It does not require `ACharacter`.

Touch damage:

- `AT66EnemyBase::BeginPlay()` binds overlap on `GetCapsuleComponent()`.
- `OnCapsuleBeginOverlap()` requires `OtherActor` to be `AT66HeroBase`.
- It rejects hits unless `OtherComp == Hero->GetCapsuleComponent()`.
- It reads hero and enemy capsule radii and applies a 2D distance guard.
- Damage is routed through `UT66RunStateSubsystem::ApplyDamage(DamageHP, this, "EnemyTouch", this)`.

Answer: touch damage depends on a capsule-shaped primitive and hero capsule overlap, not on CharacterMovement. If the lightweight actor keeps a `UCapsuleComponent` as its collision root, this path is easy to port. If the lightweight path removes per-enemy primitive collision entirely, touch damage must move to manager-owned spatial checks and will need careful debug visualization.

## 5. Combat, Registry, Projectile, Debug, And Pool Coupling

### Combat Target Handles

File: `Source/T66/Gameplay/T66CombatTargetTypes.h`

`FT66CombatTargetHandle` stores:

- `TWeakObjectPtr<AActor> Actor`
- `TWeakObjectPtr<UPrimitiveComponent> HitComponent`
- hit zone type/name
- aim point

It is actor-typed, not `ACharacter`-typed.

Files:

- `Source/T66/Gameplay/T66CombatComponent.*`
- `Source/T66/Gameplay/T66HeroProjectile.cpp`
- `Source/T66/Gameplay/T66PlayerController_Combat.cpp`

Combat code casts targets to `AT66EnemyBase` or `AT66BossBase`, not to `ACharacter`. It calls enemy APIs like `CurrentHP`, `ResolveCombatTargetHandle()`, `TakeDamageFromHeroHitZone()`, status methods, pull/push, and lock indicator methods. Those APIs can survive an `AActor` base if they are preserved.

### Actor Registry

Files:

- `Source/T66/Core/T66ActorRegistrySubsystem.h`
- `Source/T66/Core/T66ActorRegistrySubsystem.cpp`

The registry stores enemies as `TArray<TWeakObjectPtr<AT66EnemyBase>>`. It does not use `ACharacter` APIs. HUD and combat can keep using it if `AT66EnemyBase` remains the common target class. If the implementation introduces a separate lightweight class instead of changing `AT66EnemyBase`, registry/provider APIs must be widened.

### Projectiles

Files:

- `Source/T66/Gameplay/Enemies/T66RangedEnemy.cpp`
- `Source/T66/Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.*`
- `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp`
- `Source/T66/Gameplay/T66UniqueDebuffProjectile.*`

`AT66EnemyProjectileBase` is already an `AActor` with a sphere root, `UProjectileMovementComponent`, visual meshes, and active-count tracking. It does not assume its owner is `ACharacter`.

The ACharacter-specific issue is at fire request construction:

- `AT66RangedEnemy::FireProjectileAtPlayer()` sets `SpawnParams.Instigator = this`.
- That only works while the enemy is an `APawn`.
- Start position and target are actor-location based. There is no muzzle socket or mesh attachment dependency.

Unique debuff enemy projectile spawning uses `SpawnActorDeferred(..., this, nullptr, ...)`, so it passes the enemy as owner and `nullptr` instigator. That is already AActor-compatible at the call shape.

### Combat Debug Draw

File: `Source/T66/Gameplay/T66CombatDebugDraw.cpp`

Debug draw uses component data directly:

- `DrawHitZone(const USphereComponent*)`
- `DrawDamageSphere(const USphereComponent*)`
- `DrawDamageBox(const UBoxComponent*)`
- `DrawDamageCapsule(const UCapsuleComponent*)`
- `DrawPlayerHurtCapsule(const UCapsuleComponent*)`

It does not pull data from CMC. Enemy base currently passes `BodyHitZone`, `HeadHitZone`, and `GetCapsuleComponent()`. An `AActor` path can keep debug parity if it keeps explicit hit zone spheres and a visible damage capsule or updates debug draw to accept the new primitive.

### Enemy Pool

Files:

- `Source/T66/Core/T66EnemyPoolSubsystem.h`
- `Source/T66/Core/T66EnemyPoolSubsystem.cpp`

The pool is typed to `AT66EnemyBase*` and `TSubclassOf<AT66EnemyBase>`, not `ACharacter*`. It has one CMC dependency in `Release()`:

- calls `Enemy->GetCharacterMovement()`
- `StopMovementImmediately()`
- `SetComponentTickEnabled(false)`

`AT66EnemyBase::ResetForReuse()` also re-enables movement component ticking and restores movement mode. Pooling needs a lightweight movement reset hook.

## 6. `AT66EnemyDirector` Spawn Path

Files:

- `Source/T66/Gameplay/T66EnemyDirector.h`
- `Source/T66/Gameplay/T66EnemyDirector.cpp`

The director does not explicitly call `SpawnDefaultController()` or `PossessedBy()`. It spawns `AT66EnemyBase` subclasses through:

- `World->SpawnActorDeferred<AT66EnemyBase>(...)`
- `UGameplayStatics::FinishSpawningActor(...)`
- `World->SpawnActor<AT66EnemyBase>(...)`

Controller possession is currently implicit through `AT66EnemyBase` constructor defaults (`AIControllerClass`, `AutoPossessAI`). A lightweight `AActor` enemy should not need this.

Spawn validation and positioning:

- Initial and runtime spawn paths use `ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn`.
- Spawn height uses a hardcoded `EnemyCapsuleHalfHeight = 88.f`.
- `FPendingEnemySpawn` stores `GroundLocation`, `ClassToSpawn`, `MobID`, mini-boss flag, wall-spawn flag, scalars, stage number, wall normal, and channel.

Rise/wall emergence:

- `SpawnNextStaggeredBatch()` calls `Enemy->StartEmergeFromWall()` or `Enemy->StartRiseFromGround()`.
- Both are custom transform animations inside `AT66EnemyBase::Tick()`, not CMC-driven.
- During emergence, capsule collision response to `ECC_Pawn` is temporarily ignored/restored.

Current Mass scaffold coupling:

- `SpawnNextStaggeredBatch()` checks `UT66MassEnemySpawnBridge::IsSpawnBridgeEnabled()`.
- If enabled and the spawn is a basic mob, it calls `MassSpawnBridge->EnqueueSpawnRequest(Slot)` before spawning the actor.
- Rollback should remove this additive shadow path and the Mass include/dependency chain.

## 7. Tick Infrastructure

Code-visible tick setup:

- `AT66EnemyBase` constructor sets `PrimaryActorTick.bCanEverTick = true`.
- `AT66BossBase` constructor sets `PrimaryActorTick.bCanEverTick = true`.
- `AT66ChestMimicEnemy` constructor sets `PrimaryActorTick.bCanEverTick = true`.
- `AT66EnemyProjectileBase` constructor sets `PrimaryActorTick.bCanEverTick = true`.
- `AT66EnemyAIController` constructor sets `PrimaryActorTick.bCanEverTick = false`.

For a typical `AT66EnemyBase`, the explicit T66-created components do not enable their own ticks in code:

- `VisualMesh` is a `UStaticMeshComponent`.
- `BodyHitZone` and `HeadHitZone` are sphere components.
- `LockIndicatorWidget` is hidden unless locked.

The main per-enemy runtime costs are:

- actor tick (`AT66EnemyBase::Tick()`);
- CMC tick;
- possible skeletal mesh animation tick if a skeletal visual is active;
- overlap/physics query work from the capsule and hit zones;
- optional lock widget work only when shown.

Essential behavior currently inside `AT66EnemyBase::Tick()`:

- VAT animation frame update;
- rise/wall-emerge transform interpolation;
- cached target resolution;
- safe-zone avoidance and loiter;
- status timers;
- movement speed modifiers;
- stun/root/freeze stops;
- confusion wander;
- family behavior dispatch;
- combat debug draw.

Central-manager candidates:

- target resolution;
- safe-zone checks;
- status timers;
- movement integration;
- family behavior;
- VAT frame updates;
- debug draw gating.

Event-driven/component behavior that can stay local:

- collision overlap if each lightweight actor keeps a capsule;
- hit zone components;
- lock widget visibility;
- projectile actor ticking until projectile pooling is tackled separately.

## 8. Animation

Current mob VAT:

- Data lives in `Content/Data/MobVertexAnimations.csv`.
- Runtime row type is `FT66MobVertexAnimationRow` in `Source/T66/Data/T66DataTypes.h`.
- Application is in `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual()`.
- Per-enemy state is stored in `AT66EnemyBase`:
  - `ActiveMobVertexAnimationRow`
  - `ActiveMobVertexAnimationMID`
  - `ActiveMobVertexAnimationClip`
  - `MobVertexAnimationClipTime`
  - `MobVertexAnimationOverrideSecondsRemaining`
  - `bUsingMobVertexAnimation`
- Ticking happens from `AT66EnemyBase::Tick()` through `TickMobVertexAnimationState()`.
- It depends on `UStaticMeshComponent`, not `USkeletalMeshComponent`.

The current VAT path is not directly ISM-ready because it writes scalar parameters on a per-enemy MID. It can coexist with `UInstancedStaticMeshComponent` only after a rendering strategy change:

- Use per-instance custom data for frame/clip and modify the material to read it; or
- maintain separate ISM buckets by frame/clip, which avoids material changes but may introduce instance churn.

Current non-VAT static enemies can be rendered by ISM as static mesh instances immediately, but they will not animate unless new VAT rows or a cheap mesh-frame strategy is created.

## 9. Mini-Bosses And Special Actors

### Goblin Thief

Files:

- `Source/T66/Gameplay/T66GoblinThiefEnemy.h`
- `Source/T66/Gameplay/T66GoblinThiefEnemy.cpp`

Dependencies:

- Inherits `AT66EnemyBase`.
- Constructor sets `GetCharacterMovement()->MaxWalkSpeed = 430.f`.
- Uses `GetMesh()` in `SetRarity()` when applying character visuals.
- Replaces base capsule overlap handler by calling `GetCapsuleComponent()` and binding `OnCapsuleBeginOverlapThief()`.
- Touch behavior steals gold via run state instead of heart damage.

Recommendation: keep on the richer actor path initially. It is not a performance-critical swarm mob, and it mutates overlap behavior and rarity visuals.

### Unique Debuff Enemy

Files:

- `Source/T66/Gameplay/T66UniqueDebuffEnemy.h`
- `Source/T66/Gameplay/T66UniqueDebuffEnemy.cpp`

Dependencies:

- Inherits `AT66EnemyBase`.
- Constructor sets `GetCharacterMovement()->MaxWalkSpeed = 420.f`.
- Constructor sets `Move->SetMovementMode(MOVE_Flying)`.
- Uses base tick plus a lifetime countdown.
- Uses a timer to fire unique debuff projectiles.
- Lifts itself by `HoverHeight` with `SetActorLocation()`.

Recommendation: keep on the richer actor path initially. It is timer/projectile/status-effect heavy and special by design.

### Bosses

Files:

- `Source/T66/Gameplay/T66BossBase.h`
- `Source/T66/Gameplay/T66BossBase.cpp`
- `Source/T66/Gameplay/T66GamblerBoss.cpp`

Dependencies:

- `AT66BossBase` inherits `ACharacter`.
- Constructor sets AI controller, auto possess, CMC max speed/orientation/rotation rate, and `bUseControllerRotationYaw = false`.
- Uses `GetMesh()` for skeletal/static visual application.
- Has boss part hit zones, attack profiles, timers, ground AOE, projectile volleys, lane blockers, boss state persistence, and extensive status effect logic.
- Helper `T66ApplyBossDisplacement(ACharacter* Character, ...)` is ACharacter-specific.

Recommendation: bosses should stay on their current actor/character path indefinitely unless a separate boss-specific refactor is justified.

### Mini-Bosses

Mini-bosses are currently regular stage mobs scaled up by the director:

- `FPendingEnemySpawn::bIsMiniBoss`
- `Enemy->ApplyMiniBossMultipliers(...)`
- `ActiveMiniBoss = Enemy`

They share basic mob classes today, but the locked replacement architecture says mini-bosses remain actor-only indefinitely. That means the director needs a clear split: basic spawns can use the lightweight path, while `bIsMiniBoss` spawns continue to instantiate the rich actor class.

### Chest Mimic And Scripted/Test Enemies

`AT66ChestMimicEnemy` inherits `AT66EnemyBase`, sets `InitialLifeSpan = 5.0f`, changes movement speed, touch damage, and loot. Tutorial, lab, tower guardian, overlay test spawns, and idol VFX targets also spawn or store `AT66EnemyBase` directly. These should be treated as special/rich-actor usages until the basic runtime path is stable.

## 10. Risks And Unknowns

### Harder Than The High-Level Proposal Suggests

- `AActor` does not have `AddMovementInput()`, `AIControllerClass`, `AutoPossessAI`, `GetCharacterMovement()`, `GetMesh()`, or `GetCapsuleComponent()`. Many usages are straightforward, but the compile blast radius is real.
- `AT66EnemyBase` is the type used by combat, registry, HUD, traps, run state, lab, tutorial, tower, and pool systems. Most of those systems do not need `ACharacter`, but they do need the same enemy API.
- `AT66RangedEnemy::FireProjectileAtPlayer()` uses `this` as `FActorSpawnParameters::Instigator`. That is an `APawn*` slot and must be changed.
- `UT66CharacterVisualSubsystem::ApplyMobVertexAnimationVisual()` has ACharacter-only capsule-bottom alignment. VAT mobs may visually shift after an AActor migration unless this is updated.
- `UT66EnemyPoolSubsystem` and `AT66EnemyBase::ResetForReuse()` explicitly manipulate CMC tick and movement mode.
- The director's spawn height uses a hardcoded capsule half height. A lightweight collision shape needs an equivalent shape descriptor.
- If ISM rendering replaces per-actor `VisualMesh`, any code expecting a visible mesh component on the enemy actor becomes a risk. Current combat hit zones are independent, but lock indicator, debug visuals, screenshots, and lab previews need validation.

### Looks Fine But Could Break In Playtest

- Touch damage could become invisible again if manager spatial checks replace capsule overlaps without debug parity.
- Hero auto-target and lock-on are actor-list based. If lightweight enemies are no longer individual target actors, combat needs a target provider or handle abstraction before the actor path is removed.
- VAT frame choice currently uses `GetVelocity()`. Custom movement must provide real per-enemy velocity for animation state.
- Safe-zone avoidance is cached per actor today. A central manager can improve it, but the exact update cadence affects enemies near safe-zone boundaries.
- Rise-from-ground and wall-emerge are currently tick-local state machines. Moving them to the manager requires preserving the collision-response timing.
- Lab, collector overlay, tower gate guardians, tutorial enemies, idol VFX test targets, and test-room automation all spawn `AT66EnemyBase` directly. Those flows may not show up in the main Dungeon benchmark.
- Networking was not a current focus in the inspected enemy code. If multiplayer replication is revived later, removing ACharacter movement replication changes assumptions.
- Save/load of transient enemy state does not look central for regular mobs, but boss state and run-state damage logs are real. Do not apply the lightweight path to bosses.

## 11. Honest Opinion

I agree with the Lightweight Actor direction for T66's regular enemy scale. The evidence is:

- The current regular enemy AI has already abandoned NavMesh pathing.
- Family behavior is vector chase/flee/rush/hover, not behavior tree or path-following heavy.
- Regular enemy visuals are static mesh or VAT static mesh, not skeletal.
- Combat handles are actor-based, not character-based.
- The Mass plugin scaffold regressed the 90-cap baseline before spawning any Mass entities.
- T66's stated target is 50-80 peak enemies in normal gameplay, with a 300-cap stress comparison point. That scale does not justify paying a 17 percent 90-cap fixed cost for a framework whose payoff arrives much later.

I do not recommend changing the single `AT66EnemyBase` inheritance while all special enemies still inherit it. The safer implementation shape is one of these:

1. Preferred: introduce a new lightweight basic-mob actor path and keep `AT66EnemyBase` as the rich actor path for specials until parity is proven.
2. If the final architecture must keep the name `AT66EnemyBase` for basic mobs: first create a rich `AT66CharacterEnemyBase : ACharacter`, move goblin thief, unique debuff, chest mimic, mini-boss/special spawns, tutorial scripted special cases, and any promoted enemies to it, then convert `AT66EnemyBase` to `AActor`.

The second path preserves the architectural intent while avoiding a giant one-step break of special actors.

## 12. Recommended Pass Sequence

### Should Rollback And Lightweight Actor Be Separate?

Yes. Keep Pass A rollback and Pass B Lightweight Actor separate.

Reason:

- The Mass scaffold has a measured fixed regression independent of gameplay logic.
- Rollback has a simple pass/fail measurement: remove the fixed cost and recover the prior 90-cap baseline.
- Lightweight Actor changes will touch enemy inheritance, movement, visuals, pooling, projectile fire, director spawn routing, HUD/combat assumptions, and likely test hooks. Mixing that with rollback would make any regression ambiguous.

### Minimum Verification Between Pass A And Pass B

After Pass A rollback:

- Build Development standalone.
- Refresh the staged standalone build because the playable build is affected.
- Verify the taskbar shortcut still points to `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Run the standard 90-cap `enemywaveperf` capture.
- Confirm `MassGameplay`, `ZoneGraph`, `ZoneGraphAnnotations`, `SmartObjects`, and `StateTree` are not mounted through the Mass path in the runtime log.
- Confirm the `>= 80` saturated band is back within 5 percent of the fixed-instrumentation 90-cap actor baseline, or document why not.
- Confirm no `T66.Mass.SpawnBridge.Enabled` path, Mass counters, or Mass schema additions remain unless deliberately retained as inert docs only.

### If Combined Anyway, Build-Green Order

Not recommended, but if rollback and Lightweight Actor are combined, keep the build green in this order:

1. Remove Mass plugin enablement, Mass module dependencies, Mass scaffold files, Mass PerformanceSystem counters, schema changes, and the director Mass spawn-bridge calls. Build.
2. Add a lightweight enemy manager/subsystem with no behavior changes. Build.
3. Extract basic movement/status/family calculations behind functions or a small runtime state adapter while `AT66EnemyBase` is still `ACharacter`. Build and run a short combat smoke.
4. Split the rich special path if using the `AT66EnemyBase` name for lightweight mobs. Move goblin thief, unique debuff, chest mimic, mini-boss/special director spawns, tutorial special actors, and promoted/locked actors to the rich `ACharacter` path. Build.
5. Add explicit collision/visual components and compatibility accessors for the lightweight basic actor: capsule root or damage capsule, body/head hit zones, lock widget, visual identity, stored velocity, and actor-location APIs. Build.
6. Replace `AddMovementInput()`, CMC speed writes, CMC movement modes, CMC stops, and CMC velocity writes with central manager integration. Build.
7. Fix projectile fire instigator assumptions and pool reset/release movement hooks. Build.
8. Move VAT frame ticking from per-actor tick to the manager, still using per-actor visual components first. Build and visual-smoke.
9. Disable per-basic-enemy actor tick. Build and benchmark.
10. Add ISM/HISM rendering only after gameplay parity is stable, because ISM changes the visual/debug proof surface.

This keeps each checkpoint attributable. The most important discipline is not to combine inheritance, movement, and ISM rendering in the same first pass.

## Bottom Line

The Lightweight Actor pivot is codebase-appropriate. T66's regular enemies already behave like simple direct-movement actors, and the current visual data is friendly to static/VAT instancing. The real work is preserving the existing `AT66EnemyBase` gameplay contract while replacing the inherited `ACharacter`/CMC convenience layer. The risky part is not movement math; it is keeping combat targeting, debug visibility, touch damage, pooling, lab/tutorial/tower spawns, and VAT alignment identical while changing the foundation.
