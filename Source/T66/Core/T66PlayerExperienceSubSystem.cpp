// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PlayerExperienceSubSystem.h"

namespace
{
	static constexpr const TCHAR* T66PlayerExperienceDataTablePath = TEXT("/Game/Data/DT_PlayerExperience.DT_PlayerExperience");

	static void LoadDifficultyRow(const UDataTable* DataTable, const FName RowName, FT66PlayerExperienceDifficultyTuning& OutTuning)
	{
		if (!DataTable)
		{
			return;
		}

		if (const FT66PlayerExperienceDifficultyTuning* Row = DataTable->FindRow<FT66PlayerExperienceDifficultyTuning>(RowName, TEXT("PlayerExperienceDataTable")))
		{
			OutTuning = *Row;
			return;
		}

		UE_LOG(LogTemp, Error, TEXT("PlayerExperience DataTable '%s' is missing row '%s'."), *DataTable->GetPathName(), *RowName.ToString());
	}
}

void FT66PlayerExperienceTuningTable::LoadFromDataTable(const UDataTable* DataTable)
{
	LoadDifficultyRow(DataTable, TEXT("Easy"), Easy);
	LoadDifficultyRow(DataTable, TEXT("Medium"), Medium);
	LoadDifficultyRow(DataTable, TEXT("Hard"), Hard);
	LoadDifficultyRow(DataTable, TEXT("VeryHard"), VeryHard);
	LoadDifficultyRow(DataTable, TEXT("Impossible"), Impossible);
}

const FT66PlayerExperienceDifficultyTuning& FT66PlayerExperienceTuningTable::Get(const ET66Difficulty Difficulty) const
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return Easy;
	case ET66Difficulty::Medium: return Medium;
	case ET66Difficulty::Hard: return Hard;
	case ET66Difficulty::VeryHard: return VeryHard;
	case ET66Difficulty::Impossible: return Impossible;
	default: return Easy;
	}
}

void UT66PlayerExperienceSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UDataTable* PlayerExperienceDataTable = LoadObject<UDataTable>(nullptr, T66PlayerExperienceDataTablePath);
	if (!PlayerExperienceDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load player experience DataTable at '%s'."), T66PlayerExperienceDataTablePath);
		return;
	}

	CachedTuning.LoadFromDataTable(PlayerExperienceDataTable);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyIndex(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(static_cast<int32>(Difficulty), 0, 999);
}

const FT66PlayerExperienceDifficultyTuning& UT66PlayerExperienceSubSystem::GetDifficultyTuning(const ET66Difficulty Difficulty) const
{
	return CachedTuning.Get(Difficulty);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyStartStage(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(GetDifficultyTuning(Difficulty).StartStage, 1, 23);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyEndStage(const ET66Difficulty Difficulty) const
{
	const FT66PlayerExperienceDifficultyTuning& Tuning = GetDifficultyTuning(Difficulty);
	const int32 StartStage = FMath::Clamp(Tuning.StartStage, 1, 23);
	return FMath::Clamp(Tuning.EndStage, StartStage, 23);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyStartGoldBonus(const ET66Difficulty Difficulty) const
{
	return FMath::Max(0, GetDifficultyTuning(Difficulty).StartGoldBonus);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyStartLootBags(const ET66Difficulty Difficulty) const
{
	return FMath::Max(0, GetDifficultyTuning(Difficulty).StartLootBags);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyStartHeroBonusLevels(const ET66Difficulty Difficulty) const
{
	return FMath::Max(0, GetDifficultyTuning(Difficulty).StartHeroBonusLevels);
}

bool UT66PlayerExperienceSubSystem::ShouldSpawnSupportVendorAtRunStart(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).bSpawnSupportVendorAtRunStart;
}

bool UT66PlayerExperienceSubSystem::ShouldSupportVendorAllowSteal(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).bSupportVendorAllowSteal;
}

float UT66PlayerExperienceSubSystem::GetDifficultyEnemyLootBagDropChanceBase(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(GetDifficultyTuning(Difficulty).EnemyLootBagDropChanceBase, 0.f, 1.f);
}

FT66IntRange UT66PlayerExperienceSubSystem::GetDifficultyEnemyLootBagCountOnDrop(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).EnemyLootBagCountOnDrop;
}

FT66RarityWeights UT66PlayerExperienceSubSystem::GetDifficultyEnemyLootBagRarityWeights(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).EnemyLootBagRarityWeights;
}

FT66RarityWeights UT66PlayerExperienceSubSystem::GetDifficultyCatchUpLootBagRarityWeights(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).CatchUpLootBagRarityWeights;
}

FT66IntRange UT66PlayerExperienceSubSystem::GetDifficultyChestCountRange(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).ChestsPerStage;
}

FT66RarityWeights UT66PlayerExperienceSubSystem::GetDifficultyChestRarityWeights(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).ChestRarityWeights;
}

FT66IntRange UT66PlayerExperienceSubSystem::GetDifficultyChestGoldRange(const ET66Difficulty Difficulty, const ET66Rarity Rarity) const
{
	const FT66RarityIntRanges& GoldByRarity = GetDifficultyTuning(Difficulty).ChestGoldRangeByRarity;
	switch (Rarity)
	{
	case ET66Rarity::Black: return GoldByRarity.Black;
	case ET66Rarity::Red: return GoldByRarity.Red;
	case ET66Rarity::Yellow: return GoldByRarity.Yellow;
	case ET66Rarity::White: return GoldByRarity.White;
	default: return GoldByRarity.Black;
	}
}

float UT66PlayerExperienceSubSystem::GetDifficultyChestMimicChance(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(GetDifficultyTuning(Difficulty).ChestMimicChance, 0.f, 1.f);
}

FT66IntRange UT66PlayerExperienceSubSystem::GetDifficultyWheelCountRange(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).WheelsPerStage;
}

FT66RarityWeights UT66PlayerExperienceSubSystem::GetDifficultyWheelRarityWeights(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).WheelRarityWeights;
}

FT66FloatRange UT66PlayerExperienceSubSystem::GetDifficultyWheelGoldRange(const ET66Difficulty Difficulty, const ET66Rarity Rarity) const
{
	const FT66PlayerExperienceDifficultyTuning& Tuning = GetDifficultyTuning(Difficulty);
	switch (Rarity)
	{
	case ET66Rarity::Black: return Tuning.WheelGoldRangeBlack;
	case ET66Rarity::Red: return Tuning.WheelGoldRangeRed;
	case ET66Rarity::Yellow: return Tuning.WheelGoldRangeYellow;
	case ET66Rarity::White: return Tuning.WheelGoldRangeWhite;
	default: return Tuning.WheelGoldRangeBlack;
	}
}

FT66IntRange UT66PlayerExperienceSubSystem::GetDifficultyCrateCountRange(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).CratesPerStage;
}

FT66RarityWeights UT66PlayerExperienceSubSystem::GetDifficultyCrateRarityWeights(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).CrateRarityWeights;
}

float UT66PlayerExperienceSubSystem::GetDifficultyGamblerCheatSuccessChanceBase(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(GetDifficultyTuning(Difficulty).GamblerCheatSuccessChanceBase, 0.f, 1.f);
}

float UT66PlayerExperienceSubSystem::GetDifficultyVendorStealSuccessChanceOnTimingHitBase(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(GetDifficultyTuning(Difficulty).VendorStealSuccessChanceOnTimingHitBase, 0.f, 1.f);
}
