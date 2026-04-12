// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatShared.h"

#include "Components/CapsuleComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
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

	float GetCategorySubScaleMultiplier(const UT66RunStateSubsystem* RunState, const ET66AttackCategory Category)
	{
		if (!RunState)
		{
			return 1.f;
		}

		const ET66SecondaryStatType StatType = GetScaleSecondaryForCategory(Category);
		const float Baseline = RunState->GetSecondaryStatBaselineValue(StatType);
		const float HeroScaleMult = FMath::Max(0.01f, RunState->GetHeroScaleMultiplier());
		const float Value = RunState->GetSecondaryStatValue(StatType);
		if (Baseline <= KINDA_SMALL_NUMBER)
		{
			return 1.f;
		}

		return FMath::Clamp(Value / (Baseline * HeroScaleMult), 0.25f, 5.f);
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
