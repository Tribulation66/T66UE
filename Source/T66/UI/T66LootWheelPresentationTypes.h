// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66Rarity.h"
#include "Data/T66DataTypes.h"

enum class ET66LootWheelRewardVisualType : uint8
{
	Gold,
	Item,
	Boost,
};

struct FT66LootWheelPresentationParams
{
	ET66Rarity WheelRarity = ET66Rarity::Black;
	ET66LootWheelRewardVisualType RewardType = ET66LootWheelRewardVisualType::Gold;
	int32 Gold = 0;
	FName ItemID = NAME_None;
	ET66ItemRarity ItemRarity = ET66ItemRarity::Black;
	ET66HeroStatType BoostBaseStatType = ET66HeroStatType::Damage;
	ET66StatType BoostStatType = ET66StatType::None;
	bool bBoostUsesStat = false;
	int32 BoostBonusStatPoints = 8;
	float BoostDurationSeconds = 10.f;
	TFunction<void()> OnLandingCommit;
	TFunction<void()> OnFinished;
};
