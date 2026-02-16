// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * One-time migration: Hero renumbering (Hero_1 removed, Hero_2..5 -> Hero_1..4).
 * Use when loading profile, run saves, or leaderboard run summary saves that may contain old HeroIDs.
 */
inline FName T66MigrateHeroIDFromSave(FName HeroID)
{
	if (HeroID == FName(TEXT("Hero_2"))) return FName(TEXT("Hero_1"));
	if (HeroID == FName(TEXT("Hero_3"))) return FName(TEXT("Hero_2"));
	if (HeroID == FName(TEXT("Hero_4"))) return FName(TEXT("Hero_3"));
	if (HeroID == FName(TEXT("Hero_5"))) return FName(TEXT("Hero_4"));
	return HeroID; // Hero_1 (old New Chad) -> Hero_1 (new Knight); any other unchanged
}
