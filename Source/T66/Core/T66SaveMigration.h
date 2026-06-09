// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

constexpr int32 T66SparseActiveHeroIdProfileSaveVersion = 16;
constexpr int32 T66PetProfileSaveVersion = 17;
constexpr int32 T66CurrentProfileSaveVersion = T66PetProfileSaveVersion;
constexpr int32 T66SparseActiveHeroIdRunSaveVersion = 11;
constexpr int32 T66RunModeCategoryRunSaveVersion = 12;
constexpr int32 T66CollectedMobLootRunSaveVersion = 13;
constexpr int32 T66MobLootRunSummaryCountersRunSaveVersion = 14;
constexpr int32 T66PercentDamageRunSaveVersion = 15;
constexpr int32 T66CurrentRunSaveVersion = T66PercentDamageRunSaveVersion;
constexpr int32 T66SparseActiveHeroIdRunSummarySchemaVersion = 22;
constexpr int32 T66RunSummaryIdolTierSchemaVersion = 23;
constexpr int32 T66RunSummaryEnrichedProjectionSchemaVersion = 24;
constexpr int32 T66CurrentRunSummarySchemaVersion = T66RunSummaryEnrichedProjectionSchemaVersion;
constexpr int32 T66SavedEquippedIdolSlotCount = 3;

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

inline FName T66MigrateLegacyIdolID(FName IdolID)
{
	if (IdolID.IsNone())
	{
		return NAME_None;
	}

	if (IdolID == FName(TEXT("Idol_Water")))     return FName(TEXT("Idol_Ice_AOE"));
	if (IdolID == FName(TEXT("Idol_Light")))     return FName(TEXT("Idol_Electricity_Pierce"));
	if (IdolID == FName(TEXT("Idol_Electric")))  return FName(TEXT("Idol_Electricity_Bounce"));
	if (IdolID == FName(TEXT("Idol_Poison")))    return FName(TEXT("Idol_Nature_DOT"));
	if (IdolID == FName(TEXT("Idol_Lava")))      return FName(TEXT("Idol_Fire_DOT"));
	if (IdolID == FName(TEXT("Idol_Ice")))       return FName(TEXT("Idol_Ice_Bounce"));
	if (IdolID == FName(TEXT("Idol_Storm")))     return FName(TEXT("Idol_Electricity_AOE"));
	if (IdolID == FName(TEXT("Idol_Earth")))     return FName(TEXT("Idol_Nature_AOE"));
	if (IdolID == FName(TEXT("Idol_Wood")))      return FName(TEXT("Idol_Nature_Pierce"));
	if (IdolID == FName(TEXT("Idol_Curse")))     return FName(TEXT("Idol_Fire_Bounce"));
	if (IdolID == FName(TEXT("Idol_Shadow")))    return FName(TEXT("Idol_Nature_Bounce"));
	if (IdolID == FName(TEXT("Idol_Steel")))     return FName(TEXT("Idol_Fire_Pierce"));
	if (IdolID == FName(TEXT("Idol_Bleed")))     return FName(TEXT("Idol_Fire_DOT"));
	if (IdolID == FName(TEXT("Idol_Star")))      return FName(TEXT("Idol_Electricity_DOT"));
	if (IdolID == FName(TEXT("Idol_BlackHole"))) return FName(TEXT("Idol_Ice_DOT"));
	if (IdolID == FName(TEXT("Idol_Bone")))      return FName(TEXT("Idol_Ice_Pierce"));

	if (IdolID == FName(TEXT("Idol_Shock")))     return FName(TEXT("Idol_Electricity_Bounce"));
	if (IdolID == FName(TEXT("Idol_Silence")))   return FName(TEXT("Idol_Nature_Bounce"));
	if (IdolID == FName(TEXT("Idol_Mark")))      return FName(TEXT("Idol_Electricity_Pierce"));
	if (IdolID == FName(TEXT("Idol_Pierce")))    return FName(TEXT("Idol_Fire_Pierce"));
	if (IdolID == FName(TEXT("Idol_Knockback"))) return FName(TEXT("Idol_Nature_Pierce"));
	if (IdolID == FName(TEXT("Idol_Hex")))       return FName(TEXT("Idol_Fire_Bounce"));
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) return FName(TEXT("Idol_Fire_DOT"));
	if (IdolID == FName(TEXT("Idol_Lightning"))) return FName(TEXT("Idol_Electricity_Bounce"));
	if (IdolID == FName(TEXT("Idol_Darkness")))  return FName(TEXT("Idol_Nature_Bounce"));
	if (IdolID == FName(TEXT("Idol_Metal")))     return FName(TEXT("Idol_Fire_Pierce"));
	if (IdolID == FName(TEXT("Idol_Spectral")))  return FName(TEXT("Idol_Fire_Bounce"));
	if (IdolID == FName(TEXT("Idol_Frost")))     return FName(TEXT("Idol_Ice_Bounce"));

	return IdolID;
}

inline bool T66NormalizeEquippedIdolSaveArrays(TArray<FName>& EquippedIdols, TArray<uint8>& EquippedIdolTiers)
{
	bool bChanged = false;

	if (EquippedIdols.Num() != T66SavedEquippedIdolSlotCount)
	{
		const int32 OldNum = EquippedIdols.Num();
		EquippedIdols.SetNum(T66SavedEquippedIdolSlotCount);
		for (int32 Index = OldNum; Index < EquippedIdols.Num(); ++Index)
		{
			EquippedIdols[Index] = NAME_None;
		}
		bChanged = true;
	}

	if (EquippedIdolTiers.Num() != T66SavedEquippedIdolSlotCount)
	{
		const int32 OldNum = EquippedIdolTiers.Num();
		EquippedIdolTiers.SetNum(T66SavedEquippedIdolSlotCount);
		for (int32 Index = OldNum; Index < EquippedIdolTiers.Num(); ++Index)
		{
			EquippedIdolTiers[Index] = 0;
		}
		bChanged = true;
	}

	for (int32 Index = 0; Index < T66SavedEquippedIdolSlotCount; ++Index)
	{
		const FName OldIdol = EquippedIdols[Index];
		const FName MigratedIdol = T66MigrateLegacyIdolID(OldIdol);
		if (OldIdol != MigratedIdol)
		{
			EquippedIdols[Index] = MigratedIdol;
			bChanged = true;
		}

		const bool bHasIdol = !EquippedIdols[Index].IsNone();
		const uint8 OldTier = EquippedIdolTiers[Index];
		const uint8 NewTier = bHasIdol
			? static_cast<uint8>(FMath::Clamp<int32>(OldTier == 0 ? 1 : OldTier, 1, 4))
			: 0;
		if (OldTier != NewTier)
		{
			EquippedIdolTiers[Index] = NewTier;
			bChanged = true;
		}
	}

	return bChanged;
}

inline bool T66MigrateLocalRunSummarySaveFields(
	int32& SchemaVersion,
	FName& HeroID,
	TArray<FName>& EquippedIdols,
	TArray<uint8>& EquippedIdolTiers)
{
	bool bChanged = false;
	const int32 LoadedSchemaVersion = SchemaVersion;

	if (LoadedSchemaVersion < T66SparseActiveHeroIdRunSummarySchemaVersion)
	{
		const FName MigratedHeroID = T66MigrateSparseActiveHeroID(HeroID);
		if (HeroID != MigratedHeroID)
		{
			HeroID = MigratedHeroID;
			bChanged = true;
		}
		SchemaVersion = T66SparseActiveHeroIdRunSummarySchemaVersion;
		bChanged = true;
	}

	if (LoadedSchemaVersion < T66RunSummaryIdolTierSchemaVersion)
	{
		bChanged |= T66NormalizeEquippedIdolSaveArrays(EquippedIdols, EquippedIdolTiers);
		SchemaVersion = T66RunSummaryIdolTierSchemaVersion;
		bChanged = true;
	}
	else
	{
		bChanged |= T66NormalizeEquippedIdolSaveArrays(EquippedIdols, EquippedIdolTiers);
	}

	if (SchemaVersion < T66CurrentRunSummarySchemaVersion)
	{
		SchemaVersion = T66CurrentRunSummarySchemaVersion;
		bChanged = true;
	}

	return bChanged;
}
