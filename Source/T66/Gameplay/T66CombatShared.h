// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class AActor;
class UT66RunStateSubsystem;
class UWorld;

namespace T66CombatShared
{
	const TCHAR* GetItemRarityName(ET66ItemRarity Rarity);
	float GetCategorySubScaleMultiplier(const UT66RunStateSubsystem* RunState, ET66AttackCategory Category);
	float GetIdolRarityVisualScale(ET66ItemRarity Rarity);
	float GetIdolRarityVisualQuantity(ET66ItemRarity Rarity);
	FVector ResolveGroundAnchor(UWorld* World, const FVector& ApproxLocation, const AActor* IgnoreActor);
	FVector ResolveProcVFXAnchor(AActor* TargetActor, UWorld* World);
}
