// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HeroOneAttackVFX.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66DotMarkerVFX.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66AudioSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66RngSubsystem.h"
#include "Data/T66DataTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY(LogT66Combat);

// Capture/proof-only override for the readable Bounce link travel window. Default 0
// (off) means gameplay uses the normal ReadableBounceLinkTravelSeconds floor so the
// bounce keeps its production feel. When the Bounce VFX proof staging sets this > 0,
// each visual link travels over this longer window so the frame-rate-limited capture
// (which advances ~0.22s of game time per rendered frame) samples multiple frames of
// each link's hero->primary and primary->second travel. Runtime damage/targeting are
// resolved independently and unaffected; this only stretches the visual mover window.
static TAutoConsoleVariable<float> CVarBounceProofReadableTravelSeconds(
	TEXT("T66.Bounce.ProofReadableTravelSeconds"),
	0.f,
	TEXT("Proof-only override (seconds) for Bounce visual link travel time. 0 = use gameplay default."),
	ECVF_Default);

// Capture/proof-only override for the readable DOT projectile travel window. Default 0
// (off) means gameplay arrival is near-instant so DOT ticking starts without regression.
// When the DOT VFX proof staging sets this > 0, the single hero->target DOT shot travels
// over this longer window so the frame-rate-limited capture samples the moving projectile
// before impact, the marker reveal, and the first DOT ticks. Runtime damage/targeting are
// resolved independently; this only stretches the visual mover window.
static TAutoConsoleVariable<float> CVarDotProofReadableTravelSeconds(
	TEXT("T66.DOT.ProofReadableTravelSeconds"),
	0.f,
	TEXT("Proof-only override (seconds) for the DOT visual projectile travel time. 0 = near-instant gameplay default."),
	ECVF_Default);

namespace
{
	const TCHAR* GetT66AttackCategoryName(const ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::Pierce: return TEXT("Pierce");
		case ET66AttackCategory::AOE:    return TEXT("AOE");
		case ET66AttackCategory::Bounce: return TEXT("Bounce");
		case ET66AttackCategory::DOT:    return TEXT("DOT");
		default:                         return TEXT("Unknown");
		}
	}

	const TCHAR* GetT66CombatVFXSourceTypeName(const ET66CombatVFXBindingSourceType SourceType)
	{
		switch (SourceType)
		{
		case ET66CombatVFXBindingSourceType::WeaponBase:   return TEXT("WeaponBase");
		case ET66CombatVFXBindingSourceType::IdolModifier: return TEXT("IdolModifier");
		default:                                           return TEXT("Unknown");
		}
	}

	static TAutoConsoleVariable<int32> CVarT66CombatImpactSourceVerbose(
		TEXT("T66.Combat.ImpactSourceVerbose"),
		0,
		TEXT("Emit verbose weapon/idol combat impact context logs for VFX and damage-source proof runs."));

	FName GetT66AttackCategoryAudioEvent(const ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::Pierce: return FName(TEXT("Hero.Attack.Pierce"));
		case ET66AttackCategory::AOE:    return FName(TEXT("Hero.Attack.AOE"));
		case ET66AttackCategory::Bounce: return FName(TEXT("Hero.Attack.Bounce"));
		case ET66AttackCategory::DOT:    return FName(TEXT("Hero.Attack.DOT"));
		default:                         return FName(TEXT("Hero.Attack.Generic"));
		}
	}

	float GetIdolTierFloat(const ET66ItemRarity Rarity, const float Black, const float Red, const float Yellow, const float White)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return Black;
		case ET66ItemRarity::Red:    return Red;
		case ET66ItemRarity::Yellow: return Yellow;
		case ET66ItemRarity::White:  return White;
		default:                     return Black;
		}
	}

	int32 GetIdolTierInt(const ET66ItemRarity Rarity, const int32 Black, const int32 Red, const int32 Yellow, const int32 White)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return Black;
		case ET66ItemRarity::Red:    return Red;
		case ET66ItemRarity::Yellow: return Yellow;
		case ET66ItemRarity::White:  return White;
		default:                     return Black;
		}
	}

	bool RollTierChance(const float Chance01, UT66RngSubsystem* RngSub)
	{
		const float ClampedChance = FMath::Clamp(Chance01, 0.f, 1.f);
		if (ClampedChance <= 0.f)
		{
			return false;
		}

		return RngSub
			? (RngSub->GetRunStream().GetFraction() < ClampedChance)
			: (FMath::FRand() < ClampedChance);
	}

	void T66AddUniqueActor(TArray<AActor*>& Targets, AActor* Candidate)
	{
		if (Candidate)
		{
			Targets.AddUnique(Candidate);
		}
	}

	FVector T66ResolvePlanarDirection(const FVector& Direction, const AActor* FallbackActor)
	{
		FVector PlanarDirection(Direction.X, Direction.Y, 0.f);
		if (PlanarDirection.Normalize())
		{
			return PlanarDirection;
		}

		if (FallbackActor)
		{
			PlanarDirection = FVector(FallbackActor->GetActorForwardVector().X, FallbackActor->GetActorForwardVector().Y, 0.f);
			if (PlanarDirection.Normalize())
			{
				return PlanarDirection;
			}
		}

		return FVector::ForwardVector;
	}

	bool T66IsPointInsidePlanarSector(
		const FVector& Center,
		const FVector& Forward,
		const FVector& Point,
		const float Radius,
		const float HalfAngleDegrees,
		const float InnerRadius)
	{
		if (Radius <= 0.f)
		{
			return false;
		}

		FVector Delta(Point.X - Center.X, Point.Y - Center.Y, 0.f);
		const float DistSq = Delta.SizeSquared();
		const float ClampedInnerRadius = FMath::Clamp(InnerRadius, 0.f, FMath::Max(0.f, Radius - 1.f));
		if (DistSq > FMath::Square(Radius) || DistSq < FMath::Square(ClampedInnerRadius))
		{
			return false;
		}

		if (!Delta.Normalize())
		{
			return true;
		}

		const FVector PlanarForward = T66ResolvePlanarDirection(Forward, nullptr);
		const float MinDot = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(HalfAngleDegrees, 1.f, 179.f)));
		return FVector::DotProduct(PlanarForward, Delta) >= MinDot;
	}

	void T66AppendAwakenedBossTargetsInSphere(UWorld* World, const AActor* IgnoredActor, const FVector& Center, const float Radius, TArray<AActor*>& InOutTargets)
	{
		if (!World || Radius <= 0.f)
		{
			return;
		}

		const UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
		if (!Registry)
		{
			return;
		}

		const float RadiusSq = Radius * Radius;
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			AT66BossBase* Boss = WeakBoss.Get();
			if (!Boss || Boss == IgnoredActor || !Boss->IsAwakened() || !Boss->IsAlive())
			{
				continue;
			}

			if (FVector::DistSquared(Center, Boss->GetActorLocation()) <= RadiusSq)
			{
				T66AddUniqueActor(InOutTargets, Boss);
			}
		}
	}

	void T66AppendDamageableTargetsInSphere(UWorld* World, const AActor* IgnoredActor, const FVector& Center, const float Radius, TArray<AActor*>& InOutTargets)
	{
		if (!World || Radius <= 0.f)
		{
			return;
		}

		const UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
		if (!Registry)
		{
			return;
		}

		const float RadiusSq = Radius * Radius;
		Registry->ForEachDamageableTarget([&](AActor* Target)
		{
			if (!Target || Target == IgnoredActor)
			{
				return;
			}

			if (FVector::DistSquared(Center, Target->GetActorLocation()) <= RadiusSq)
			{
				T66AddUniqueActor(InOutTargets, Target);
			}
		});
	}

	TArray<AActor*> T66GatherAttackTargetsInSphere(UWorld* World, const AActor* IgnoredActor, const FVector& Center, const float Radius)
	{
		TArray<AActor*> Targets;
		if (!World || Radius <= 0.f)
		{
			return Targets;
		}

		FCollisionQueryParams Params;
		if (IgnoredActor)
		{
			Params.AddIgnoredActor(IgnoredActor);
		}

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);
		for (const FOverlapResult& Overlap : Overlaps)
		{
			T66AddUniqueActor(Targets, Overlap.GetActor());
		}

		T66AppendDamageableTargetsInSphere(World, IgnoredActor, Center, Radius, Targets);
		T66AppendAwakenedBossTargetsInSphere(World, IgnoredActor, Center, Radius, Targets);
		return Targets;
	}

}

UT66CombatComponent::UT66CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SlashVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));
	PixelVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle")));
}

void UT66CombatComponent::PrimeCombatPresentationAssetsAsync()
{
	if (!CachedSlashVFXNiagara) { CachedSlashVFXNiagara = SlashVFXNiagara.Get(); }
	if (!CachedPixelVFXNiagara) { CachedPixelVFXNiagara = PixelVFXNiagara.Get(); }
	if ((CachedSlashVFXNiagara && CachedPixelVFXNiagara) || CombatPresentationAssetsLoadHandle.IsValid())
	{
		return;
	}

	TArray<FSoftObjectPath> AssetPaths;
	if (!CachedSlashVFXNiagara && !SlashVFXNiagara.IsNull()) { AssetPaths.AddUnique(SlashVFXNiagara.ToSoftObjectPath()); }
	if (!CachedPixelVFXNiagara && !PixelVFXNiagara.IsNull()) { AssetPaths.AddUnique(PixelVFXNiagara.ToSoftObjectPath()); }
	if (AssetPaths.Num() <= 0)
	{
		return;
	}

	CombatPresentationAssetsLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		AssetPaths,
		FStreamableDelegate::CreateUObject(this, &UT66CombatComponent::HandleCombatPresentationAssetsLoaded));
	if (!CombatPresentationAssetsLoadHandle.IsValid())
	{
		HandleCombatPresentationAssetsLoaded();
	}
}

void UT66CombatComponent::HandleCombatPresentationAssetsLoaded()
{
	CombatPresentationAssetsLoadHandle.Reset();
	if (!CachedSlashVFXNiagara) { CachedSlashVFXNiagara = SlashVFXNiagara.Get(); }
	if (!CachedPixelVFXNiagara) { CachedPixelVFXNiagara = PixelVFXNiagara.Get(); }
}

void UT66CombatComponent::SetLockedTarget(const FT66CombatTargetHandle& InTarget)
{
	LockedTarget = InTarget;
}

void UT66CombatComponent::SetLockedTarget(AActor* InTarget)
{
	LockedTarget = MakeActorTargetHandle(InTarget);
}

void UT66CombatComponent::ClearLockedTarget()
{
	LockedTarget.Reset();
}

void UT66CombatComponent::PerformScopedPiercingShot(const FVector& Start, const FVector& End)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World)
	{
		return;
	}

	const float LineLength = FVector::Dist(Start, End);
	if (LineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float TubeRadius = 55.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	const FVector MidPoint = (Start + End) * 0.5f;
	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, MidPoint, (LineLength * 0.5f) + TubeRadius);

	TSet<AActor*> HitActors;
	for (AActor* Target : Targets)
	{
		if (!Target || HitActors.Contains(Target) || !IsValidAutoTarget(Target))
		{
			continue;
		}

		const float DistSq = FMath::PointDistToSegmentSquared(Target->GetActorLocation(), Start, End);
		if (DistSq <= (TubeRadius * TubeRadius))
		{
			HitActors.Add(Target);
			if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
			{
				const float DamageMultiplier = FMath::Max(0.01f, 1.f - Enemy->GetEffectiveArmor());
				const int32 RawDamage = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Enemy->CurrentHP) / DamageMultiplier));
				Enemy->TakeDamageFromHero(RawDamage, SourceID, NAME_None);
			}
			else if (AT66MobBase* Mob = Cast<AT66MobBase>(Target))
			{
				ApplyDamageToActor(Target, FMath::RoundToInt(Mob->CurrentHP) + 9999, NAME_None, SourceID);
			}
			else if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
			{
				const int32 DesiredFinalDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Boss->MaxHP) * 0.05f));
				const float DamageMultiplier = FMath::Max(0.01f, 1.f - Boss->GetEffectiveArmor());
				const int32 RawDamage = FMath::Max(1, FMath::CeilToInt(static_cast<float>(DesiredFinalDamage) / DamageMultiplier));
				Boss->TakeDamageFromHeroHit(RawDamage, SourceID, NAME_None);
			}
		}
	}

	SpawnPierceVFX(Start, End, FT66TemporaryProjectileSystem::HeroProjectileColor());
	PlayCombatAudioEvent(FName(TEXT("Hero.Ultimate.ScopedSniper.Fire")), (Start + End) * 0.5f);
}

// ---------------------------------------------------------------------------
// BeginPlay — set up delegates, range sphere, fire timer.
// ---------------------------------------------------------------------------
void UT66CombatComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseAttackRange = AttackRange;
	BaseFireIntervalSeconds = FireIntervalSeconds;
	BaseDamagePerShot = DamagePerShot;

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		CachedRunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		CachedFloatingCombatText = GI->GetSubsystem<UT66FloatingCombatTextSubsystem>();
		CachedIdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>();
		CachedWeaponManager = GI->GetSubsystem<UT66WeaponManagerSubsystem>();
		if (CachedRunState)
		{
			CachedRunState->InventoryChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->HeroProgressChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->DevCheatsChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->SetDOTDamageApplier([this](AActor* Target, int32 Damage, FName SourceIdolID)
			{
				ApplyDamageToActor(Target, Damage, UT66FloatingCombatTextSubsystem::EventType_DoT, SourceIdolID);
			});
		}
		if (CachedIdolManager)
		{
			CachedIdolManager->IdolStateChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		}
		if (CachedWeaponManager)
		{
			CachedWeaponManager->OnWeaponStateChanged.AddUObject(this, &UT66CombatComponent::HandleInventoryChanged);
		}
	}

	RecomputeFromRunState();

	PrimeCombatPresentationAssetsAsync();
	WarmupVFXSystems();

	// --- Create the range detection sphere ---
	AActor* Owner = GetOwner();
	if (Owner)
	{
		RangeSphere = NewObject<USphereComponent>(Owner, TEXT("CombatRangeSphere"));
		if (RangeSphere)
		{
			RangeSphere->SetupAttachment(Owner->GetRootComponent());
			RangeSphere->SetSphereRadius(AttackRange);
			RangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
			RangeSphere->SetGenerateOverlapEvents(true);
			RangeSphere->SetHiddenInGame(true);
			RangeSphere->SetCanEverAffectNavigation(false);
			// No physics response — purely a query volume.
			RangeSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
			RangeSphere->RegisterComponent();

			RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UT66CombatComponent::OnRangeBeginOverlap);
			RangeSphere->OnComponentEndOverlap.AddDynamic(this, &UT66CombatComponent::OnRangeEndOverlap);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);

	UE_LOG(LogT66Combat, Log, TEXT("[GOLD] CombatComponent: initialized — overlap sphere (radius=%.0f), VFX pooling (AutoRelease), timer-based fire (%.2fs)"),
		AttackRange, EffectiveFireIntervalSeconds);
}

void UT66CombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}

	if (CachedRunState)
	{
		CachedRunState->InventoryChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->HeroProgressChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->DevCheatsChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}
	if (CachedIdolManager)
	{
		CachedIdolManager->IdolStateChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}
	if (CachedWeaponManager)
	{
		CachedWeaponManager->OnWeaponStateChanged.RemoveAll(this);
	}
	CombatPresentationAssetsLoadHandle.Reset();

	// Clean up the sphere.
	if (RangeSphere)
	{
		RangeSphere->DestroyComponent();
		RangeSphere = nullptr;
	}
	EnemiesInRange.Empty();

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Overlap callbacks — maintain EnemiesInRange.
// ---------------------------------------------------------------------------
void UT66CombatComponent::OnRangeBeginOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (!OtherActor) return;

	// Only track enemies, lightweight mobs, and bosses (Gambler/Shop are AT66BossBase).
	if (Cast<AT66EnemyBase>(OtherActor) || Cast<AT66MobBase>(OtherActor) || Cast<AT66BossBase>(OtherActor))
	{
		EnemiesInRange.AddUnique(OtherActor);
	}
}

void UT66CombatComponent::OnRangeEndOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	if (!OtherActor) return;
	EnemiesInRange.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& Weak) { return !Weak.IsValid() || Weak.Get() == OtherActor; });
}

// ---------------------------------------------------------------------------
// IsValidAutoTarget — returns true if the actor is alive and targetable.
// ---------------------------------------------------------------------------
bool UT66CombatComponent::IsValidAutoTarget(AActor* A)
{
	if (!A) return false;
	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(A)) return E->CurrentHP > 0;
	if (AT66MobBase* M = Cast<AT66MobBase>(A)) return M->IsAliveAndActive();
	if (AT66BossBase* B = Cast<AT66BossBase>(A)) return B->IsAwakened() && B->IsAlive();
	return false;
}

bool UT66CombatComponent::IsValidTargetHandle(const FT66CombatTargetHandle& TargetHandle)
{
	return TargetHandle.IsValid() && IsValidAutoTarget(TargetHandle.Actor.Get());
}

FString UT66CombatComponent::MakeTargetHandleKey(const FT66CombatTargetHandle& TargetHandle)
{
	AActor* TargetActor = TargetHandle.Actor.Get();
	const uint64 ActorKey = static_cast<uint64>(reinterpret_cast<UPTRINT>(TargetActor));
	const FString ZoneName = TargetHandle.HitZoneName.IsNone()
		? FString::FromInt(static_cast<int32>(TargetHandle.HitZoneType))
		: TargetHandle.HitZoneName.ToString();
	return FString::Printf(TEXT("%llu|%s"), ActorKey, *ZoneName);
}

FT66CombatTargetHandle UT66CombatComponent::MakeActorTargetHandle(AActor* Actor, const ET66HitZoneType PreferredHitZone) const
{
	FT66CombatTargetHandle Handle;
	if (!Actor)
	{
		return Handle;
	}

	if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Actor))
	{
		return Enemy->ResolveCombatTargetHandle(nullptr, PreferredHitZone);
	}
	if (AT66MobBase* Mob = Cast<AT66MobBase>(Actor))
	{
		return Mob->ResolveCombatTargetHandle(nullptr, PreferredHitZone);
	}
	if (AT66BossBase* Boss = Cast<AT66BossBase>(Actor))
	{
		return Boss->ResolveCombatTargetHandle(nullptr, PreferredHitZone == ET66HitZoneType::None ? ET66HitZoneType::Core : PreferredHitZone);
	}

	Handle.Actor = Actor;
	Handle.HitZoneType = PreferredHitZone == ET66HitZoneType::None ? ET66HitZoneType::Body : PreferredHitZone;
	Handle.HitZoneName = Handle.HitZoneType == ET66HitZoneType::None ? NAME_None : FName(*UEnum::GetValueAsString(Handle.HitZoneType));
	Handle.AimPoint = Actor->GetActorLocation();
	return Handle;
}

FT66CombatTargetHandle UT66CombatComponent::ResolveAutoAttackTargetHandle(AActor* Actor, const bool bFavorLockedZone, UT66RngSubsystem* RngSub) const
{
	if (!Actor)
	{
		return FT66CombatTargetHandle{};
	}

	if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Actor))
	{
		if (bFavorLockedZone && LockedTarget.Actor.Get() == Actor && LockedTarget.HitZoneType != ET66HitZoneType::None)
		{
			return Enemy->ResolveCombatTargetHandle(Cast<UPrimitiveComponent>(LockedTarget.HitComponent.Get()), LockedTarget.HitZoneType);
		}

		ET66HitZoneType PreferredZone = ET66HitZoneType::Body;
		if (Enemy->SupportsCombatHitZones())
		{
			const float AccuracyChance = CachedRunState ? CachedRunState->GetAccuracyChance01() : 0.f;
			if (RollTierChance(AccuracyChance, RngSub))
			{
				PreferredZone = ET66HitZoneType::Head;
			}
		}

		return Enemy->ResolveCombatTargetHandle(nullptr, PreferredZone);
	}

	if (AT66MobBase* Mob = Cast<AT66MobBase>(Actor))
	{
		if (bFavorLockedZone && LockedTarget.Actor.Get() == Actor && LockedTarget.HitZoneType != ET66HitZoneType::None)
		{
			return Mob->ResolveCombatTargetHandle(Cast<UPrimitiveComponent>(LockedTarget.HitComponent.Get()), LockedTarget.HitZoneType);
		}

		ET66HitZoneType PreferredZone = ET66HitZoneType::Body;
		if (Mob->SupportsCombatHitZones())
		{
			const float AccuracyChance = CachedRunState ? CachedRunState->GetAccuracyChance01() : 0.f;
			if (RollTierChance(AccuracyChance, RngSub))
			{
				PreferredZone = ET66HitZoneType::Head;
			}
		}

		return Mob->ResolveCombatTargetHandle(nullptr, PreferredZone);
	}

	if (AT66BossBase* Boss = Cast<AT66BossBase>(Actor))
	{
		if (bFavorLockedZone && LockedTarget.Actor.Get() == Actor && LockedTarget.HitZoneType != ET66HitZoneType::None)
		{
			return Boss->ResolveCombatTargetHandle(Cast<UPrimitiveComponent>(LockedTarget.HitComponent.Get()), LockedTarget.HitZoneType);
		}

		ET66HitZoneType PreferredZone = ET66HitZoneType::Core;
		if (Boss->SupportsCombatHitZones())
		{
			const float AccuracyChance = CachedRunState ? CachedRunState->GetAccuracyChance01() : 0.f;
			if (RollTierChance(AccuracyChance, RngSub))
			{
				PreferredZone = ET66HitZoneType::Head;
			}
		}

		return Boss->ResolveCombatTargetHandle(nullptr, PreferredZone);
	}

	return MakeActorTargetHandle(Actor);
}

FVector UT66CombatComponent::GetTargetAimPoint(const FT66CombatTargetHandle& TargetHandle)
{
	if (UPrimitiveComponent* HitComponent = TargetHandle.HitComponent.Get())
	{
		return HitComponent->GetComponentLocation();
	}

	if (!TargetHandle.AimPoint.IsNearlyZero())
	{
		return TargetHandle.AimPoint;
	}

	if (AActor* TargetActor = TargetHandle.Actor.Get())
	{
		return TargetActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}

bool UT66CombatComponent::HasUnblockedAutoAttackPath(const FVector& FromLocation, const FT66CombatTargetHandle& TargetHandle) const
{
	AActor* OwnerActor = GetOwner();
	AActor* TargetActor = TargetHandle.Actor.Get();
	UWorld* World = GetWorld();
	if (!OwnerActor || !TargetActor || !World || !IsValidTargetHandle(TargetHandle))
	{
		return false;
	}

	const FVector TargetPoint = GetTargetAimPoint(TargetHandle);
	if (TargetPoint.IsNearlyZero())
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66AutoAttackLineOfSight), true);
	Params.AddIgnoredActor(OwnerActor);
	Params.AddIgnoredActor(TargetActor);
	if (RangeSphere)
	{
		Params.AddIgnoredComponent(RangeSphere.Get());
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	FHitResult BlockerHit;
	const bool bBlocked = World->LineTraceSingleByObjectType(BlockerHit, FromLocation, TargetPoint, ObjectParams, Params);
	return !bBlocked;
}

// ---------------------------------------------------------------------------
// FindClosestEnemyInRange — walks EnemiesInRange (small list) instead of all
// world actors.  Returns nullptr if nothing valid is in range.
// ---------------------------------------------------------------------------
AActor* UT66CombatComponent::FindClosestEnemyInRange(const FVector& FromLocation, float MaxRangeSq,
	const TSet<AActor*>* ExcludeSet) const
{
	AActor* Best = nullptr;
	float BestDistSq = MaxRangeSq;

	for (const TWeakObjectPtr<AActor>& Weak : EnemiesInRange)
	{
		AActor* A = Weak.Get();
		if (!A) continue;
		if (ExcludeSet && ExcludeSet->Contains(A)) continue;
		if (!IsValidAutoTarget(A)) continue;
		const FT66CombatTargetHandle CandidateHandle = MakeActorTargetHandle(A);
		if (!HasUnblockedAutoAttackPath(FromLocation, CandidateHandle)) continue;
		const float DistSq = FVector::DistSquared(FromLocation, GetTargetAimPoint(CandidateHandle));
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = A;
		}
	}

	if (const UWorld* World = GetWorld())
	{
		if (const UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->ForEachDamageableTarget([&](AActor* CandidateActor)
			{
				if (!CandidateActor) return;
				if (ExcludeSet && ExcludeSet->Contains(CandidateActor)) return;
				if (!IsValidAutoTarget(CandidateActor)) return;
				const FT66CombatTargetHandle CandidateHandle = MakeActorTargetHandle(CandidateActor);
				if (!HasUnblockedAutoAttackPath(FromLocation, CandidateHandle)) return;
				const float DistSq = FVector::DistSquared(FromLocation, GetTargetAimPoint(CandidateHandle));
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					Best = CandidateActor;
				}
			});

			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
			{
				AT66BossBase* Boss = WeakBoss.Get();
				if (!Boss) continue;
				if (ExcludeSet && ExcludeSet->Contains(Boss)) continue;
				if (!IsValidAutoTarget(Boss)) continue;
				const FT66CombatTargetHandle CandidateHandle = MakeActorTargetHandle(Boss);
				if (!HasUnblockedAutoAttackPath(FromLocation, CandidateHandle)) continue;
				const float DistSq = FVector::DistSquared(FromLocation, GetTargetAimPoint(CandidateHandle));
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					Best = Boss;
				}
			}
		}
	}
	return Best;
}

FT66CombatTargetHandle UT66CombatComponent::FindClosestTargetHandleInRange(const FVector& FromLocation, float MaxRangeSq, const TSet<FString>* ExcludeKeys) const
{
	FT66CombatTargetHandle BestHandle;
	float BestDistSq = MaxRangeSq;
	TSet<const AActor*> SeenActors;

	auto ConsiderHandle = [&](const FT66CombatTargetHandle& CandidateHandle)
	{
		if (!IsValidTargetHandle(CandidateHandle))
		{
			return;
		}

		const FString CandidateKey = MakeTargetHandleKey(CandidateHandle);
		if (ExcludeKeys && ExcludeKeys->Contains(CandidateKey))
		{
			return;
		}

		if (!HasUnblockedAutoAttackPath(FromLocation, CandidateHandle))
		{
			return;
		}

		const float DistSq = FVector::DistSquared(FromLocation, GetTargetAimPoint(CandidateHandle));
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestHandle = CandidateHandle;
		}
	};

	auto ConsiderActor = [&](AActor* CandidateActor)
	{
		if (!CandidateActor || SeenActors.Contains(CandidateActor) || !IsValidAutoTarget(CandidateActor))
		{
			return;
		}
		SeenActors.Add(CandidateActor);

		if (AT66BossBase* Boss = Cast<AT66BossBase>(CandidateActor))
		{
			TSet<FString> SeenBossParts;
			static const ET66HitZoneType CandidateZones[] =
			{
				ET66HitZoneType::Head,
				ET66HitZoneType::WeakPoint,
				ET66HitZoneType::Core,
				ET66HitZoneType::LeftArm,
				ET66HitZoneType::RightArm,
				ET66HitZoneType::LeftLeg,
				ET66HitZoneType::RightLeg,
				ET66HitZoneType::Body,
			};

			for (const ET66HitZoneType CandidateZone : CandidateZones)
			{
				const FT66CombatTargetHandle CandidateHandle = Boss->ResolveCombatTargetHandle(nullptr, CandidateZone);
				if (!CandidateHandle.IsValid())
				{
					continue;
				}

				const FString BossPartKey = MakeTargetHandleKey(CandidateHandle);
				if (SeenBossParts.Contains(BossPartKey))
				{
					continue;
				}

				SeenBossParts.Add(BossPartKey);
				ConsiderHandle(CandidateHandle);
			}
			return;
		}

		ConsiderHandle(MakeActorTargetHandle(CandidateActor));
	};

	for (const TWeakObjectPtr<AActor>& Weak : EnemiesInRange)
	{
		ConsiderActor(Weak.Get());
	}

	if (const UWorld* World = GetWorld())
	{
		if (const UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->ForEachDamageableTarget([&](AActor* CandidateActor)
			{
				ConsiderActor(CandidateActor);
			});

			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
			{
				ConsiderActor(WeakBoss.Get());
			}
		}
	}

	return BestHandle;
}

void UT66CombatComponent::HandleInventoryChanged()
{
	UE_LOG(LogT66Combat, Verbose, TEXT("[IDOL CACHE] HandleInventoryChanged owner=%s"), GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
	const float OldInterval = EffectiveFireIntervalSeconds;
	RecomputeFromRunState();
	if (!FMath::IsNearlyEqual(OldInterval, EffectiveFireIntervalSeconds, 0.001f))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FireTimerHandle);
			World->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);
		}
	}

	// Keep the hero's range ring in sync (attack range changes with Scale).
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetOwner()))
	{
		Hero->RefreshAttackRangeRing();
	}

	// Update the detection sphere to match the new attack range.
	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}
}

void UT66CombatComponent::RecomputeFromRunState()
{
	AttackRange = BaseAttackRange;
	EffectiveFireIntervalSeconds = BaseFireIntervalSeconds;
	EffectiveDamagePerShot = BaseDamagePerShot;
	ProjectileScaleMultiplier = 1.f;
	bHasCachedHeroData = false;
	CachedHeroData = FHeroData{};
	bHasCachedWeaponData = false;
	CachedWeaponData = FWeaponData{};
	CachedIdolSlots.Reset();

	if (!CachedRunState)
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GIBase = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* GI = Cast<UT66GameInstance>(GIBase);
	if (GI)
	{
		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetOwner()))
		{
			bHasCachedHeroData = GI->GetHeroData(Hero->HeroID, CachedHeroData);
		}

		UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>();
		const TArray<FName>& Idols = IdolManager ? IdolManager->GetEquippedIdols() : CachedRunState->GetEquippedIdols();
		CachedIdolSlots.Reserve(Idols.Num());
		for (int32 Slot = 0; Slot < Idols.Num(); ++Slot)
		{
			FCachedIdolSlot Entry;
			Entry.IdolID = Idols[Slot];
			Entry.Rarity = IdolManager ? IdolManager->GetEquippedIdolRarityInSlot(Slot) : CachedRunState->GetEquippedIdolRarityInSlot(Slot);
			Entry.bValid = !Entry.IdolID.IsNone() && GI->GetIdolData(Entry.IdolID, Entry.IdolData);
			CachedIdolSlots.Add(MoveTemp(Entry));
		}

		UT66WeaponManagerSubsystem* WeaponManager = GI->GetSubsystem<UT66WeaponManagerSubsystem>();
		bHasCachedWeaponData = WeaponManager && WeaponManager->GetEquippedWeaponData(CachedWeaponData);
	}

	int32 ValidIdolCount = 0;
	for (int32 Slot = 0; Slot < CachedIdolSlots.Num(); ++Slot)
	{
		const FCachedIdolSlot& Entry = CachedIdolSlots[Slot];
		if (!Entry.bValid || Entry.IdolID.IsNone())
		{
			continue;
		}

		++ValidIdolCount;
		UE_LOG(
			LogT66Combat,
			Verbose,
			TEXT("[IDOL CACHE] Slot=%d Idol=%s Rarity=%s Category=%s"),
			Slot,
			*Entry.IdolID.ToString(),
			T66CombatShared::GetItemRarityName(Entry.Rarity),
			GetT66AttackCategoryName(Entry.IdolData.Category));
	}
	UE_LOG(
		LogT66Combat,
		Verbose,
		TEXT("[IDOL CACHE] owner=%s totalSlots=%d validSlots=%d"),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		CachedIdolSlots.Num(),
		ValidIdolCount);

	const float AttackSpeedMult = CachedRunState->GetItemAttackSpeedMultiplier();
	const float DamageMult = CachedRunState->GetItemDamageMultiplier();
	const float ScaleMult = CachedRunState->GetItemScaleMultiplier();

	const float HeroAttackSpeedMult = CachedRunState->GetHeroAttackSpeedMultiplier() * CachedRunState->GetRallyAttackSpeedMultiplier() * CachedRunState->GetEnduranceAttackSpeedMultiplier();
	const float HeroDamageMult = CachedRunState->GetHeroDamageMultiplier() * CachedRunState->GetEnduranceDamageMultiplier() * CachedRunState->GetBrawlersFuryDamageMultiplier();
	const float HeroScaleMult = CachedRunState->GetHeroScaleMultiplier();

	const float TotalAttackSpeed = AttackSpeedMult * HeroAttackSpeedMult;
	const float TotalDamage = DamageMult * HeroDamageMult;
	const float TotalScale = ScaleMult * HeroScaleMult;
	const ET66AttackCategory EquippedAttackCategory = bHasCachedWeaponData ? CachedWeaponData.Branch : (bHasCachedHeroData ? CachedHeroData.PrimaryCategory : ET66AttackCategory::Pierce);
	const float CategoryDamageMult = T66CombatShared::GetCategorySubDamageMultiplier(CachedRunState, EquippedAttackCategory);
	const float CategoryAttackSpeedMult = T66CombatShared::GetCategorySubAttackSpeedMultiplier(CachedRunState, EquippedAttackCategory);
	const float CategoryScaleMult = T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, EquippedAttackCategory);
	const float WeaponDamageMult = bHasCachedWeaponData ? FMath::Max(0.01f, CachedWeaponData.DamageMultiplier) : 1.f;
	const float WeaponAttackSpeedMult = bHasCachedWeaponData ? FMath::Max(0.01f, CachedWeaponData.AttackSpeedMultiplier) : 1.f;
	const float WeaponScaleMult = bHasCachedWeaponData ? FMath::Max(0.01f, CachedWeaponData.AttackScaleMultiplier) : 1.f;
	const float WeaponRangeMult = bHasCachedWeaponData ? FMath::Max(0.01f, CachedWeaponData.RangeMultiplier) : 1.f;
	const float WeaponBaseDamage = static_cast<float>(BaseDamagePerShot + (bHasCachedWeaponData ? CachedWeaponData.BonusHitDamage : 0));

	// Per HUD/combat spec: Scale stat affects attack range (larger scale = larger range).
	// Use the DataTable base range (hero-specific) instead of the C++ default.
	const float DataTableRange = CachedRunState->GetHeroBaseAttackRange();
	const float EffectiveBaseRange = (DataTableRange > 0.f) ? DataTableRange : BaseAttackRange;
	AttackRange = FMath::Clamp(EffectiveBaseRange * FMath::Max(0.1f, TotalScale * WeaponRangeMult), 200.f, 50000.f);

	EffectiveFireIntervalSeconds = FMath::Clamp(BaseFireIntervalSeconds / FMath::Max(0.01f, TotalAttackSpeed * CategoryAttackSpeedMult * WeaponAttackSpeedMult), 0.05f, 10.f);
	EffectiveDamagePerShot = FMath::Clamp(FMath::RoundToInt(WeaponBaseDamage * TotalDamage * CategoryDamageMult * WeaponDamageMult), 1, 999999);
	ProjectileScaleMultiplier = FMath::Clamp(TotalScale * CategoryScaleMult * WeaponScaleMult, 0.1f, 10.f);

	UE_LOG(
		LogT66Combat,
		Verbose,
		TEXT("CombatItemCategoryTuning AttackCategory=%s CategoryDamageMult=%.3f CategoryAttackSpeedMult=%.3f CategoryScaleMult=%.3f EffectiveDamagePerShot=%d EffectiveFireIntervalSeconds=%.3f ProjectileScaleMultiplier=%.3f"),
		GetT66AttackCategoryName(EquippedAttackCategory),
		CategoryDamageMult,
		CategoryAttackSpeedMult,
		CategoryScaleMult,
		EffectiveDamagePerShot,
		EffectiveFireIntervalSeconds,
		ProjectileScaleMultiplier);

	// Dev Power toggle: auto-attack does absurd damage.
	if (CachedRunState->GetDevPowerEnabled())
	{
		EffectiveDamagePerShot = 999999;
	}

	// Keep the detection sphere in sync whenever stats are recomputed.
	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}
}

void UT66CombatComponent::PlayCombatAudioEvent(const FName EventID, const FVector& Location) const
{
	AActor* OwnerActor = GetOwner();
	UT66AudioSubsystem::PlayEventFromWorldContext(OwnerActor ? static_cast<UObject*>(OwnerActor) : const_cast<UT66CombatComponent*>(this), EventID, Location, OwnerActor);
}

void UT66CombatComponent::PlayHeroAttackSfx(const FName& HeroID, const ET66AttackCategory AttackCategory, const FVector& Location) const
{
	AActor* OwnerActor = GetOwner();
	UObject* WorldContext = OwnerActor ? static_cast<UObject*>(OwnerActor) : const_cast<UT66CombatComponent*>(this);

	if (!HeroID.IsNone())
	{
		const FName HeroEventID(*FString::Printf(TEXT("Hero.Attack.%s"), *HeroID.ToString()));
		if (UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, HeroEventID, Location, OwnerActor))
		{
			return;
		}
	}

	if (UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, GetT66AttackCategoryAudioEvent(AttackCategory), Location, OwnerActor))
	{
		return;
	}

	UT66AudioSubsystem::PlayEventFromWorldContext(WorldContext, FName(TEXT("Hero.Attack.Generic")), Location, OwnerActor);
}

float UT66CombatComponent::GetAutoAttackCooldownProgress() const
{
	UWorld* World = GetWorld();
	if (!World || EffectiveFireIntervalSeconds <= 0.f) return 1.f;
	const float Now = static_cast<float>(World->GetTimeSeconds());
	const float Elapsed = Now - LastFireTime;
	return FMath::Clamp(Elapsed / EffectiveFireIntervalSeconds, 0.f, 1.f);
}

bool UT66CombatComponent::ResolveCombatVFXBinding(
	const ET66CombatVFXBindingSourceType SourceType,
	const FName SourceID,
	const ET66AttackCategory AttackCategory,
	FT66CombatVFXBindingData& OutBindingData,
	UNiagaraSystem*& OutSystem) const
{
	OutSystem = nullptr;
	if (SourceID.IsNone())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	UT66GameInstance* GameInstance = World ? World->GetGameInstance<UT66GameInstance>() : nullptr;
	if (!GameInstance || !GameInstance->GetCombatVFXBindingData(SourceType, SourceID, AttackCategory, OutBindingData))
	{
		return false;
	}

	OutSystem = OutBindingData.NiagaraSystem.LoadSynchronous();
	if (!OutSystem)
	{
		UE_LOG(
			LogT66Combat,
			Warning,
			TEXT("CombatVFXFallbackPlaceholder Reason=MissingNiagaraSystem Binding=%s SourceType=%s SourceID=%s AttackCategory=%s NiagaraSystem=%s DevFallbackAllowed=%s"),
			*OutBindingData.BindingID.ToString(),
			GetT66CombatVFXSourceTypeName(SourceType),
			*SourceID.ToString(),
			GetT66AttackCategoryName(AttackCategory),
			*OutBindingData.NiagaraSystem.ToString(),
			OutBindingData.bDevelopmentFallbackAllowed ? TEXT("true") : TEXT("false"));
		return false;
	}

	return true;
}

bool UT66CombatComponent::ShouldSuppressWeaponBaseProjectileVisual(const ET66AttackCategory AttackCategory) const
{
	if (!bHasCachedWeaponData || CachedWeaponData.WeaponID.IsNone())
	{
		return false;
	}

	FT66CombatVFXBindingData Binding;
	UNiagaraSystem* BoundSystem = nullptr;
	return ResolveCombatVFXBinding(
		ET66CombatVFXBindingSourceType::WeaponBase,
		CachedWeaponData.WeaponID,
		AttackCategory,
		Binding,
		BoundSystem) && Binding.bSuppressTemporaryProjectile;
}

bool UT66CombatComponent::TrySpawnBoundWeaponBaseSlashVFX(
	const FT66CombatImpactContext& WeaponImpactContext,
	const int32 EffectiveDamage,
	const FName HeroID,
	const ET66AttackCategory AttackCategory)
{
	if (!bHasCachedWeaponData || CachedWeaponData.WeaponID.IsNone())
	{
		return false;
	}

	FT66CombatVFXBindingData Binding;
	UNiagaraSystem* BoundSystem = nullptr;
	if (!ResolveCombatVFXBinding(
		ET66CombatVFXBindingSourceType::WeaponBase,
		CachedWeaponData.WeaponID,
		AttackCategory,
		Binding,
		BoundSystem))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World || !BoundSystem)
	{
		return false;
	}

	const float EffectiveSlashRadius = WeaponImpactContext.Radius;
	const float EffectiveSlashInnerRadius = WeaponImpactContext.InnerRadius;
	const FVector DamageCenter = WeaponImpactContext.bDamageCenterValid
		? WeaponImpactContext.DamageCenter
		: WeaponImpactContext.ImpactPoint;
	const FVector ImpactPoint = WeaponImpactContext.bImpactPointValid
		? WeaponImpactContext.ImpactPoint
		: DamageCenter;
	const FVector Forward = T66ResolvePlanarDirection(WeaponImpactContext.Forward, GetOwner());
	const bool bBandAnchoredCarrier =
		WeaponImpactContext.bUsesFrontalSector
		&& EffectiveSlashInnerRadius > KINDA_SMALL_NUMBER
		&& EffectiveSlashRadius > EffectiveSlashInnerRadius
		&& !ImpactPoint.Equals(DamageCenter, 0.5f);
	// Pierce is a forward lane carrier; AOE/Slash keep their existing radial anchor models.
	const bool bPathAnchoredCarrier = (AttackCategory == ET66AttackCategory::Pierce);
	// Bounce links are point impacts: a small fixed-footprint slash anchored at each chain hit.
	const bool bImpactAnchoredCarrier = (AttackCategory == ET66AttackCategory::Bounce);
	const float ImpactOffsetFromDamageCenter = FVector::Dist2D(DamageCenter, ImpactPoint);
	const float BaseVisualRadius = FMath::Max(0.f, Binding.BaseVisualRadius);
	const float VisualScale = FMath::Max(
		0.01f,
		(BaseVisualRadius > KINDA_SMALL_NUMBER)
			? (EffectiveSlashRadius / BaseVisualRadius) * Binding.VisualScaleMultiplier
			: ProjectileScaleMultiplier * Binding.VisualScaleMultiplier);

	// Resolve the carrier transform per anchor model. Radial (AOE/Slash) carriers
	// anchor on the damage center with uniform scale; the Pierce PathAnchored carrier
	// anchors at the hero attack origin and maps the vertical slash to LineLength /
	// TubeRadius with a non-uniform scale.
	FVector VisualPivot = DamageCenter;
	FRotator SpawnRotation = Forward.Rotation();
	FVector VisualScaleVec(VisualScale);
	const TCHAR* VisualAnchorModel = bBandAnchoredCarrier ? TEXT("BandAnchored") : TEXT("CenterAnchored");
	float PathLineLength = WeaponImpactContext.LineLength;
	float PathTubeRadius = WeaponImpactContext.TubeRadius;
	if (bPathAnchoredCarrier)
	{
		const FVector LaneStart = WeaponImpactContext.AttackOrigin.IsNearlyZero()
			? DamageCenter
			: WeaponImpactContext.AttackOrigin;
		PathLineLength = FMath::Max(
			50.f,
			(WeaponImpactContext.LineLength > KINDA_SMALL_NUMBER)
				? WeaponImpactContext.LineLength
				: FVector::Dist(LaneStart, ImpactPoint));
		PathTubeRadius = FMath::Max(
			8.f,
			(WeaponImpactContext.TubeRadius > KINDA_SMALL_NUMBER)
				? WeaponImpactContext.TubeRadius
				: BaseVisualRadius);
		VisualPivot = LaneStart;
		VisualScaleVec = FVector(PathLineLength, PathTubeRadius, PathTubeRadius)
			* FMath::Max(0.01f, Binding.VisualScaleMultiplier);
		VisualAnchorModel = TEXT("PathAnchored");
	}
	else if (bImpactAnchoredCarrier)
	{
		// Bounce: place a small horizontal slash at each authoritative chain impact
		// point. The footprint is a fixed authored scale per link (optionally tuned by
		// the binding's VisualScaleMultiplier); it does not scale with a damage radius
		// because Bounce links are point impacts, not radial/lane footprints.
		VisualPivot = ImpactPoint;
		VisualScaleVec = FVector(FMath::Max(0.01f, Binding.VisualScaleMultiplier));
		SpawnRotation = Forward.Rotation();
		VisualAnchorModel = TEXT("ImpactAnchored");
	}
	constexpr float MinReadableSlashPlaybackSeconds = 0.20f;
	constexpr float MaxReadableSlashPlaybackMultiplier = 2.50f;
	const float RawVisualPlaybackMultiplier = (BaseFireIntervalSeconds > KINDA_SMALL_NUMBER && EffectiveFireIntervalSeconds > KINDA_SMALL_NUMBER)
		? BaseFireIntervalSeconds / EffectiveFireIntervalSeconds
		: 1.0f;
	const float DurationBoundPlaybackMultiplier = (Binding.BasePlaybackSeconds > KINDA_SMALL_NUMBER)
		? Binding.BasePlaybackSeconds / MinReadableSlashPlaybackSeconds
		: MaxReadableSlashPlaybackMultiplier;
	const float VisualPlaybackMultiplier = FMath::Clamp(
		RawVisualPlaybackMultiplier,
		0.50f,
		FMath::Max(0.50f, FMath::Min(MaxReadableSlashPlaybackMultiplier, DurationBoundPlaybackMultiplier)));
	const float ExpectedVisualDuration = (VisualPlaybackMultiplier > KINDA_SMALL_NUMBER)
		? Binding.BasePlaybackSeconds / VisualPlaybackMultiplier
		: Binding.BasePlaybackSeconds;

	const FVector SpawnLocation = (bPathAnchoredCarrier || bImpactAnchoredCarrier)
		? VisualPivot
		: (VisualPivot + FVector(0.f, 0.f, 70.f));
	UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		BoundSystem,
		SpawnLocation,
		SpawnRotation,
		VisualScaleVec,
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
	if (!Component)
	{
		UE_LOG(
			LogT66Combat,
			Warning,
			TEXT("CombatVFXFallbackPlaceholder Reason=SpawnFailed Binding=%s SourceType=WeaponBase SourceID=%s AttackCategory=%s System=%s"),
			*Binding.BindingID.ToString(),
			*CachedWeaponData.WeaponID.ToString(),
			GetT66AttackCategoryName(AttackCategory),
			*BoundSystem->GetPathName());
		return false;
	}

	Component->SetCustomTimeDilation(VisualPlaybackMultiplier);
	Component->SetTranslucentSortPriority(14);
	UE_LOG(
		LogT66Combat,
		Display,
		TEXT("CombatVFXProductionSpawned Binding=%s SourceType=WeaponBase SourceID=%s HeroID=%s AttackCategory=%s System=%s Location=%s VisualPivot=%s DamageCenter=%s DamageCenterValid=%d ImpactPoint=%s ImpactPointValid=%d VisualAnchorModel=%s ImpactOffsetFromDamageCenter=%.2f Forward=%s EffectiveSlashRadius=%.2f EffectiveSlashInnerRadius=%.2f AoeInnerRadiusRatio=%.3f BaseVisualRadius=%.2f ProjectileScaleMultiplier=%.3f VisualScale=%.3f BaseFireIntervalSeconds=%.3f EffectiveFireIntervalSeconds=%.3f RawVisualPlaybackMultiplier=%.3f VisualPlaybackMultiplier=%.3f BasePlaybackSeconds=%.3f ExpectedVisualDuration=%.3f EffectiveDamagePerShot=%d VFXProfile=%s EffectPacketID=%s PathLineLength=%.2f PathTubeRadius=%.2f VisualScaleVec=%s"),
		*Binding.BindingID.ToString(),
		*CachedWeaponData.WeaponID.ToString(),
		*HeroID.ToString(),
		GetT66AttackCategoryName(AttackCategory),
		*BoundSystem->GetPathName(),
		*SpawnLocation.ToCompactString(),
		*VisualPivot.ToCompactString(),
		*DamageCenter.ToCompactString(),
		WeaponImpactContext.bDamageCenterValid ? 1 : 0,
		*ImpactPoint.ToCompactString(),
		WeaponImpactContext.bImpactPointValid ? 1 : 0,
		VisualAnchorModel,
		ImpactOffsetFromDamageCenter,
		*Forward.ToCompactString(),
		EffectiveSlashRadius,
		EffectiveSlashInnerRadius,
		CachedWeaponData.AoeInnerRadiusRatio,
		BaseVisualRadius,
		ProjectileScaleMultiplier,
		VisualScale,
		BaseFireIntervalSeconds,
		EffectiveFireIntervalSeconds,
		RawVisualPlaybackMultiplier,
		VisualPlaybackMultiplier,
		Binding.BasePlaybackSeconds,
		ExpectedVisualDuration,
		EffectiveDamage,
		*Binding.VFXProfile.ToString(),
		*Binding.EffectPacketID.ToString(),
		PathLineLength,
		PathTubeRadius,
		*VisualScaleVec.ToCompactString());

	return true;
}

// ---------------------------------------------------------------------------
// Bounce moving-projectile presentation.
// Bounce reads as a moving two-link projectile sequence: one visible link travels
// from the hero attack origin to the primary target, and only after that visual
// projectile arrives does the next link travel from the primary impact point to
// the next chained target. Exactly one link is in flight per segment — never a
// burst of simultaneous projectiles and never a static impact-only slash. Damage
// and the per-link impact contexts are resolved up front in PerformBounce; these
// helpers are visual-only.
// ---------------------------------------------------------------------------
void UT66CombatComponent::StageBounceProjectileChain(
	const TArray<FVector>& ChainPositions,
	const FLinearColor& Color,
	const float ProjectileSpeed,
	const float ScaleMultiplier,
	UNiagaraSystem* CarrierSystem,
	const float CarrierVisualScale,
	const float MinLinkTravelSeconds,
	const float CarrierPlaybackSeconds)
{
	UWorld* World = GetWorld();
	if (!World || ChainPositions.Num() < 2)
	{
		return;
	}

	SpawnBounceChainLinkSequential(
		ChainPositions,
		Color,
		ProjectileSpeed,
		ScaleMultiplier,
		0,
		CarrierSystem,
		CarrierVisualScale,
		MinLinkTravelSeconds,
		CarrierPlaybackSeconds);
}

void UT66CombatComponent::SpawnBounceChainLinkSequential(
	const TArray<FVector>& ChainPositions,
	const FLinearColor& Color,
	const float ProjectileSpeed,
	const float ScaleMultiplier,
	const int32 LinkIndex,
	UNiagaraSystem* CarrierSystem,
	const float CarrierVisualScale,
	const float MinLinkTravelSeconds,
	const float CarrierPlaybackSeconds)
{
	if (ChainPositions.Num() < 2)
	{
		return;
	}

	const int32 LinkCount = ChainPositions.Num() - 1;
	if (LinkIndex < 0 || LinkIndex >= LinkCount)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("CombatVFXBounceChainSequentialSkipped Reason=InvalidLinkIndex LinkIndex=%d LinkCount=%d ChainPoints=%d"), LinkIndex, LinkCount, ChainPositions.Num());
		return;
	}

	const FVector RawLinkStart = ChainPositions[LinkIndex];
	const FVector LinkEnd = ChainPositions[LinkIndex + 1];
	const FVector LinkDirection = (LinkEnd - RawLinkStart).GetSafeNormal();
	const FVector LinkStart = (LinkIndex > 0 && !LinkDirection.IsNearlyZero())
		? RawLinkStart + (LinkDirection * 36.f)
		: RawLinkStart;
	const float Speed = FMath::Max(1.f, ProjectileSpeed);
	const float MinTravelSeconds = FMath::Max(0.f, MinLinkTravelSeconds);
	const float LinkDistance = FVector::Dist(LinkStart, LinkEnd);
	// Presentation-only minimum travel time: short Bounce links (e.g. ~150uu) move so
	// fast at the raw projectile speed that the slash carrier is unreadable in capture.
	// Slow this link's *visual* speed (not damage timing) so it travels for at least
	// the authored playback window. Damage was already resolved in PerformBounce.
	const float RawTravelSeconds = LinkDistance / Speed;
	const float TravelSeconds = FMath::Max(RawTravelSeconds, MinTravelSeconds);
	const float LinkSpeed = (TravelSeconds > KINDA_SMALL_NUMBER)
		? FMath::Max(1.f, LinkDistance / TravelSeconds)
		: Speed;

	if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(
			LogT66Combat,
			Display,
			TEXT("CombatVFXBounceChainSequentialAttempt LinkIndex=%d LinkCount=%d Start=%s End=%s Distance=%.2f LinkSpeed=%.2f TravelSeconds=%.3f"),
			LinkIndex,
			LinkCount,
			*LinkStart.ToCompactString(),
			*LinkEnd.ToCompactString(),
			LinkDistance,
			LinkSpeed,
			TravelSeconds);
	}

	AT66HeroProjectile* Projectile = SpawnBounceLinkProjectile(
		LinkStart,
		LinkEnd,
		Color,
		LinkSpeed,
		ScaleMultiplier,
		LinkIndex,
		LinkCount,
		CarrierSystem,
		CarrierVisualScale,
		CarrierPlaybackSeconds);

	if (!Projectile || LinkIndex + 1 >= LinkCount)
	{
		if (!Projectile)
		{
			UE_LOG(LogT66Combat, Warning, TEXT("CombatVFXBounceChainSequentialStopped Reason=ProjectileSpawnFailed LinkIndex=%d LinkCount=%d"), LinkIndex, LinkCount);
		}
		return;
	}

	TWeakObjectPtr<UT66CombatComponent> WeakThis(this);
	TWeakObjectPtr<UNiagaraSystem> WeakCarrier(CarrierSystem);
	TArray<FVector> ChainPositionsCopy = ChainPositions;
	const FLinearColor LinkColor = Color;
	const int32 NextLinkIndex = LinkIndex + 1;
	Projectile->SetVisualArrivalCallback(
		[WeakThis, ChainPositionsCopy, LinkColor, ProjectileSpeed, ScaleMultiplier, NextLinkIndex, WeakCarrier, CarrierVisualScale, MinLinkTravelSeconds, CarrierPlaybackSeconds]()
		{
			if (UT66CombatComponent* Self = WeakThis.Get())
			{
				if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
				{
					UE_LOG(
						LogT66Combat,
						Display,
						TEXT("CombatVFXBounceLinkArrivalCallback NextLinkIndex=%d ChainPoints=%d CarrierValid=%d Time=%.3f"),
						NextLinkIndex,
						ChainPositionsCopy.Num(),
						WeakCarrier.IsValid() ? 1 : 0,
						Self->GetWorld() ? Self->GetWorld()->GetTimeSeconds() : -1.f);
				}
				if (UWorld* CallbackWorld = Self->GetWorld())
				{
					TWeakObjectPtr<UT66CombatComponent> WeakSelfForNextTick(Self);
					TWeakObjectPtr<UNiagaraSystem> WeakCarrierForNextTick = WeakCarrier;
					TArray<FVector> ChainPositionsForNextTick = ChainPositionsCopy;
					CallbackWorld->GetTimerManager().SetTimerForNextTick(
						FTimerDelegate::CreateLambda(
							[WeakSelfForNextTick, ChainPositionsForNextTick, LinkColor, ProjectileSpeed, ScaleMultiplier, NextLinkIndex, WeakCarrierForNextTick, CarrierVisualScale, MinLinkTravelSeconds, CarrierPlaybackSeconds]()
							{
								if (UT66CombatComponent* SelfForNextTick = WeakSelfForNextTick.Get())
								{
									if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
									{
										UE_LOG(
											LogT66Combat,
											Display,
											TEXT("CombatVFXBounceLinkArrivalDeferredSpawn NextLinkIndex=%d ChainPoints=%d CarrierValid=%d Time=%.3f"),
											NextLinkIndex,
											ChainPositionsForNextTick.Num(),
											WeakCarrierForNextTick.IsValid() ? 1 : 0,
											SelfForNextTick->GetWorld() ? SelfForNextTick->GetWorld()->GetTimeSeconds() : -1.f);
									}
									SelfForNextTick->SpawnBounceChainLinkSequential(
										ChainPositionsForNextTick,
										LinkColor,
										ProjectileSpeed,
										ScaleMultiplier,
										NextLinkIndex,
										WeakCarrierForNextTick.Get(),
										CarrierVisualScale,
										MinLinkTravelSeconds,
										CarrierPlaybackSeconds);
								}
							}));
				}
				else
				{
					UE_LOG(LogT66Combat, Warning, TEXT("CombatVFXBounceLinkArrivalCallbackSkipped Reason=NoWorld NextLinkIndex=%d ChainPoints=%d"), NextLinkIndex, ChainPositionsCopy.Num());
				}
			}
			else
			{
				UE_LOG(LogT66Combat, Warning, TEXT("CombatVFXBounceLinkArrivalCallbackSkipped Reason=InvalidCombatComponent NextLinkIndex=%d ChainPoints=%d"), NextLinkIndex, ChainPositionsCopy.Num());
			}
		});
}

AT66HeroProjectile* UT66CombatComponent::SpawnBounceLinkProjectile(
	const FVector& Start,
	const FVector& End,
	const FLinearColor& Color,
	const float ProjectileSpeed,
	const float ScaleMultiplier,
	const int32 LinkIndex,
	const int32 LinkCount,
	UNiagaraSystem* CarrierSystem,
	const float CarrierVisualScale,
	const float CarrierPlaybackSeconds)
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("CombatVFXBounceLinkProjectileSkipped Reason=%s"), !World ? TEXT("NoWorld") : TEXT("NoOwner"));
		return nullptr;
	}

	// Orient the authored horizontal slash with the PLANAR segment direction (Z
	// flattened), matching the readable first-iteration ImpactAnchored spawn in
	// TrySpawnBoundWeaponBaseSlashVFX, which used Forward.Rotation() of the planar
	// direction. Using the full 3D direction here pitched the slash edge-on — the hero
	// attack origin sits above the enemy aim point, so the descending segment rolled the
	// horizontal slash into a thin streak that read as "coming from outside" instead of
	// a horizontal slash travelling along the path.
	const FRotator SpawnRotation = T66ResolvePlanarDirection(End - Start, OwnerActor).Rotation();
	const FTransform SpawnTransform(SpawnRotation, Start);
	AT66HeroProjectile* Projectile = World->SpawnActorDeferred<AT66HeroProjectile>(
		AT66HeroProjectile::StaticClass(),
		SpawnTransform,
		OwnerActor,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("CombatVFXBounceLinkProjectileSkipped Reason=SpawnActorFailed Start=%s End=%s"), *Start.ToCompactString(), *End.ToCompactString());
		return nullptr;
	}

	Projectile->SetVisualOnly(true);
	Projectile->SetActorTickEnabled(true);
	if (Projectile->CollisionSphere)
	{
		Projectile->CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Projectile->FinishSpawning(SpawnTransform);
	Projectile->Damage = 0;
	Projectile->DamageSourceID = UT66DamageLogSubsystem::SourceID_AutoAttack;
	if (Projectile->ProjectileMovement)
	{
		// Keep the authored slash level (its fixed planar SpawnRotation) while it
		// translates along the link; do not re-pitch the carrier toward the descending
		// 3D velocity vector, which is what turned the horizontal slash edge-on.
		Projectile->ProjectileMovement->bRotationFollowsVelocity = false;
	}

	const float TravelSeconds = (ProjectileSpeed > KINDA_SMALL_NUMBER)
		? (FVector::Dist(Start, End) / ProjectileSpeed)
		: 0.f;
	const bool bUsesAuthoredCarrier = (CarrierSystem != nullptr);
	bool bSpawnedAuthoredCarrier = false;
	if (bUsesAuthoredCarrier)
	{
		// Spawn the authored Bounce slash exactly as the proven first-iteration
		// ImpactAnchored path does (SpawnSystemAtLocation yields a fully initialized
		// component), then drive that component's world location along this link so the
		// readable horizontal slash travels from the segment start to the impact point
		// instead of appearing statically on the target. Routing the authored system
		// through a runtime SetAsset() on a pre-created subobject left the slash
		// unrendered; this restores the first-iteration look as a moving carrier.
		const float CarrierScale = FMath::Max(0.01f, CarrierVisualScale);
		// Attach the authored slash to the moving projectile's root so it inherits the
		// actor transform every frame. The prior approach spawned the carrier free-standing
		// and drove it with SetWorldLocation each tick; that left the bLocalSpace slash
		// rendering at its spawn point instead of travelling, so the readable slash only
		// appeared once the carrier reached the target. Hard attachment guarantees the
		// rendered slash follows the SetActorLocation travel from hero->target.
		USceneComponent* CarrierAttachRoot = Projectile->GetRootComponent();
		UNiagaraComponent* CarrierComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CarrierSystem,
			CarrierAttachRoot,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		if (CarrierComponent)
		{
			CarrierComponent->SetWorldScale3D(FVector(CarrierScale));
			CarrierComponent->SetVariableLinearColor(FName(TEXT("User.Color")), Color);
			CarrierComponent->SetVariableLinearColor(FName(TEXT("User.Tint")), Color);
			CarrierComponent->SetVariableLinearColor(FName(TEXT("Color")), Color);
			CarrierComponent->SetTranslucentSortPriority(14);
			// The authored Niagara slash is the production carrier. Hide the inherited
			// temporary profile meshes so the proof cannot read as a blue placeholder
			// sphere/cube travelling with the same projectile.
			FT66TemporaryProjectileSystem::HideMesh(Projectile->VisualMesh);
			FT66TemporaryProjectileSystem::HideMesh(Projectile->AccentMesh);
			// Do not register the carrier as a SetWorldLocation-driven component: it is now
			// hard-attached and follows the projectile's SetActorLocation travel directly.
			bSpawnedAuthoredCarrier = true;
		}
	}
	if (!bSpawnedAuthoredCarrier)
	{
		// Development fallback only (no authored Niagara resolved): keep the temporary
		// profile mover so the link is still visible during bring-up.
		Projectile->ConfigureTemporaryProjectileVisual(
			FT66TemporaryProjectileSystem::ProfileHeroBounce(),
			Color,
			FMath::Max(0.1f, ScaleMultiplier));
	}
	// Drive the visual-only link by game-time interpolation over the presentation travel
	// window (not ProjectileMovement speed). A large capture-frame delta previously let the
	// speed-based mover overshoot the entire segment in one tick, so the age-revealed slash
	// only became visible once the carrier had been dragged onto the target. Time-based
	// travel keeps the carrier on the hero->impact path across frames. It still self-
	// destructs on arrival and fires the arrival callback that stages the next link.
	Projectile->SetTimedVisualTravel(Start, End, TravelSeconds);

	if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
	{
		UE_LOG(
			LogT66Combat,
			Display,
			TEXT("CombatVFXBounceLinkProjectile LinkIndex=%d LinkCount=%d Start=%s End=%s Speed=%.1f TravelSeconds=%.3f Carrier=%s Time=%.3f"),
			LinkIndex,
			LinkCount,
			*Start.ToCompactString(),
			*End.ToCompactString(),
			ProjectileSpeed,
			TravelSeconds,
			bSpawnedAuthoredCarrier ? *CarrierSystem->GetPathName() : TEXT("TemporaryProfileFallback"),
			World->GetTimeSeconds());
	}

	return Projectile;
}

AT66HeroProjectile* UT66CombatComponent::SpawnVisualTravelProjectile(
	const FVector& Start,
	const FVector& End,
	const FLinearColor& Color,
	const FName ProfileID,
	const float ScaleMultiplier,
	const float TravelSeconds,
	UNiagaraSystem* CarrierSystem,
	const float CarrierVisualScale)
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return nullptr;
	}

	const FRotator SpawnRotation = T66ResolvePlanarDirection(End - Start, OwnerActor).Rotation();
	const FTransform SpawnTransform(SpawnRotation, Start);
	AT66HeroProjectile* Projectile = World->SpawnActorDeferred<AT66HeroProjectile>(
		AT66HeroProjectile::StaticClass(),
		SpawnTransform,
		OwnerActor,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("T66VisualTravelProjectileSkipped Reason=SpawnActorFailed Start=%s End=%s"), *Start.ToCompactString(), *End.ToCompactString());
		return nullptr;
	}

	// Visual-only mover: no collision, no damage. Damage stays authoritative in the caller.
	Projectile->SetVisualOnly(true);
	Projectile->SetActorTickEnabled(true);
	if (Projectile->CollisionSphere)
	{
		Projectile->CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	Projectile->FinishSpawning(SpawnTransform);
	Projectile->Damage = 0;
	Projectile->DamageSourceID = UT66DamageLogSubsystem::SourceID_AutoAttack;
	if (Projectile->ProjectileMovement)
	{
		Projectile->ProjectileMovement->bRotationFollowsVelocity = false;
	}
	bool bSpawnedAuthoredCarrier = false;
	if (CarrierSystem)
	{
		// Attach the authored Niagara carrier to the moving projectile root exactly as the
		// proven Bounce link carrier does (SpawnSystemAttached yields a fully initialized,
		// rendering component that inherits the actor transform each frame). The authored
		// silhouette IS the visible shot; runtime only transports it from Start to End. The
		// inherited temporary profile meshes are hidden so the shot cannot read as the blue
		// placeholder cube/sphere.
		USceneComponent* CarrierAttachRoot = Projectile->GetRootComponent();
		UNiagaraComponent* CarrierComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CarrierSystem,
			CarrierAttachRoot,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		if (CarrierComponent)
		{
			CarrierComponent->SetWorldScale3D(FVector(FMath::Max(0.01f, CarrierVisualScale)));
			CarrierComponent->SetVariableLinearColor(FName(TEXT("User.Color")), Color);
			CarrierComponent->SetVariableLinearColor(FName(TEXT("User.Tint")), Color);
			CarrierComponent->SetVariableLinearColor(FName(TEXT("Color")), Color);
			CarrierComponent->SetTranslucentSortPriority(14);
			FT66TemporaryProjectileSystem::HideMesh(Projectile->VisualMesh);
			FT66TemporaryProjectileSystem::HideMesh(Projectile->AccentMesh);
			bSpawnedAuthoredCarrier = true;
		}
	}
	if (!bSpawnedAuthoredCarrier)
	{
		// Development fallback only (no authored Niagara resolved): keep the temporary profile
		// mover so the shot is still visible during bring-up.
		Projectile->ConfigureTemporaryProjectileVisual(
			ProfileID,
			Color,
			FMath::Max(0.1f, ScaleMultiplier));
	}
	Projectile->SetTimedVisualTravel(Start, End, FMath::Max(0.f, TravelSeconds));
	return Projectile;
}

void UT66CombatComponent::SpawnDOTApplicatorMarkers(AActor* FollowTarget, const FLinearColor& Color, const float Duration, const float MarkerScale)
{
	UWorld* World = GetWorld();
	if (!World || !FollowTarget)
	{
		return;
	}

	FActorSpawnParameters MarkerSpawnParams;
	MarkerSpawnParams.Owner = GetOwner();
	MarkerSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AT66DotMarkerVFX* Markers = World->SpawnActor<AT66DotMarkerVFX>(
		AT66DotMarkerVFX::StaticClass(),
		FollowTarget->GetActorLocation(),
		FRotator::ZeroRotator,
		MarkerSpawnParams);
	if (!Markers)
	{
		UE_LOG(LogT66Combat, Warning, TEXT("T66DotApplicatorMarkersSkipped Reason=SpawnActorFailed Target=%s"), *GetNameSafe(FollowTarget));
		return;
	}

	constexpr int32 DotMarkerCount = 3;
	Markers->InitializeMarkers(FollowTarget, DotMarkerCount, Color, FMath::Max(0.01f, MarkerScale), Duration);
	UE_LOG(
		LogT66Combat,
		Display,
		TEXT("T66DotApplicatorMarkersSpawned Target=%s MarkerCount=%d Duration=%.2f (visual-only; single DOT payload unchanged)"),
		*GetNameSafe(FollowTarget),
		Markers->GetSpawnedMarkerCount(),
		Duration);

	// DOT-proof alignment hook: after snap-attach the marker root must sit on the target
	// with a near-zero target-relative offset. Log target/marker world positions and the
	// resulting offset so the marker-on-target requirement is validatable from logs alone.
	const FVector TargetLoc = FollowTarget->GetActorLocation();
	const FVector MarkerLoc = Markers->GetActorLocation();
	const FVector TargetRelativeOffset = Markers->GetRootComponent()
		? Markers->GetRootComponent()->GetRelativeLocation()
		: (MarkerLoc - TargetLoc);
	UE_LOG(
		LogT66Combat,
		Display,
		TEXT("T66DotMarkerAlignment Target=%s TargetLoc=%s MarkerLoc=%s TargetRelativeOffset=%s OffsetSize=%.3f (expect ~0; markers on target)"),
		*GetNameSafe(FollowTarget),
		*TargetLoc.ToString(),
		*MarkerLoc.ToString(),
		*TargetRelativeOffset.ToString(),
		TargetRelativeOffset.Size());
}

// ---------------------------------------------------------------------------
// TryFire — the hero auto-attack heartbeat.
// Target finding now walks the small EnemiesInRange list (maintained by sphere
// overlap events) instead of doing 3x TActorIterator over the entire world.
// ---------------------------------------------------------------------------
void UT66CombatComponent::TryFire()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UWorld* World = GetWorld();
	if (!World) return;

	if (bSuppressAutoAttack)
	{
		return;
	}

	if (!bHasCachedWeaponData)
	{
		return;
	}

	FLagScopedScope LagScope(World, TEXT("CombatComponent::TryFire"));
	UT66RngSubsystem* RngSub = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;

	// Safe zone rule: if hero is inside any NPC safe bubble, do not fire.
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OwnerActor))
	{
		if (Hero->IsInSafeZone())
		{
			return;
		}
	}

	struct FResolvedAutoAttackHit
	{
		int32 Damage = 0;
		FName EventType = NAME_None;
		bool bCrit = false;
	};

	// Crit: roll per hit; multiply damage and pass EventType_Crit for floating text.
	auto ResolveCrit = [this, RngSub](int32 BaseDamage) -> FResolvedAutoAttackHit
	{
		if (!CachedRunState) return { BaseDamage, NAME_None, false };
		const float CritChance = CachedRunState->GetCritChance01();
		const bool bCrit = RollTierChance(CritChance, RngSub);
		if (bCrit)
		{
			const float Mult = CachedRunState->GetCritDamageMultiplier();
			return { FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult)), UT66FloatingCombatTextSubsystem::EventType_Crit, true };
		}
		return { BaseDamage, NAME_None, false };
	};

	auto ApplyResolvedAutoAttackDamage = [this, RngSub](const FT66CombatTargetHandle& TargetHandle, const FResolvedAutoAttackHit& Resolved, const FName RangeEvent)
	{
		ApplyDamageToTargetHandle(TargetHandle, Resolved.Damage, Resolved.EventType, NAME_None, RangeEvent);
		if (!Resolved.bCrit || !CachedRunState)
		{
			return;
		}

		const float ExecuteChance = CachedRunState->GetExecuteChance01();
		if (ExecuteChance > 0.f && RollTierChance(ExecuteChance, RngSub))
		{
			T66CombatShared::TryApplyNonBossOHKO(TargetHandle.Actor.Get(), &TargetHandle, FName(TEXT("Execute")), NAME_None);
		}
	};

	FVector MyLoc = OwnerActor->GetActorLocation();
	const FVector AttackOrigin = MyLoc + FVector(0.f, 0.f, 64.f);

	// Close/Long range damage: multiply base damage by range-based multiplier (close = 0–10% of range, long = 90–100%).
	// OutRangeEvent set when in close/long zone and multiplier != 1 (for floating text on hero).
	auto GetRangeMultipliedDamage = [this, MyLoc](int32 BaseDamage, AActor* Target, FName* OutRangeEvent = nullptr) -> int32
	{
		if (!CachedRunState || !Target) return BaseDamage;
		const float Dist = FVector::Dist(MyLoc, Target->GetActorLocation());
		const float CloseThresh = CachedRunState->GetCloseRangeThreshold();
		const float LongThresh = CachedRunState->GetLongRangeThreshold();
		if (Dist <= CloseThresh)
		{
			const float Mult = CachedRunState->GetCloseRangeDamageMultiplier();
			if (OutRangeEvent && Mult != 1.f) *OutRangeEvent = UT66FloatingCombatTextSubsystem::EventType_CloseRange;
			return FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult));
		}
		if (Dist >= LongThresh)
		{
			const float Mult = CachedRunState->GetLongRangeDamageMultiplier();
			if (OutRangeEvent && Mult != 1.f) *OutRangeEvent = UT66FloatingCombatTextSubsystem::EventType_LongRange;
			return FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult));
		}
		return BaseDamage;
	};

	const float RangeSq = AttackRange * AttackRange;

	bool bHasCachedPierceTargets = false;
	AActor* CachedPiercePrimaryTarget = nullptr;
	float CachedPierceLineLength = 0.f;
	float CachedPierceRadius = 0.f;
	FVector CachedPierceDirection = FVector::ForwardVector;
	TArray<AActor*> CachedPierceTargets;

	bool bHasCachedSlashTargets = false;
	AActor* CachedSlashPrimaryTarget = nullptr;
	float CachedSlashRadius = 0.f;
	bool bCachedSlashUsesFrontalSector = false;
	FVector CachedSlashCenter = FVector::ZeroVector;
	FVector CachedSlashForward = FVector::ForwardVector;
	float CachedSlashHalfAngleDegrees = 90.f;
	float CachedSlashInnerRadius = 0.f;
	TArray<AActor*> CachedSlashTargets;

	auto BuildPierceTargets = [&](AActor* QueryPrimaryTarget, float LineLength, float PierceRadius, TArray<AActor*>& OutTargets, FVector& OutDir, const FVector* OverrideTargetLocation = nullptr)
	{
		if (!QueryPrimaryTarget && !OverrideTargetLocation)
		{
			OutTargets.Reset();
			OutDir = FVector::ForwardVector;
			return;
		}

		const bool bCanUseCache = (OverrideTargetLocation == nullptr);
		if (bCanUseCache
			&& bHasCachedPierceTargets
			&& CachedPiercePrimaryTarget == QueryPrimaryTarget
			&& FMath::IsNearlyEqual(CachedPierceLineLength, LineLength)
			&& FMath::IsNearlyEqual(CachedPierceRadius, PierceRadius))
		{
			OutTargets = CachedPierceTargets;
			OutDir = CachedPierceDirection;
			return;
		}

		const FVector TargetLoc = OverrideTargetLocation ? *OverrideTargetLocation : QueryPrimaryTarget->GetActorLocation();
		OutDir = (TargetLoc - MyLoc).GetSafeNormal();
		if (OutDir.IsNearlyZero())
		{
			OutDir = OwnerActor->GetActorForwardVector().GetSafeNormal();
			if (OutDir.IsNearlyZero())
			{
				OutDir = FVector::ForwardVector;
			}
		}
		const float HalfLen = FMath::Max(1.f, (LineLength * 0.5f) - PierceRadius);
		const FVector MidPoint = MyLoc + OutDir * (LineLength * 0.5f);
		const FQuat Rot = FQuat::FindBetweenNormals(FVector::UpVector, OutDir);
		const FCollisionShape Cap = FCollisionShape::MakeCapsule(PierceRadius, HalfLen);
		T66CombatDebugDraw::DrawDamageCapsule(World, MidPoint, Rot, HalfLen, PierceRadius, TEXT("Hero Pierce Tube Damage"), true);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(Overlaps, MidPoint, Rot, ECC_Pawn, Cap, Params);

		const FVector SegmentStart = MyLoc;
		const FVector SegmentEnd = MyLoc + OutDir * LineLength;
		const float PierceRadiusSq = PierceRadius * PierceRadius;
		auto TryAddPierceTarget = [&](AActor* Candidate, const bool bRequireSegmentContainment)
		{
			if (!Candidate
				|| Candidate == QueryPrimaryTarget
				|| !IsValidAutoTarget(Candidate)
				|| !(Cast<AT66EnemyBase>(Candidate) || Cast<AT66MobBase>(Candidate) || Cast<AT66BossBase>(Candidate)))
			{
				return;
			}

			const FT66CombatTargetHandle CandidateHandle = MakeActorTargetHandle(Candidate);
			if (!CandidateHandle.IsValid())
			{
				return;
			}

			const FVector CandidatePoint = GetTargetAimPoint(CandidateHandle);
			if (CandidatePoint.IsNearlyZero()
				|| !HasUnblockedAutoAttackPath(AttackOrigin, CandidateHandle))
			{
				return;
			}
			if (bRequireSegmentContainment
				&& FMath::PointDistToSegmentSquared(CandidatePoint, SegmentStart, SegmentEnd) > PierceRadiusSq)
			{
				return;
			}

			OutTargets.AddUnique(Candidate);
		};

		OutTargets.Reset();
		if (QueryPrimaryTarget && IsValidAutoTarget(QueryPrimaryTarget))
		{
			OutTargets.Add(QueryPrimaryTarget);
		}
		for (const FOverlapResult& O : Overlaps)
		{
			TryAddPierceTarget(O.GetActor(), false);
		}

		const TArray<AActor*> SphereCandidates = T66GatherAttackTargetsInSphere(
			World,
			OwnerActor,
			MidPoint,
			(LineLength * 0.5f) + PierceRadius);
		for (AActor* Candidate : SphereCandidates)
		{
			TryAddPierceTarget(Candidate, true);
		}
		OutTargets.Sort([&MyLoc](const AActor& A, const AActor& B)
		{
			return FVector::DistSquared(MyLoc, A.GetActorLocation()) < FVector::DistSquared(MyLoc, B.GetActorLocation());
		});

		if (bCanUseCache)
		{
			bHasCachedPierceTargets = true;
			CachedPiercePrimaryTarget = QueryPrimaryTarget;
			CachedPierceLineLength = LineLength;
			CachedPierceRadius = PierceRadius;
			CachedPierceDirection = OutDir;
			CachedPierceTargets = OutTargets;
		}
	};

	auto BuildSlashTargets = [&](
		AActor* QueryPrimaryTarget,
		float Radius,
		TArray<AActor*>& OutTargets,
		bool bUseFrontalSector,
		const FVector& PreferredForward,
		float HalfAngleDegrees,
		float InnerRadius,
		const FVector* OverrideCenter = nullptr,
		FString DebugLabelOverride = FString())
	{
		if (!QueryPrimaryTarget && !OverrideCenter)
		{
			OutTargets.Reset();
			return;
		}

		const FT66CombatTargetHandle QueryPrimaryHandle = QueryPrimaryTarget ? MakeActorTargetHandle(QueryPrimaryTarget) : FT66CombatTargetHandle{};
		const FVector Center = OverrideCenter
			? *OverrideCenter
			: (QueryPrimaryHandle.IsValid() ? GetTargetAimPoint(QueryPrimaryHandle) : QueryPrimaryTarget->GetActorLocation());
		const FVector SectorForward = T66ResolvePlanarDirection(PreferredForward, OwnerActor);
		const float ClampedHalfAngle = FMath::Clamp(HalfAngleDegrees, 1.f, 179.f);
		const float ClampedInnerRadius = FMath::Clamp(InnerRadius, 0.f, FMath::Max(0.f, Radius - 1.f));
		const bool bCanUseCache = (OverrideCenter == nullptr);
		if (bCanUseCache
			&& bHasCachedSlashTargets
			&& CachedSlashPrimaryTarget == QueryPrimaryTarget
			&& FMath::IsNearlyEqual(CachedSlashRadius, Radius)
			&& bCachedSlashUsesFrontalSector == bUseFrontalSector
			&& CachedSlashCenter.Equals(Center, 0.5f)
			&& CachedSlashForward.Equals(SectorForward, 0.01f)
			&& FMath::IsNearlyEqual(CachedSlashHalfAngleDegrees, ClampedHalfAngle)
			&& FMath::IsNearlyEqual(CachedSlashInnerRadius, ClampedInnerRadius))
		{
			OutTargets = CachedSlashTargets;
			return;
		}

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);
		if (QueryPrimaryTarget)
		{
			Params.AddIgnoredActor(QueryPrimaryTarget);
		}

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(
			Overlaps,
			Center,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(Radius),
			Params);

		const FString DamageDebugLabel = DebugLabelOverride.IsEmpty()
			? (bUseFrontalSector ? FString(TEXT("Hero AOE Sector Damage")) : FString(TEXT("Hero AOE Sphere Damage")))
			: DebugLabelOverride;
		if (bUseFrontalSector)
		{
			T66CombatDebugDraw::DrawDamageSector(World, Center, SectorForward, Radius, ClampedHalfAngle, DamageDebugLabel, true, ClampedInnerRadius);
		}
		else
		{
			T66CombatDebugDraw::DrawDamageSphere(World, Center, Radius, DamageDebugLabel, true);
		}

		OutTargets.Reset();
		if (QueryPrimaryTarget && IsValidAutoTarget(QueryPrimaryTarget))
		{
			OutTargets.Add(QueryPrimaryTarget);
		}
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (AActor* Hit = Overlap.GetActor(); Hit && IsValidAutoTarget(Hit))
			{
				const FT66CombatTargetHandle CandidateHandle = MakeActorTargetHandle(Hit);
				if (bUseFrontalSector && !T66IsPointInsidePlanarSector(Center, SectorForward, GetTargetAimPoint(CandidateHandle), Radius, ClampedHalfAngle, ClampedInnerRadius))
				{
					continue;
				}
				const FVector LineOfSightOrigin = bUseFrontalSector ? AttackOrigin : Center;
				if (!HasUnblockedAutoAttackPath(LineOfSightOrigin, CandidateHandle))
				{
					continue;
				}
				OutTargets.AddUnique(Hit);
			}
		}

		if (bCanUseCache)
		{
			bHasCachedSlashTargets = true;
			CachedSlashPrimaryTarget = QueryPrimaryTarget;
			CachedSlashRadius = Radius;
			bCachedSlashUsesFrontalSector = bUseFrontalSector;
			CachedSlashCenter = Center;
			CachedSlashForward = SectorForward;
			CachedSlashHalfAngleDegrees = ClampedHalfAngle;
			CachedSlashInnerRadius = ClampedInnerRadius;
			CachedSlashTargets = OutTargets;
		}
	};

	// Purge stale weak pointers (destroyed actors that didn't fire EndOverlap).
	EnemiesInRange.RemoveAll([](const TWeakObjectPtr<AActor>& Weak) { return !Weak.IsValid(); });

	// Hero attack category (Pierce/Bounce/AOE/DOT) and data for Bounce/DOT params.
	ET66AttackCategory AttackCategory = ET66AttackCategory::AOE;
	FHeroData HeroDataForPrimary;
	bool bHaveHeroData = bHasCachedHeroData;
	FName CurrentHeroID = NAME_None;
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OwnerActor))
	{
		CurrentHeroID = Hero->HeroID;
		if (bHasCachedHeroData)
		{
			AttackCategory = CachedHeroData.PrimaryCategory;
			HeroDataForPrimary = CachedHeroData;
		}
	}
	if (bHasCachedWeaponData)
	{
		AttackCategory = CachedWeaponData.Branch;
	}
	FT66CombatTargetHandle PrimaryTargetHandle;
	TArray<AActor*> WeaponHitActors;
	WeaponHitActors.Reserve(12);
	TArray<FT66CombatImpactContext> WeaponImpactContexts;
	WeaponImpactContexts.Reserve(2);
	FT66CombatImpactContext PrimaryWeaponImpactContext;
	bool bHasPrimaryWeaponImpactContext = false;

	auto ResolveWeaponImpactSourceID = [&]() -> FName
	{
		return (bHasCachedWeaponData && !CachedWeaponData.WeaponID.IsNone())
			? CachedWeaponData.WeaponID
			: CurrentHeroID;
	};

	auto AddImpactTargetHandleUnique = [this](FT66CombatImpactContext& Context, const FT66CombatTargetHandle& TargetHandle)
	{
		if (!TargetHandle.IsValid())
		{
			return;
		}

		const FString TargetKey = MakeTargetHandleKey(TargetHandle);
		for (const FT66CombatTargetHandle& ExistingHandle : Context.HitTargetHandles)
		{
			if (MakeTargetHandleKey(ExistingHandle) == TargetKey)
			{
				return;
			}
		}
		Context.HitTargetHandles.Add(TargetHandle);
	};

	auto LogCombatImpactContext = [](const FT66CombatImpactContext& Context, const TCHAR* Phase)
	{
		if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() == 0)
		{
			return;
		}

		UE_LOG(
			LogT66Combat,
			Display,
			TEXT("CombatImpactContext Phase=%s SourceType=%s SourceID=%s ParentSourceID=%s HeroID=%s AttackCategory=%s DamageCenter=%s DamageCenterValid=%d ImpactPoint=%s ImpactPointValid=%d AttackOrigin=%s Forward=%s Radius=%.2f InnerRadius=%.2f HalfAngleDegrees=%.2f LineLength=%.2f TubeRadius=%.2f ChainIndex=%d HitTargets=%d EffectiveDamage=%d"),
			Phase ? Phase : TEXT("Unknown"),
			GetT66CombatVFXSourceTypeName(Context.SourceType),
			*Context.SourceID.ToString(),
			*Context.ParentSourceID.ToString(),
			*Context.HeroID.ToString(),
			GetT66AttackCategoryName(Context.AttackCategory),
			*Context.DamageCenter.ToCompactString(),
			Context.bDamageCenterValid ? 1 : 0,
			*Context.ImpactPoint.ToCompactString(),
			Context.bImpactPointValid ? 1 : 0,
			*Context.AttackOrigin.ToCompactString(),
			*Context.Forward.ToCompactString(),
			Context.Radius,
			Context.InnerRadius,
			Context.HalfAngleDegrees,
			Context.LineLength,
			Context.TubeRadius,
			Context.ChainIndex,
			Context.HitTargetHandles.Num(),
			Context.EffectiveDamage);
	};

	auto PublishWeaponImpactContext = [&](FT66CombatImpactContext Context, const bool bPrimary)
	{
		Context.SourceType = ET66CombatVFXBindingSourceType::WeaponBase;
		Context.SourceID = ResolveWeaponImpactSourceID();
		Context.HeroID = CurrentHeroID;
		Context.AttackOrigin = AttackOrigin;
		Context.AttackCategory = AttackCategory;
		LogCombatImpactContext(Context, bPrimary ? TEXT("WeaponPrimary") : TEXT("WeaponSecondary"));
		WeaponImpactContexts.Add(Context);
		if (bPrimary && !bHasPrimaryWeaponImpactContext)
		{
			PrimaryWeaponImpactContext = Context;
			bHasPrimaryWeaponImpactContext = true;
		}
	};

	// Proof allowlist for the category-native idol impact-presentation lane. Any idol listed
	// in the centralized set is driven from the official weapon impact point and dispatches to
	// its own category-native presentation/damage in the loop below. The dispatch itself is
	// general (switch on FIdolData.Category); this allowlist only gates WHICH idols enter the
	// lane for the current proof phase: Water=AOE, Light=Pierce, Electric=Bounce, Poison=DOT.
	// Membership is centralized in T66CombatShared so runtime and overlay harness cannot drift.
	auto UsesImpactPresentationForIdol = [](const FCachedIdolSlot& CachedIdolSlot) -> bool
	{
		if (!CachedIdolSlot.bValid || CachedIdolSlot.IdolID.IsNone())
		{
			return false;
		}
		return T66CombatShared::GetImpactPresentationProofIdols().Contains(CachedIdolSlot.IdolID);
	};

	auto SpawnWeaponProjectileVisual = [&](AActor* Target, const FName& SourceIdolID, const int32 PayloadIndex, const int32 PayloadCount)
	{
		if (!Target || !IsValidAutoTarget(Target))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector RightVector = OwnerActor->GetActorRightVector().GetSafeNormal();
		const float CenteredIndex = static_cast<float>(PayloadIndex) - (static_cast<float>(FMath::Max(1, PayloadCount) - 1) * 0.5f);
		const FVector SpawnLoc = MyLoc + FVector(0.f, 0.f, 48.f) + RightVector * (CenteredIndex * 34.f);
		const FVector TargetLoc = Target->GetActorLocation() + FVector(0.f, 0.f, 36.f);
		const FRotator SpawnRot = (TargetLoc - SpawnLoc).Rotation();

		AT66HeroProjectile* Projectile = World->SpawnActor<AT66HeroProjectile>(
			AT66HeroProjectile::StaticClass(),
			SpawnLoc,
			SpawnRot,
			SpawnParams);
		if (!Projectile)
		{
			return;
		}

		Projectile->SetVisualOnly(true);
		Projectile->Damage = 0;
		Projectile->DamageSourceID = SourceIdolID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : SourceIdolID;
		const FLinearColor CoreColor = FT66TemporaryProjectileSystem::HeroProjectileColor();
		const FName CoreProfile = FT66TemporaryProjectileSystem::GetHeroAttackProfile(AttackCategory);
		const FName OverlayProfile = SourceIdolID.IsNone() ? NAME_None : FT66TemporaryProjectileSystem::ProfileIdolOverlay();
		const FLinearColor OverlayColor = SourceIdolID.IsNone()
			? FLinearColor::Transparent
			: FT66TemporaryProjectileSystem::HeroProjectileColor();
		Projectile->ConfigureTemporaryProjectileVisual(
			CoreProfile,
			CoreColor,
			ProjectileScaleMultiplier * (SourceIdolID.IsNone() ? 1.f : 0.94f),
			OverlayProfile,
			OverlayColor,
			1.f);
		if (bHaveHeroData && HeroDataForPrimary.ProjectileSpeed > 0.f)
		{
			Projectile->SetProjectileSpeed(HeroDataForPrimary.ProjectileSpeed);
		}
		Projectile->SetTargetActor(Target);
	};

	// --- Pierce (straight line): full range so enemies behind the first are hit; 10% damage reduction per pierced target. ---
	auto PerformPierce = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;

		const FT66CombatTargetHandle PrimaryHandle = (PrimaryTarget == PrimaryTargetHandle.Actor.Get())
			? PrimaryTargetHandle
			: ResolveAutoAttackTargetHandle(PrimaryTarget, false, RngSub);
		const FVector TargetLoc = GetTargetAimPoint(PrimaryHandle);
		const float LineLength = AttackRange;
		const float PierceRadius = 80.f * ProjectileScaleMultiplier;
		FVector Dir = FVector::ForwardVector;
		TArray<AActor*> InLine;
		BuildPierceTargets(PrimaryTarget, LineLength, PierceRadius, InLine, Dir, &TargetLoc);
		FT66CombatImpactContext PierceImpactContext;
		PierceImpactContext.PrimaryTargetHandle = PrimaryHandle;
		PierceImpactContext.ImpactPoint = TargetLoc;
		PierceImpactContext.Forward = T66ResolvePlanarDirection(Dir, OwnerActor);
		PierceImpactContext.LineLength = LineLength;
		PierceImpactContext.TubeRadius = PierceRadius;
		PierceImpactContext.EffectiveDamage = EffectiveDamagePerShot;
		PierceImpactContext.bImpactPointValid = true;
		// Lane start for the PathAnchored Pierce VFX: the hero attack origin, with the
		// vertical slash carrier extending forward along LineLength / TubeRadius.
		PierceImpactContext.AttackOrigin = AttackOrigin;
		const int32 BasePierceTargets = bHaveHeroData ? FMath::Max(0, HeroDataForPrimary.BasePierceCount) + 1 : 1;
		const int32 WeaponPierceTargets = bHasCachedWeaponData ? FMath::Max(0, CachedWeaponData.BonusPierceCount) : 0;
		const int32 MaxPierceTargets = FMath::Max(1, BasePierceTargets + WeaponPierceTargets);
		if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
		{
			UE_LOG(
				LogT66Combat,
				Display,
				TEXT("PierceTargetResolution SourceID=%s CandidateTargets=%d MaxPierceTargets=%d BasePierceTargets=%d WeaponPierceTargets=%d LineLength=%.2f TubeRadius=%.2f"),
				*ResolveWeaponImpactSourceID().ToString(),
				InLine.Num(),
				MaxPierceTargets,
				BasePierceTargets,
				WeaponPierceTargets,
				LineLength,
				PierceRadius);
		}
		if (InLine.Num() > MaxPierceTargets)
		{
			InLine.SetNum(MaxPierceTargets, EAllowShrinking::No);
		}
		const float FalloffPerHit = FMath::Clamp(
			((bHaveHeroData && HeroDataForPrimary.FalloffPerHit > 0.f) ? HeroDataForPrimary.FalloffPerHit : 0.10f)
			* (bHasCachedWeaponData ? FMath::Max(0.f, CachedWeaponData.FalloffPerHitMultiplier) : 1.f),
			0.f,
			0.95f);

		for (int32 i = 0; i < InLine.Num(); ++i)
		{
			const FT66CombatTargetHandle HitHandle = (i == 0)
				? PrimaryHandle
				: ResolveAutoAttackTargetHandle(InLine[i], false, RngSub);
			const float Multiplier = FMath::Max(0.1f, 1.f - FalloffPerHit * static_cast<float>(i));
			const float PrimaryMult = (i == 0) ? PrimaryDamageMult : 1.f;
			const int32 BaseDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * Multiplier * PrimaryMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BaseDmg, InLine[i], &RangeEvent);
			const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
			ApplyResolvedAutoAttackDamage(HitHandle, Resolved, RangeEvent);
			WeaponHitActors.AddUnique(InLine[i]);
			AddImpactTargetHandleUnique(PierceImpactContext, HitHandle);
		}

		if (PierceImpactContext.HitTargetHandles.Num() > 0)
		{
			PublishWeaponImpactContext(PierceImpactContext, true);
		}

		TrySpawnBoundWeaponBaseSlashVFX(PierceImpactContext, EffectiveDamagePerShot, CurrentHeroID, AttackCategory);

		PlayHeroAttackSfx(CurrentHeroID, AttackCategory, TargetLoc);
		return true;
	};

	// --- AoE Slash ---
	auto PerformSlash = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;

		const FT66CombatTargetHandle PrimaryHandle = (PrimaryTarget == PrimaryTargetHandle.Actor.Get())
			? PrimaryTargetHandle
			: ResolveAutoAttackTargetHandle(PrimaryTarget, false, RngSub);
		const FVector SlashCenter = GetTargetAimPoint(PrimaryHandle);
		const float BaseSlashRadius = (bHaveHeroData && HeroDataForPrimary.AoeRadius > 0.f) ? HeroDataForPrimary.AoeRadius : SlashRadius;
		const float WeaponAoeRadius = bHasCachedWeaponData ? FMath::Max(0.f, CachedWeaponData.BonusAoeRadius) : 0.f;
		const float EffectiveSlashRadius = BaseSlashRadius * ProjectileScaleMultiplier + WeaponAoeRadius;
		const bool bUseHeroOneFrontalSector = (CurrentHeroID == FName(TEXT("Hero_1")));
		const FVector SlashForward = T66ResolvePlanarDirection(SlashCenter - AttackOrigin, OwnerActor);
		constexpr float HeroOneAoeHalfAngleDegrees = 90.f;
		const float AoeInnerRadiusRatio = bHasCachedWeaponData
			? FMath::Clamp(CachedWeaponData.AoeInnerRadiusRatio, 0.f, 0.95f)
			: 0.f;
		const float EffectiveSlashInnerRadius = bUseHeroOneFrontalSector
			? EffectiveSlashRadius * AoeInnerRadiusRatio
			: 0.f;

		TArray<AActor*> SlashTargets;
		BuildSlashTargets(PrimaryTarget, EffectiveSlashRadius, SlashTargets, bUseHeroOneFrontalSector, SlashForward, HeroOneAoeHalfAngleDegrees, EffectiveSlashInnerRadius);

		FVector SlashContextImpactPoint = SlashCenter;
		const bool bUseCrescentBandImpactPoint =
			bUseHeroOneFrontalSector
			&& EffectiveSlashInnerRadius > KINDA_SMALL_NUMBER
			&& EffectiveSlashRadius > EffectiveSlashInnerRadius;
		if (bUseCrescentBandImpactPoint)
		{
			// Idol overlays should trigger from the visible crescent band, not the hollow damage-query center.
			const float BandMidpointRadius = (EffectiveSlashInnerRadius + EffectiveSlashRadius) * 0.5f;
			SlashContextImpactPoint = SlashCenter + SlashForward * BandMidpointRadius;
		}

		FT66CombatImpactContext SlashImpactContext;
		SlashImpactContext.PrimaryTargetHandle = PrimaryHandle;
		SlashImpactContext.DamageCenter = SlashCenter;
		SlashImpactContext.ImpactPoint = SlashContextImpactPoint;
		SlashImpactContext.Forward = SlashForward;
		SlashImpactContext.Radius = EffectiveSlashRadius;
		SlashImpactContext.InnerRadius = EffectiveSlashInnerRadius;
		SlashImpactContext.HalfAngleDegrees = bUseHeroOneFrontalSector ? HeroOneAoeHalfAngleDegrees : 0.f;
		SlashImpactContext.EffectiveDamage = EffectiveDamagePerShot;
		SlashImpactContext.bUsesFrontalSector = bUseHeroOneFrontalSector;
		SlashImpactContext.bDamageCenterValid = true;
		SlashImpactContext.bImpactPointValid = true;
		for (int32 TargetIndex = 0; TargetIndex < SlashTargets.Num(); ++TargetIndex)
		{
			AActor* SlashTarget = SlashTargets[TargetIndex];
			const FT66CombatTargetHandle SlashTargetHandle = (TargetIndex == 0 && SlashTarget == PrimaryTarget)
				? PrimaryHandle
				: MakeActorTargetHandle(SlashTarget);
			AddImpactTargetHandleUnique(SlashImpactContext, SlashTargetHandle);
		}
		if (SlashImpactContext.HitTargetHandles.Num() > 0)
		{
			PublishWeaponImpactContext(SlashImpactContext, true);
		}

		TrySpawnBoundWeaponBaseSlashVFX(SlashImpactContext, EffectiveDamagePerShot, CurrentHeroID, AttackCategory);

		const int32 HitCount = SlashTargets.Num();
		float ArcaneMult = 1.f;
		if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::ArcaneAmplification)
		{
			if (HitCount >= 5) ArcaneMult = 1.35f;
			else if (HitCount >= 3) ArcaneMult = 1.2f;
		}

		const int32 PrimaryDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult * ArcaneMult));
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(PrimaryDmg, PrimaryTarget, &RangeEvent);
			const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
			ApplyResolvedAutoAttackDamage(PrimaryHandle, Resolved, RangeEvent);
			WeaponHitActors.AddUnique(PrimaryTarget);
		}

		const int32 SplashDmg = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EffectiveDamagePerShot) * ArcaneMult));
		for (int32 TargetIndex = 1; TargetIndex < SlashTargets.Num(); ++TargetIndex)
		{
			AActor* Hit = SlashTargets[TargetIndex];
			if (Hit)
			{
				const FT66CombatTargetHandle HitHandle = ResolveAutoAttackTargetHandle(Hit, false, RngSub);
				FName RangeEvent;
				const int32 RangeDmg = GetRangeMultipliedDamage(SplashDmg, Hit, &RangeEvent);
				const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
				ApplyResolvedAutoAttackDamage(HitHandle, Resolved, RangeEvent);
				WeaponHitActors.AddUnique(Hit);
			}
		}

		PlayHeroAttackSfx(CurrentHeroID, AttackCategory, SlashCenter);
		return true;
	};

	// --- Bounce ---
	auto PerformBounce = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;
		const FT66CombatTargetHandle PrimaryHandle = (PrimaryTarget == PrimaryTargetHandle.Actor.Get())
			? PrimaryTargetHandle
			: ResolveAutoAttackTargetHandle(PrimaryTarget, false, RngSub);
		const int32 BaseBounce = FMath::Max(1, bHaveHeroData ? HeroDataForPrimary.BaseBounceCount : 1);
		const int32 ChaosBonus = CachedRunState ? CachedRunState->GetChaosTheoryBonusBounceCount() : 0;
		const float TimeNow = static_cast<float>(World->GetTimeSeconds());
		const int32 JuicedBonus = (JuicedEndTime > TimeNow) ? JuicedBonusBounce : 0;
		const int32 WeaponBounceBonus = bHasCachedWeaponData ? FMath::Max(0, CachedWeaponData.BonusBounceCount) : 0;
		const int32 BounceCount = BaseBounce + ChaosBonus + JuicedBonus + WeaponBounceBonus;
		const float Falloff = FMath::Clamp(
			(bHaveHeroData ? HeroDataForPrimary.FalloffPerHit : 0.f)
			* (bHasCachedWeaponData ? FMath::Max(0.f, CachedWeaponData.FalloffPerHitMultiplier) : 1.f),
			0.f,
			0.95f);
		const float BounceRangeSq = AttackRange * AttackRange;
		const FVector PrimaryLoc = GetTargetAimPoint(PrimaryHandle);
		TArray<FVector> ChainPositions;
		ChainPositions.Add(MyLoc);
		ChainPositions.Add(PrimaryLoc);

		// PerChainLink impact-context policy: publish one official Bounce weapon impact
		// context per resolved chain link at each authoritative hit point, so future
		// idol/chaining systems read official per-hit contexts instead of one aggregated
		// context. Damage and contexts are resolved here; the moving Bounce projectile
		// presentation is staged separately after the full chain resolves (see
		// StageBounceProjectileChain) so Bounce reads as a moving link sequence rather than
		// static impact slashes.
		auto PublishBounceLink = [&](const FT66CombatTargetHandle& LinkHandle, const FVector& PrevLoc, const FVector& ImpactLoc, const int32 LinkChainIndex, const int32 LinkEffectiveDamage)
		{
			FT66CombatImpactContext LinkContext;
			LinkContext.PrimaryTargetHandle = LinkHandle;
			LinkContext.ImpactPoint = ImpactLoc;
			LinkContext.DamageCenter = ImpactLoc;
			LinkContext.Forward = T66ResolvePlanarDirection(ImpactLoc - PrevLoc, OwnerActor);
			LinkContext.Radius = 0.f;
			LinkContext.ChainIndex = LinkChainIndex;
			LinkContext.EffectiveDamage = LinkEffectiveDamage;
			LinkContext.bDamageCenterValid = true;
			LinkContext.bImpactPointValid = true;
			AddImpactTargetHandleUnique(LinkContext, LinkHandle);
			PublishWeaponImpactContext(LinkContext, LinkChainIndex == 0);
		};

		const int32 PrimaryDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult));
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(PrimaryDmg, PrimaryTarget, &RangeEvent);
			const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
			ApplyResolvedAutoAttackDamage(PrimaryHandle, Resolved, RangeEvent);
			WeaponHitActors.AddUnique(PrimaryTarget);
		}
		PublishBounceLink(PrimaryHandle, MyLoc, PrimaryLoc, 0, PrimaryDmg);

		FVector CurrentLoc = PrimaryLoc;
		TSet<FString> HitKeys;
		HitKeys.Add(MakeTargetHandleKey(PrimaryHandle));
		int32 BouncesLeft = BounceCount - 1;
		float DamageMult = 1.f - Falloff;
		int32 LinkChainIndex = 1;
		while (BouncesLeft > 0)
		{
			const FT66CombatTargetHandle NextHandle = FindClosestTargetHandleInRange(CurrentLoc, BounceRangeSq, &HitKeys);
			AActor* Next = NextHandle.Actor.Get();
			if (!Next) break;
			const FVector NextLoc = GetTargetAimPoint(NextHandle);
			ChainPositions.Add(NextLoc);
			HitKeys.Add(MakeTargetHandleKey(NextHandle));
			const int32 BounceDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * DamageMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BounceDmg, Next, &RangeEvent);
			const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
			ApplyResolvedAutoAttackDamage(NextHandle, Resolved, RangeEvent);
			WeaponHitActors.AddUnique(Next);
			// StaticCharge: 20% chance to confuse bounced targets.
			if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::StaticCharge)
			{
				if (RollTierChance(0.2f, RngSub))
				{
					if (AT66EnemyBase* BounceEnemy = Cast<AT66EnemyBase>(Next))
						BounceEnemy->ApplyConfusion(1.5f);
				}
			}
			PublishBounceLink(NextHandle, CurrentLoc, NextLoc, LinkChainIndex, BounceDmg);
			CurrentLoc = NextLoc;
			DamageMult *= (1.f - Falloff);
			--BouncesLeft;
			++LinkChainIndex;
		}

		// Moving Bounce presentation: stage one visible projectile link at a time along the
		// resolved chain — hero attack origin -> primary, then primary -> each next target —
		// instead of placing static slashes at every impact point simultaneously. The first
		// link is elevated to the hero attack origin so it reads from the hero, not the floor.
		if (ChainPositions.Num() >= 2)
		{
			TArray<FVector> ProjectileChain = ChainPositions;
			ProjectileChain[0] = AttackOrigin;
			const float BounceProjectileSpeed = (bHaveHeroData && HeroDataForPrimary.ProjectileSpeed > 0.f)
				? HeroDataForPrimary.ProjectileSpeed
				: 2400.f;

			// Resolve the authored Bounce slash binding so the moving carrier IS the
			// production red/blue horizontal slash Niagara, not the temporary cube. If the
			// binding can't be resolved, StageBounceProjectileChain falls back to the
			// temporary profile mover (dev fallback) so bring-up is never invisible.
			FT66CombatVFXBindingData BounceBinding;
			UNiagaraSystem* BounceCarrierSystem = nullptr;
			const bool bResolvedBounceCarrier = ResolveCombatVFXBinding(
				ET66CombatVFXBindingSourceType::WeaponBase,
				ResolveWeaponImpactSourceID(),
				AttackCategory,
				BounceBinding,
				BounceCarrierSystem);
			const float CarrierVisualScale = bResolvedBounceCarrier
				? FMath::Max(0.01f, BounceBinding.VisualScaleMultiplier)
				: ProjectileScaleMultiplier;
			// Presentation-only readable travel window. The authored slash reveals along its
			// length by normalized particle age, so it is only fully drawn near end-of-life;
			// the carrier must therefore spend enough on-screen time travelling the hero->
			// target path for the reveal to read as a moving slash rather than a flash at the
			// impact point. This floor also keeps the link travelling across multiple captured
			// frames even if a single frame hitches. Damage timing was resolved above and is
			// unaffected. CarrierPlaybackSeconds stays the authored window so the slash is
			// time-dilated to stretch its reveal across the whole travel (see
			// SpawnBounceLinkProjectile).
			constexpr float ReadableBounceLinkTravelSeconds = 0.60f;
			// Proof staging may stretch the travel window so the frame-rate-limited capture
			// samples each link's full hero->primary / primary->second path (see CVar comment).
			const float ProofReadableTravelSeconds = FMath::Max(0.f, CVarBounceProofReadableTravelSeconds.GetValueOnGameThread());
			const float EffectiveReadableTravelSeconds = ProofReadableTravelSeconds > KINDA_SMALL_NUMBER
				? ProofReadableTravelSeconds
				: ReadableBounceLinkTravelSeconds;
			const float AuthoredCarrierPlaybackSeconds = bResolvedBounceCarrier
				? FMath::Max(0.f, BounceBinding.BasePlaybackSeconds)
				: 0.32f;
			const float MinLinkTravelSeconds = FMath::Max(EffectiveReadableTravelSeconds, AuthoredCarrierPlaybackSeconds);
			const float CarrierPlaybackSeconds = AuthoredCarrierPlaybackSeconds;

			StageBounceProjectileChain(
				ProjectileChain,
				FT66TemporaryProjectileSystem::HeroProjectileColor(),
				BounceProjectileSpeed,
				ProjectileScaleMultiplier,
				BounceCarrierSystem,
				CarrierVisualScale,
				MinLinkTravelSeconds,
				CarrierPlaybackSeconds);
		}

		PlayHeroAttackSfx(CurrentHeroID, AttackCategory, PrimaryLoc);
		return true;
	};

	// --- DOT ---
	auto PerformDOT = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget || !CachedRunState) return false;
		const FT66CombatTargetHandle PrimaryHandle = (PrimaryTarget == PrimaryTargetHandle.Actor.Get())
			? PrimaryTargetHandle
			: ResolveAutoAttackTargetHandle(PrimaryTarget, false, RngSub);
		static const FName HeroPrimaryDotSource(TEXT("HeroPrimaryDot"));
		const float WeaponDotDuration = bHasCachedWeaponData ? FMath::Max(0.f, CachedWeaponData.BonusDotDuration) : 0.f;
		const float WeaponDotTickDamageMultiplier = bHasCachedWeaponData ? FMath::Max(0.01f, CachedWeaponData.BonusDotTickDamageMultiplier) : 1.f;
		const float Duration = ((bHaveHeroData && HeroDataForPrimary.DotDuration > 0.f) ? HeroDataForPrimary.DotDuration : 3.f) + WeaponDotDuration;
		const float TickInterval = (bHaveHeroData && HeroDataForPrimary.DotTickInterval > 0.f) ? HeroDataForPrimary.DotTickInterval : 0.5f;
		const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
		const int32 InitialDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EffectiveDamagePerShot) * 0.5f * PrimaryDamageMult));
		const float DotTotalDamage = static_cast<float>(FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult) - InitialDamage)) * WeaponDotTickDamageMultiplier;
		const float DamagePerTick = DotTotalDamage / static_cast<float>(Ticks);
		FT66CombatImpactContext DotImpactContext;
		DotImpactContext.PrimaryTargetHandle = PrimaryHandle;
		DotImpactContext.ImpactPoint = GetTargetAimPoint(PrimaryHandle);
		DotImpactContext.Forward = T66ResolvePlanarDirection(DotImpactContext.ImpactPoint - AttackOrigin, OwnerActor);
		DotImpactContext.Radius = 0.f;
		DotImpactContext.EffectiveDamage = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult));
		DotImpactContext.bImpactPointValid = true;
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(InitialDamage, PrimaryTarget, &RangeEvent);
			const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
			ApplyResolvedAutoAttackDamage(PrimaryHandle, Resolved, RangeEvent);
			WeaponHitActors.AddUnique(PrimaryTarget);
			AddImpactTargetHandleUnique(DotImpactContext, PrimaryHandle);
		}
		PublishWeaponImpactContext(DotImpactContext, true);

		// DOT ticking + the placeholder applicator markers apply when the single visible
		// hero->target shot reaches the enemy. The shot is visual-only; the initial contact
		// damage and impact context above stay authoritative on this frame so idol overlays
		// still consume the weapon impact context. There is exactly ONE ApplyDOT call here ->
		// a single authoritative DOT payload. The three sphere markers are visual applicators
		// only and never multiply or own DOT damage.
		const FVector DotShotStart = AttackOrigin;
		const FVector DotShotEnd = DotImpactContext.ImpactPoint;
		const FLinearColor DotVisualColor = FT66TemporaryProjectileSystem::HeroProjectileColor();
		const float DotProofTravelSeconds = FMath::Max(0.f, CVarDotProofReadableTravelSeconds.GetValueOnGameThread());
		// Proof-only readability: when the capture stretches travel (CVar > 0), enlarge the
		// placeholder projectile and markers so the frame is legible. Gameplay (CVar == 0) keeps
		// the small placeholder sizes. This affects presentation only, never damage authority.
		const bool bDotProofReadable = DotProofTravelSeconds > 0.f;
		const float DotShotScale = bDotProofReadable ? 3.0f : 1.f;
		const float DotMarkerScale = bDotProofReadable ? 0.6f : 0.18f;

		TWeakObjectPtr<UT66CombatComponent> WeakSelf(this);
		TWeakObjectPtr<AActor> WeakDotTarget(PrimaryTarget);
		auto ApplyDotPayload = [WeakSelf, WeakDotTarget, Duration, TickInterval, DamagePerTick, DotVisualColor, DotMarkerScale]()
		{
			UT66CombatComponent* Self = WeakSelf.Get();
			AActor* DotTarget = WeakDotTarget.Get();
			if (!Self || !DotTarget || !Self->CachedRunState)
			{
				return;
			}
			Self->CachedRunState->ApplyDOT(DotTarget, Duration, TickInterval, DamagePerTick, HeroPrimaryDotSource);
			Self->SpawnDOTApplicatorMarkers(DotTarget, DotVisualColor, Duration, DotMarkerScale);
			UE_LOG(
				LogT66Combat,
				Display,
				TEXT("T66DotPayloadApplied Target=%s Duration=%.2f TickInterval=%.2f DamagePerTick=%.2f Source=HeroPrimaryDot (single payload)"),
				*GetNameSafe(DotTarget),
				Duration,
				TickInterval,
				DamagePerTick);
		};

		// Resolve the authored Hero1Axe_DOT_Base carrier so the single moving hero->target
		// DOT shot IS the production aura-ring Niagara silhouette, transported by the visual
		// projectile (same carrier method class as the Bounce link slash). If the binding can't
		// be resolved, SpawnVisualTravelProjectile falls back to the temporary DOT profile so
		// bring-up is never invisible. This is presentation-only: the initial contact damage,
		// the single HeroPrimaryDot payload, and the marker cadence above/below are unchanged.
		FT66CombatVFXBindingData DotBinding;
		UNiagaraSystem* DotCarrierSystem = nullptr;
		const bool bResolvedDotCarrier = ResolveCombatVFXBinding(
			ET66CombatVFXBindingSourceType::WeaponBase,
			ResolveWeaponImpactSourceID(),
			AttackCategory,
			DotBinding,
			DotCarrierSystem);
		const float DotCarrierVisualScale = bResolvedDotCarrier
			? FMath::Max(0.01f, DotBinding.VisualScaleMultiplier)
			: 1.f;

		AT66HeroProjectile* DotShot = SpawnVisualTravelProjectile(
			DotShotStart,
			DotShotEnd,
			DotVisualColor,
			FT66TemporaryProjectileSystem::ProfileHeroDOT(),
			DotShotScale,
			DotProofTravelSeconds,
			bResolvedDotCarrier ? DotCarrierSystem : nullptr,
			DotCarrierVisualScale);
		if (DotShot)
		{
			UE_LOG(
				LogT66Combat,
				Display,
				TEXT("T66DotShotSpawned Target=%s Start=%s End=%s ProofTravelSeconds=%.3f Carrier=%s CarrierVisualScale=%.3f"),
				*GetNameSafe(PrimaryTarget),
				*DotShotStart.ToCompactString(),
				*DotShotEnd.ToCompactString(),
				DotProofTravelSeconds,
				bResolvedDotCarrier ? *DotCarrierSystem->GetPathName() : TEXT("TemporaryProfileFallback"),
				DotCarrierVisualScale);
			DotShot->SetVisualArrivalCallback(MoveTemp(ApplyDotPayload));
		}
		else
		{
			// No visible shot spawned: apply the single DOT payload + markers immediately so
			// gameplay never loses DOT ticking.
			ApplyDotPayload();
		}

		// Frostbite: DOT attacks slow enemy move speed by 30%.
		if (CachedRunState->HasFrostbite())
		{
			if (AT66EnemyBase* DotEnemy = Cast<AT66EnemyBase>(PrimaryTarget))
				DotEnemy->ApplyMoveSlow(0.7f, Duration);
			else if (AT66MobBase* DotMob = Cast<AT66MobBase>(PrimaryTarget))
				DotMob->ApplyMoveSlow(0.7f, Duration);
		}
		PlayHeroAttackSfx(CurrentHeroID, AttackCategory, GetTargetAimPoint(PrimaryHandle));
		return true;
	};

	auto ShowTargetStatus = [&](AActor* Target, const FName EventType)
	{
		if (CachedFloatingCombatText && Target && !EventType.IsNone())
		{
			CachedFloatingCombatText->ShowStatusEvent(Target, EventType);
		}
	};

	auto ApplyMoveSlowToTarget = [&](AActor* Target, const float SpeedMultiplier, const float DurationSeconds)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyMoveSlow(SpeedMultiplier, DurationSeconds);
			return true;
		}
		if (AT66MobBase* Mob = Cast<AT66MobBase>(Target))
		{
			Mob->ApplyMoveSlow(SpeedMultiplier, DurationSeconds);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyMoveSlow(SpeedMultiplier, DurationSeconds);
			return true;
		}
		return false;
	};

	auto ApplyConfusionToTarget = [&](AActor* Target, const float DurationSeconds)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyConfusion(DurationSeconds);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyConfusion(DurationSeconds);
			return true;
		}
		return false;
	};

	auto ApplyFearToTarget = [&](AActor* Target, const float DurationSeconds)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyForcedRunAway(DurationSeconds);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyForcedRunAway(DurationSeconds);
			return true;
		}
		return false;
	};

	auto ApplyArmorBreakToTarget = [&](AActor* Target, const float ReductionAmount, const float DurationSeconds)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
			return true;
		}
		return false;
	};

	auto ApplyStunToTarget = [&](AActor* Target, const float DurationSeconds)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyStun(DurationSeconds);
			return true;
		}
		if (AT66MobBase* Mob = Cast<AT66MobBase>(Target))
		{
			Mob->ApplyStun(DurationSeconds);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyStun(DurationSeconds);
			return true;
		}
		return false;
	};

	auto ApplyRootToTarget = [&](AActor* Target, const float DurationSeconds)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyRoot(DurationSeconds);
			return true;
		}
		if (AT66MobBase* Mob = Cast<AT66MobBase>(Target))
		{
			Mob->ApplyRoot(DurationSeconds);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyRoot(DurationSeconds);
			return true;
		}
		return false;
	};

	auto ApplyFreezeToTarget = [&](AActor* Target, const float DurationSeconds)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyFreeze(DurationSeconds);
			return true;
		}
		if (AT66MobBase* Mob = Cast<AT66MobBase>(Target))
		{
			Mob->ApplyFreeze(DurationSeconds);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyFreeze(DurationSeconds);
			return true;
		}
		return false;
	};

	auto ApplyPullToTarget = [&](AActor* Target, const FVector& Origin, const float Distance)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyPullTowards(Origin, Distance);
			return true;
		}
		if (AT66MobBase* Mob = Cast<AT66MobBase>(Target))
		{
			Mob->ApplyPullTowards(Origin, Distance);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyPullTowards(Origin, Distance);
			return true;
		}
		return false;
	};

	auto ApplyPushToTarget = [&](AActor* Target, const FVector& Origin, const float Distance)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			Enemy->ApplyPushAwayFrom(Origin, Distance);
			return true;
		}
		if (AT66MobBase* Mob = Cast<AT66MobBase>(Target))
		{
			Mob->ApplyPushAwayFrom(Origin, Distance);
			return true;
		}
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			Boss->ApplyPushAwayFrom(Origin, Distance);
			return true;
		}
		return false;
	};

	auto ApplyExtraDOTToTarget = [&](AActor* Target, const FName& SourceId, const float Duration, const float TickInterval, const float TotalDamage)
	{
		if (!CachedRunState || !Target || Duration <= 0.f || TickInterval <= 0.f || TotalDamage <= 0.f)
		{
			return;
		}

		const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
		CachedRunState->ApplyDOT(Target, Duration, TickInterval, TotalDamage / static_cast<float>(Ticks), SourceId);
	};

	auto TryExecuteTarget = [&](AActor* Target, const FName& SourceId, const float Chance01)
	{
		if (!Target || !RollTierChance(Chance01, RngSub))
		{
			return false;
		}

		if (T66CombatShared::TryApplyNonBossOHKO(Target, nullptr, FName(TEXT("Execute")), SourceId))
		{
			ShowTargetStatus(Target, FName(TEXT("Execute")));
			return true;
		}

		return false;
	};

	auto ApplyIdolSpecialBehavior = [&](AActor* Target, const FName& IdolID, const ET66ItemRarity IdolRarity, const int32 IdolDamage, const FVector& EffectOrigin)
	{
		if (!Target || IdolID.IsNone())
		{
			return;
		}

		if (IdolID == FName(TEXT("Idol_Curse")))
		{
			const float ConfuseChance = GetIdolTierFloat(IdolRarity, 0.f, 0.20f, 0.35f, 0.42f);
			const float ConfuseDuration = GetIdolTierFloat(IdolRarity, 0.f, 1.3f, 1.8f, 2.2f);
			if (RollTierChance(ConfuseChance, RngSub) && ApplyConfusionToTarget(Target, ConfuseDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Confusion")));
			}
			const float FearChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.16f, 0.24f);
			const float FearDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 1.0f, 1.3f);
			if (RollTierChance(FearChance, RngSub) && ApplyFearToTarget(Target, FearDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Fear")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.035f));
		}
		else if (IdolID == FName(TEXT("Idol_Lava")))
		{
			const float SlowMult = GetIdolTierFloat(IdolRarity, 1.f, 0.80f, 0.72f, 0.68f);
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 0.f, 1.1f, 1.5f, 1.8f);
			if (SlowDuration > 0.f)
			{
				ApplyMoveSlowToTarget(Target, SlowMult, SlowDuration);
			}
			const float ArmorBreakAmount = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.12f, 0.18f);
			const float ArmorBreakDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 2.2f, 3.0f);
			if (ArmorBreakAmount > 0.f && ApplyArmorBreakToTarget(Target, ArmorBreakAmount, ArmorBreakDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Armor Break")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.030f));
		}
		else if (IdolID == FName(TEXT("Idol_Poison")))
		{
			const float SlowMult = GetIdolTierFloat(IdolRarity, 1.f, 0.78f, 0.72f, 0.66f);
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 0.f, 1.8f, 2.2f, 2.6f);
			if (SlowDuration > 0.f)
			{
				ApplyMoveSlowToTarget(Target, SlowMult, SlowDuration);
			}
			const float ConfuseChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.22f, 0.32f);
			const float ConfuseDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 1.5f, 2.0f);
			if (RollTierChance(ConfuseChance, RngSub) && ApplyConfusionToTarget(Target, ConfuseDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Confusion")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.028f));
		}
		else if (IdolID == FName(TEXT("Idol_Bleed")))
		{
			const float SlowMult = GetIdolTierFloat(IdolRarity, 1.f, 0.86f, 0.78f, 0.72f);
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 0.f, 1.0f, 1.3f, 1.6f);
			if (SlowDuration > 0.f)
			{
				ApplyMoveSlowToTarget(Target, SlowMult, SlowDuration);
			}
			const float FearChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.18f, 0.26f);
			const float FearDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.8f, 1.1f);
			if (RollTierChance(FearChance, RngSub) && ApplyFearToTarget(Target, FearDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Fear")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.050f));
		}
		else if (IdolID == FName(TEXT("Idol_Electric")))
		{
			const float StunChance = GetIdolTierFloat(IdolRarity, 0.f, 0.18f, 0.28f, 0.36f);
			const float StunDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.20f, 0.42f, 0.65f);
			if (RollTierChance(StunChance, RngSub) && ApplyStunToTarget(Target, StunDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Stun")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.040f));
		}
		else if (IdolID == FName(TEXT("Idol_Ice")))
		{
			const float SlowMult = GetIdolTierFloat(IdolRarity, 0.84f, 0.72f, 0.66f, 0.58f);
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 1.0f, 1.4f, 1.8f, 2.2f);
			ApplyMoveSlowToTarget(Target, SlowMult, SlowDuration);
			const float FreezeChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.16f, 0.24f);
			const float FreezeDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.55f, 0.80f);
			if (RollTierChance(FreezeChance, RngSub) && ApplyFreezeToTarget(Target, FreezeDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Freeze")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.030f));
		}
		else if (IdolID == FName(TEXT("Idol_Shadow")))
		{
			const float ConfuseChance = GetIdolTierFloat(IdolRarity, 0.14f, 0.25f, 0.30f, 0.36f);
			const float ConfuseDuration = GetIdolTierFloat(IdolRarity, 1.0f, 1.4f, 1.6f, 1.8f);
			if (RollTierChance(ConfuseChance, RngSub) && ApplyConfusionToTarget(Target, ConfuseDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Confusion")));
			}
			const float FearChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.18f, 0.25f);
			const float FearDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 1.0f, 1.25f);
			if (RollTierChance(FearChance, RngSub) && ApplyFearToTarget(Target, FearDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Fear")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.035f));
		}
		else if (IdolID == FName(TEXT("Idol_Star")))
		{
			const float ConfuseChance = GetIdolTierFloat(IdolRarity, 0.f, 0.12f, 0.18f, 0.22f);
			const float ConfuseDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.8f, 1.0f, 1.2f);
			if (RollTierChance(ConfuseChance, RngSub) && ApplyConfusionToTarget(Target, ConfuseDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Confusion")));
			}
			const float StunChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.16f, 0.22f);
			const float StunDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.50f, 0.65f);
			if (RollTierChance(StunChance, RngSub) && ApplyStunToTarget(Target, StunDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Stun")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.030f));
		}
		else if (IdolID == FName(TEXT("Idol_Earth")))
		{
			const float RootChance = GetIdolTierFloat(IdolRarity, 0.25f, 0.35f, 0.40f, 0.46f);
			const float RootDuration = GetIdolTierFloat(IdolRarity, 0.45f, 0.75f, 0.95f, 1.10f);
			if (RollTierChance(RootChance, RngSub) && ApplyRootToTarget(Target, RootDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Root")));
			}
			const float StunChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.18f, 0.24f);
			const float StunDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.45f, 0.60f);
			if (RollTierChance(StunChance, RngSub) && ApplyStunToTarget(Target, StunDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Stun")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.030f));
		}
		else if (IdolID == FName(TEXT("Idol_Water")))
		{
			const float SlowMult = GetIdolTierFloat(IdolRarity, 0.82f, 0.70f, 0.64f, 0.58f);
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 1.2f, 1.7f, 2.1f, 2.5f);
			ApplyMoveSlowToTarget(Target, SlowMult, SlowDuration);
			const float ConfuseChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.18f, 0.24f);
			const float ConfuseDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 1.1f, 1.4f);
			if (RollTierChance(ConfuseChance, RngSub) && ApplyConfusionToTarget(Target, ConfuseDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Confusion")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.025f));
		}
		else if (IdolID == FName(TEXT("Idol_BlackHole")))
		{
			const float PullDistance = GetIdolTierFloat(IdolRarity, 120.f, 185.f, 240.f, 280.f);
			if (ApplyPullToTarget(Target, EffectOrigin, PullDistance))
			{
				ShowTargetStatus(Target, FName(TEXT("Pull")));
			}
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 0.f, 1.2f, 1.5f, 1.8f);
			if (SlowDuration > 0.f)
			{
				ApplyMoveSlowToTarget(Target, GetIdolTierFloat(IdolRarity, 1.f, 0.78f, 0.72f, 0.65f), SlowDuration);
			}
			const float RootDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.90f, 1.15f);
			if (RootDuration > 0.f && ApplyRootToTarget(Target, RootDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Root")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.020f));
		}
		else if (IdolID == FName(TEXT("Idol_Storm")))
		{
			const float SlowMult = GetIdolTierFloat(IdolRarity, 0.86f, 0.72f, 0.68f, 0.60f);
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 0.8f, 1.4f, 1.8f, 2.1f);
			ApplyMoveSlowToTarget(Target, SlowMult, SlowDuration);
			const float StunChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.15f, 0.22f);
			const float StunDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.45f, 0.65f);
			if (RollTierChance(StunChance, RngSub) && ApplyStunToTarget(Target, StunDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Stun")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.030f));
		}
		else if (IdolID == FName(TEXT("Idol_Light")))
		{
			const float ConfuseChance = GetIdolTierFloat(IdolRarity, 0.f, 0.15f, 0.20f, 0.25f);
			const float ConfuseDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.8f, 1.0f, 1.2f);
			if (RollTierChance(ConfuseChance, RngSub) && ApplyConfusionToTarget(Target, ConfuseDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Confusion")));
			}
			const float StunChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.16f, 0.22f);
			const float StunDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.40f, 0.55f);
			if (RollTierChance(StunChance, RngSub) && ApplyStunToTarget(Target, StunDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Stun")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.040f));
		}
		else if (IdolID == FName(TEXT("Idol_Steel")))
		{
			const float BleedDuration = GetIdolTierFloat(IdolRarity, 2.2f, 2.6f, 3.0f, 3.4f);
			const float BleedDamage = static_cast<float>(IdolDamage) * GetIdolTierFloat(IdolRarity, 0.35f, 0.45f, 0.55f, 0.65f);
			ApplyExtraDOTToTarget(Target, IdolID, BleedDuration, 0.35f, BleedDamage);
			ShowTargetStatus(Target, FName(TEXT("Bleed")));
			const float ArmorBreakAmount = GetIdolTierFloat(IdolRarity, 0.f, 0.12f, 0.20f, 0.24f);
			const float ArmorBreakDuration = GetIdolTierFloat(IdolRarity, 0.f, 2.0f, 3.0f, 3.4f);
			if (ArmorBreakAmount > 0.f && ApplyArmorBreakToTarget(Target, ArmorBreakAmount, ArmorBreakDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Armor Break")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.035f));
		}
		else if (IdolID == FName(TEXT("Idol_Wood")))
		{
			const float RootChance = GetIdolTierFloat(IdolRarity, 0.22f, 0.32f, 0.36f, 0.40f);
			const float RootDuration = GetIdolTierFloat(IdolRarity, 0.45f, 0.75f, 0.95f, 1.15f);
			if (RollTierChance(RootChance, RngSub) && ApplyRootToTarget(Target, RootDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Root")));
			}
			const float SlowDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 1.5f, 1.9f);
			if (SlowDuration > 0.f)
			{
				ApplyMoveSlowToTarget(Target, GetIdolTierFloat(IdolRarity, 1.f, 1.f, 0.78f, 0.70f), SlowDuration);
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.030f));
		}
		else if (IdolID == FName(TEXT("Idol_Bone")))
		{
			const float BleedDuration = GetIdolTierFloat(IdolRarity, 1.8f, 2.4f, 2.8f, 3.2f);
			const float BleedDamage = static_cast<float>(IdolDamage) * GetIdolTierFloat(IdolRarity, 0.30f, 0.45f, 0.52f, 0.60f);
			ApplyExtraDOTToTarget(Target, IdolID, BleedDuration, 0.4f, BleedDamage);
			ShowTargetStatus(Target, FName(TEXT("Bleed")));
			const float FearChance = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.18f, 0.24f);
			const float FearDuration = GetIdolTierFloat(IdolRarity, 0.f, 0.f, 1.0f, 1.3f);
			if (RollTierChance(FearChance, RngSub) && ApplyFearToTarget(Target, FearDuration))
			{
				ShowTargetStatus(Target, FName(TEXT("Fear")));
			}
			(void)TryExecuteTarget(Target, IdolID, GetIdolTierFloat(IdolRarity, 0.f, 0.f, 0.f, 0.030f));
		}
	};

	// ---------------------------------------------------------------------------
	// Resolve primary target: locked > closest in EnemiesInRange.
	// No TActorIterator — just walk the small overlap list.
	// ---------------------------------------------------------------------------
	AActor* PrimaryTarget = nullptr;
	if (AActor* Locked = LockedTarget.Actor.Get())
	{
		if (IsValidTargetHandle(LockedTarget))
		{
			const float DistSq = FVector::DistSquared(MyLoc, Locked->GetActorLocation());
			if (DistSq <= RangeSq && HasUnblockedAutoAttackPath(AttackOrigin, LockedTarget))
			{
				PrimaryTarget = Locked;
			}
		}
		else
		{
			ClearLockedTarget();
		}
	}
	if (!PrimaryTarget)
	{
		PrimaryTarget = FindClosestEnemyInRange(AttackOrigin, RangeSq);
	}

	PrimaryTargetHandle = PrimaryTarget
		? ResolveAutoAttackTargetHandle(PrimaryTarget, LockedTarget.Actor.Get() == PrimaryTarget, RngSub)
		: FT66CombatTargetHandle{};

	if (PrimaryTarget && !HasUnblockedAutoAttackPath(AttackOrigin, PrimaryTargetHandle))
	{
		PrimaryTarget = nullptr;
		PrimaryTargetHandle.Reset();
	}

	if (!PrimaryTarget)
	{
		return;
	}

	LastFireTime = static_cast<float>(World->GetTimeSeconds());

	int32 VisualPayloadCount = 1;
	const bool bSuppressWeaponBaseProjectileVisual = ShouldSuppressWeaponBaseProjectileVisual(AttackCategory);
	if (bSuppressWeaponBaseProjectileVisual)
	{
		VisualPayloadCount = 0;
	}
	for (const FCachedIdolSlot& CachedIdolSlot : CachedIdolSlots)
	{
		if (CachedIdolSlot.bValid && !CachedIdolSlot.IdolID.IsNone())
		{
			if (UsesImpactPresentationForIdol(CachedIdolSlot))
			{
				if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
				{
					UE_LOG(
						LogT66Combat,
						Display,
						TEXT("CombatVFXIdolProjectileLaneSuppressed SourceID=%s Reason=ImpactPresentationOwnsIdolPlaceholder"),
						*CachedIdolSlot.IdolID.ToString());
				}
				continue;
			}
			++VisualPayloadCount;
		}
	}

	int32 VisualPayloadIndex = 0;
	if (!bSuppressWeaponBaseProjectileVisual)
	{
		SpawnWeaponProjectileVisual(PrimaryTarget, NAME_None, VisualPayloadIndex, VisualPayloadCount);
		++VisualPayloadIndex;
	}
	for (const FCachedIdolSlot& CachedIdolSlot : CachedIdolSlots)
	{
		if (!CachedIdolSlot.bValid || CachedIdolSlot.IdolID.IsNone())
		{
			continue;
		}
		if (UsesImpactPresentationForIdol(CachedIdolSlot))
		{
			continue;
		}

		SpawnWeaponProjectileVisual(PrimaryTarget, CachedIdolSlot.IdolID, VisualPayloadIndex, VisualPayloadCount);
		++VisualPayloadIndex;
	}

	// Marksman's Focus: consecutive hits on same target stack +8% damage (max 5).
	float PrimaryDamageMultiplier = 1.f;
	if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::MarksmanFocus && PrimaryTarget)
	{
		if (PrimaryTarget == LastMarksmanTarget.Get())
			MarksmanStacks = FMath::Min(5, MarksmanStacks + 1);
		else
			MarksmanStacks = 1;
		LastMarksmanTarget = PrimaryTarget;
		PrimaryDamageMultiplier = 1.f + 0.08f * static_cast<float>(MarksmanStacks);
	}

	// QuickDraw: first attack after 2s idle = 2× damage.
	if (CachedRunState)
		PrimaryDamageMultiplier *= CachedRunState->GetQuickDrawDamageMultiplier();

	// Deadeye ultimate buff: 2× damage while active.
	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (DeadeyeEndTime > Now)
		PrimaryDamageMultiplier *= 2.f;

	// Notify RunState that an attack was fired (for Overclock counter, QuickDraw timer).
	if (CachedRunState)
	{
		CachedRunState->NotifyAttackFired();
	}

	// Hero primary attack (Pierce / Bounce / AOE / DOT). Readable projectile presentation is spawned before damage resolution.
	if (PrimaryTarget)
	{
		switch (AttackCategory)
		{
		case ET66AttackCategory::Pierce: (void)PerformPierce(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::Bounce: (void)PerformBounce(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::AOE:   (void)PerformSlash(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::DOT:   (void)PerformDOT(PrimaryTarget, PrimaryDamageMultiplier); break;
		default: (void)PerformSlash(PrimaryTarget, PrimaryDamageMultiplier); break;
		}

		// Overclock: every 8th attack fires a second time immediately.
		if (CachedRunState && CachedRunState->ShouldOverclockDouble())
		{
			switch (AttackCategory)
			{
			case ET66AttackCategory::Pierce: (void)PerformPierce(PrimaryTarget, 1.f); break;
			case ET66AttackCategory::Bounce: (void)PerformBounce(PrimaryTarget, 1.f); break;
			case ET66AttackCategory::AOE:   (void)PerformSlash(PrimaryTarget, 1.f); break;
			case ET66AttackCategory::DOT:   (void)PerformDOT(PrimaryTarget, 1.f); break;
			default: break;
			}
		}

		// Evasive: if flagged, apply bonus 3s DOT to the primary target (50% of hit damage).
		if (CachedRunState && CachedRunState->ConsumeEvasiveBonusDOT() && PrimaryTarget)
		{
			static const FName EvasiveDotSource(TEXT("Passive_EvasiveDOT"));
			const float BonusDotDamage = static_cast<float>(EffectiveDamagePerShot) * 0.5f;
			const float EvasiveTicks = 6.f;
			CachedRunState->ApplyDOT(PrimaryTarget, 3.f, 0.5f, BonusDotDamage / EvasiveTicks, EvasiveDotSource);
			SpawnDOTVFX(PrimaryTarget->GetActorLocation(), 3.f, 60.f, FT66TemporaryProjectileSystem::HeroProjectileColor());
		}

		// RabidFrenzy ultimate buff: every hit applies a short DOT.
		if (RabidFrenzyEndTime > Now && CachedRunState && PrimaryTarget)
		{
			static const FName RabidFrenzyDotSource(TEXT("Ultimate_RabidFrenzyDOT"));
			const float FrenzyDotDmg = static_cast<float>(EffectiveDamagePerShot) * 0.3f;
			const float FrenzyTicks = 4.f;
			CachedRunState->ApplyDOT(PrimaryTarget, 2.f, 0.5f, FrenzyDotDmg / FrenzyTicks, RabidFrenzyDotSource);
			SpawnDOTVFX(PrimaryTarget->GetActorLocation(), 2.f, 50.f, FT66TemporaryProjectileSystem::HeroProjectileColor());
		}

		// Idol payloads: each equipped idol adds a second visual projectile lane and applies its existing data-authored effect to the weapon hits.
		if (CachedRunState)
		{
			if (WeaponHitActors.Num() == 0 && PrimaryTarget)
			{
				WeaponHitActors.AddUnique(PrimaryTarget);
			}

			UE_LOG(
				LogT66Combat,
				Verbose,
				TEXT("[IDOL MOD] owner=%s weaponHits=%d cachedIdolSlots=%d"),
				GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
				WeaponHitActors.Num(),
				CachedIdolSlots.Num());

			int32 ImpactPresentationIdolSlots = 0;
			for (const FCachedIdolSlot& CachedIdolSlot : CachedIdolSlots)
			{
				if (UsesImpactPresentationForIdol(CachedIdolSlot))
				{
					++ImpactPresentationIdolSlots;
				}
			}

			int32 EligibleWeaponImpactContexts = 0;
			for (const FT66CombatImpactContext& WeaponImpactContext : WeaponImpactContexts)
			{
				if (WeaponImpactContext.bImpactPointValid)
				{
					++EligibleWeaponImpactContexts;
				}
			}

			// The water idol overlay is built once per impact-presentation slot from the
			// primary weapon impact context, independent of how many per-link weapon
			// contexts an attack publishes (e.g. PerChainLink Bounce publishes one per
			// link). Predict parity from the primary context so the diagnostic stays
			// honest for both single-context and per-link attacks.
			const bool bHasEligiblePrimaryWeaponContext =
				bHasPrimaryWeaponImpactContext && PrimaryWeaponImpactContext.bImpactPointValid;
			// These counters are category-general (they aggregate every impact-presentation
			// proof idol, not only Water). The aggregate "CombatIdolImpactDiagnostic
			// SourceID=Idol_Water Water*..." log line emitted below intentionally keeps its
			// Water-named field vocabulary as a preserved compatibility diagnostic that the
			// Water regression proof runners parse; only these internal identifiers are
			// de-Watered.
			const int32 ExpectedIdolImpactContexts =
				ImpactPresentationIdolSlots * (bHasEligiblePrimaryWeaponContext ? 1 : 0);
			int32 IdolImpactContextCount = 0;
			int32 IdolSkippedNoWeaponContext = 0;
			int32 IdolSkippedInvalidImpactPoint = 0;
			int32 IdolLegacyFallbackCount = 0;

			// Generalized per-idol chain diagnostic (CombatVFXImpactContextContract.md schema).
			// Emitted once per impact-presentation idol regardless of category so the proof can
			// gate on a single reusable line keyed by SourceID instead of Water-only field names.
			auto EmitIdolChainDiagnostic = [&](
				const FName DiagIdolID,
				const ET66AttackCategory DiagCategory,
				const FName DiagParentSourceID,
				const int32 DownstreamContexts,
				const int32 IdolSkippedNoWeaponContext,
				const int32 IdolSkippedInvalidImpactPoint,
				const int32 IdolLegacyFallbacks,
				const bool bDamageOwnedAndApplied)
			{
				if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() == 0)
				{
					return;
				}
				const int32 ExpectedDownstream = bHasEligiblePrimaryWeaponContext ? 1 : 0;
				const bool bParity = (ExpectedDownstream == DownstreamContexts)
					&& IdolSkippedNoWeaponContext == 0
					&& IdolSkippedInvalidImpactPoint == 0
					&& IdolLegacyFallbacks == 0
					&& bDamageOwnedAndApplied;
				UE_LOG(
					LogT66Combat,
					Display,
					TEXT("CombatImpactChainDiagnostic SourceID=%s Category=%s ParentSourceID=%s ContextParity=%s WeaponImpactContexts=%d EligibleWeaponImpactContexts=%d ExpectedDownstreamImpactContexts=%d DownstreamImpactContexts=%d SkippedNoWeaponContext=%d SkippedInvalidImpactPoint=%d SkippedDisabledOrNeutral=0 LegacyFallbacks=%d DamageByDownstreamSource=%s"),
					*DiagIdolID.ToString(),
					*UEnum::GetValueAsString(DiagCategory),
					*DiagParentSourceID.ToString(),
					bParity ? TEXT("PASS") : TEXT("FAIL"),
					WeaponImpactContexts.Num(),
					EligibleWeaponImpactContexts,
					ExpectedDownstream,
					DownstreamContexts,
					IdolSkippedNoWeaponContext,
					IdolSkippedInvalidImpactPoint,
					IdolLegacyFallbacks,
					bDamageOwnedAndApplied ? TEXT("PASS") : TEXT("FAIL"));
			};

			for (const FCachedIdolSlot& CachedIdolSlot : CachedIdolSlots)
			{
				if (!CachedIdolSlot.bValid || CachedIdolSlot.IdolID.IsNone())
				{
					continue;
				}

				const FName IdolID = CachedIdolSlot.IdolID;
				const ET66ItemRarity IdolRarity = CachedIdolSlot.Rarity;
				const FIdolData& IdolData = CachedIdolSlot.IdolData;
				const float IdolGlobalScale = FMath::Max(0.1f, ProjectileScaleMultiplier);
				const float IdolCategorySubScale = T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, IdolData.Category);
				const float IdolBehaviorScale = IdolGlobalScale * IdolCategorySubScale;
				const int32 IdolDamage = FMath::Max(1, FMath::RoundToInt(IdolData.GetDamageAtRarity(IdolRarity)));
				const bool bUsesImpactPresentation = UsesImpactPresentationForIdol(CachedIdolSlot);

				if (bUsesImpactPresentation)
				{
					if (!bHasPrimaryWeaponImpactContext)
					{
						++IdolSkippedNoWeaponContext;
						if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
						{
							UE_LOG(
								LogT66Combat,
								Display,
								TEXT("CombatIdolImpactDiagnosticSkip SourceID=%s Reason=NoPrimaryWeaponImpactContext WeaponImpactContexts=%d WeaponHitActors=%d"),
								*IdolID.ToString(),
								WeaponImpactContexts.Num(),
								WeaponHitActors.Num());
						}
						EmitIdolChainDiagnostic(IdolID, IdolData.Category, NAME_None, 0, 1, 0, 1, false);
					}
					else if (!PrimaryWeaponImpactContext.bImpactPointValid)
					{
						++IdolSkippedInvalidImpactPoint;
						if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
						{
							UE_LOG(
								LogT66Combat,
								Display,
								TEXT("CombatIdolImpactDiagnosticSkip SourceID=%s Reason=PrimaryImpactPointInvalid WeaponImpactContexts=%d WeaponHitActors=%d"),
								*IdolID.ToString(),
								WeaponImpactContexts.Num(),
								WeaponHitActors.Num());
						}
						EmitIdolChainDiagnostic(IdolID, IdolData.Category, PrimaryWeaponImpactContext.SourceID, 0, 0, 1, 1, false);
					}
					else
					{
						// Common idol impact context, built from the official weapon impact point so
						// ParentSourceID stays the upstream weapon (e.g. Hero_1_black_aoe). Category-
						// specific shape fields (Radius / LineLength / ChainIndex) are filled per case.
						FT66CombatImpactContext IdolImpactContext;
						IdolImpactContext.SourceType = ET66CombatVFXBindingSourceType::IdolModifier;
						IdolImpactContext.SourceID = IdolID;
						IdolImpactContext.ParentSourceID = PrimaryWeaponImpactContext.SourceID;
						IdolImpactContext.HeroID = CurrentHeroID;
						IdolImpactContext.AttackCategory = IdolData.Category;
						IdolImpactContext.AttackOrigin = AttackOrigin;
						IdolImpactContext.ImpactPoint = PrimaryWeaponImpactContext.ImpactPoint;
						IdolImpactContext.DamageCenter = IdolImpactContext.ImpactPoint;
						IdolImpactContext.Forward = PrimaryWeaponImpactContext.Forward;
						IdolImpactContext.EffectiveDamage = IdolDamage;
						IdolImpactContext.bDamageCenterValid = true;
						IdolImpactContext.bImpactPointValid = true;
						IdolImpactContext.PrimaryTargetHandle = PrimaryWeaponImpactContext.PrimaryTargetHandle;
						AActor* QueryPrimaryTarget = PrimaryWeaponImpactContext.PrimaryTargetHandle.Actor.Get();

						bool bIdolDamageApplied = false;

						switch (IdolData.Category)
						{
						case ET66AttackCategory::Pierce:
						{
							// Category-native Pierce: a line read off the official impact point. The
							// idol owns line/pierce damage with per-hit falloff under its own SourceID.
							const float LineLength = FMath::Max(1.f, AttackRange);
							const float PierceRadius = FMath::Max(1.f, 80.f * IdolGlobalScale);
							IdolImpactContext.Radius = 0.f;
							IdolImpactContext.LineLength = LineLength;
							IdolImpactContext.TubeRadius = PierceRadius;
							FVector PierceDir = IdolImpactContext.Forward;
							TArray<AActor*> InLine;
							BuildPierceTargets(QueryPrimaryTarget, LineLength, PierceRadius, InLine, PierceDir, &IdolImpactContext.ImpactPoint);
							IdolImpactContext.Forward = T66ResolvePlanarDirection(PierceDir, OwnerActor);
							const int32 MaxPierceTargets = FMath::Max(1, FMath::RoundToInt(IdolData.GetPropertyAtRarity(IdolRarity)) + 1);
							if (InLine.Num() > MaxPierceTargets)
							{
								InLine.SetNum(MaxPierceTargets, EAllowShrinking::No);
							}
							const float FalloffPerHit = FMath::Clamp((IdolData.FalloffPerHit > 0.f) ? IdolData.FalloffPerHit : 0.15f, 0.f, 0.95f);
							for (int32 PierceIndex = 0; PierceIndex < InLine.Num(); ++PierceIndex)
							{
								AActor* Hit = InLine[PierceIndex];
								if (!IsValidAutoTarget(Hit))
								{
									continue;
								}
								const FT66CombatTargetHandle HitHandle = (Hit == QueryPrimaryTarget && PrimaryWeaponImpactContext.PrimaryTargetHandle.IsValid())
									? PrimaryWeaponImpactContext.PrimaryTargetHandle
									: ResolveAutoAttackTargetHandle(Hit, false, RngSub);
								AddImpactTargetHandleUnique(IdolImpactContext, HitHandle);
								const float FalloffMult = FMath::Max(0.1f, 1.f - FalloffPerHit * static_cast<float>(PierceIndex));
								const int32 ShapedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(IdolDamage) * FalloffMult));
								FName RangeEvent;
								const int32 RangeDmg = GetRangeMultipliedDamage(ShapedDamage, Hit, &RangeEvent);
								const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
								ApplyDamageToTargetHandle(HitHandle, Resolved.Damage, Resolved.EventType, IdolID, RangeEvent);
								ApplyIdolSpecialBehavior(Hit, IdolID, IdolRarity, IdolDamage, GetTargetAimPoint(HitHandle));
								bIdolDamageApplied = true;
							}
							LogCombatImpactContext(IdolImpactContext, TEXT("IdolPrimary"));
							if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
							{
								UE_LOG(
									LogT66Combat,
									Display,
									TEXT("CombatIdolCategoryImpactResolved SourceID=%s Category=Pierce ParentSourceID=%s LineLength=%.2f TubeRadius=%.2f Targets=%d DamageCenter=%s ImpactPoint=%s"),
									*IdolID.ToString(),
									*IdolImpactContext.ParentSourceID.ToString(),
									LineLength,
									PierceRadius,
									IdolImpactContext.HitTargetHandles.Num(),
									*IdolImpactContext.DamageCenter.ToCompactString(),
									*IdolImpactContext.ImpactPoint.ToCompactString());
							}
							bool bBindingResolved = false;
							if (!TrySpawnBoundIdolImpactVFX(IdolImpactContext, IdolID, IdolRarity, PierceRadius, bBindingResolved))
							{
								SpawnIdolImpactPlaceholderVFX(IdolImpactContext, IdolID, IdolData.Category, 1.0f);
							}
							break;
						}
						case ET66AttackCategory::Bounce:
						{
							// Category-native Bounce: chain from the official impact point to the
							// nearest other enemy in range, idol-owned per link with falloff.
							IdolImpactContext.Radius = 0.f;
							const float BounceRangeSq = AttackRange * AttackRange;
							const int32 BounceCount = FMath::Max(1, FMath::RoundToInt(IdolData.GetPropertyAtRarity(IdolRarity)) + 1);
							const float Falloff = FMath::Clamp((IdolData.FalloffPerHit > 0.f) ? IdolData.FalloffPerHit : 0.2f, 0.f, 0.95f);
							TSet<FString> BounceHitKeys;
							FVector BounceLoc = IdolImpactContext.ImpactPoint;
							int32 BounceChainIndex = 0;
							if (QueryPrimaryTarget && IsValidAutoTarget(QueryPrimaryTarget) && PrimaryWeaponImpactContext.PrimaryTargetHandle.IsValid())
							{
								const FT66CombatTargetHandle PrimaryHandle = PrimaryWeaponImpactContext.PrimaryTargetHandle;
								AddImpactTargetHandleUnique(IdolImpactContext, PrimaryHandle);
								BounceHitKeys.Add(MakeTargetHandleKey(PrimaryHandle));
								FName RangeEvent;
								const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, QueryPrimaryTarget, &RangeEvent);
								const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
								ApplyDamageToTargetHandle(PrimaryHandle, Resolved.Damage, Resolved.EventType, IdolID, RangeEvent);
								ApplyIdolSpecialBehavior(QueryPrimaryTarget, IdolID, IdolRarity, IdolDamage, GetTargetAimPoint(PrimaryHandle));
								bIdolDamageApplied = true;
								BounceLoc = GetTargetAimPoint(PrimaryHandle);
							}
							int32 BouncesLeft = BounceCount - 1;
							float DamageMult = 1.f - Falloff;
							while (BouncesLeft > 0)
							{
								const FT66CombatTargetHandle NextHandle = FindClosestTargetHandleInRange(BounceLoc, BounceRangeSq, &BounceHitKeys);
								AActor* Next = NextHandle.Actor.Get();
								if (!Next || !IsValidAutoTarget(Next))
								{
									break;
								}
								BounceHitKeys.Add(MakeTargetHandleKey(NextHandle));
								AddImpactTargetHandleUnique(IdolImpactContext, NextHandle);
								const int32 BounceDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(IdolDamage) * DamageMult));
								FName RangeEvent;
								const int32 RangeDmg = GetRangeMultipliedDamage(BounceDamage, Next, &RangeEvent);
								const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
								ApplyDamageToTargetHandle(NextHandle, Resolved.Damage, Resolved.EventType, IdolID, RangeEvent);
								ApplyIdolSpecialBehavior(Next, IdolID, IdolRarity, IdolDamage, GetTargetAimPoint(NextHandle));
								bIdolDamageApplied = true;
								++BounceChainIndex;
								IdolImpactContext.ChainIndex = BounceChainIndex;
								BounceLoc = GetTargetAimPoint(NextHandle);
								DamageMult *= (1.f - Falloff);
								--BouncesLeft;
							}
							LogCombatImpactContext(IdolImpactContext, TEXT("IdolPrimary"));
							if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
							{
								UE_LOG(
									LogT66Combat,
									Display,
									TEXT("CombatIdolCategoryImpactResolved SourceID=%s Category=Bounce ParentSourceID=%s LinkCount=%d Targets=%d DamageCenter=%s ImpactPoint=%s"),
									*IdolID.ToString(),
									*IdolImpactContext.ParentSourceID.ToString(),
									IdolImpactContext.HitTargetHandles.Num(),
									IdolImpactContext.HitTargetHandles.Num(),
									*IdolImpactContext.DamageCenter.ToCompactString(),
									*IdolImpactContext.ImpactPoint.ToCompactString());
							}
							bool bBindingResolved = false;
							if (!TrySpawnBoundIdolImpactVFX(IdolImpactContext, IdolID, IdolRarity, FMath::Max(1.f, AttackRange), bBindingResolved))
							{
								SpawnIdolImpactPlaceholderVFX(IdolImpactContext, IdolID, IdolData.Category, 1.0f);
							}
							break;
						}
						case ET66AttackCategory::DOT:
						{
							// Category-native DOT: idol-owned ticking damage on the official impact
							// target. The damage source is the idol ID, so DamageBySource attributes the
							// ticks to the DOT idol rather than the weapon.
							IdolImpactContext.Radius = 0.f;
							const float Duration = FMath::Max(0.5f, (IdolData.DotDuration > 0.f) ? IdolData.DotDuration : 3.0f);
							const float TickInterval = FMath::Max(0.1f, (IdolData.DotTickInterval > 0.f) ? IdolData.DotTickInterval : 0.5f);
							const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
							const float DamagePerTick = static_cast<float>(IdolDamage) / static_cast<float>(Ticks);
							AActor* DotTarget = QueryPrimaryTarget;
							const FT66CombatTargetHandle DotHandle = PrimaryWeaponImpactContext.PrimaryTargetHandle;
							if (DotTarget && IsValidAutoTarget(DotTarget) && DotHandle.IsValid() && CachedRunState)
							{
								AddImpactTargetHandleUnique(IdolImpactContext, DotHandle);
								CachedRunState->ApplyDOT(DotTarget, Duration, TickInterval, DamagePerTick, IdolID);
								ApplyIdolSpecialBehavior(DotTarget, IdolID, IdolRarity, IdolDamage, GetTargetAimPoint(DotHandle));
								bIdolDamageApplied = true;
							}
							LogCombatImpactContext(IdolImpactContext, TEXT("IdolPrimary"));
							if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
							{
								UE_LOG(
									LogT66Combat,
									Display,
									TEXT("CombatIdolCategoryImpactResolved SourceID=%s Category=DOT ParentSourceID=%s Duration=%.2f TickInterval=%.2f DamagePerTick=%.2f Targets=%d DamageCenter=%s ImpactPoint=%s"),
									*IdolID.ToString(),
									*IdolImpactContext.ParentSourceID.ToString(),
									Duration,
									TickInterval,
									DamagePerTick,
									IdolImpactContext.HitTargetHandles.Num(),
									*IdolImpactContext.DamageCenter.ToCompactString(),
									*IdolImpactContext.ImpactPoint.ToCompactString());
							}
							bool bBindingResolved = false;
							if (!TrySpawnBoundIdolImpactVFX(IdolImpactContext, IdolID, IdolRarity, FMath::Max(40.f, Duration * 60.f), bBindingResolved))
							{
								SpawnIdolImpactPlaceholderVFX(IdolImpactContext, IdolID, IdolData.Category, Duration);
							}
							break;
						}
						case ET66AttackCategory::AOE:
						default:
						{
							// Preserved Water/AOE proof path (radius read from FIdolData.AoeRadius).
							const float IdolRadius = FMath::Max(1.f, IdolData.AoeRadius);
							IdolImpactContext.Radius = IdolRadius;

							TArray<AActor*> IdolImpactTargets;
							BuildSlashTargets(
								QueryPrimaryTarget,
								IdolRadius,
								IdolImpactTargets,
								false,
								IdolImpactContext.Forward,
								90.f,
								0.f,
								&IdolImpactContext.DamageCenter,
								FString::Printf(TEXT("Idol %s AOE Damage"), *IdolID.ToString()));

							for (int32 TargetIndex = 0; TargetIndex < IdolImpactTargets.Num(); ++TargetIndex)
							{
								AActor* Hit = IdolImpactTargets[TargetIndex];
								if (!IsValidAutoTarget(Hit))
								{
									continue;
								}

								const FT66CombatTargetHandle HitHandle = (Hit == QueryPrimaryTarget && PrimaryWeaponImpactContext.PrimaryTargetHandle.IsValid())
									? PrimaryWeaponImpactContext.PrimaryTargetHandle
									: ResolveAutoAttackTargetHandle(Hit, false, RngSub);
								AddImpactTargetHandleUnique(IdolImpactContext, HitHandle);
							}

							LogCombatImpactContext(IdolImpactContext, TEXT("IdolPrimary"));
							if (CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
							{
								UE_LOG(
									LogT66Combat,
									Display,
									TEXT("CombatIdolWaterImpactResolved SourceID=%s ParentSourceID=%s Radius=%.2f RadiusSource=FIdolData.AoeRadius AoeDelay=%.3f DelayApplied=false Reason=LegacyImmediatePreserved Targets=%d DamageCenter=%s ImpactPoint=%s"),
									*IdolID.ToString(),
									*IdolImpactContext.ParentSourceID.ToString(),
									IdolRadius,
									IdolData.AoeDelay,
									IdolImpactContext.HitTargetHandles.Num(),
									*IdolImpactContext.DamageCenter.ToCompactString(),
									*IdolImpactContext.ImpactPoint.ToCompactString());
							}

							bool bBindingResolved = false;
							if (!TrySpawnBoundIdolImpactVFX(IdolImpactContext, IdolID, IdolRarity, IdolRadius, bBindingResolved))
							{
								SpawnWaterIdolImpactPlaceholderVFX(IdolImpactContext, IdolRadius);
							}

							for (const FT66CombatTargetHandle& HitHandle : IdolImpactContext.HitTargetHandles)
							{
								AActor* Hit = HitHandle.Actor.Get();
								if (!IsValidAutoTarget(Hit))
								{
									continue;
								}

								FName RangeEvent;
								const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, Hit, &RangeEvent);
								const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
								ApplyDamageToTargetHandle(HitHandle, Resolved.Damage, Resolved.EventType, IdolID, RangeEvent);
								if (Resolved.bCrit && CachedRunState && RollTierChance(CachedRunState->GetExecuteChance01(), RngSub))
								{
									T66CombatShared::TryApplyNonBossOHKO(HitHandle.Actor.Get(), &HitHandle, FName(TEXT("Execute")), NAME_None);
								}
								ApplyIdolSpecialBehavior(Hit, IdolID, IdolRarity, IdolDamage, IdolImpactContext.ImpactPoint);
								bIdolDamageApplied = true;
							}
							break;
						}
						}

						++IdolImpactContextCount;
						EmitIdolChainDiagnostic(IdolID, IdolData.Category, IdolImpactContext.ParentSourceID, 1, 0, 0, 0, bIdolDamageApplied);
						continue;
					}

					++IdolLegacyFallbackCount;
				}

				for (AActor* Hit : WeaponHitActors)
				{
					if (!IsValidAutoTarget(Hit))
					{
						continue;
					}

					FName RangeEvent;
					const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, Hit, &RangeEvent);
					const FResolvedAutoAttackHit Resolved = ResolveCrit(RangeDmg);
					ApplyDamageToActor(Hit, Resolved.Damage, Resolved.EventType, IdolID, RangeEvent);
					if (Resolved.bCrit && CachedRunState && RollTierChance(CachedRunState->GetExecuteChance01(), RngSub))
					{
						const FT66CombatTargetHandle ExecuteHandle = MakeActorTargetHandle(Hit);
						T66CombatShared::TryApplyNonBossOHKO(Hit, &ExecuteHandle, FName(TEXT("Execute")), NAME_None);
					}
					ApplyIdolSpecialBehavior(Hit, IdolID, IdolRarity, IdolDamage, Hit->GetActorLocation());

					if (IdolData.Category == ET66AttackCategory::DOT)
					{
						const float Duration = FMath::Max(0.5f, IdolData.GetPropertyAtRarity(IdolRarity) * IdolBehaviorScale);
						const float TickInterval = FMath::Max(0.1f, IdolData.DotTickInterval);
						const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
						const float DamagePerTick = static_cast<float>(IdolDamage) / static_cast<float>(Ticks);
						CachedRunState->ApplyDOT(Hit, Duration, TickInterval, DamagePerTick, IdolID);
					}
				}
			}

			const bool bTouchedIdolImpactDiagnostics = ImpactPresentationIdolSlots > 0
				|| IdolImpactContextCount > 0
				|| IdolSkippedNoWeaponContext > 0
				|| IdolSkippedInvalidImpactPoint > 0
				|| IdolLegacyFallbackCount > 0;
			if (bTouchedIdolImpactDiagnostics && CVarT66CombatImpactSourceVerbose.GetValueOnGameThread() != 0)
			{
				const bool bIdolContextParity = ExpectedIdolImpactContexts == IdolImpactContextCount;
				// NOTE: the log field vocabulary below (WaterIdolContextParity, Expected/
				// WaterIdolImpactContexts, WaterSkipped*, WaterLegacyFallbacks) is a preserved
				// Water-compatibility diagnostic parsed by the idol impact proof runners. Keep
				// the emitted text byte-identical even though the backing locals are de-Watered.
				UE_LOG(
					LogT66Combat,
					Display,
					TEXT("CombatIdolImpactDiagnostic SourceID=Idol_Water WaterIdolContextParity=%s WeaponImpactContexts=%d EligibleWeaponImpactContexts=%d ImpactPresentationIdolSlots=%d ExpectedWaterIdolImpactContexts=%d WaterIdolImpactContexts=%d WaterSkippedNoWeaponContext=%d WaterSkippedInvalidImpactPoint=%d WaterLegacyFallbacks=%d WeaponHitActors=%d PrimaryWeaponImpactContextValid=%d PrimaryImpactPointValid=%d"),
					bIdolContextParity ? TEXT("PASS") : TEXT("FAIL"),
					WeaponImpactContexts.Num(),
					EligibleWeaponImpactContexts,
					ImpactPresentationIdolSlots,
					ExpectedIdolImpactContexts,
					IdolImpactContextCount,
					IdolSkippedNoWeaponContext,
					IdolSkippedInvalidImpactPoint,
					IdolLegacyFallbackCount,
					WeaponHitActors.Num(),
					bHasPrimaryWeaponImpactContext ? 1 : 0,
					(bHasPrimaryWeaponImpactContext && PrimaryWeaponImpactContext.bImpactPointValid) ? 1 : 0);
			}
		}

	}
}

#if !UE_BUILD_SHIPPING
void UT66CombatComponent::PerformAutomationAutoAttackNow()
{
	TryFire();
}
#endif

// ---------------------------------------------------------------------------
// ApplyDamageToActor — dispatches to the correct TakeDamage method per type.
// ---------------------------------------------------------------------------
void UT66CombatComponent::ApplyDamageToTargetHandle(const FT66CombatTargetHandle& TargetHandle, int32 DamageAmount, FName EventType, FName SourceID, FName RangeEventForHero)
{
	AActor* Target = TargetHandle.Actor.Get();
	if (!Target) return;
	// Toxin Stacking (Rogue): enemies with active DOT take +15% damage from all sources.
	if (CachedRunState)
	{
		const float ToxinMult = CachedRunState->GetToxinStackingDamageMultiplier(Target);
		if (ToxinMult > 1.f)
			DamageAmount = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageAmount) * ToxinMult));
	}
	const FName ResolvedSource = SourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : SourceID;
	UT66FloatingCombatTextSubsystem* FloatingText = CachedFloatingCombatText;
	AActor* Hero = GetOwner();

	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
	{
		if (E->CurrentHP > 0)
		{
			const FT66CombatTargetHandle ResolvedHandle = TargetHandle.IsValid() ? TargetHandle : MakeActorTargetHandle(Target);
			const bool bEnemyDied = E->TakeDamageFromHeroHitZone(DamageAmount, ResolvedHandle, ResolvedSource, EventType);
			if (bEnemyDied && LockedTarget.Actor.Get() == E)
			{
				ClearLockedTarget();
			}
			if (!bEnemyDied && ResolvedSource == UT66DamageLogSubsystem::SourceID_AutoAttack && Hero)
			{
				E->ApplyAutoAttackKnockback(Hero->GetActorLocation());
			}
		}
	}
	else if (AT66MobBase* M = Cast<AT66MobBase>(Target))
	{
		if (M->IsAliveAndActive())
		{
			const FT66CombatTargetHandle ResolvedHandle = TargetHandle.IsValid() ? TargetHandle : MakeActorTargetHandle(Target);
			const bool bMobDied = M->TakeDamageFromHeroHitZone(DamageAmount, ResolvedHandle, ResolvedSource, EventType);
			if (bMobDied && LockedTarget.Actor.Get() == M)
			{
				ClearLockedTarget();
			}
			if (!bMobDied && ResolvedSource == UT66DamageLogSubsystem::SourceID_AutoAttack && Hero)
			{
				M->ApplyAutoAttackKnockback(Hero->GetActorLocation());
			}
		}
	}
	else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
	{
		if (B->IsAwakened() && B->IsAlive())
		{
			const FT66CombatTargetHandle ResolvedHandle = TargetHandle.IsValid() ? TargetHandle : MakeActorTargetHandle(Target, ET66HitZoneType::Core);
			B->TakeDamageFromHeroHitZone(DamageAmount, ResolvedHandle, ResolvedSource, EventType);
		}
	}

	// Life steal: % chance per hit; when it procs, heal 10% of damage dealt.
	if (DamageAmount > 0 && CachedRunState)
	{
		UT66RngSubsystem* RngSub = GetWorld() && GetWorld()->GetGameInstance()
			? GetWorld()->GetGameInstance()->GetSubsystem<UT66RngSubsystem>()
			: nullptr;
		const float LsChance = FMath::Clamp(CachedRunState->GetLifeStealFraction(), 0.f, 1.f);
		if (RollTierChance(LsChance, RngSub))
		{
			CachedRunState->HealHP(static_cast<float>(DamageAmount) * 0.1f);
			if (FloatingText && Hero) FloatingText->ShowStatusEvent(Hero, UT66FloatingCombatTextSubsystem::EventType_LifeSteal);
		}
	}

	// Invisibility: chance to confuse (apply confusion) the enemy we hit.
	if (Target && DamageAmount > 0 && CachedRunState)
	{
		UT66RngSubsystem* RngSub = GetWorld() && GetWorld()->GetGameInstance()
			? GetWorld()->GetGameInstance()->GetSubsystem<UT66RngSubsystem>()
			: nullptr;
		const float InvisChance = CachedRunState->GetInvisibilityChance01();
		if (RollTierChance(InvisChance, RngSub))
		{
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
			{
				E->ApplyConfusion(3.f);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Confusion);
				if (FloatingText && Hero) FloatingText->ShowStatusEvent(Hero, UT66FloatingCombatTextSubsystem::EventType_Invisibility);
			}
		}
	}

	// Taunt: on hit, apply armor debuff to the target (enemy or boss) using Taunt stat.
	if (Target && DamageAmount > 0 && CachedRunState)
	{
		const float Aggro = CachedRunState->GetAggroMultiplier();
		if (Aggro > 1.f)
		{
			const float ReductionAmount = FMath::Min(0.5f, Aggro - 1.f);
			const float DurationSeconds = 3.f;
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
			{
				E->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Taunt);
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
			{
				B->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Taunt);
			}
		}
	}

	// Close/Long range bonus: show on hero when applicable.
	if (FloatingText && Hero && !RangeEventForHero.IsNone())
	{
		FloatingText->ShowStatusEvent(Hero, RangeEventForHero);
	}
}

void UT66CombatComponent::ApplyDamageToActor(AActor* Target, int32 DamageAmount, FName EventType, FName SourceID, FName RangeEventForHero)
{
	ApplyDamageToTargetHandle(MakeActorTargetHandle(Target), DamageAmount, EventType, SourceID, RangeEventForHero);
}

void UT66CombatComponent::PerformUltimateSpearStorm(int32 UltimateDamage, const FVector& Start, const FVector& End)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const float LineLength = FVector::Dist(Start, End);
	if (LineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float TubeRadius = 180.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	const FVector MidPoint = (Start + End) * 0.5f;
	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, MidPoint, (LineLength * 0.5f) + TubeRadius);

	TSet<AActor*> AlreadyHit;
	for (AActor* Target : Targets)
	{
		if (!Target || !IsValidAutoTarget(Target) || AlreadyHit.Contains(Target))
		{
			continue;
		}

		const FT66CombatTargetHandle TargetHandle = MakeActorTargetHandle(Target);
		if (!IsValidTargetHandle(TargetHandle))
		{
			continue;
		}

		const float DistSq = FMath::PointDistToSegmentSquared(GetTargetAimPoint(TargetHandle), Start, End);
		if (DistSq > (TubeRadius * TubeRadius))
		{
			continue;
		}

		AlreadyHit.Add(Target);
		ApplyDamageToTargetHandle(TargetHandle, UltimateDamage, NAME_None, SourceID);
	}

	SpawnArthurUltimateSwordVFX(Start, End);
}

void UT66CombatComponent::PerformUltimateChainLightning(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float ChainRange = 1500.f;
	const float ChainRangeSq = ChainRange * ChainRange;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FVector> ChainPositions;
	ChainPositions.Add(HeroLoc);
	FVector CurrentLoc = HeroLoc;
	TSet<FString> HitKeys;

	while (true)
	{
		const FT66CombatTargetHandle NearestHandle = FindClosestTargetHandleInRange(CurrentLoc, ChainRangeSq, &HitKeys);
		if (!NearestHandle.IsValid()) break;
		HitKeys.Add(MakeTargetHandleKey(NearestHandle));
		ApplyDamageToTargetHandle(NearestHandle, UltimateDamage, NAME_None, SourceID);
		CurrentLoc = GetTargetAimPoint(NearestHandle);
		ChainPositions.Add(CurrentLoc);
	}

	if (ChainPositions.Num() >= 2 && GetActiveVFXSystem())
		SpawnBounceVFX(ChainPositions, FT66TemporaryProjectileSystem::HeroProjectileColor());
}

void UT66CombatComponent::PerformUltimatePrecisionStrike(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const float LineLength = AttackRange;
	const float TubeRadius = 100.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	const FVector End = HeroLoc + Forward * LineLength;

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc + Forward * (LineLength * 0.5f), LineLength * 0.5f);

	for (AActor* A : Targets)
	{
		if (!A || !IsValidAutoTarget(A)) continue;
		const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), HeroLoc, End);
		if (DistSq <= TubeRadius * TubeRadius)
			ApplyDamageToActor(A, UltimateDamage * 5, NAME_None, SourceID);
	}
	SpawnPierceVFX(HeroLoc, End, FT66TemporaryProjectileSystem::HeroProjectileColor());
}

void UT66CombatComponent::PerformUltimateFanTheHammer(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const float LineLength = AttackRange;
	const float TubeRadius = 80.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	constexpr int32 NumShots = 6;
	constexpr float ConeAngleDeg = 60.f;

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc, LineLength);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumShots; ++i)
	{
		const float AngleOff = -ConeAngleDeg * 0.5f + ConeAngleDeg * (static_cast<float>(i) / static_cast<float>(NumShots - 1));
		const FVector Dir = Forward.RotateAngleAxis(AngleOff, FVector::UpVector);
		const FVector End = HeroLoc + Dir * LineLength;

		for (AActor* A : Targets)
		{
			if (!A || !IsValidAutoTarget(A)) continue;
			const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), HeroLoc, End);
			if (DistSq <= TubeRadius * TubeRadius && !AlreadyHit.Contains(A))
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		SpawnPierceVFX(HeroLoc, End, FT66TemporaryProjectileSystem::HeroProjectileColor());
	}
}

void UT66CombatComponent::PerformUltimateDeadeye(int32 UltimateDamage)
{
	UWorld* World = GetWorld();
	if (!World) return;
	DeadeyeEndTime = static_cast<float>(World->GetTimeSeconds()) + 4.f;
}

void UT66CombatComponent::PerformUltimateDischarge(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc, Range);

	TArray<FVector> Positions;
	Positions.Add(HeroLoc);
	for (AActor* A : Targets)
	{
		if (A && IsValidAutoTarget(A))
		{
			ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			Positions.Add(A->GetActorLocation());
		}
	}
	if (Positions.Num() >= 2 && GetActiveVFXSystem())
		SpawnBounceVFX(Positions, FT66TemporaryProjectileSystem::HeroProjectileColor());
}

void UT66CombatComponent::PerformUltimateJuiced()
{
	UWorld* World = GetWorld();
	if (!World) return;
	JuicedEndTime = static_cast<float>(World->GetTimeSeconds()) + 5.f;
	JuicedBonusBounce = 5;
}

void UT66CombatComponent::PerformUltimateDeathSpiral(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const float LineLength = 1000.f;
	const float TubeRadius = 80.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	constexpr int32 NumRays = 12;

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc, LineLength);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumRays; ++i)
	{
		const float AngleDeg = (360.f / NumRays) * static_cast<float>(i);
		const FVector Dir = Forward.RotateAngleAxis(AngleDeg, FVector::UpVector);
		const FVector End = HeroLoc + Dir * LineLength;

		for (AActor* A : Targets)
		{
			if (!A || !IsValidAutoTarget(A) || AlreadyHit.Contains(A)) continue;
			const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), HeroLoc, End);
			if (DistSq <= TubeRadius * TubeRadius)
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		SpawnBounceVFX({HeroLoc, End}, FT66TemporaryProjectileSystem::HeroProjectileColor());
	}
}

void UT66CombatComponent::PerformUltimateShockwave(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Radius = AttackRange * 2.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc, Radius);

	for (AActor* A : Targets)
	{
		if (A && IsValidAutoTarget(A))
			ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
	}
	SpawnSlashVFX(HeroLoc, Radius, FT66TemporaryProjectileSystem::HeroProjectileColor());
}

void UT66CombatComponent::PerformUltimateTidalWave(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FVector Forward = OwnerActor->GetActorRotation().Vector();
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	const float LineLength = AttackRange;
	const float TubeRadius = 80.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
	constexpr int32 NumLines = 5;
	const float Spacing = 120.f;

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc + Forward * (LineLength * 0.5f), LineLength);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumLines; ++i)
	{
		const float Offset = (static_cast<float>(i) - static_cast<float>(NumLines - 1) * 0.5f) * Spacing;
		const FVector Start = HeroLoc + Right * Offset;
		const FVector End = Start + Forward * LineLength;

		for (AActor* A : Targets)
		{
			if (!A || !IsValidAutoTarget(A) || AlreadyHit.Contains(A)) continue;
			const float DistSq = FMath::PointDistToSegmentSquared(A->GetActorLocation(), Start, End);
			if (DistSq <= TubeRadius * TubeRadius)
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		SpawnPierceVFX(Start, End, FT66TemporaryProjectileSystem::HeroProjectileColor());
	}
}

void UT66CombatComponent::PerformUltimateGoldRush(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;
	const float ExplosionRadius = 250.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc, Range);
	for (int32 Index = Targets.Num() - 1; Index >= 0; --Index)
	{
		if (AActor* A = Targets[Index]; A && IsValidAutoTarget(A))
		{
			continue;
		}
		Targets.RemoveAtSwap(Index, 1, EAllowShrinking::No);
	}

	const int32 MaxExplosions = FMath::Min(8, Targets.Num());
	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < MaxExplosions; ++i)
	{
		const FVector Center = Targets[i]->GetActorLocation();
		if (!AlreadyHit.Contains(Targets[i]))
		{
			AlreadyHit.Add(Targets[i]);
			ApplyDamageToActor(Targets[i], UltimateDamage, NAME_None, SourceID);
		}
		for (AActor* A : Targets)
		{
			if (A && IsValidAutoTarget(A) && !AlreadyHit.Contains(A))
			{
				if (FVector::DistSquared(A->GetActorLocation(), Center) <= ExplosionRadius * ExplosionRadius)
				{
					AlreadyHit.Add(A);
					ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
				}
			}
		}
		SpawnSlashVFX(Center, ExplosionRadius, FT66TemporaryProjectileSystem::HeroProjectileColor());
	}
}

void UT66CombatComponent::PerformUltimateMiasmaBomb(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || !CachedRunState) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;

	AActor* NearestTarget = FindClosestEnemyInRange(HeroLoc, Range * Range);
	const FVector Center = NearestTarget ? NearestTarget->GetActorLocation() : (HeroLoc + OwnerActor->GetActorRotation().Vector() * 500.f);
	const float Radius = 600.f;
	const float Duration = 5.f;
	const float TickInterval = 0.5f;
	const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
	const float DmgPerTick = static_cast<float>(UltimateDamage) / static_cast<float>(Ticks);

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, Center, Radius);

	for (AActor* A : Targets)
	{
		if (A && IsValidAutoTarget(A))
		{
			static const FName PoisonCloudSource(TEXT("Ultimate_PoisonCloudDOT"));
			CachedRunState->ApplyDOT(A, Duration, TickInterval, DmgPerTick, PoisonCloudSource);
		}
	}
	SpawnDOTVFX(Center, Duration, Radius, FT66TemporaryProjectileSystem::HeroProjectileColor());
}

void UT66CombatComponent::PerformUltimateRabidFrenzy()
{
	UWorld* World = GetWorld();
	if (!World) return;
	RabidFrenzyEndTime = static_cast<float>(World->GetTimeSeconds()) + 4.f;
}

void UT66CombatComponent::PerformUltimateBlizzard(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || !CachedRunState) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float Range = AttackRange;
	const float Duration = 6.f;
	const float TickInterval = 0.5f;
	const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
	const float DmgPerTick = static_cast<float>(UltimateDamage) / static_cast<float>(Ticks);
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	const TArray<AActor*> Targets = T66GatherAttackTargetsInSphere(World, OwnerActor, HeroLoc, Range);

	for (AActor* A : Targets)
	{
		if (A && IsValidAutoTarget(A))
		{
			static const FName BlizzardSource(TEXT("Ultimate_BlizzardDOT"));
			CachedRunState->ApplyDOT(A, Duration, TickInterval, DmgPerTick, BlizzardSource);
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(A))
				E->ApplyMoveSlow(0.6f, Duration);
			else if (AT66MobBase* M = Cast<AT66MobBase>(A))
				M->ApplyMoveSlow(0.6f, Duration);
		}
	}
	SpawnSlashVFX(HeroLoc, Range * 0.5f, FT66TemporaryProjectileSystem::HeroProjectileColor());
}
