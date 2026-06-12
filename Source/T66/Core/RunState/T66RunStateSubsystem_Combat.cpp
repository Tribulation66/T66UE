// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66AudioSubsystem.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "Gameplay/T66LavaPatch.h"
#include "Gameplay/T66LoanShark.h"
#include "Gameplay/T66MiasmaBoundary.h"
#include "Gameplay/T66MiasmaManager.h"
#include "Gameplay/T66MiasmaTile.h"
#include "Gameplay/T66TutorialManager.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/Physics/T66HeroPhysicsComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

using namespace T66RunStatePrivate;

DEFINE_LOG_CATEGORY_STATIC(LogT66DamageReceived, Log, All);
DEFINE_LOG_CATEGORY_STATIC(LogT66RunState, Log, All);

namespace
{
	static const FName T66BackroomsChaserTouchDelivery(TEXT("BackroomsChaserTouch"));
	static const FName T66OuroborosLethalZoneDelivery(TEXT("OuroborosLethalZone"));
	static float GT66DamagePercentGainScaleAt100 = 1.75f;
	static float GT66DamagePercentLaunchScaleAt100 = 2.25f;
	static float GT66DamagePercentBaseLaunchXY = 1250.f;
	static float GT66DamagePercentLaunchXYPerPercent = 32.f;
	static float GT66DamagePercentBaseLaunchZ = 460.f;
	static float GT66DamagePercentLaunchZPerPercent = 6.f;
	static float GT66EnemyDamageDisableStartPercent = 50.f;
	static float GT66EnemyDamageDisableFullPercent = 99.f;
	static float GT66EnemyDamageKnockbackOnlyXY = 520.f;
	static float GT66EnemyDamageKnockbackOnlyZ = 140.f;

	static FAutoConsoleVariableRef CVarT66DamagePercentGainScaleAt100(
		TEXT("t66.HealthPercent.DamageGainScaleAt100"),
		GT66DamagePercentGainScaleAt100,
		TEXT("Damage-percent gain multiplier when the hero is already at 100%. Interpolates from 1.0 at 0%."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66DamagePercentLaunchScaleAt100(
		TEXT("t66.HealthPercent.LaunchScaleAt100"),
		GT66DamagePercentLaunchScaleAt100,
		TEXT("Physics launch multiplier when the hero is already at 100%. Interpolates from 1.0 at 0%."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66DamagePercentBaseLaunchXY(
		TEXT("t66.HealthPercent.BaseLaunchXY"),
		GT66DamagePercentBaseLaunchXY,
		TEXT("Base horizontal launch speed requested for every accepted hero damage source."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66DamagePercentLaunchXYPerPercent(
		TEXT("t66.HealthPercent.LaunchXYPerPercent"),
		GT66DamagePercentLaunchXYPerPercent,
		TEXT("Additional horizontal launch speed per gained damage percent."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66DamagePercentBaseLaunchZ(
		TEXT("t66.HealthPercent.BaseLaunchZ"),
		GT66DamagePercentBaseLaunchZ,
		TEXT("Base vertical launch speed requested for every accepted hero damage source."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66DamagePercentLaunchZPerPercent(
		TEXT("t66.HealthPercent.LaunchZPerPercent"),
		GT66DamagePercentLaunchZPerPercent,
		TEXT("Additional vertical launch speed per gained damage percent."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66EnemyDamageDisableStartPercent(
		TEXT("t66.HealthPercent.EnemyDisableStartPercent"),
		GT66EnemyDamageDisableStartPercent,
		TEXT("Enemy damage only starts throw/disable reactions after the hero damage percent rises above this value."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66EnemyDamageDisableFullPercent(
		TEXT("t66.HealthPercent.EnemyDisableFullPercent"),
		GT66EnemyDamageDisableFullPercent,
		TEXT("Enemy throw distance and disable duration reach their full percent-scaled value at this hero damage percent."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66EnemyDamageKnockbackOnlyXY(
		TEXT("t66.HealthPercent.EnemyKnockbackOnlyXY"),
		GT66EnemyDamageKnockbackOnlyXY,
		TEXT("Horizontal launch speed for enemy damage that is below the throw/disable threshold."),
		ECVF_Default);

	static FAutoConsoleVariableRef CVarT66EnemyDamageKnockbackOnlyZ(
		TEXT("t66.HealthPercent.EnemyKnockbackOnlyZ"),
		GT66EnemyDamageKnockbackOnlyZ,
		TEXT("Vertical launch speed for enemy damage that is below the throw/disable threshold."),
		ECVF_Default);

	bool T66IsEnemyDamageReactionSource(const AActor* Attacker, const FName DeliveryMethod)
	{
		if (Cast<AT66EnemyBase>(Attacker) || Cast<AT66MobBase>(Attacker))
		{
			return true;
		}

		const FString DeliveryText = DeliveryMethod.ToString();
		return DeliveryText.StartsWith(TEXT("Enemy"));
	}

	float T66GetEnemyDamageDisableStartPercent()
	{
		return FMath::Clamp(GT66EnemyDamageDisableStartPercent, 0.f, 99.f);
	}

	float T66GetEnemyDamageDisableFullPercent()
	{
		return FMath::Clamp(
			GT66EnemyDamageDisableFullPercent,
			T66GetEnemyDamageDisableStartPercent() + 1.f,
			100.f);
	}

	float T66GetEnemyDamageThrowAlpha(const float DamagePercent)
	{
		const float StartPercent = T66GetEnemyDamageDisableStartPercent();
		const float FullPercent = T66GetEnemyDamageDisableFullPercent();
		return FMath::Clamp((DamagePercent - StartPercent) / FMath::Max(1.f, FullPercent - StartPercent), 0.f, 1.f);
	}

	FString T66FormatDamageActorLocation(const AActor* Actor)
	{
		return Actor ? Actor->GetActorLocation().ToCompactString() : FString(TEXT("None"));
	}

	FString T66FormatDamageActorName(const AActor* Actor)
	{
		return Actor ? Actor->GetName() : FString(TEXT("None"));
	}

	FString T66FormatDamageActorClass(const AActor* Actor)
	{
		return Actor && Actor->GetClass() ? Actor->GetClass()->GetName() : FString(TEXT("None"));
	}

	FName T66ResolveDamageReceivedSourceID(AActor* Attacker)
	{
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Attacker))
		{
			return Enemy->MobID.IsNone() ? FName(TEXT("Enemy")) : Enemy->MobID;
		}

		if (AT66MobBase* Mob = Cast<AT66MobBase>(Attacker))
		{
			return Mob->GetMobID().IsNone() ? FName(TEXT("Mob")) : Mob->GetMobID();
		}

		if (AT66BossBase* Boss = Cast<AT66BossBase>(Attacker))
		{
			return Boss->BossID.IsNone() ? FName(TEXT("Boss")) : Boss->BossID;
		}

		if (AT66TrapBase* Trap = Cast<AT66TrapBase>(Attacker))
		{
			const FName TrapTypeID = Trap->GetTrapTypeID();
			if (!TrapTypeID.IsNone())
			{
				return TrapTypeID;
			}

			const FName TrapFamilyID = Trap->GetTrapFamilyID();
			if (!TrapFamilyID.IsNone())
			{
				return TrapFamilyID;
			}

			return FName(TEXT("Trap"));
		}

		if (Cast<AT66MiasmaBoundary>(Attacker))
		{
			return FName(TEXT("MiasmaBoundary"));
		}

		if (Cast<AT66MiasmaManager>(Attacker))
		{
			return FName(TEXT("MiasmaCoverage"));
		}

		if (Cast<AT66MiasmaTile>(Attacker))
		{
			return FName(TEXT("MiasmaTile"));
		}

		if (Cast<AT66LavaPatch>(Attacker))
		{
			return FName(TEXT("LavaPatch"));
		}

		if (Cast<AT66LoanShark>(Attacker))
		{
			return FName(TEXT("LoanShark"));
		}

		if (Cast<AT66TutorialManager>(Attacker))
		{
			return FName(TEXT("TutorialScriptedDamage"));
		}

		if (Attacker && Attacker->GetClass())
		{
			return FName(*Attacker->GetClass()->GetName());
		}

		return UT66DamageLogSubsystem::SourceID_Environment;
	}

	FName T66ResolveDeliveryMethod(AActor* Attacker, const FName RequestedMethod)
	{
		if (!RequestedMethod.IsNone())
		{
			return RequestedMethod;
		}

		if (Cast<AT66EnemyBase>(Attacker))
		{
			return FName(TEXT("EnemyUnspecified"));
		}

		if (Cast<AT66MobBase>(Attacker))
		{
			return FName(TEXT("EnemyUnspecified"));
		}

		if (Cast<AT66BossBase>(Attacker))
		{
			return FName(TEXT("BossUnspecified"));
		}

		if (Cast<AT66TrapBase>(Attacker))
		{
			return FName(TEXT("TrapUnspecified"));
		}

		if (Cast<AT66MiasmaBoundary>(Attacker))
		{
			return FName(TEXT("MiasmaBoundary"));
		}

		if (Cast<AT66MiasmaManager>(Attacker))
		{
			return FName(TEXT("MiasmaCoverage"));
		}

		if (Cast<AT66MiasmaTile>(Attacker))
		{
			return FName(TEXT("MiasmaTile"));
		}

		if (Cast<AT66LavaPatch>(Attacker))
		{
			return FName(TEXT("LavaPatch"));
		}

		if (Cast<AT66LoanShark>(Attacker))
		{
			return FName(TEXT("LoanSharkTouch"));
		}

		if (Cast<AT66TutorialManager>(Attacker))
		{
			return FName(TEXT("TutorialScriptedDamage"));
		}

		return FName(TEXT("Unspecified"));
	}

	float T66Distance2DOrNegative(const AActor* A, const AActor* B)
	{
		return (A && B) ? FVector::Dist2D(A->GetActorLocation(), B->GetActorLocation()) : -1.f;
	}

	float T66Distance3DOrNegative(const AActor* A, const AActor* B)
	{
		return (A && B) ? FVector::Dist(A->GetActorLocation(), B->GetActorLocation()) : -1.f;
	}

	FString T66ResolveLineOfSightStatus(
		UWorld* World,
		const AActor* SourceActor,
		const AActor* HeroActor,
		const AActor* Attacker,
		const AActor* DamageCauser,
		FString& OutBlockerName)
	{
		OutBlockerName = TEXT("None");
		if (!World || !SourceActor || !HeroActor)
		{
			return TEXT("Unknown");
		}

		const FVector Start = SourceActor->GetActorLocation();
		const FVector End = HeroActor->GetActorLocation();
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66DamageProvenanceLOS), false);
		Params.AddIgnoredActor(Attacker);
		Params.AddIgnoredActor(DamageCauser);
		Params.AddIgnoredActor(HeroActor);

		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			return TEXT("Clear");
		}

		OutBlockerName = Hit.GetActor()
			? FString::Printf(TEXT("%s/%s"), *Hit.GetActor()->GetName(), Hit.GetActor()->GetClass() ? *Hit.GetActor()->GetClass()->GetName() : TEXT("None"))
			: FString(TEXT("WorldStatic"));
		return TEXT("Blocked");
	}
}

int32 UT66RunStateSubsystem::GetHeartDisplayTier() const
{
	int32 HighestTier = 0;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		HighestTier = FMath::Max(HighestTier, GetHeartSlotTier(SlotIndex));
	}
	return HighestTier;
}


int32 UT66RunStateSubsystem::GetHeartSlotTier(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0;
	}

	const int32 StoredTier = HeartSlotTiers.IsValidIndex(SlotIndex)
		? static_cast<int32>(HeartSlotTiers[SlotIndex])
		: 0;
	return FMath::Clamp(StoredTier, 0, MaxHeartTier);
}


float UT66RunStateSubsystem::GetHPForHeartTier(const int32 Tier)
{
	return HPPerRedHeart * FMath::Pow(HeartTierScale, static_cast<float>(FMath::Clamp(Tier, 0, MaxHeartTier)));
}


float UT66RunStateSubsystem::GetHeartSlotCapacity(const int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0.f;
	}

	return GetHPForHeartTier(GetHeartSlotTier(SlotIndex));
}


float UT66RunStateSubsystem::GetTotalHeartCapacity() const
{
	float TotalCapacity = 0.f;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		TotalCapacity += GetHeartSlotCapacity(SlotIndex);
	}
	return TotalCapacity;
}


void UT66RunStateSubsystem::ResetHeartSlotTiers()
{
	HeartSlotTiers.Init(0, DefaultMaxHearts);
}


void UT66RunStateSubsystem::SyncMaxHPToHeartTiers()
{
	const float BaseMaxHP = FMath::Max(DefaultMaxHP, GetTotalHeartCapacity());
	MaxHP = FMath::Max(1.0f, BaseMaxHP);
}


void UT66RunStateSubsystem::SyncCompatibilityHPFromHeroDamagePercent()
{
	MaxHP = FMath::Max(1.f, MaxHP);
	const float RemainingFraction = 1.f - FMath::Clamp(HeroDamagePercent / HeroDamageDeathPercent, 0.f, 1.f);
	CurrentHP = FMath::Clamp(MaxHP * RemainingFraction, 0.f, MaxHP);
}


void UT66RunStateSubsystem::SetHeroDamagePercent(const float NewPercent, const bool bBroadcast)
{
	const float ClampedPercent = FMath::Clamp(NewPercent, 0.f, HeroDamageDeathPercent);
	const bool bChanged = !FMath::IsNearlyEqual(HeroDamagePercent, ClampedPercent, 0.01f);
	HeroDamagePercent = ClampedPercent;
	SyncCompatibilityHPFromHeroDamagePercent();
	if (bBroadcast && bChanged)
	{
		HeartsChanged.Broadcast();
	}
}


float UT66RunStateSubsystem::GetHeroDamagePercentGainScale(const float PercentBeforeDamage) const
{
	const float Alpha = FMath::Clamp(PercentBeforeDamage / HeroDamageDeathPercent, 0.f, 1.f);
	return FMath::Lerp(1.f, FMath::Max(1.f, GT66DamagePercentGainScaleAt100), Alpha);
}


float UT66RunStateSubsystem::GetHeroDamageLaunchScale(const float PercentBeforeDamage) const
{
	const float Alpha = FMath::Clamp(PercentBeforeDamage / HeroDamageDeathPercent, 0.f, 1.f);
	return FMath::Lerp(1.f, FMath::Max(1.f, GT66DamagePercentLaunchScaleAt100), Alpha);
}


float UT66RunStateSubsystem::ApplyHeroDamagePercent(const float BaseDamagePercent)
{
	if (BaseDamagePercent <= 0.f)
	{
		return 0.f;
	}

	const float PreviousPercent = HeroDamagePercent;
	const float ScaledGain = BaseDamagePercent * GetHeroDamagePercentGainScale(PreviousPercent);
	SetHeroDamagePercent(PreviousPercent + ScaledGain, false);
	return FMath::Max(0.f, HeroDamagePercent - PreviousPercent);
}


float UT66RunStateSubsystem::HealHeroDamagePercent(const float PercentAmount)
{
	if (PercentAmount <= 0.f)
	{
		return 0.f;
	}

	const float PreviousPercent = HeroDamagePercent;
	SetHeroDamagePercent(PreviousPercent - PercentAmount, true);
	return FMath::Max(0.f, PreviousPercent - HeroDamagePercent);
}


void UT66RunStateSubsystem::ResetHeroDamagePercent()
{
	SetHeroDamagePercent(0.f, true);
}


void UT66RunStateSubsystem::ApplyDamagePhysicsReaction(
	AActor* Attacker,
	AActor* DamageCauser,
	APawn* HeroPawn,
	const FName SourceTag,
	const float AppliedDamagePercent,
	const float PercentBeforeDamage,
	const float PercentAfterDamage) const
{
	if (!HeroPawn)
	{
		return;
	}

	const AActor* LaunchSource = DamageCauser ? DamageCauser : Attacker;
	FVector Direction = FVector::ZeroVector;
	if (LaunchSource)
	{
		Direction = HeroPawn->GetActorLocation() - LaunchSource->GetActorLocation();
	}
	if (Direction.IsNearlyZero())
	{
		Direction = HeroPawn->GetActorForwardVector();
	}
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}

	const bool bEnemyDamageReaction = T66IsEnemyDamageReactionSource(Attacker, SourceTag);
	const float EnemyDisableStartPercent = T66GetEnemyDamageDisableStartPercent();
	const bool bEnemyBelowDisableThreshold = bEnemyDamageReaction && PercentAfterDamage <= EnemyDisableStartPercent;
	if (bEnemyBelowDisableThreshold)
	{
		const FVector RequestedVelocity =
			Direction * FMath::Max(0.f, GT66EnemyDamageKnockbackOnlyXY)
			+ FVector::UpVector * FMath::Max(0.f, GT66EnemyDamageKnockbackOnlyZ);
		bool bAppliedKnockback = false;
		if (ACharacter* HeroCharacter = Cast<ACharacter>(HeroPawn))
		{
			HeroCharacter->LaunchCharacter(RequestedVelocity, true, true);
			bAppliedKnockback = true;
		}

		UE_LOG(
			LogT66DamageReceived,
			Log,
			TEXT("[CombatDamagePhysics] Applied=%d Mode=EnemyKnockbackOnly Source=%s PercentBefore=%.2f PercentAfter=%.2f Threshold=%.2f PercentGained=%.2f RequestedVelocity=%s"),
			bAppliedKnockback ? 1 : 0,
			*SourceTag.ToString(),
			PercentBeforeDamage,
			PercentAfterDamage,
			EnemyDisableStartPercent,
			AppliedDamagePercent,
			*RequestedVelocity.ToCompactString());
		return;
	}

	UT66HeroPhysicsComponent* HeroPhysics = HeroPawn->FindComponentByClass<UT66HeroPhysicsComponent>();
	if (!HeroPhysics)
	{
		return;
	}

	const float HorizontalSpeed = FMath::Max(0.f, GT66DamagePercentBaseLaunchXY)
		+ FMath::Max(0.f, AppliedDamagePercent) * FMath::Max(0.f, GT66DamagePercentLaunchXYPerPercent);
	const float UpSpeed = FMath::Max(0.f, GT66DamagePercentBaseLaunchZ)
		+ FMath::Max(0.f, AppliedDamagePercent) * FMath::Max(0.f, GT66DamagePercentLaunchZPerPercent);
	const FVector RequestedVelocity = Direction * HorizontalSpeed + FVector::UpVector * UpSpeed;
	const float EnemyThrowAlpha = bEnemyDamageReaction ? T66GetEnemyDamageThrowAlpha(PercentAfterDamage) : 0.f;
	const float LaunchScale = bEnemyDamageReaction
		? FMath::Lerp(1.f, FMath::Max(1.f, GT66DamagePercentLaunchScaleAt100), EnemyThrowAlpha)
		: GetHeroDamageLaunchScale(PercentBeforeDamage);
	const float DurationDamagePercent = bEnemyDamageReaction ? PercentAfterDamage : PercentBeforeDamage;
	const float DurationScaleStartPercent = bEnemyDamageReaction ? EnemyDisableStartPercent : 0.f;
	const float DurationScaleFullPercent = bEnemyDamageReaction ? T66GetEnemyDamageDisableFullPercent() : -1.f;
	const bool bApplied = HeroPhysics->ApplyPhysicsReaction(
		RequestedVelocity,
		HeroPawn->GetActorLocation(),
		SourceTag,
		LaunchScale,
		DurationDamagePercent,
		DurationScaleStartPercent,
		DurationScaleFullPercent);

	UE_LOG(
		LogT66DamageReceived,
		Log,
		TEXT("[CombatDamagePhysics] Applied=%d Mode=%s Source=%s PercentBefore=%.2f PercentAfter=%.2f PercentGained=%.2f LaunchScale=%.2f ThrowAlpha=%.3f DurationScaleStart=%.2f DurationScaleFull=%.2f RequestedVelocity=%s"),
		bApplied ? 1 : 0,
		bEnemyDamageReaction ? TEXT("EnemyRagdoll") : TEXT("Ragdoll"),
		*SourceTag.ToString(),
		PercentBeforeDamage,
		PercentAfterDamage,
		AppliedDamagePercent,
		LaunchScale,
		EnemyThrowAlpha,
		DurationScaleStartPercent,
		DurationScaleFullPercent,
		*RequestedVelocity.ToCompactString());
}


int32 UT66RunStateSubsystem::FindUpgradeableHeartSlot() const
{
	int32 BestSlot = INDEX_NONE;
	int32 LowestTier = MaxHeartTier + 1;
	for (int32 SlotIndex = 0; SlotIndex < DefaultMaxHearts; ++SlotIndex)
	{
		const int32 Tier = GetHeartSlotTier(SlotIndex);
		if (Tier >= MaxHeartTier)
		{
			continue;
		}

		if (Tier < LowestTier)
		{
			LowestTier = Tier;
			BestSlot = SlotIndex;
		}
	}

	return BestSlot;
}


void UT66RunStateSubsystem::RebuildHeartSlotTiersFromMaxHP()
{
	ResetHeartSlotTiers();

	MaxHP = FMath::Max(DefaultMaxHP, MaxHP);
	float RemainingExtraHP = MaxHP - DefaultMaxHP;
	while (RemainingExtraHP > KINDA_SMALL_NUMBER)
	{
		const int32 SlotIndex = FindUpgradeableHeartSlot();
		if (SlotIndex == INDEX_NONE)
		{
			break;
		}

		const int32 CurrentTier = GetHeartSlotTier(SlotIndex);
		const float UpgradeDelta = GetHPForHeartTier(CurrentTier + 1) - GetHPForHeartTier(CurrentTier);
		if (RemainingExtraHP + 0.01f < UpgradeDelta)
		{
			break;
		}

		HeartSlotTiers[SlotIndex] = static_cast<uint8>(CurrentTier + 1);
		RemainingExtraHP -= UpgradeDelta;
	}

	SyncMaxHPToHeartTiers();
	SyncCompatibilityHPFromHeroDamagePercent();
}


float UT66RunStateSubsystem::GetHeartSlotFill(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DefaultMaxHearts)
	{
		return 0.f;
	}

	float SlotStart = 0.f;
	for (int32 Index = 0; Index < SlotIndex; ++Index)
	{
		SlotStart += GetHeartSlotCapacity(Index);
	}

	const float SlotCapacity = GetHeartSlotCapacity(SlotIndex);
	if (SlotCapacity <= 0.f)
	{
		return 0.f;
	}

	const float SlotEnd = SlotStart + SlotCapacity;
	if (CurrentHP <= SlotStart) return 0.f;
	if (CurrentHP >= SlotEnd) return 1.f;
	return (CurrentHP - SlotStart) / SlotCapacity;
}


void UT66RunStateSubsystem::AddMaxHearts(int32 DeltaHearts)
{
	if (DeltaHearts <= 0)
	{
		return;
	}

	if (HeartSlotTiers.Num() != DefaultMaxHearts)
	{
		RebuildHeartSlotTiersFromMaxHP();
	}

	bool bUpgraded = false;
	for (int32 UpgradeIndex = 0; UpgradeIndex < DeltaHearts; ++UpgradeIndex)
	{
		const int32 SlotIndex = FindUpgradeableHeartSlot();
		if (SlotIndex == INDEX_NONE)
		{
			break;
		}

		HeartSlotTiers[SlotIndex] = static_cast<uint8>(GetHeartSlotTier(SlotIndex) + 1);
		bUpgraded = true;
	}

	if (!bUpgraded)
	{
		return;
	}

	SyncMaxHPToHeartTiers();
	SyncCompatibilityHPFromHeroDamagePercent();
	HeartsChanged.Broadcast();
}


float UT66RunStateSubsystem::ApplyAutomationHeroHPOverride(const float RequestedHP, const TCHAR* Reason)
{
	// Automation-only HP override; cap is high enough for stationary performance captures
	// to outlast saturated ranged projectile pressure that real gameplay would dodge.
	static constexpr float MaxAutomationHeroHPOverride = 50000.f;
	if (RequestedHP <= 0.f)
	{
		return CurrentHP;
	}

	const float AppliedHP = FMath::Clamp(RequestedHP, 1.f, MaxAutomationHeroHPOverride);
	if (!FMath::IsNearlyEqual(AppliedHP, RequestedHP))
	{
		UE_LOG(
			LogT66RunState,
			Warning,
			TEXT("[PerfAutomation] Requested hero HP override %.1f exceeded packet cap %.1f; applying %.1f. Reason=%s"),
			RequestedHP,
			MaxAutomationHeroHPOverride,
			AppliedHP,
			Reason ? Reason : TEXT("Unknown"));
	}

	MaxHP = AppliedHP;
	SetHeroDamagePercent(0.f, false);
	if (HeartSlotTiers.Num() != DefaultMaxHearts)
	{
		ResetHeartSlotTiers();
	}
	for (uint8& Tier : HeartSlotTiers)
	{
		Tier = static_cast<uint8>(MaxHeartTier);
	}
	HeartsChanged.Broadcast();
	return AppliedHP;
}


void UT66RunStateSubsystem::NotifyEnemyKilledByHero()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	const double Now = World->GetTimeSeconds();

	if (PassiveType == ET66PassiveType::RallyingBlow)
	{
		RallyStacks = FMath::Min(3, RallyStacks + 1);
		RallyTimerEndWorldTime = Now + 3.0;
	}

	if (PassiveType == ET66PassiveType::ChaosTheory)
	{
		ChaosTheoryBounceStacks = FMath::Min(3, ChaosTheoryBounceStacks + 1);
		ChaosTheoryTimerEndWorldTime = Now + 5.0;
	}

}


float UT66RunStateSubsystem::GetRallyAttackSpeedMultiplier() const
{
	if (PassiveType != ET66PassiveType::RallyingBlow || RallyStacks <= 0) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World || World->GetTimeSeconds() >= RallyTimerEndWorldTime) return 1.f;
	return 1.f + 0.15f * static_cast<float>(RallyStacks);
}


float UT66RunStateSubsystem::GetQuickDrawDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::QuickDraw) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return 1.f;
	const double Now = World->GetTimeSeconds();
	return (Now - LastAttackFireWorldTime >= 2.0) ? 2.f : 1.f;
}


void UT66RunStateSubsystem::NotifyAttackFired()
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	LastAttackFireWorldTime = World->GetTimeSeconds();
	if (PassiveType == ET66PassiveType::Overclock)
		OverclockAttackCounter++;
}


bool UT66RunStateSubsystem::ShouldOverclockDouble() const
{
	if (PassiveType != ET66PassiveType::Overclock) return false;
	return (OverclockAttackCounter % 8) == 0 && OverclockAttackCounter > 0;
}


int32 UT66RunStateSubsystem::GetChaosTheoryBonusBounceCount() const
{
	if (PassiveType != ET66PassiveType::ChaosTheory || ChaosTheoryBounceStacks <= 0) return 0;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World || World->GetTimeSeconds() >= ChaosTheoryTimerEndWorldTime) return 0;
	return ChaosTheoryBounceStacks;
}


float UT66RunStateSubsystem::GetEnduranceAttackSpeedMultiplier() const
{
	if (PassiveType != ET66PassiveType::Endurance) return 1.f;
	return (HeroDamagePercent >= 70.f && HeroDamagePercent < HeroDamageDeathPercent) ? 2.f : 1.f;
}


float UT66RunStateSubsystem::GetEnduranceDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::Endurance) return 1.f;
	return (HeroDamagePercent >= 70.f && HeroDamagePercent < HeroDamageDeathPercent) ? 1.25f : 1.f;
}


float UT66RunStateSubsystem::GetBrawlersFuryDamageMultiplier() const
{
	if (PassiveType != ET66PassiveType::BrawlersFury) return 1.f;
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return 1.f;
	return (World->GetTimeSeconds() < BrawlersFuryEndWorldTime) ? 1.3f : 1.f;
}


float UT66RunStateSubsystem::GetTreasureHunterGoldMultiplier() const
{
	return (PassiveType == ET66PassiveType::TreasureHunter) ? 1.25f : 1.f;
}


void UT66RunStateSubsystem::NotifyEvasionProc()
{
	if (PassiveType == ET66PassiveType::Evasive)
		bEvasiveNextAttackBonusDOT = true;
}


bool UT66RunStateSubsystem::ConsumeEvasiveBonusDOT()
{
	if (PassiveType != ET66PassiveType::Evasive || !bEvasiveNextAttackBonusDOT) return false;
	bEvasiveNextAttackBonusDOT = false;
	return true;
}


void UT66RunStateSubsystem::ApplyStageSpeedBoost(float MoveSpeedMultiplier, float DurationSeconds)
{
	const float Mult = FMath::Clamp(MoveSpeedMultiplier, 0.25f, 5.f);
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;

	StageMoveSpeedMultiplier = FMath::Max(StageMoveSpeedMultiplier, Mult);
	StageMoveSpeedSecondsRemaining = FMath::Max(StageMoveSpeedSecondsRemaining, Dur);
	HeroProgressChanged.Broadcast(); // movement uses derived stat refresh
}


void UT66RunStateSubsystem::ApplyTemporaryBaseStatAmplifier(
	const ET66HeroStatType StatType,
	const int32 BonusStatPoints,
	const float DurationSeconds)
{
	if (StatType == ET66HeroStatType::Special || BonusStatPoints <= 0 || DurationSeconds <= 0.f)
	{
		return;
	}

	FT66TemporaryBaseStatAmplifier& Amplifier = TemporaryBaseStatAmplifiers.AddDefaulted_GetRef();
	Amplifier.StatType = StatType;
	Amplifier.BonusTenths = WholeStatToTenths(FMath::Max(1, BonusStatPoints));
	Amplifier.SecondsRemaining = FMath::Max(0.1f, DurationSeconds);
	HeroProgressChanged.Broadcast();
}

void UT66RunStateSubsystem::ApplyTemporaryStatAmplifier(
	const ET66StatType StatType,
	const int32 BonusStatPoints,
	const float DurationSeconds)
{
	if (!T66IsLiveStatType(StatType) || BonusStatPoints <= 0 || DurationSeconds <= 0.f)
	{
		return;
	}

	FT66TemporaryStatAmplifier& Amplifier = TemporaryStatAmplifiers.AddDefaulted_GetRef();
	Amplifier.StatType = StatType;
	Amplifier.BonusTenths = WholeStatToTenths(FMath::Max(1, BonusStatPoints));
	Amplifier.SecondsRemaining = FMath::Max(0.1f, DurationSeconds);
	HeroProgressChanged.Broadcast();
}


bool UT66RunStateSubsystem::ApplyTrueDamage(int32 /*DamageHP*/)
{
	return false;
}


void UT66RunStateSubsystem::ApplyStatusBurn(float /*DurationSeconds*/, float /*DamagePerSecond*/) {}


void UT66RunStateSubsystem::ApplyStatusChill(float /*DurationSeconds*/, float /*MoveSpeedMultiplier*/) {}


void UT66RunStateSubsystem::ApplyStatusCurse(float /*DurationSeconds*/) {}


bool UT66RunStateSubsystem::TryActivateUltimate()
{
	return false;
}

void UT66RunStateSubsystem::AddUltimateCharge(const float Amount)
{
	static_cast<void>(Amount);
}

void UT66RunStateSubsystem::TickHeroTimers(float DeltaTime)
{
	if (bBackroomsGameplayPaused)
	{
		return;
	}

	// HP regen (numerical)
	ApplyHpRegen(DeltaTime);

	// Stage speed boost timer
	if (StageMoveSpeedSecondsRemaining > 0.f)
	{
		StageMoveSpeedSecondsRemaining = FMath::Max(0.f, StageMoveSpeedSecondsRemaining - DeltaTime);
		if (StageMoveSpeedSecondsRemaining <= 0.f)
		{
			StageMoveSpeedMultiplier = 1.f;
			HeroProgressChanged.Broadcast();
		}
	}

	bool bAmplifiersChanged = false;
	for (int32 Index = TemporaryBaseStatAmplifiers.Num() - 1; Index >= 0; --Index)
	{
		FT66TemporaryBaseStatAmplifier& Amplifier = TemporaryBaseStatAmplifiers[Index];
		Amplifier.SecondsRemaining = FMath::Max(0.f, Amplifier.SecondsRemaining - DeltaTime);
		if (Amplifier.SecondsRemaining <= 0.f)
		{
			TemporaryBaseStatAmplifiers.RemoveAtSwap(Index, 1, EAllowShrinking::No);
			bAmplifiersChanged = true;
		}
	}
	for (int32 Index = TemporaryStatAmplifiers.Num() - 1; Index >= 0; --Index)
	{
		FT66TemporaryStatAmplifier& Amplifier = TemporaryStatAmplifiers[Index];
		Amplifier.SecondsRemaining = FMath::Max(0.f, Amplifier.SecondsRemaining - DeltaTime);
		if (Amplifier.SecondsRemaining <= 0.f)
		{
			TemporaryStatAmplifiers.RemoveAtSwap(Index, 1, EAllowShrinking::No);
			bAmplifiersChanged = true;
		}
	}
	if (bAmplifiersChanged)
	{
		HeroProgressChanged.Broadcast();
	}

	// Status effects removed — enemies no longer apply Burn/Chill/Curse.
}

void UT66RunStateSubsystem::SetBackroomsGameplayPaused(const bool bPaused)
{
	if (bBackroomsGameplayPaused == bPaused)
	{
		return;
	}

	bBackroomsGameplayPaused = bPaused;
}


bool UT66RunStateSubsystem::IsBossDamageSource(const AActor* Attacker)
{
	return Cast<AT66BossBase>(Attacker) != nullptr;
}


void UT66RunStateSubsystem::HandleLethalDamage(AActor* Attacker, const FName DeliveryMethod)
{
	const bool bBackroomsChaserTouch = DeliveryMethod == T66BackroomsChaserTouchDelivery;
	const bool bOuroborosLethalZone = DeliveryMethod == T66OuroborosLethalZoneDelivery;
	const bool bBypassesQuickRevive = bBackroomsChaserTouch || bOuroborosLethalZone;
	// Dev Immortality: never end the run.
	if (bDevImmortality)
	{
		SetHeroDamagePercent(FMath::Min(HeroDamagePercent, HeroDamageDeathPercent - 0.1f), false);
		LastDamageTime = -9999.f;
		HeartsChanged.Broadcast();
		return;
	}

	if (!bBypassesQuickRevive && ConsumeBackroomsQuickReviveItem())
	{
		ResetHeroDamagePercent();
		if (UWorld* World = GetWorld())
		{
			LastDamageTime = static_cast<float>(World->GetTimeSeconds());
		}
		else
		{
			LastDamageTime = 0.f;
		}
		return;
	}

	SetHeroDamagePercent(HeroDamageDeathPercent, false);
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
}


bool UT66RunStateSubsystem::ApplyDamage(int32 DamageHP, AActor* Attacker, const FName DeliveryMethod, AActor* DamageCauser)
{
	if (DamageHP <= 0) return false;
	const int32 RequestedDamageHP = DamageHP;
	const bool bBackroomsChaserTouch = DeliveryMethod == T66BackroomsChaserTouchDelivery;

	if (UGameInstance* GIForBackrooms = GetGameInstance())
	{
		if (UWorld* WorldForBackrooms = GIForBackrooms->GetWorld())
		{
			if (AT66GameMode* GameMode = WorldForBackrooms->GetAuthGameMode<AT66GameMode>())
			{
				if (GameMode->IsBackroomsChallengeActive() && !bBackroomsChaserTouch)
				{
					return false;
				}
			}
		}
	}

	if (Cast<AT66TrapBase>(Attacker) == nullptr && (Cast<AT66EnemyBase>(Attacker) || Cast<AT66BossBase>(Attacker)))
	{
		DamageHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageHP) * FMath::Max(0.1f, ActiveRunModifiers.EnemyDamageMultiplier)));
	}

	// Iron Will: flat damage reduction before armor.
	if (PassiveType == ET66PassiveType::IronWill)
	{
		const int32 FlatReduction = GetArmorStat() * 2;
		DamageHP = FMath::Max(1, DamageHP - FlatReduction);
	}

	// Unflinching: permanent 15% damage reduction.
	if (PassiveType == ET66PassiveType::Unflinching)
	{
		DamageHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageHP) * 0.85f));
	}

	// Evasion: dodge the entire hit. On dodge: Assassinate (OHKO) and CounterAttack (deal fraction of would-be damage to attacker).
	UT66RngSubsystem* RngSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
	const float Evade = GetEvasionChance01();
	AntiCheatIncomingHitChecks = FMath::Clamp(AntiCheatIncomingHitChecks + 1, 0, 1000000);
	AntiCheatTotalEvasionChance = FMath::Clamp(AntiCheatTotalEvasionChance + FMath::Clamp(Evade, 0.f, 1.f), 0.f, 1000000.f);
	const bool bDodged = !bBackroomsChaserTouch && Evade > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < Evade) : (FMath::FRand() < Evade));
	if (bDodged)
	{
		RecordAntiCheatHitCheckEvent(Evade, true, false);
		AntiCheatDodgeCount = FMath::Clamp(AntiCheatDodgeCount + 1, 0, 1000000);
		AntiCheatCurrentConsecutiveDodges = FMath::Clamp(AntiCheatCurrentConsecutiveDodges + 1, 0, 1000000);
		AntiCheatMaxConsecutiveDodges = FMath::Max(AntiCheatMaxConsecutiveDodges, AntiCheatCurrentConsecutiveDodges);
		UWorld* DodgeWorld = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
		if (DodgeWorld)
		{
			if (APawn* HeroPawn = DodgeWorld->GetFirstPlayerController() ? DodgeWorld->GetFirstPlayerController()->GetPawn() : nullptr)
			{
				if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FloatingText->ShowStatusEvent(HeroPawn, UT66FloatingCombatTextSubsystem::EventType_Dodge);
				}
			}
		}
		if (Attacker)
		{
			const float AssassinateChance = GetAssassinateChance01();
			if (AssassinateChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < AssassinateChance) : (FMath::FRand() < AssassinateChance)))
			{
				T66CombatShared::TryApplyNonBossOHKO(Attacker, nullptr, FName(TEXT("Assassinate")), NAME_None);
			}
			const float CounterChance = FMath::Clamp(GetCounterAttackFraction(), 0.f, 1.f);
			if (CounterChance > 0.f && DamageHP > 0 && (RngSub ? (RngSub->GetRunStream().GetFraction() < CounterChance) : (FMath::FRand() < CounterChance)))
			{
				const int32 CounterDmg = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageHP) * 0.5f));
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker)) { if (E->CurrentHP > 0) E->TakeDamageFromHero(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66VendorBoss* GB = Cast<AT66VendorBoss>(Attacker)) { if (GB->CurrentHP > 0) GB->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker)) { if (B->IsAwakened() && B->IsAlive()) B->TakeDamageFromHeroHit(CounterDmg, FName(TEXT("CounterAttack")), NAME_None); }
				if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
				{
					FloatingText->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_CounterAttack);
				}
			}
		}
		NotifyEvasionProc();
		return false;
	}
	// Armor: reduce the hit (still at least 1 HP if hit > 0).
	const float Armor = GetArmorReduction01();
	const float Reduced = static_cast<float>(FMath::Max(1, FMath::CeilToInt(static_cast<float>(DamageHP) * (1.f - Armor))));
	const int32 IncomingDamageHP = DamageHP;

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	APawn* HeroPawn = nullptr;
	FString HeroLocation(TEXT("None"));
	AT66HeroBase* HeroActor = nullptr;
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			HeroPawn = PC->GetPawn();
			HeroActor = Cast<AT66HeroBase>(HeroPawn);
			if (HeroPawn)
			{
				HeroLocation = HeroPawn->GetActorLocation().ToCompactString();
			}
		}
	}

	const bool bWithinDamageInvuln = !bBackroomsChaserTouch && Now - LastDamageTime < InvulnDurationSeconds;
	const bool bBypassedDamageInvulnForKnockback = bWithinDamageInvuln && HeroActor && HeroActor->IsKnockbackActive();
	if (bWithinDamageInvuln && !bBypassedDamageInvulnForKnockback)
	{
		// Any resolved non-dodge hit check breaks the consecutive dodge streak,
		// even when invulnerability prevents damage from being applied.
		AntiCheatCurrentConsecutiveDodges = 0;
		RecordAntiCheatHitCheckEvent(Evade, false, false);
		return false;
	}

	// Reflect: % chance to reflect; when it procs, 50% of reduced damage back to the attacker. Crush: chance to OHKO when reflect fires.
	if (Attacker && Reduced > 0.f)
	{
		const float ReflectChance = FMath::Clamp(GetReflectDamageFraction(), 0.f, 1.f);
		if (ReflectChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < ReflectChance) : (FMath::FRand() < ReflectChance)))
		{
			const int32 ReflectedAmount = FMath::Max(1, FMath::RoundToInt(Reduced * 0.5f));
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Attacker))
			{
				if (E->CurrentHP > 0) E->TakeDamageFromHero(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66VendorBoss* GB = Cast<AT66VendorBoss>(Attacker))
			{
				if (GB->CurrentHP > 0) GB->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Attacker))
			{
				if (B->IsAwakened() && B->IsAlive()) B->TakeDamageFromHeroHit(ReflectedAmount, FName(TEXT("Reflect")), NAME_None);
			}
			if (UGameInstance* ReflectGI = GetGameInstance())
			{
				if (UT66FloatingCombatTextSubsystem* FloatingText = ReflectGI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
				{
					FloatingText->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_Reflect);
				}
			}

			const float CrushChance = GetCrushChance01();
			if (CrushChance > 0.f && (RngSub ? (RngSub->GetRunStream().GetFraction() < CrushChance) : (FMath::FRand() < CrushChance)))
			{
				T66CombatShared::TryApplyNonBossOHKO(Attacker, nullptr, FName(TEXT("Crush")), NAME_None);
				if (UGameInstance* CrushGI = GetGameInstance())
				{
						if (UT66FloatingCombatTextSubsystem* CrushFloating = CrushGI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
					{
						CrushFloating->ShowStatusEvent(Attacker, UT66FloatingCombatTextSubsystem::EventType_Crush);
					}
				}
			}
		}
	}

	LastDamageTime = Now;
	const float PreviousHP = CurrentHP;
	const float PreviousDamagePercent = HeroDamagePercent;
	const float AppliedDamagePercent = ApplyHeroDamagePercent(Reduced);
	const int32 AppliedDamagePercentRounded = FMath::Max(1, FMath::RoundToInt(AppliedDamagePercent));
	const FName DamageSourceID = T66ResolveDamageReceivedSourceID(Attacker);
	if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageLog->RecordDamageReceived(DamageSourceID, AppliedDamagePercentRounded);
	}

	AActor* ResolvedDamageCauser = DamageCauser ? DamageCauser : Attacker;
	const FName ResolvedDeliveryMethod = T66ResolveDeliveryMethod(Attacker, DeliveryMethod);
	FString LOSBlockerName;
	const FString LOSStatus = T66ResolveLineOfSightStatus(
		World,
		ResolvedDamageCauser,
		HeroPawn,
		Attacker,
		ResolvedDamageCauser,
		LOSBlockerName);
	const float SourceDist2D = T66Distance2DOrNegative(Attacker, HeroPawn);
	const float SourceDist3D = T66Distance3DOrNegative(Attacker, HeroPawn);
	const float CauserDist2D = T66Distance2DOrNegative(ResolvedDamageCauser, HeroPawn);
	const float CauserDist3D = T66Distance3DOrNegative(ResolvedDamageCauser, HeroPawn);
	ApplyDamagePhysicsReaction(Attacker, ResolvedDamageCauser, HeroPawn, ResolvedDeliveryMethod, AppliedDamagePercent, PreviousDamagePercent, HeroDamagePercent);

	UE_LOG(
		LogT66DamageReceived,
		Log,
		TEXT("[CombatDamage] AppliedPercent=%.2f Percent=%.2f->%.2f RequestedHP=%d IncomingHP=%d ReducedBaseHP=%.2f BypassedInvulnForKnockback=%d SourceID=%s Delivery=%s SourceActor=%s SourceClass=%s DamageCauser=%s CauserClass=%s HeroHP=%.1f->%.1f MaxHP=%.1f HeroLoc=%s SourceLoc=%s CauserLoc=%s SourceDist2D=%.1f SourceDist3D=%.1f CauserDist2D=%.1f CauserDist3D=%.1f CauserLOS=%s LOSBlocker=%s Stage=%d WorldTime=%.2f"),
		AppliedDamagePercent,
		PreviousDamagePercent,
		HeroDamagePercent,
		RequestedDamageHP,
		IncomingDamageHP,
		Reduced,
		bBypassedDamageInvulnForKnockback ? 1 : 0,
		*DamageSourceID.ToString(),
		*ResolvedDeliveryMethod.ToString(),
		*T66FormatDamageActorName(Attacker),
		*T66FormatDamageActorClass(Attacker),
		*T66FormatDamageActorName(ResolvedDamageCauser),
		*T66FormatDamageActorClass(ResolvedDamageCauser),
		PreviousHP,
		CurrentHP,
		MaxHP,
		*HeroLocation,
		*T66FormatDamageActorLocation(Attacker),
		*T66FormatDamageActorLocation(ResolvedDamageCauser),
		SourceDist2D,
		SourceDist3D,
		CauserDist2D,
		CauserDist3D,
		*LOSStatus,
		*LOSBlockerName,
		GetCurrentStage(),
		Now);

	UT66AudioSubsystem::PlayEventFromWorldContext(World, FName(TEXT("Hero.Damage")), FVector::ZeroVector, nullptr);
	AntiCheatDamageTakenHitCount = FMath::Clamp(AntiCheatDamageTakenHitCount + 1, 0, 1000000);
	AntiCheatCurrentConsecutiveDodges = 0;
	RecordAntiCheatHitCheckEvent(Evade, false, true);

	// BrawlersFury: taking damage triggers +30% damage dealt for 3s.
	if (PassiveType == ET66PassiveType::BrawlersFury && World)
	{
		BrawlersFuryEndWorldTime = World->GetTimeSeconds() + 3.0;
	}

	if (HeroPawn)
	{
		if (UT66FloatingCombatTextSubsystem* FCT = GI ? GI->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
		{
			FCT->ShowDamageTaken(HeroPawn, AppliedDamagePercentRounded);
		}
	}

	HeartsChanged.Broadcast();

	if (HeroDamagePercent >= HeroDamageDeathPercent)
	{
		HandleLethalDamage(Attacker, ResolvedDeliveryMethod);
	}
	return true;
}


void UT66RunStateSubsystem::HealToFull()
{
	ResetHeroDamagePercent();
}


void UT66RunStateSubsystem::HealHP(float Amount)
{
	if (Amount <= 0.f) return;

	HealHeroDamagePercent(Amount);
}


void UT66RunStateSubsystem::HealHPFromCompanion(float Amount)
{
	if (Amount <= 0.f) return;

	const float PreviousPercent = HeroDamagePercent;
	HealHP(Amount);
	const float AppliedHealing = FMath::Max(0.f, PreviousPercent - HeroDamagePercent);
	if (AppliedHealing > 0.f)
	{
		CompanionHealingDoneThisRun += AppliedHealing;
	}
}


void UT66RunStateSubsystem::HealHearts(int32 Hearts)
{
	if (Hearts <= 0) return;
	HealHP(static_cast<float>(Hearts) * HPPerRedHeart);
}


void UT66RunStateSubsystem::ApplyHpRegen(float DeltaTime)
{
	const float Rate = GetHpRegenPerSecond();
	if (Rate <= 0.f || DeltaTime <= 0.f) return;

	const float Healed = Rate * DeltaTime;
	HealHP(Healed);
}


void UT66RunStateSubsystem::KillPlayer(const FName DeliveryMethod)
{
	SetHeroDamagePercent(HeroDamageDeathPercent, false);
	HandleLethalDamage(nullptr, DeliveryMethod);
}
