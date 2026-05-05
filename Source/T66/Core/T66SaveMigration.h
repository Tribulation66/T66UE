// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

constexpr int32 T66SparseActiveHeroIdProfileSaveVersion = 16;
constexpr int32 T66SparseActiveHeroIdRunSaveVersion = 11;
constexpr int32 T66SparseActiveHeroIdRunSummarySchemaVersion = 21;

/**
 * One-time migration for the active roster moving from sparse authored IDs
 * (1,3,4,5,7,8,9,11,12,13,14,15) to contiguous active IDs (1-12).
 *
 * Call only from version-gated migration code. After this migration, IDs such as
 * Hero_3 are valid current IDs and must not be remapped unconditionally.
 */
inline FName T66MigrateSparseActiveHeroID(FName HeroID)
{
	if (HeroID == FName(TEXT("Hero_3"))) return FName(TEXT("Hero_2"));
	if (HeroID == FName(TEXT("Hero_4"))) return FName(TEXT("Hero_3"));
	if (HeroID == FName(TEXT("Hero_5"))) return FName(TEXT("Hero_4"));
	if (HeroID == FName(TEXT("Hero_7"))) return FName(TEXT("Hero_5"));
	if (HeroID == FName(TEXT("Hero_8"))) return FName(TEXT("Hero_6"));
	if (HeroID == FName(TEXT("Hero_9"))) return FName(TEXT("Hero_7"));
	if (HeroID == FName(TEXT("Hero_11"))) return FName(TEXT("Hero_8"));
	if (HeroID == FName(TEXT("Hero_12"))) return FName(TEXT("Hero_9"));
	if (HeroID == FName(TEXT("Hero_13"))) return FName(TEXT("Hero_10"));
	if (HeroID == FName(TEXT("Hero_14"))) return FName(TEXT("Hero_11"));
	if (HeroID == FName(TEXT("Hero_15"))) return FName(TEXT("Hero_12"));
	return HeroID;
}

/** Historical profile-only migration used before the sparse active roster pass. */
inline FName T66MigratePreSaveVersion9HeroID(FName HeroID)
{
	if (HeroID == FName(TEXT("Hero_2"))) return FName(TEXT("Hero_1"));
	if (HeroID == FName(TEXT("Hero_3"))) return FName(TEXT("Hero_2"));
	if (HeroID == FName(TEXT("Hero_4"))) return FName(TEXT("Hero_3"));
	if (HeroID == FName(TEXT("Hero_5"))) return FName(TEXT("Hero_4"));
	return HeroID;
}
