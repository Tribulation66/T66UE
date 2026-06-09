// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class AActor;
class AT66HeroBase;
struct FT66CombatTargetHandle;
class UT66RunStateSubsystem;
class UPrimitiveComponent;
class UWorld;

namespace T66CombatShared
{
	const TCHAR* GetItemRarityName(ET66ItemRarity Rarity);
	float GetCategorySubDamageMultiplier(const UT66RunStateSubsystem* RunState, ET66AttackCategory Category);
	float GetCategorySubAttackSpeedMultiplier(const UT66RunStateSubsystem* RunState, ET66AttackCategory Category);
	float GetCategorySubScaleMultiplier(const UT66RunStateSubsystem* RunState, ET66AttackCategory Category);
	ET66StatType GetElementPowerStatForIdolElement(ET66IdolElement Element);
	float GetIdolElementPowerMultiplier(const UT66RunStateSubsystem* RunState, ET66IdolElement Element);
	float GetIdolRarityVisualScale(ET66ItemRarity Rarity);
	float GetIdolRarityVisualQuantity(ET66ItemRarity Rarity);
	bool IsHeroHurtboxComponent(const AT66HeroBase* Hero, const UPrimitiveComponent* Component);
	bool TryApplyNonBossOHKO(AActor* TargetActor, const FT66CombatTargetHandle* TargetHandle, FName DamageSourceID, FName EventType);

	// Single source of truth for the current idol impact-presentation proof phase:
	//   Idol_Ice_AOE=AOE, Idol_Electricity_Pierce=Pierce,
	//   Idol_Electricity_Bounce=Bounce, Idol_Nature_DOT=DOT.
	// These idols enter the category-native idol impact-presentation lane (driven from the
	// official weapon impact point). Idol_Nature_AOE is deliberately excluded: it is the neutral
	// control that must NOT enter the lane.
	const TSet<FName>& GetImpactPresentationProofIdols();
	// Full set of idols the Hero 1 axe proof harness accepts as -T66Hero1AxeAOEProofIdol input.
	// This is the impact-presentation set plus Idol_Nature_AOE (neutral control input).
	const TSet<FName>& GetSupportedProofIdols();
	FString DescribePrimitiveComponentForCombatLog(const UPrimitiveComponent* Component);
	FVector ResolveGroundAnchor(UWorld* World, const FVector& ApproxLocation, const AActor* IgnoreActor);
	FVector ResolveProcVFXAnchor(AActor* TargetActor, UWorld* World);
}
