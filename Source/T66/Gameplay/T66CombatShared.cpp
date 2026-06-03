// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatShared.h"

#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobBase.h"

namespace
{
	ET66SecondaryStatType GetDamageSecondaryForCategory(const ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::AOE:    return ET66SecondaryStatType::AoeDamage;
		case ET66AttackCategory::Bounce: return ET66SecondaryStatType::BounceDamage;
		case ET66AttackCategory::Pierce: return ET66SecondaryStatType::PierceDamage;
		case ET66AttackCategory::DOT:    return ET66SecondaryStatType::DotDamage;
		default:                         return ET66SecondaryStatType::PierceDamage;
		}
	}

	ET66SecondaryStatType GetAttackSpeedSecondaryForCategory(const ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::AOE:    return ET66SecondaryStatType::AoeSpeed;
		case ET66AttackCategory::Bounce: return ET66SecondaryStatType::BounceSpeed;
		case ET66AttackCategory::Pierce: return ET66SecondaryStatType::PierceSpeed;
		case ET66AttackCategory::DOT:    return ET66SecondaryStatType::DotSpeed;
		default:                         return ET66SecondaryStatType::PierceSpeed;
		}
	}

	ET66SecondaryStatType GetScaleSecondaryForCategory(const ET66AttackCategory Category)
	{
		switch (Category)
		{
		case ET66AttackCategory::AOE:    return ET66SecondaryStatType::AoeScale;
		case ET66AttackCategory::Bounce: return ET66SecondaryStatType::BounceScale;
		case ET66AttackCategory::Pierce: return ET66SecondaryStatType::PierceScale;
		case ET66AttackCategory::DOT:    return ET66SecondaryStatType::DotScale;
		default:                         return ET66SecondaryStatType::AttackRange;
		}
	}

	float GetCategorySubMultiplier(
		const UT66RunStateSubsystem* RunState,
		const ET66SecondaryStatType StatType,
		const float HeroMultiplier,
		const float MaxMultiplier)
	{
		if (!RunState)
		{
			return 1.f;
		}

		const float Baseline = RunState->GetSecondaryStatBaselineValue(StatType);
		if (Baseline <= KINDA_SMALL_NUMBER)
		{
			return 1.f;
		}

		const float Value = RunState->GetSecondaryStatValue(StatType);
		return FMath::Clamp(Value / (Baseline * FMath::Max(0.01f, HeroMultiplier)), 0.25f, MaxMultiplier);
	}

	int32 ResolveOHKODamage(const float CurrentHP, const float MaxHP)
	{
		return FMath::Max(1, FMath::CeilToInt(CurrentHP + MaxHP + 1.f));
	}
}

namespace T66CombatShared
{
	const TCHAR* GetItemRarityName(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return TEXT("Black");
		case ET66ItemRarity::Red:    return TEXT("Red");
		case ET66ItemRarity::Yellow: return TEXT("Yellow");
		case ET66ItemRarity::White:  return TEXT("White");
		default:                     return TEXT("Unknown");
		}
	}

	const TSet<FName>& GetImpactPresentationProofIdols()
	{
		// 4x4 grid proof idols: Ice AOE, Electricity Pierce/Bounce, Nature DOT.
		// Idol_Nature_AOE is intentionally absent (neutral control: it must not enter the lane).
		static const TSet<FName> ImpactPresentationProofIdols = {
			FName(TEXT("Idol_Ice_AOE")),
			FName(TEXT("Idol_Electricity_Pierce")),
			FName(TEXT("Idol_Electricity_Bounce")),
			FName(TEXT("Idol_Nature_DOT")),
		};
		return ImpactPresentationProofIdols;
	}

	const TSet<FName>& GetSupportedProofIdols()
	{
		// Impact-presentation proof idols plus the full 4x4 Stream B traveler grid.
		static const TSet<FName> SupportedProofIdols = []()
		{
			TSet<FName> Set = GetImpactPresentationProofIdols();
			Set.Add(FName(TEXT("Idol_Fire_AOE")));
			Set.Add(FName(TEXT("Idol_Fire_Pierce")));
			Set.Add(FName(TEXT("Idol_Fire_Bounce")));
			Set.Add(FName(TEXT("Idol_Fire_DOT")));
			Set.Add(FName(TEXT("Idol_Ice_AOE")));
			Set.Add(FName(TEXT("Idol_Ice_Pierce")));
			Set.Add(FName(TEXT("Idol_Ice_Bounce")));
			Set.Add(FName(TEXT("Idol_Ice_DOT")));
			Set.Add(FName(TEXT("Idol_Electricity_AOE")));
			Set.Add(FName(TEXT("Idol_Electricity_Pierce")));
			Set.Add(FName(TEXT("Idol_Electricity_Bounce")));
			Set.Add(FName(TEXT("Idol_Electricity_DOT")));
			Set.Add(FName(TEXT("Idol_Nature_AOE")));
			Set.Add(FName(TEXT("Idol_Nature_Pierce")));
			Set.Add(FName(TEXT("Idol_Nature_Bounce")));
			Set.Add(FName(TEXT("Idol_Nature_DOT")));
			return Set;
		}();
		return SupportedProofIdols;
	}

	float GetCategorySubDamageMultiplier(const UT66RunStateSubsystem* RunState, const ET66AttackCategory Category)
	{
		return GetCategorySubMultiplier(
			RunState,
			GetDamageSecondaryForCategory(Category),
			RunState ? RunState->GetHeroDamageMultiplier() : 1.f,
			5.f);
	}

	float GetCategorySubAttackSpeedMultiplier(const UT66RunStateSubsystem* RunState, const ET66AttackCategory Category)
	{
		return GetCategorySubMultiplier(
			RunState,
			GetAttackSpeedSecondaryForCategory(Category),
			RunState ? RunState->GetHeroAttackSpeedMultiplier() : 1.f,
			5.f);
	}

	float GetCategorySubScaleMultiplier(const UT66RunStateSubsystem* RunState, const ET66AttackCategory Category)
	{
		return GetCategorySubMultiplier(
			RunState,
			GetScaleSecondaryForCategory(Category),
			RunState ? RunState->GetHeroScaleMultiplier() : 1.f,
			5.f);
	}

	ET66SecondaryStatType GetElementPowerSecondaryForIdolElement(const ET66IdolElement Element)
	{
		return T66GetElementPowerStatType(Element);
	}

	float GetIdolElementPowerMultiplier(const UT66RunStateSubsystem* RunState, const ET66IdolElement Element)
	{
		return RunState
			? FMath::Max(0.1f, RunState->GetSecondaryStatValue(GetElementPowerSecondaryForIdolElement(Element)))
			: 1.f;
	}

	float GetIdolRarityVisualScale(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return 1.00f;
		case ET66ItemRarity::Red:    return 1.18f;
		case ET66ItemRarity::Yellow: return 1.38f;
		case ET66ItemRarity::White:  return 1.60f;
		default:                     return 1.00f;
		}
	}

	float GetIdolRarityVisualQuantity(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return 1.00f;
		case ET66ItemRarity::Red:    return 1.35f;
		case ET66ItemRarity::Yellow: return 1.75f;
		case ET66ItemRarity::White:  return 2.20f;
		default:                     return 1.00f;
		}
	}

	bool IsHeroHurtboxComponent(const AT66HeroBase* Hero, const UPrimitiveComponent* Component)
	{
		return Hero && Component && Component == Hero->GetCapsuleComponent();
	}

	bool TryApplyNonBossOHKO(AActor* TargetActor, const FT66CombatTargetHandle* TargetHandle, const FName DamageSourceID, const FName EventType)
	{
		if (!TargetActor || Cast<AT66BossBase>(TargetActor))
		{
			return false;
		}

		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(TargetActor))
		{
			if (Enemy->CurrentHP <= 0)
			{
				return false;
			}

			const int32 LethalDamage = ResolveOHKODamage(static_cast<float>(Enemy->CurrentHP), static_cast<float>(Enemy->MaxHP));
			const FT66CombatTargetHandle ResolvedHandle = (TargetHandle && TargetHandle->IsValid())
				? *TargetHandle
				: Enemy->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
			if (ResolvedHandle.IsValid())
			{
				return Enemy->TakeDamageFromHeroHitZone(LethalDamage, ResolvedHandle, DamageSourceID, EventType);
			}

			return Enemy->TakeDamageFromHero(LethalDamage, DamageSourceID, EventType);
		}

		if (AT66MobBase* Mob = Cast<AT66MobBase>(TargetActor))
		{
			if (Mob->CurrentHP <= 0.f)
			{
				return false;
			}

			const FT66CombatTargetHandle ResolvedHandle = (TargetHandle && TargetHandle->IsValid())
				? *TargetHandle
				: Mob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
			if (!ResolvedHandle.IsValid())
			{
				return false;
			}

			const int32 LethalDamage = ResolveOHKODamage(Mob->CurrentHP, Mob->MaxHP);
			return Mob->TakeDamageFromHeroHitZone(LethalDamage, ResolvedHandle, DamageSourceID, EventType);
		}

		return false;
	}

	FString DescribePrimitiveComponentForCombatLog(const UPrimitiveComponent* Component)
	{
		if (!Component)
		{
			return TEXT("None");
		}

		const AActor* Owner = Component->GetOwner();
		return FString::Printf(
			TEXT("%s.%s/%s Collision=%d ObjectType=%d"),
			Owner ? *Owner->GetName() : TEXT("NoOwner"),
			*Component->GetName(),
			Component->GetClass() ? *Component->GetClass()->GetName() : TEXT("None"),
			static_cast<int32>(Component->GetCollisionEnabled()),
			static_cast<int32>(Component->GetCollisionObjectType()));
	}

	FVector ResolveGroundAnchor(UWorld* World, const FVector& ApproxLocation, const AActor* IgnoreActor)
	{
		if (!World)
		{
			return ApproxLocation;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66IdolVFXGroundAnchor), false, IgnoreActor);
		FHitResult Hit;
		const FVector Start = ApproxLocation + FVector(0.f, 0.f, 400.f);
		const FVector End = ApproxLocation - FVector(0.f, 0.f, 1400.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) ||
			World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			return Hit.ImpactPoint;
		}

		return ApproxLocation;
	}

	FVector ResolveProcVFXAnchor(AActor* TargetActor, UWorld* World)
	{
		if (!TargetActor)
		{
			return FVector::ZeroVector;
		}

		FVector ApproxLocation = TargetActor->GetActorLocation();
		if (const UCapsuleComponent* Capsule = TargetActor->FindComponentByClass<UCapsuleComponent>())
		{
			ApproxLocation.Z -= Capsule->GetScaledCapsuleHalfHeight();
		}

		return ResolveGroundAnchor(World, ApproxLocation, TargetActor);
	}
}
