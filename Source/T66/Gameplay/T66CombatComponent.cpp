// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HeroOneAttackVFX.h"
#include "Gameplay/T66CombatShared.h"
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
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY(LogT66Combat);

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
	if (!CachedSlashVFXNiagara)
	{
		CachedSlashVFXNiagara = SlashVFXNiagara.Get();
	}
	if (!CachedPixelVFXNiagara)
	{
		CachedPixelVFXNiagara = PixelVFXNiagara.Get();
	}
	if ((CachedSlashVFXNiagara && CachedPixelVFXNiagara) || CombatPresentationAssetsLoadHandle.IsValid())
	{
		return;
	}

	TArray<FSoftObjectPath> AssetPaths;
	if (!CachedSlashVFXNiagara && !SlashVFXNiagara.IsNull())
	{
		AssetPaths.AddUnique(SlashVFXNiagara.ToSoftObjectPath());
	}
	if (!CachedPixelVFXNiagara && !PixelVFXNiagara.IsNull())
	{
		AssetPaths.AddUnique(PixelVFXNiagara.ToSoftObjectPath());
	}
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
	if (!CachedSlashVFXNiagara)
	{
		CachedSlashVFXNiagara = SlashVFXNiagara.Get();
	}
	if (!CachedPixelVFXNiagara)
	{
		CachedPixelVFXNiagara = PixelVFXNiagara.Get();
	}
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
			else if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
			{
				const int32 DesiredFinalDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Boss->MaxHP) * 0.05f));
				const float DamageMultiplier = FMath::Max(0.01f, 1.f - Boss->GetEffectiveArmor());
				const int32 RawDamage = FMath::Max(1, FMath::CeilToInt(static_cast<float>(DesiredFinalDamage) / DamageMultiplier));
				Boss->TakeDamageFromHeroHit(RawDamage, SourceID, NAME_None);
			}
		}
	}

	SpawnPierceVFX(Start, End, FLinearColor(0.95f, 0.95f, 1.f));
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
		if (CachedRunState)
		{
			CachedRunState->InventoryChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->HeroProgressChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
			CachedRunState->SurvivalChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
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
		CachedRunState->SurvivalChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->DevCheatsChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}
	if (CachedIdolManager)
	{
		CachedIdolManager->IdolStateChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
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

	// Only track enemies and bosses (Gambler/Vendor are AT66BossBase).
	if (Cast<AT66EnemyBase>(OtherActor) || Cast<AT66BossBase>(OtherActor))
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
		const float DistSq = FVector::DistSquared(FromLocation, A->GetActorLocation());
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
			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
			{
				AT66BossBase* Boss = WeakBoss.Get();
				if (!Boss) continue;
				if (ExcludeSet && ExcludeSet->Contains(Boss)) continue;
				if (!IsValidAutoTarget(Boss)) continue;
				const float DistSq = FVector::DistSquared(FromLocation, Boss->GetActorLocation());
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

	const float HeroAttackSpeedMult = CachedRunState->GetHeroAttackSpeedMultiplier() * CachedRunState->GetLastStandAttackSpeedMultiplier() * CachedRunState->GetRallyAttackSpeedMultiplier() * CachedRunState->GetEnduranceAttackSpeedMultiplier();
	const float HeroDamageMult = CachedRunState->GetHeroDamageMultiplier() * CachedRunState->GetEnduranceDamageMultiplier() * CachedRunState->GetBrawlersFuryDamageMultiplier();
	const float HeroScaleMult = CachedRunState->GetHeroScaleMultiplier();

	const float TotalAttackSpeed = AttackSpeedMult * HeroAttackSpeedMult;
	const float TotalDamage = DamageMult * HeroDamageMult;
	const float TotalScale = ScaleMult * HeroScaleMult;

	// Per HUD/combat spec: Scale stat affects attack range (larger scale = larger range).
	// Use the DataTable base range (hero-specific) instead of the C++ default.
	const float DataTableRange = CachedRunState->GetHeroBaseAttackRange();
	const float EffectiveBaseRange = (DataTableRange > 0.f) ? DataTableRange : BaseAttackRange;
	AttackRange = FMath::Clamp(EffectiveBaseRange * FMath::Max(0.1f, TotalScale), 200.f, 50000.f);

	EffectiveFireIntervalSeconds = FMath::Clamp(BaseFireIntervalSeconds / FMath::Max(0.01f, TotalAttackSpeed), 0.05f, 10.f);
	EffectiveDamagePerShot = FMath::Clamp(FMath::RoundToInt(static_cast<float>(BaseDamagePerShot) * TotalDamage), 1, 999999);
	ProjectileScaleMultiplier = FMath::Clamp(TotalScale, 0.1f, 10.f);

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

	LastFireTime = static_cast<float>(World->GetTimeSeconds());

	// Crit: roll per hit; multiply damage and pass EventType_Crit for floating text.
	auto ResolveCrit = [this, RngSub](int32 BaseDamage) -> TPair<int32, FName>
	{
		if (!CachedRunState) return { BaseDamage, NAME_None };
		const float CritChance = CachedRunState->GetCritChance01();
		const bool bCrit = RollTierChance(CritChance, RngSub);
		if (bCrit)
		{
			const float Mult = CachedRunState->GetCritDamageMultiplier();
			return { FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult)), UT66FloatingCombatTextSubsystem::EventType_Crit };
		}
		return { BaseDamage, NAME_None };
	};

	FVector MyLoc = OwnerActor->GetActorLocation();

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

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(Overlaps, MidPoint, Rot, ECC_Pawn, Cap, Params);

		OutTargets.Reset();
		if (QueryPrimaryTarget && IsValidAutoTarget(QueryPrimaryTarget))
		{
			OutTargets.Add(QueryPrimaryTarget);
		}
		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (A && IsValidAutoTarget(A) && A != QueryPrimaryTarget && (Cast<AT66EnemyBase>(A) || Cast<AT66BossBase>(A)))
			{
				OutTargets.AddUnique(A);
			}
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

	auto BuildSlashTargets = [&](AActor* QueryPrimaryTarget, float Radius, TArray<AActor*>& OutTargets, const FVector* OverrideCenter = nullptr)
	{
		if (!QueryPrimaryTarget && !OverrideCenter)
		{
			OutTargets.Reset();
			return;
		}

		const bool bCanUseCache = (OverrideCenter == nullptr);
		if (bCanUseCache
			&& bHasCachedSlashTargets
			&& CachedSlashPrimaryTarget == QueryPrimaryTarget
			&& FMath::IsNearlyEqual(CachedSlashRadius, Radius))
		{
			OutTargets = CachedSlashTargets;
			return;
		}

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);
		Params.AddIgnoredActor(QueryPrimaryTarget);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(
			Overlaps,
			OverrideCenter ? *OverrideCenter : QueryPrimaryTarget->GetActorLocation(),
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(Radius),
			Params);

		OutTargets.Reset();
		if (QueryPrimaryTarget && IsValidAutoTarget(QueryPrimaryTarget))
		{
			OutTargets.Add(QueryPrimaryTarget);
		}
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (AActor* Hit = Overlap.GetActor(); Hit && IsValidAutoTarget(Hit))
			{
				OutTargets.AddUnique(Hit);
			}
		}

		if (bCanUseCache)
		{
			bHasCachedSlashTargets = true;
			CachedSlashPrimaryTarget = QueryPrimaryTarget;
			CachedSlashRadius = Radius;
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
	FT66CombatTargetHandle PrimaryTargetHandle;

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

		for (int32 i = 0; i < InLine.Num(); ++i)
		{
			const FT66CombatTargetHandle HitHandle = (i == 0)
				? PrimaryHandle
				: ResolveAutoAttackTargetHandle(InLine[i], false, RngSub);
			const float Multiplier = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
			const float PrimaryMult = (i == 0) ? PrimaryDamageMult : 1.f;
			const int32 BaseDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * Multiplier * PrimaryMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BaseDmg, InLine[i], &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToTargetHandle(HitHandle, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}

		if (CurrentHeroID == FName(TEXT("Hero_1")))
		{
			const float FurthestDistance = InLine.Num() > 0
				? FVector::Dist2D(MyLoc, InLine.Last()->GetActorLocation())
				: FVector::Dist2D(MyLoc, TargetLoc);
			const FVector VFXStart = MyLoc + FVector(0.f, 0.f, 8.f);
			const FVector VFXEnd = MyLoc + Dir * FMath::Min(LineLength, FurthestDistance + 80.f) + FVector(0.f, 0.f, 8.f);
			const FVector ImpactLoc = TargetLoc + FVector(0.f, 0.f, 10.f);
			SpawnHeroOnePierceVFX(VFXStart, VFXEnd, ImpactLoc);
		}
		else
		{
			const float FurthestDistance = InLine.Num() > 0
				? FVector::Dist2D(MyLoc, InLine.Last()->GetActorLocation())
				: FVector::Dist2D(MyLoc, TargetLoc);
			const FVector VFXStart = MyLoc + FVector(0.f, 0.f, 8.f);
			const FVector VFXEnd = MyLoc + Dir * FMath::Min(LineLength, FurthestDistance + 80.f) + FVector(0.f, 0.f, 8.f);
			const FVector ImpactLoc = TargetLoc + FVector(0.f, 0.f, 10.f);
			const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
			SpawnHeroPierceVFX(VFXStart, VFXEnd, ImpactLoc, HeroTint, CurrentHeroID);
		}
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
		const float EffectiveSlashRadius = SlashRadius * ProjectileScaleMultiplier;

		TArray<AActor*> SlashTargets;
		BuildSlashTargets(PrimaryTarget, EffectiveSlashRadius, SlashTargets);

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
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToTargetHandle(PrimaryHandle, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
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
				const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
				ApplyDamageToTargetHandle(HitHandle, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
			}
		}

		const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
		SpawnHeroSlashVFX(SlashCenter, EffectiveSlashRadius, HeroTint, CurrentHeroID);
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
		const int32 BounceCount = BaseBounce + ChaosBonus + JuicedBonus;
		const float Falloff = FMath::Clamp(bHaveHeroData ? HeroDataForPrimary.FalloffPerHit : 0.f, 0.f, 0.95f);
		const float BounceRangeSq = AttackRange * AttackRange;
		const FVector PrimaryLoc = GetTargetAimPoint(PrimaryHandle);
		TArray<FVector> ChainPositions;
		ChainPositions.Add(MyLoc);
		ChainPositions.Add(PrimaryLoc);
		const int32 PrimaryDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult));
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(PrimaryDmg, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToTargetHandle(PrimaryHandle, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}
		FVector CurrentLoc = PrimaryLoc;
		TSet<FString> HitKeys;
		HitKeys.Add(MakeTargetHandleKey(PrimaryHandle));
		int32 BouncesLeft = BounceCount - 1;
		float DamageMult = 1.f - Falloff;
		while (BouncesLeft > 0)
		{
			const FT66CombatTargetHandle NextHandle = FindClosestTargetHandleInRange(CurrentLoc, BounceRangeSq, &HitKeys);
			AActor* Next = NextHandle.Actor.Get();
			if (!Next) break;
			ChainPositions.Add(GetTargetAimPoint(NextHandle));
			HitKeys.Add(MakeTargetHandleKey(NextHandle));
			const int32 BounceDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * DamageMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BounceDmg, Next, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToTargetHandle(NextHandle, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
			// StaticCharge: 20% chance to confuse bounced targets.
			if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::StaticCharge)
			{
				if (RollTierChance(0.2f, RngSub))
				{
					if (AT66EnemyBase* BounceEnemy = Cast<AT66EnemyBase>(Next))
						BounceEnemy->ApplyConfusion(1.5f);
				}
			}
			CurrentLoc = GetTargetAimPoint(NextHandle);
			DamageMult *= (1.f - Falloff);
			--BouncesLeft;
		}
		const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
		SpawnHeroBounceVFX(ChainPositions, HeroTint, CurrentHeroID);
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
		const float Duration = (bHaveHeroData && HeroDataForPrimary.DotDuration > 0.f) ? HeroDataForPrimary.DotDuration : 3.f;
		const float TickInterval = (bHaveHeroData && HeroDataForPrimary.DotTickInterval > 0.f) ? HeroDataForPrimary.DotTickInterval : 0.5f;
		const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
		const int32 InitialDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EffectiveDamagePerShot) * 0.5f * PrimaryDamageMult));
		const float DotTotalDamage = static_cast<float>(FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult) - InitialDamage));
		const float DamagePerTick = DotTotalDamage / static_cast<float>(Ticks);
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(InitialDamage, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToTargetHandle(PrimaryHandle, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}
		CachedRunState->ApplyDOT(PrimaryTarget, Duration, TickInterval, DamagePerTick, HeroPrimaryDotSource);
		// Frostbite: DOT attacks slow enemy move speed by 30%.
		if (CachedRunState->HasFrostbite())
		{
			if (AT66EnemyBase* DotEnemy = Cast<AT66EnemyBase>(PrimaryTarget))
				DotEnemy->ApplyMoveSlow(0.7f, Duration);
		}
		const FLinearColor HeroTint = bHaveHeroData ? HeroDataForPrimary.PlaceholderColor : FLinearColor::White;
		SpawnHeroDOTVFX(PrimaryTarget, GetTargetAimPoint(PrimaryHandle), Duration, 80.f, HeroTint, CurrentHeroID);
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

		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
		{
			if (Enemy->CurrentHP > 0)
			{
				ApplyDamageToActor(Target, Enemy->CurrentHP + 9999, FName(TEXT("Execute")), SourceId);
				ShowTargetStatus(Target, FName(TEXT("Execute")));
				return true;
			}
			return false;
		}

		if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
		{
			if (Boss->IsAlive())
			{
				const int32 BurstDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Boss->CurrentHP) * 0.12f));
				ApplyDamageToActor(Target, BurstDamage, FName(TEXT("Execute")), SourceId);
				ShowTargetStatus(Target, FName(TEXT("Execute")));
				return true;
			}
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
			if (DistSq <= RangeSq)
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
		PrimaryTarget = FindClosestEnemyInRange(MyLoc, RangeSq);
	}

	PrimaryTargetHandle = PrimaryTarget
		? ResolveAutoAttackTargetHandle(PrimaryTarget, LockedTarget.Actor.Get() == PrimaryTarget, RngSub)
		: FT66CombatTargetHandle{};

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
		CachedRunState->NotifyAttackFired();

	// Hero primary attack (Pierce / Bounce / AOE / DOT; VFX white).
	if (PrimaryTarget)
	{
		const FVector PrimaryTargetImpactLocation = GetTargetAimPoint(PrimaryTargetHandle);
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
			SpawnDOTVFX(PrimaryTarget->GetActorLocation(), 3.f, 60.f, FLinearColor(0.6f, 0.2f, 0.8f));
		}

		// RabidFrenzy ultimate buff: every hit applies a short DOT.
		if (RabidFrenzyEndTime > Now && CachedRunState && PrimaryTarget)
		{
			static const FName RabidFrenzyDotSource(TEXT("Ultimate_RabidFrenzyDOT"));
			const float FrenzyDotDmg = static_cast<float>(EffectiveDamagePerShot) * 0.3f;
			const float FrenzyTicks = 4.f;
			CachedRunState->ApplyDOT(PrimaryTarget, 2.f, 0.5f, FrenzyDotDmg / FrenzyTicks, RabidFrenzyDotSource);
			SpawnDOTVFX(PrimaryTarget->GetActorLocation(), 2.f, 50.f, FLinearColor(0.9f, 0.3f, 0.1f));
		}

		// Idol attacks: one per equipped idol, each with unique color.
		if (CachedRunState)
		{
			const float IdolRange = AttackRange;
			// Bounce search radius = hero attack range, centered on the last hit enemy each step.
			const float BounceSearchRadius = IdolRange;
			UE_LOG(
				LogT66Combat,
				Verbose,
				TEXT("[IDOL PROC] owner=%s target=%s cachedIdolSlots=%d"),
				GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
				*PrimaryTarget->GetName(),
				CachedIdolSlots.Num());

			int32 IdolVisualIndex = 0;
			for (const FCachedIdolSlot& CachedIdolSlot : CachedIdolSlots)
			{
				if (!CachedIdolSlot.bValid || CachedIdolSlot.IdolID.IsNone()) continue;
				const FName IdolID = CachedIdolSlot.IdolID;
				const ET66ItemRarity IdolRarity = CachedIdolSlot.Rarity;
				const FIdolData& IdolData = CachedIdolSlot.IdolData;
				const float IdolVisualDelay = static_cast<float>(IdolVisualIndex) * 0.035f;
				const float IdolGlobalScale = FMath::Max(0.1f, ProjectileScaleMultiplier);
				const float IdolCategorySubScale = T66CombatShared::GetCategorySubScaleMultiplier(CachedRunState, IdolData.Category);
				const float IdolBehaviorScale = IdolGlobalScale * IdolCategorySubScale;

				const int32 IdolDamage = FMath::Max(1, FMath::RoundToInt(IdolData.GetDamageAtRarity(IdolRarity)));
				const FVector PrimaryLoc = PrimaryTargetImpactLocation;
				const FVector PrimaryVFXLoc = T66CombatShared::ResolveGroundAnchor(World, PrimaryLoc, PrimaryTarget);
				UE_LOG(
					LogT66Combat,
					Verbose,
					TEXT("[IDOL PROC] Index=%d Idol=%s Rarity=%s Category=%s Damage=%d Delay=%.3f targetLoc=%s vfxLoc=%s"),
					IdolVisualIndex,
					*IdolID.ToString(),
					T66CombatShared::GetItemRarityName(IdolRarity),
					GetT66AttackCategoryName(IdolData.Category),
					IdolDamage,
					IdolVisualDelay,
					*PrimaryLoc.ToCompactString(),
					*PrimaryVFXLoc.ToCompactString());

				switch (IdolData.Category)
				{
				case ET66AttackCategory::Pierce:
				{
					const float LineLength = IdolRange * IdolCategorySubScale;
					const float PierceRadius = 60.f * IdolBehaviorScale;
					const int32 PierceCount = FMath::Max(0, FMath::RoundToInt(IdolData.GetPropertyAtRarity(IdolRarity)));
					FVector Dir = FVector::ForwardVector;
					TArray<AActor*> InLine;
					BuildPierceTargets(PrimaryTarget, LineLength, PierceRadius, InLine, Dir, &PrimaryLoc);
					const int32 MaxTargets = FMath::Max(1, PierceCount + 1);
					if (InLine.Num() > MaxTargets)
					{
						InLine.SetNum(MaxTargets, EAllowShrinking::No);
					}
					for (int32 i = 0; i < InLine.Num(); ++i)
					{
						const float Mult = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
						const int32 Dmg = FMath::Max(1, FMath::RoundToInt(IdolDamage * Mult));
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(Dmg, InLine[i], &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(InLine[i], Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						ApplyIdolSpecialBehavior(InLine[i], IdolID, IdolRarity, Dmg, PrimaryLoc);
					}
					SpawnIdolPierceVFX(IdolID, IdolRarity, PrimaryVFXLoc, PrimaryVFXLoc + Dir * (LineLength * 0.5f), PrimaryVFXLoc, IdolVisualDelay);
					break;
				}
				case ET66AttackCategory::AOE:
				{
					const float Radius = FMath::Max(50.f, IdolData.GetPropertyAtRarity(IdolRarity) * IdolBehaviorScale);
					if (IsValidAutoTarget(PrimaryTarget))
					{
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, PrimaryTarget, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						ApplyIdolSpecialBehavior(PrimaryTarget, IdolID, IdolRarity, IdolDamage, PrimaryLoc);
					}
					TArray<AActor*> SlashTargets;
					BuildSlashTargets(PrimaryTarget, Radius, SlashTargets, &PrimaryLoc);
					for (int32 TargetIndex = 0; TargetIndex < SlashTargets.Num(); ++TargetIndex)
					{
						if (AActor* Hit = SlashTargets[TargetIndex])
						{
							if (Hit == PrimaryTarget)
							{
								continue;
							}
							FName RangeEvent;
							const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, Hit, &RangeEvent);
							const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
							ApplyDamageToActor(Hit, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
							ApplyIdolSpecialBehavior(Hit, IdolID, IdolRarity, IdolDamage, PrimaryLoc);
						}
					}
					SpawnIdolAOEVFX(IdolID, IdolRarity, PrimaryVFXLoc, Radius, IdolVisualDelay);
					break;
				}
				case ET66AttackCategory::Bounce:
				{
					// BounceCount = number of jumps FROM the primary target to other enemies.
					// At Black rarity (BaseProperty=1) the bolt bounces once (hits 1 extra enemy).
					const int32 BounceCount = FMath::Max(1, FMath::RoundToInt(IdolData.GetPropertyAtRarity(IdolRarity) * IdolBehaviorScale));
					const float IdolFalloff = FMath::Clamp(IdolData.FalloffPerHit, 0.f, 0.95f);
					const float BounceRangeSq = BounceSearchRadius * BounceSearchRadius;

					// Damage the primary target (auto-attack already hit it; idol adds extra damage).
					if (IsValidAutoTarget(PrimaryTarget))
					{
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, PrimaryTarget, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						ApplyIdolSpecialBehavior(PrimaryTarget, IdolID, IdolRarity, IdolDamage, PrimaryLoc);
					}

					// Chain starts FROM the primary target (not from the hero).
					// VFX will only show enemy->enemy segments so it looks like a bolt bouncing off.
					TArray<FVector> ChainPositions;
					ChainPositions.Add(PrimaryVFXLoc);

					FVector CurrentLoc = PrimaryLoc;
					TSet<FString> HitKeys;
					HitKeys.Add(MakeTargetHandleKey(MakeActorTargetHandle(PrimaryTarget)));
					int32 BouncesLeft = BounceCount;
					float IdolDamageMult = 1.f - IdolFalloff;

					while (BouncesLeft > 0)
					{
						const FT66CombatTargetHandle NextHandle = FindClosestTargetHandleInRange(CurrentLoc, BounceRangeSq, &HitKeys);
						AActor* Next = NextHandle.Actor.Get();
						if (!Next) break;
						ChainPositions.Add(GetTargetAimPoint(NextHandle));
						HitKeys.Add(MakeTargetHandleKey(NextHandle));
						const int32 BounceDmg = FMath::Max(1, FMath::RoundToInt(IdolDamage * IdolDamageMult));
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(BounceDmg, Next, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToTargetHandle(NextHandle, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						ApplyIdolSpecialBehavior(Next, IdolID, IdolRarity, BounceDmg, CurrentLoc);
						CurrentLoc = GetTargetAimPoint(NextHandle);
						IdolDamageMult *= (1.f - IdolFalloff);
						--BouncesLeft;
					}
					SpawnIdolBounceVFX(IdolID, IdolRarity, ChainPositions, IdolVisualDelay);
					break;
				}
				case ET66AttackCategory::DOT:
				{
					const float Duration = FMath::Max(0.5f, IdolData.GetPropertyAtRarity(IdolRarity) * IdolBehaviorScale);
					const float TickInterval = FMath::Max(0.1f, IdolData.DotTickInterval);
					const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
					const float DamagePerTick = static_cast<float>(IdolDamage) / static_cast<float>(Ticks);
					if (IsValidAutoTarget(PrimaryTarget))
					{
						CachedRunState->ApplyDOT(PrimaryTarget, Duration, TickInterval, DamagePerTick, IdolID);
						ApplyIdolSpecialBehavior(PrimaryTarget, IdolID, IdolRarity, IdolDamage, PrimaryLoc);
					}
					SpawnIdolDOTVFX(IdolID, IdolRarity, IsValidAutoTarget(PrimaryTarget) ? PrimaryTarget : nullptr, PrimaryVFXLoc, Duration, 80.f, IdolVisualDelay);
					break;
				}
				default:
					break;
				}

				++IdolVisualIndex;
			}
		}
	}
}

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
		SpawnBounceVFX(ChainPositions, FLinearColor(1.f, 0.85f, 0.2f, 1.f));
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
	SpawnPierceVFX(HeroLoc, End, FLinearColor(0.95f, 0.95f, 1.f));
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
		SpawnPierceVFX(HeroLoc, End, FLinearColor(0.9f, 0.7f, 0.2f));
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
		SpawnBounceVFX(Positions, FLinearColor(0.5f, 0.8f, 1.f));
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
		SpawnBounceVFX({HeroLoc, End}, FLinearColor(0.4f, 0.1f, 0.6f));
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
	SpawnSlashVFX(HeroLoc, Radius, FLinearColor(0.9f, 0.5f, 0.1f));
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
		SpawnPierceVFX(Start, End, FLinearColor(0.3f, 0.6f, 0.9f));
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
		SpawnSlashVFX(Center, ExplosionRadius, FLinearColor(0.95f, 0.85f, 0.1f));
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
	SpawnDOTVFX(Center, Duration, Radius, FLinearColor(0.5f, 0.9f, 0.2f));
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
		}
	}
	SpawnSlashVFX(HeroLoc, Range * 0.5f, FLinearColor(0.5f, 0.8f, 1.f));
}
