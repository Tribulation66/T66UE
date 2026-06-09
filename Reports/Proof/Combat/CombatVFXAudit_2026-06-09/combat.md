# Combat & VFX Audit — Pre-Inflation Rebuild

**Date:** 2026-06-09
**Mode:** Read-only evidence dump. No edits made.
**Project:** C:\UE\T66 (UE 5.7)
**Purpose:** Document the CURRENT combat system before we rebuild around the inflation / physical-balloon concept.

All claims are backed by quoted code with `file:line` references. No recommendations — facts only.

---

## TL;DR (one screen)

| Topic | What we have today |
|---|---|
| Hero projectile actor | `AT66HeroProjectile : AActor` with `USphereComponent` (overlap), `UStaticMeshComponent` (visual), `UNiagaraComponent` (trail), `UProjectileMovementComponent`. |
| Movement | `UProjectileMovementComponent` at 2400 u/s, optional homing, OR overridden by deterministic timed lerp for bounce/travel carriers. |
| Hit detection | Pure `OnComponentBeginOverlap` (overlap only) + per-tick distance check for "arrived at target". No traces. |
| Damage flow | Projectile → `AT66EnemyBase::TakeDamageFromHero(int32, FName, FName)` or `AT66BossBase::TakeDamageFromHeroHit(...)`. Int32 HP on enemies, float HP on hero. **Custom path; not UE `TakeDamage`/`FDamageEvent`.** |
| VFX authoring | Commandlet-built Niagara systems (`T66Hero1Axe{AOE,Pierce,Bounce,DOT}VFXCommandlet`) emit `NS_Hero1Axe*_MeshSlash.uasset` to `/Game/VFX/Hero1/Axe/...`. Pixel VFX via `NS_PixelParticle`. **No JSON/MCP authoring bridge** — closest analogue is the commandlet pattern itself. |
| Renderer mix | Mesh renderers for slash arcs (`SM_Hero1AxeAOE_SlashArc` built in code), sprite renderers for support flares & pixel sprays, no ribbon on hero attacks. |
| Spawn vs pool | Bounce/travel carriers: per-shot `SpawnActorDeferred<AT66HeroProjectile>`. Travelers (idol projectiles + DOT ticks): pooled via `UT66OutgoingTravelerPoolSubsystem` (20,000 preallocated slots, instanced Niagara). |
| Elements | 5 elements × 4 categories = 16 idols. `ET66IdolElement { Fire, Ice, Electricity, Nature, Wind }`. Element → secondary stat power multiplier + visual profile ID string + Niagara asset path lookup. **Element is NOT a field on the projectile actor.** |
| Health model | Hero: `float CurrentHP/MaxHP` on `UT66RunStateSubsystem` (default 100; hearts = HP/20 for UI only). Enemy: `int32 CurrentHP/MaxHP` on `AT66EnemyBase` (default 100). Boss: int32 aggregated on RunState. |
| Death VFX | `SpawnDeathBurstAtLocation` (16 pixels @ 60 radius) + `T66SpawnBloodSpray` (84 pixels @ 150 radius) via `NS_PixelParticle`. No mesh/explosion. |
| Knockback on hit | **Velocity stagger only** — direct `CharacterMovement->Velocity = AwayFromHit * Speed` for ~0.25s. NO impulse, NO ragdoll, NO Z component. Only auto-attacks trigger it; projectiles apply zero knockback. |
| Physics state | Enemy meshes `ECollisionEnabled::NoCollision`; capsule is QueryAndPhysics but kinematic. No `SetSimulatePhysics(true)` anywhere on enemies. Hero ragdoll component (`UT66KnockbackComponent`) exists but is hero-side only. |

---

## 1. Projectiles / Hero Attacks

### 1.1 The projectile actor

`AT66HeroProjectile` is a plain `AActor` (not `APawn`/`ACharacter`). It owns 5 components.

`C:/UE/T66/Source/T66/Gameplay/T66HeroProjectile.h` declares the components and the damage payload:

```cpp
// T66HeroProjectile.h
UCLASS()
class T66_API AT66HeroProjectile : public AActor
{
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
    TObjectPtr<USphereComponent> CollisionSphere;          // ~30u sphere, QueryAndPhysics, ECR_Overlap

    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent>      VisualMesh;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent>      AccentMesh;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UNiagaraComponent>         TrailVFXComponent;

    UPROPERTY() int32  Damage = 20;
    UPROPERTY() FName  DamageSourceID;
    UPROPERTY() TWeakObjectPtr<AActor> TargetActor;
    UPROPERTY() bool   bVisualOnly = false;
};
```

Defaults from the constructor (`T66HeroProjectile.cpp` ~line 23/49):

```cpp
InitialLifeSpan = 10.0f;                                  // Auto-destruct fallback
ProjectileMovement->InitialSpeed = 2400.f;
ProjectileMovement->MaxSpeed     = 2400.f;
ProjectileMovement->bRotationFollowsVelocity = true;
ProjectileMovement->ProjectileGravityScale   = 0.f;
```

### 1.2 Spawn paths — two systems coexist

**(A) Per-shot `SpawnActorDeferred` for bounce/travel visual carriers.** Called from inside `UT66CombatComponent::TryFire()` lambdas (`PerformBounce`, `PerformPierce`, etc.):

```cpp
// T66CombatComponent.cpp:1786 (SpawnBounceLinkProjectile)
AT66HeroProjectile* Projectile = World->SpawnActorDeferred<AT66HeroProjectile>(
    AT66HeroProjectile::StaticClass(),
    SpawnTransform,
    OwnerActor,
    nullptr,
    ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
// ... configure visuals, damage, target ...
Projectile->FinishSpawning(SpawnTransform);
```

`SpawnVisualTravelProjectile()` (`T66CombatComponent.cpp:1901`) uses the identical pattern for pierce/DOT visual carriers. These are **not pooled** — one actor per shot, destroyed on arrival or after 10s `InitialLifeSpan`.

**(B) `UT66OutgoingTravelerPoolSubsystem` for idol travelers and DOT ticks.** Slot-based pool, no actors:

```cpp
// T66OutgoingTravelerPoolSubsystem.h:265
bool FireOutgoingTraveler(
    const FT66OutgoingTravelerFireParams& FireParams,
    FT66OutgoingTravelerHandle& OutHandle);
```

The header (line ~271) preallocates up to **20,000** traveler slots and renders them through a single shared `UNiagaraComponent` with per-instance transforms uploaded each frame. Used at `T66CombatComponent.cpp:3430` and `:3521` for bounce-link and DOT ticks.

### 1.3 Movement model

Default flight is purely `UProjectileMovementComponent` (no Tick override). When a target is set, homing is enabled:

```cpp
// T66HeroProjectile.cpp ~line 218 (SetTargetActor)
ProjectileMovement->bIsHomingProjectile = true;
ProjectileMovement->HomingTargetComponent = TargetComp->GetRootComponent();
```

For bounce/travel carriers, `SetTimedVisualTravel()` switches to a deterministic lerp inside `Tick`:

```cpp
// T66HeroProjectile.cpp:87 (Tick, when bTimedVisualTravel is true)
const float Alpha = FMath::Clamp(VisualTravelElapsed / VisualTravelDuration, 0.f, 1.f);
const FVector NewLoc = FMath::Lerp(VisualTravelStart, TargetLocation, Alpha);
SetActorLocation(NewLoc);
// DeltaTime capped at 0.04s to survive capture hitches.
```

### 1.4 Hit detection — overlap only

Registered in `BeginPlay`:

```cpp
// T66HeroProjectile.cpp:60
CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66HeroProjectile::OnSphereOverlap);
```

Collision profile is hand-set:
```cpp
// T66HeroProjectile.cpp:25
CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
CollisionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
// VisualMesh / AccentMesh / TrailVFX → NoCollision
```

**No traces are performed** per tick. The overlap handler routes damage:

```cpp
// T66HeroProjectile.cpp:420 (OnSphereOverlap)
void AT66HeroProjectile::OnSphereOverlap(... AActor* OtherActor ...)
{
    if (bVisualOnly) return;                              // travel carriers ignore overlaps
    if (AActor* Intended = TargetActor.Get()) {
        if (OtherActor == Intended) {
            ApplyDamageToTarget(Intended);                // line 448
            Destroy();
        }
        return;                                            // ignore non-intended targets while bound
    }
    Enemy->TakeDamageFromHero(Damage, SourceID, NAME_None); // line 456
    Destroy();
}
```

There is also a per-Tick "did I arrive at target?" check for timed-travel carriers (`T66HeroProjectile.cpp:142`) that calls the same `ApplyDamageToTarget` when within `HitRadius` (or guaranteed to overshoot this tick).

### 1.5 Damage application — single funnel into enemy/boss

```cpp
// T66HeroProjectile.cpp:395 (ApplyDamageToTarget)
if (auto* Enemy = Cast<AT66EnemyBase>(Target))
    Enemy->TakeDamageFromHero(Damage, SourceID, NAME_None);   // ~line 406
else if (auto* Boss = Cast<AT66BossBase>(Target))
    Boss->TakeDamageFromHeroHit(Damage, SourceID, NAME_None); // ~line 414
```

`DamageSourceID` defaults to `UT66DamageLogSubsystem::SourceID_AutoAttack` and is what telemetry / floating-combat-text keys on.

### 1.6 VFX coupling

The projectile owns one Niagara component (the trail), assigned **after** spawn:

```cpp
// T66HeroProjectile.cpp:43
TrailVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailVFX"));
TrailVFXComponent->SetupAttachment(VisualMesh);
TrailVFXComponent->SetAutoActivate(false);

// T66HeroProjectile.cpp:258 (SetTrailVFX)
TrailVFXComponent->SetAsset(InTrailSystem);
TrailVFXComponent->SetVariableLinearColor(FName(TEXT("User.Color")), TrailColor);
TrailVFXComponent->Activate(true);
```

The big "slash" carrier VFX is **not parented to the projectile**. It is spawned independently and may be hard-attached to the projectile root for bounce/travel:

```cpp
// T66CombatComponent.cpp:1837 (carrier attach inside SpawnBounceLinkProjectile)
UNiagaraComponent* CarrierComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
    CarrierSystem,
    CarrierAttachRoot,
    NAME_None,
    FVector::ZeroVector, FRotator::ZeroRotator,
    EAttachLocation::SnapToTarget,
    /*bAutoDestroy*/true, /*bAutoActivate*/true,
    ENCPoolMethod::AutoRelease, true);
```

### 1.7 Pooling summary

| Category | Pooled? | Pool implementation |
|---|---|---|
| Hero bounce/travel carrier (`AT66HeroProjectile`) | No — `SpawnActorDeferred` per shot | n/a |
| Idol travelers (Fire/Ice/Electricity/Nature/Wind × Pierce/Bounce/AOE/DOT) | Yes | `UT66OutgoingTravelerPoolSubsystem`, 20k slots, instanced Niagara |
| DOT periodic ticks | Yes | Same traveler subsystem |
| Enemy/boss projectiles | Yes (HISM) | `UT66ProjectileManagerSubsystem` (separate, not used by hero) |

---

## 2. VFX System

### 2.1 Authoring origin — commandlets, not Niagara editor

The big surprise is that the hero attack Niagara systems are **constructed in C++** by editor commandlets and serialized to `.uasset` on disk. The commandlets live in `Source/T66/Gameplay/T66Hero1Axe{AOE,Pierce,Bounce,DOT}VFXCommandlet.{h,cpp}`.

Representative entry point and asset construction (`T66Hero1AxeAOEVFXCommandlet.cpp`):

```cpp
// :1085 (Main)
int32 UT66Hero1AxeAOEVFXCommandlet::Main(const FString& Params) { ... }

// :832 (build the system)
UNiagaraSystem* T66CreateSlashNiagaraSystem(const bool bCarrierOnly, const float DevSlowFactor)
{
    UPackage* Package = CreatePackage(*SlashNiagaraPackagePath);
    UNiagaraSystem* SlashSystem = NewObject<UNiagaraSystem>(
        Package, FName(T66Hero1AxeAOESlashNiagaraObjectName), RF_Public | RF_Standalone);
    UNiagaraSystemFactoryNew::InitializeSystem(SlashSystem, true);

    for (const FT66SlashLayerConfig& Config : T66Hero1AxeAOESlashLayerConfigs) {
        if (!T66AddSlashLayerEmitter(*SlashSystem, Config, DevSlowFactor)) return nullptr;
    }
    FAssetRegistryModule::AssetCreated(SlashSystem);
    return SlashSystem;
}
```

The slash arc mesh itself is generated in code:

```cpp
// T66Hero1AxeAOEVFXCommandlet.cpp:904  (T66BuildSlashArcMesh) → SM_Hero1AxeAOE_SlashArc
```

Run via: `UnrealEditor-Cmd.exe T66.uproject -run=UT66Hero1AxeAOEVFXCommandlet [-T66Hero1AxeAOEProduction] [-T66Hero1AxeAOEFullLayerStack] [-T66Hero1AxeAOEDevSlow=6.0]`.

### 2.2 Renderer types

The commandlets configure renderer modules per emitter:

- **Mesh renderers** — every slash layer (the curved arc that sweeps across the hit). One particle per emitter spawn; mesh reference is the in-code `SM_Hero1AxeAOE_SlashArc`. Mesh rotation is driven by an `InitialMeshRotation` module + `MeshRotationForce` module configured per `FT66SlashLayerConfig`.
- **Sprite renderers** — support flares, sparks, motes (config block `T66Hero1AxeAOESlashLayerConfigs` ~lines 160-222 of `T66Hero1AxeAOEVFXCommandlet.cpp`). Alignment/facing set via `ENiagaraSpriteAlignment` / `ENiagaraSpriteFacingMode`. Sizes around 14×14 px.
- **No ribbon renderers** are present in the hero attack stack.

Pixel VFX (`NS_PixelParticle`) is a sprite-only system used for death sprays and fallback attacks; spawned in budgeted bursts (`T66SpawnBudgetedPixel`, `T66CombatVFX.cpp:1067`).

### 2.3 Spawn APIs

| Effect | Site | API |
|---|---|---|
| Carrier slash (Pierce/AOE/Bounce/DOT) | `T66CombatComponent.cpp:1508` (`TrySpawnBoundWeaponBaseSlashVFX`) — called from `PerformPierce` (2935), `PerformSlash` (3222), `PerformBounce` (3291), `PerformDOT` (3452) | `UNiagaraFunctionLibrary::SpawnSystemAtLocation` |
| Trail on projectile | `T66HeroProjectile.cpp:258` (`SetTrailVFX`) | `UNiagaraComponent::SetAsset` + `Activate` (attached at construction) |
| Hard-attached carrier (bounce/travel) | `T66CombatComponent.cpp:1837` | `UNiagaraFunctionLibrary::SpawnSystemAttached` |
| Idol-themed impact import | `T66CombatVFX.cpp:264` (`SpawnImportedNiagaraAtLocation`) | `UNiagaraFunctionLibrary::SpawnSystemAtLocation` |
| Death burst (16 pixels) | `T66EnemyBase.cpp:1586` calls `UT66CombatComponent::SpawnDeathBurstAtLocation(World, Loc, 16, 60.f)` | Internally → `T66SpawnBudgetedPixel` |
| Larger blood spray (84 pixels) | `T66CombatVFX.cpp:1453` (`SpawnDeathVFX`) → `T66SpawnBloodSpray(World, VFX, Loc, 84, 150.f, 0.09f)` | Internally → `T66SpawnBudgetedPixel` |

### 2.4 Coupling to projectile

Three patterns coexist:

1. **Owned trail on projectile** (`UNiagaraComponent TrailVFXComponent` member of `AT66HeroProjectile`).
2. **Standalone spawn at impact** — the dominant pattern for slash carriers. `TrySpawnBoundWeaponBaseSlashVFX` reads an impact context (`PublishWeaponImpactContext`, `T66CombatComponent.cpp:2932`) with `ImpactPoint`, `Forward`, `Radius`, and spawns the system at that transform. The projectile actor has no NS reference for the slash.
3. **Driven carrier** — for bounce, the projectile holds a `DrivenCarrierComponent` (`T66HeroProjectile.h:99`), parented and updated by tick. Visual-only.

### 2.5 Authoring / scripted bridge

There is **no JSON spec or external Model-Context-Protocol bridge** for Niagara authoring. The closest analogue is:

- The `UT66Hero1Axe*VFXCommandlet` family: hardcoded `FT66SlashLayerConfig` / `FT66SupportEmitterConfig` C++ structs, run headlessly to emit `.uasset` files. Command-line flags only toggle production vs lab paths and dev slowdown.
- `FT66CombatVFXBindingData` data binding resolved at runtime by `ResolveCombatVFXBinding` (`T66CombatComponent.cpp:1406`) — picks the slash NS from a weapon/attack key. Stored as data, but not user-authored JSON.

The Blender MCP that is connected in this session is for `.blend` work, not Niagara.

### 2.6 NS_* / FX asset paths observed in code

Hero-attack systems (commandlet-emitted):
- `/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash` (`T66Hero1AxeAOEVFXCommandlet.cpp:43`)
- `/Game/VFX/Hero1/Axe/Pierce/NS_Hero1AxePierce_MeshSlash`
- `/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash`
- `/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash`
- Lab variants under `/Game/VFXLab/Hero1/Axe/...` when `-T66Hero1AxeAOEProduction` is not set.

Shared / fallback:
- `/Game/VFX/NS_PixelParticle` (death burst, blood spray, fallback attack pixels) — `T66CombatVFX.cpp:1015, 1028`.
- `/Game/VFX/VFX_Attack1` — legacy fallback (`T66CombatVFX.cpp:1016, 1033`).

Stylized pack imports referenced by element/idol resolution (`T66CombatVFX.cpp:439-450`):
- `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire`
- `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01` (Fire pierce/bounce)
- `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Ice_Projectile_02.P_Ice_Projectile_02`
- `/Game/Stylized_VFX_StPack/Particles/P_Electric_Projectile_02.P_Electric_Projectile_02`
- `/Game/Stylized_VFX_StPack/Blueprints/BP_Storm.BP_Storm_C` (Electricity AOE)
- `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Poison_02.P_Poison_02` (Nature DOT)
- `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02` (Nature AOE)
- `/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Web_Projectile_01.P_Web_Projectile_01` (Nature pierce/bounce)

---

## 3. Damage / Health Model

### 3.1 Storage

**Hero — float, stored on the run-state subsystem** (not on the pawn):

```cpp
// T66RunStateSubsystem.h:1720
float CurrentHP = DefaultMaxHP;   // 100.0
float MaxHP     = DefaultMaxHP;   // 100.0
```

UI hearts are a pure display projection:

```cpp
// T66RunStateSubsystem.h:337
int32 GetCurrentHearts() const { return FMath::RoundToInt(CurrentHP / HPPerRedHeart); } // /20
int32 GetMaxHearts()     const { return FMath::RoundToInt(MaxHP     / HPPerRedHeart); }
```

**Enemy — int32, on the actor:**

```cpp
// T66EnemyBase.h:28
int32 MaxHP     = 100;
int32 CurrentHP = 100;
```

**Boss — int32, aggregated on RunState** (`T66RunStateSubsystem_Private.h:1942`):
```cpp
int32 BossMaxHP     = 100;
int32 BossCurrentHP = 0;
```

### 3.2 Damage signatures

**This is a custom path, not UE's `TakeDamage(FDamageEvent)`.** Hero side:

```cpp
// T66RunStateSubsystem.h:782
bool ApplyDamage(int32 DamageHP,
                 AActor* Attacker      = nullptr,
                 FName   DeliveryMethod = NAME_None,
                 AActor* DamageCauser   = nullptr);
```

Implements armor, dodge/evasion, Iron Will, Unflinching (15%), Counter-Attack, Assassinate (`T66RunStateSubsystem_Combat.cpp:1051-1107`).

Enemy side:

```cpp
// T66EnemyBase.h:126
virtual bool TakeDamageFromHero(int32 Damage, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
// :131
virtual bool TakeDamageFromHeroHitZone(int32 Damage, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID = NAME_None, FName EventType = NAME_None);
// :136
virtual bool TakeDamageFromEnvironment(int32 Damage, AActor* DamageCauser = nullptr, FName EventType = NAME_None);
```

All three funnel through `ApplyResolvedDamage`:

```cpp
// T66EnemyBase.cpp:1600
bool AT66EnemyBase::TakeDamageFromHero(int32 Damage, FName DamageSourceID, FName EventType)
{
    return ApplyResolvedDamage(Damage, /*bCreditHeroKill*/true, DamageSourceID, EventType);
}
```

### 3.3 Death trigger

`ApplyResolvedDamage` is where HP is mutated and death is detected:

```cpp
// T66EnemyBase.cpp:1570 (inside ApplyResolvedDamage)
if (CurrentHP <= 0)
{
    bLastDeathCreditedToHero = bCreditHeroKill;
    if (UWorld* World = GetWorld())
    {
        if (bCreditHeroKill) { RunState->NotifyEnemyKilledByHero(); }
        UT66CombatComponent::SpawnDeathBurstAtLocation(World, GetActorLocation(), 16, 60.f);
    }
    T66PlayEnemyAudioEvent(this, TEXT("Combat.Enemy.Death"), FName(TEXT("Combat.Enemy.Death")));
    SetMobVertexAnimationClip(T66MobVATClip_Death, 0.45f);
    OnDeath();
    return true;
}
```

`OnDeath()` (`T66EnemyBase.cpp:1780+`) hides lock indicator, awards score+XP, runs achievement checks, and notifies the director.

### 3.4 All current damage sources flowing into this funnel

| Source | Callsite | Function called |
|---|---|---|
| Hero auto-attack projectile | `T66HeroProjectile.cpp:395` `ApplyDamageToTarget` | `Enemy->TakeDamageFromHero(Damage, SourceID, NAME_None)` |
| Hero bounce (in-component) | `T66CombatComponent.cpp:3325` | same |
| Boss-floor AOE | `T66BossGroundAOE.cpp:300,307` | `TakeDamageFromHero` / `TakeDamageFromHeroHit` |
| Hero plague-cloud DOT | `T66HeroPlagueCloud.cpp` | `TakeDamageFromHero` |
| Lava | `T66LavaPatch.cpp:484` | `RunState->ApplyDamage(DamagePerTick, this, "LavaPatch")` (hero side) |
| Miasma | `T66MiasmaTile.cpp:100` | `RunState->ApplyDamage(20, this, "MiasmaTile")` |
| Floor traps (spike / flame) | `T66TrapDamageUtils` | `Enemy->TakeDamageFromEnvironment` |
| Enemy touch on hero | `T66EnemyBase.cpp:1536` `OnCapsuleBeginOverlap` | `RunState->ApplyDamage(DamageHP, this, "EnemyTouch")` |
| Enemy projectile on hero | `AT66EnemyProjectileBase` | `RunState->ApplyDamage(...)` |
| Boss hit on enemy/boss | `T66BossBase.cpp:3265` | `TakeDamageFromHeroHit` (boss-side variant) |

### 3.5 Element/idol tag on damage path?

**No.** Damage functions take `int32 Damage` and `FName DamageSourceID` only. Element power is applied at the **source** (CombatComponent multiplying base damage by `GetIdolElementPowerMultiplier` before calling `TakeDamageFromHero`), not at the receiver. There is no element field on `ApplyResolvedDamage` or `ApplyDamage`.

---

## 4. Elements / Idols

### 4.1 The enum and the "16"

Five elements, four delivery categories — 5 × 4 = 16 idols (the "16 elemental idols" name is the cross-product, not 16 elements).

```cpp
// T66DataTypes.h:1034
enum class ET66IdolElement : uint8
{
    Fire UMETA(DisplayName = "Fire"),
    Ice UMETA(DisplayName = "Ice"),
    Electricity UMETA(DisplayName = "Electricity"),
    Nature UMETA(DisplayName = "Nature"),
    Wind UMETA(DisplayName = "Wind"),
};
```

Idol rows live in `Content/Data/Idols.csv` (rows 2-21), keyed to `FIdolData` (`T66DataTypes.h:1968-2116`).

### 4.2 Active idol storage

Per-game-instance subsystem holds up to 3 equipped idol IDs:

```cpp
// T66IdolManagerSubsystem.h:71
UPROPERTY() TArray<FName> EquippedIdolIDs;       // MaxEquippedIdolSlots = 3
UPROPERTY() TArray<uint8> EquippedIdolLevels;    // rarity 1=Black .. 4=White
```

Cached into `UT66CombatComponent` in `BeginPlay` (`T66CombatComponent.cpp:1131`).

### 4.3 Attack → element resolution at fire time

```cpp
// T66CombatComponent.cpp:3904  (per cached idol slot, on fire)
const FIdolData& IdolData = CachedIdolSlot.IdolData;       // contains .Element
const float IdolElementPowerMult =
    T66CombatShared::GetIdolElementPowerMultiplier(CachedRunState, IdolData.Element);
const int32 IdolDamage =
    FMath::Max(1, FMath::RoundToInt(IdolData.GetDamageAtRarity(IdolRarity) * IdolElementPowerMult));

const FName        VisualProfileID =
    GetT66TravelerVisualProfileID(IdolID, IdolData.Element, IdolData.Category); // "TravelerVisual.Fire.Pierce"
const FLinearColor TravelerColor   =
    GetT66IdolElementTravelerColor(IdolID, IdolData.Element);
```

The `VisualProfileID` is a `FName` like `TravelerVisual.Fire.Pierce` and is what the traveler pool maps to mesh + Niagara recipe.

### 4.4 Power multiplier maps to a secondary stat per element

```cpp
// T66CombatShared.cpp:170
float GetIdolElementPowerMultiplier(const UT66RunStateSubsystem* RunState, const ET66IdolElement Element)
{
    return RunState
        ? FMath::Max(0.1f, RunState->GetSecondaryStatValue(
            GetElementPowerSecondaryForIdolElement(Element))) // FirePower / IcePower / ElectricityPower / NaturePower / WindPower
        : 1.f;
}
```

### 4.5 Niagara asset chosen by IdolID string lookup

```cpp
// T66CombatVFX.cpp:439-450  (paraphrased; chained if/return on IdolID)
if (IdolID == "Idol_Fire_DOT" || IdolID == "Idol_Fire_AOE")
    return "/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire";
if (IdolID == "Idol_Ice_*")
    return "/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Ice_Projectile_02.P_Ice_Projectile_02";
if (IdolID == "Idol_Electricity_*")
    return "/Game/Stylized_VFX_StPack/Particles/P_Electric_Projectile_02.P_Electric_Projectile_02";
// ... Nature, Wind, etc.
```

### 4.6 Per-element on-hit behavior

```cpp
// T66CombatComponent.cpp:3603
void UT66CombatComponent::ApplyIdolSpecialBehavior(...)
{
    // no-op stub
}
```

So element today changes: **damage scale**, **visual profile string**, **carrier Niagara asset**, **tint color**. It does **not** introduce per-element on-hit logic or different damage type categories.

### 4.7 Proof set used for VFX impact testing

```cpp
// T66CombatShared.cpp:99
const TSet<FName>& GetImpactPresentationProofIdols()
{
    static const TSet<FName> ImpactPresentationProofIdols = {
        FName(TEXT("Idol_Ice_AOE")),
        FName(TEXT("Idol_Electricity_Pierce")),
        FName(TEXT("Idol_Electricity_Bounce")),
        FName(TEXT("Idol_Nature_DOT")),
    };
    return ImpactPresentationProofIdols;
}
```

These are the four idols used by the existing axe VFX commandlets — one per category, but **not one per element** (Wind and Fire are absent from the proof set; Electricity appears twice).

### 4.8 Fusion vs lookup?

The projectile actor (`AT66HeroProjectile`) has **no** `Element` member. Element selection is entirely external string/enum lookups feeding into:
- damage `int32` (already scaled),
- traveler visual profile FName (string-keyed),
- Niagara asset path (string-keyed if-chain),
- tint `FLinearColor`.

Swapping the projectile asset per-element is therefore a clean enum→class lookup, not a refactor of projectile internals.

---

## 5. Enemy Death + Knockback

### 5.1 Knockback component (hero-only ragdoll)

`UT66KnockbackComponent` exists but is **owned by the hero**, not enemies. It does impulse-based ragdoll:

```cpp
// T66KnockbackComponent.cpp:121 (ApplyKnockbackLaunch)
const FVector ResolvedLaunchVelocity = T66ClampLaunchVelocity(LaunchVelocity, ActiveProfile);
MeshComponent->SetAllBodiesSimulatePhysics(true);
MeshComponent->SetAllBodiesPhysicsBlendWeight(1.f);
ApplyRagdollPhysicsResponseProfile(MeshComponent, Profile);
MeshComponent->WakeAllRigidBodies();
ApplyLaunchImpulse(MeshComponent, LaunchVelocity, Profile);   // mass-scaled impulse per body
```

Profile defaults (from the struct):
- `LaunchVelocityScale = 1.0`
- `MaxLaunchVelocity = 4200 u/s`
- `MinIncapacitationSeconds = 0.15`
- `MaxRagdollSeconds = 0.4`
- `SettleSpeed = 165 u/s`

### 5.2 Enemy-side knockback today is velocity-only

Only auto-attacks (not projectiles) trigger an enemy stagger, and it's not physics:

```cpp
// T66CombatComponent.cpp:4665
if (!bEnemyDied && ResolvedSource == UT66DamageLogSubsystem::SourceID_AutoAttack && Hero) {
    E->ApplyAutoAttackKnockback(Hero->GetActorLocation());
}
// :4681  (same for lightweight mobs)
if (!bMobDied && ResolvedSource == UT66DamageLogSubsystem::SourceID_AutoAttack && Hero) {
    M->ApplyAutoAttackKnockback(Hero->GetActorLocation());
}
```

```cpp
// T66EnemyBase.cpp:1625 (ApplyAutoAttackKnockback)
const float KnockbackSpeed = AutoAttackKnockbackSpeed * FMath::Max(0.f, StrengthScale);
FVector AwayFromHit = GetActorLocation() - HitOrigin;
AwayFromHit.Z = 0.f;
AwayFromHit.Normalize();
if (UCharacterMovementComponent* Move = GetCharacterMovement())
{
    Move->StopMovementImmediately();
    Move->Velocity = AwayFromHit * KnockbackSpeed;          // direct velocity assignment
}
AutoAttackKnockbackSecondsRemaining = AutoAttackKnockbackStutterSeconds; // ~0.25s
```

Hero projectiles (`AT66HeroProjectile::ApplyDamageToTarget`, `T66HeroProjectile.cpp:395-418`) call only `TakeDamageFromHero` — they apply **zero** knockback / impulse / launch on hit.

### 5.3 Enemy death sequence (rich `AT66EnemyBase`)

```
TakeDamageFromHero(...)
 └─> ApplyResolvedDamage(...)
       ├─ CurrentHP -= Damage
       └─ if (CurrentHP <= 0):
            ├─ bLastDeathCreditedToHero = ...
            ├─ RunState->NotifyEnemyKilledByHero()
            ├─ UT66CombatComponent::SpawnDeathBurstAtLocation(World, Loc, 16, 60.f)
            ├─ T66PlayEnemyAudioEvent(this, "Combat.Enemy.Death", ...)
            ├─ SetMobVertexAnimationClip(T66MobVATClip_Death, 0.45f)
            └─ OnDeath()
                 ├─ HideLockIndicator
                 ├─ Award score / XP
                 ├─ Achievement checks
                 └─ NotifyOwningDirectorOfDeath()
                       (actor is then destroyed via normal AActor flow / EndPlay)
```

### 5.4 Lightweight mob (`AT66MobBase`) — pooled death

```cpp
// T66MobBase.cpp:750  (when CurrentHP <= 0 inside TakeDamageFromHeroHitZone)
LifecycleState   = ET66MobLifecycleState::Dying;
StoredVelocity   = FVector::ZeroVector;
KnockbackVelocity= FVector::ZeroVector;
SetMobVertexAnimationClip(T66MobVATClip_Death, 0.45f);

RunState->AddHeroXP(XPValue);
MobLoot->SpawnMobLootFromNonBossDeath(this, MobID, bIsMiniBoss, ...);
T66TrySpawnMobKillLootBags(this, bIsMiniBoss);
T66TrySpawnMobKillStatBoost(this, bIsMiniBoss);

NotifyOwningDirectorOfDeath();                       // :772
Manager->NotifyMobDying(this);                       // :778 -> pool return next tick
```

Pool return is deferred one frame:

```cpp
// T66MobManagerSubsystem.cpp:2521 (NotifyMobDying)
Mob->LifecycleState = ET66MobLifecycleState::Dying;
World->GetTimerManager().SetTimerForNextTick(
    FTimerDelegate::CreateLambda([Manager, MobToRelease]() { Manager->ReleaseMob(MobToRelease); }));

// :2425 (ReleaseMob)
Mob->SetActorHiddenInGame(true);
Mob->SetActorEnableCollision(false);
Mob->SetActorTickEnabled(false);
Mob->GetCharacterMovement()->StopMovementImmediately();
Mob->GetCharacterMovement()->SetComponentTickEnabled(false);
Mob->SetActorLocation(FVector(0.f, 0.f, -50000.f));   // park below world
Pool.FindOrAdd(Mob->GetClass()).Add(Mob);
```

### 5.5 Death VFX assets

```cpp
// T66EnemyBase.cpp:1586          (in-line, every enemy death)
UT66CombatComponent::SpawnDeathBurstAtLocation(World, GetActorLocation(), 16, 60.f);

// T66CombatVFX.cpp:1453          (SpawnDeathVFX, callable on demand)
UNiagaraSystem* VFX = GetActiveVFXSystem();    // → NS_PixelParticle path resolved via PixelVFXSubsystem
T66SpawnBloodSpray(World, VFX, Location, /*count*/84, /*radius*/150.f, /*scale*/0.09f);
```

Both ultimately spawn pixel sprites through `T66SpawnBudgetedPixel` (`T66CombatVFX.cpp:1067`) backed by `NS_PixelParticle`. **There is no mesh-based death VFX** — no balloon pop, no rigid shrapnel, no skeletal explode-from-pose.

### 5.6 Physics state on enemies

```cpp
// T66EnemyBase.cpp:368
if (USkeletalMeshComponent* Skel = GetMesh()) {
    Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Skel->SetVisibility(false, true);
}

// T66MobBase.cpp:299
VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
```

Capsule is `QueryAndPhysics` + `ECC_Pawn` but kinematic — no `SetSimulatePhysics(true)` anywhere on enemies/mobs. Movement is `UCharacterMovementComponent` (walking/flying/ranged-chase). **There is no ragdoll path on enemies today.**

---

## 6. Integration Points for the Inflation / Balloon Rebuild

Quoting only — recommendations are out of scope for this audit.

### 6.1 Swap projectile visuals to spawned meshes

The clean seams to attach a per-element balloon mesh are:

- `AT66HeroProjectile`'s existing `VisualMesh` / `AccentMesh` `UStaticMeshComponent` slots (`T66HeroProjectile.h:25-29`). They already accept any static mesh via the temporary-projectile-system profile path:
  ```cpp
  // T66CombatComponent.cpp:1869
  Projectile->ConfigureTemporaryProjectileVisual(
      FT66TemporaryProjectileSystem::ProfileHeroBounce(), Color, ScaleMultiplier);
  ```
  `T66TemporaryProjectileSystem.{h,cpp}` declares the profile catalog (Sphere/Cone/Cylinder/Cube today).
- The traveler pool's per-instance mesh selection, keyed by `VisualProfileID` (`T66CombatComponent.cpp:3931`). Adding `TravelerVisual.<Element>.Balloon` strings would route through the same lookup the system already uses for `TravelerVisual.Fire.Pierce`.
- The slash NS systems are emitted by commandlets in `Source/T66/Gameplay/T66Hero1Axe{AOE,Pierce,Bounce,DOT}VFXCommandlet.cpp` — replacing or augmenting their mesh references (`SM_Hero1AxeAOE_SlashArc` built by `T66BuildSlashArcMesh`, `T66Hero1AxeAOEVFXCommandlet.cpp:904`) is contained to those files.

### 6.2 Attach a collision shape for physical hits / knockback

The hooks already exist:

- `AT66HeroProjectile::CollisionSphere` is a `USphereComponent` with `QueryAndPhysics` + `ECR_Overlap` (`T66HeroProjectile.h:25`, `.cpp:25`). Changing the response set, swapping to box/capsule, or adding a second physics-block shape lands here.
- The overlap hookup is in one place: `T66HeroProjectile.cpp:60` (`CollisionSphere->OnComponentBeginOverlap.AddDynamic(...)`).
- Enemy capsule already accepts physics queries (`ECC_Pawn`, `QueryAndPhysics`); enemies don't simulate physics today, so layering knockback on the receiving side is centralized in `T66EnemyBase::ApplyAutoAttackKnockback` (`T66EnemyBase.cpp:1625`) and the hero-side `UT66KnockbackComponent::ApplyKnockbackLaunch` (`T66KnockbackComponent.cpp:121`) — both are isolated and currently velocity-only on enemies, ragdoll-on-hero.
- The existing call gate for who-applies-knockback is `T66CombatComponent.cpp:4665/4681` (one branch per enemy / mob, gated on `SourceID_AutoAttack`). Projectile callers do not flow through this gate.

### 6.3 Replace the health model with inflation %

The narrow waist of the damage path is two functions:

- Enemy: `AT66EnemyBase::ApplyResolvedDamage(int32 Damage, bool bCreditHeroKill, FName DamageSourceID, FName EventType)` (`T66EnemyBase.cpp:1543-1598`) — sole mutator of `CurrentHP` and sole trigger of `OnDeath()`.
- Hero: `UT66RunStateSubsystem::ApplyDamage(int32, AActor*, FName, FName)` (`T66RunStateSubsystem.h:782`, body in `T66RunStateSubsystem_Combat.cpp:1051+`) — sole mutator of hero `CurrentHP`.

Boss aggregation: `BossCurrentHP / BossMaxHP` on `T66RunStateSubsystem_Private.h:1942` plus `AT66BossBase::TakeDamageFromHeroHit` (`T66BossBase.cpp:3265`).

Hearts are already a derived UI projection (`T66RunStateSubsystem.h:337`) — remapping to inflation % is a display-side concern at that function and does not require touching the storage.

The damage log subsystem (`UT66DamageLogSubsystem`, `Source/T66/Core/T66DamageLogSubsystem.{h,cpp}`) keys by `DamageSourceID` `FName`, so swapping HP→inflation does not break telemetry.

---

## Appendix A — Key file list

```
Source/T66/Gameplay/T66HeroProjectile.{h,cpp}                       projectile actor
Source/T66/Gameplay/T66TemporaryProjectileSystem.{h,cpp}            profile catalog (shape/mesh)
Source/T66/Gameplay/T66ProjectileManagerSubsystem.{h,cpp}           HISM-pooled enemy/boss projectiles
Source/T66/Gameplay/T66OutgoingTravelerPoolSubsystem.h              20k traveler pool, instanced niagara
Source/T66/Gameplay/T66CombatComponent.{h,cpp}                      TryFire + Perform{Pierce,Slash,Bounce,DOT}
Source/T66/Gameplay/T66CombatVFX.cpp                                niagara/pixel spawn helpers, idol asset lookup
Source/T66/Gameplay/T66Hero1Axe{AOE,Pierce,Bounce,DOT}VFXCommandlet.{h,cpp}   niagara/mesh authoring commandlets
Source/T66/Gameplay/T66HeroOneAttackVFX.{h,cpp}                     wrapper helpers
Source/T66/Gameplay/T66DotMarkerVFX.{h,cpp}                         DOT impact marker
Source/T66/Gameplay/T66HeroPlagueCloud.{h,cpp}                      DOT ground cloud
Source/T66/Gameplay/T66HeroBase.{h,cpp}                             hero pawn
Source/T66/Gameplay/T66EnemyBase.{h,cpp}                            rich enemy + damage funnel + death
Source/T66/Gameplay/T66MobBase.{h,cpp}                              lightweight pooled mob + death
Source/T66/Gameplay/T66BossBase.{h,cpp}                             boss damage receiver
Source/T66/Gameplay/T66KnockbackComponent.{h,cpp}                   hero ragdoll-on-hit
Source/T66/Gameplay/T66CombatShared.{h,cpp}                         element power, proof idol set
Source/T66/Gameplay/T66CombatHitZoneComponent.{h,cpp}               hit-zone routing
Source/T66/Core/T66IdolManagerSubsystem.{h,cpp}                     equipped idol slots
Source/T66/Core/T66RunStateSubsystem.{h,cpp}                        hero HP, hearts, ApplyDamage
Source/T66/Core/T66RunStateSubsystem_Combat.cpp                     hero damage resolution (armor/dodge)
Source/T66/Core/T66DamageLogSubsystem.{h,cpp}                       damage telemetry by SourceID
Source/T66/Core/T66PixelVFXSubsystem.{h,cpp}                        pixel/sprite spray
Source/T66/Data/T66DataTypes.h                                      ET66IdolElement, FIdolData
Content/Data/Idols.csv                                              16 idol rows
```

## Appendix B — Asset paths observed

```
/Game/VFX/Hero1/Axe/AOE/NS_Hero1AxeAOE_MeshSlash             (commandlet-emitted)
/Game/VFX/Hero1/Axe/Pierce/NS_Hero1AxePierce_MeshSlash       (commandlet-emitted)
/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash       (commandlet-emitted)
/Game/VFX/Hero1/Axe/DOT/NS_Hero1AxeDOT_MeshSlash             (commandlet-emitted)
/Game/VFXLab/Hero1/Axe/...                                    (lab variants when -T66Hero1AxeAOEProduction not set)
/Game/VFX/NS_PixelParticle                                    (death + fallback pixel sprays)
/Game/VFX/VFX_Attack1                                         (legacy fallback)
/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Fire.P_Fire
/Game/Stylized_VFX_StPack/Particles/UPDATE_1_4/P_Weapon_01.P_Weapon_01
/Game/Stylized_VFX_StPack/Particles/UPDATE_1_2/P_Ice_Projectile_02.P_Ice_Projectile_02
/Game/Stylized_VFX_StPack/Particles/P_Electric_Projectile_02.P_Electric_Projectile_02
/Game/Stylized_VFX_StPack/Blueprints/BP_Storm.BP_Storm_C
/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Poison_02.P_Poison_02
/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Dirt_Spikes_02.P_Dirt_Spikes_02
/Game/Stylized_VFX_StPack/Particles/UPDATE_1_3/P_Web_Projectile_01.P_Web_Projectile_01
```

— END OF AUDIT —
