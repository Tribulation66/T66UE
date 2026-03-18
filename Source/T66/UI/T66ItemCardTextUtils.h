// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class UT66LocalizationSubsystem;

namespace T66ItemCardTextUtils
{
	FText GetPrimaryStatLabel(const UT66LocalizationSubsystem* Loc, ET66HeroStatType Type);

	FText BuildItemCardDescription(
		const UT66LocalizationSubsystem* Loc,
		const FItemData& ItemData,
		ET66ItemRarity ItemRarity,
		int32 MainValue,
		float CurrentHeroScaleMultiplier);
}
