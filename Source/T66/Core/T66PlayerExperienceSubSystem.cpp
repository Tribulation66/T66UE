// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PlayerExperienceSubSystem.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66PlayerExperience, Log, All);

namespace
{
	static constexpr const TCHAR* T66PlayerExperienceDataTablePath = TEXT("/Game/Data/DT_PlayerExperience.DT_PlayerExperience");

	static bool LoadDifficultyRow(const UDataTable* DataTable, const FName RowName, FT66PlayerExperienceDifficultyTuning& OutTuning)
	{
		if (!DataTable)
		{
			return false;
		}

		if (const FT66PlayerExperienceDifficultyTuning* Row = DataTable->FindRow<FT66PlayerExperienceDifficultyTuning>(RowName, TEXT("PlayerExperienceDataTable")))
		{
			OutTuning = *Row;
			return true;
		}

		UE_LOG(LogT66PlayerExperience, Error, TEXT("PlayerExperience DataTable '%s' is missing row '%s'."), *DataTable->GetPathName(), *RowName.ToString());
		return false;
	}
}

bool FT66PlayerExperienceTuningTable::LoadFromDataTable(const UDataTable* DataTable)
{
	bool bLoadedAllRows = true;
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Easy"), Easy);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Medium"), Medium);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Hard"), Hard);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("VeryHard"), VeryHard);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Impossible"), Impossible);
	return bLoadedAllRows;
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
	QueueTuningDataTableLoad();
}

void UT66PlayerExperienceSubSystem::QueueTuningDataTableLoad()
{
	const FSoftObjectPath TuningTablePath(T66PlayerExperienceDataTablePath);
	if (UDataTable* ResidentTable = Cast<UDataTable>(TuningTablePath.ResolveObject()))
	{
		if (ResidentTable->GetRowStruct() != FT66PlayerExperienceDifficultyTuning::StaticStruct())
		{
			UE_LOG(LogT66PlayerExperience, Error, TEXT("PlayerExperience DataTable '%s' has row struct '%s', expected '%s'."),
				*ResidentTable->GetPathName(),
				ResidentTable->GetRowStruct() ? *ResidentTable->GetRowStruct()->GetName() : TEXT("<null>"),
				*FT66PlayerExperienceDifficultyTuning::StaticStruct()->GetName());
			return;
		}

		bTuningLoaded = CachedTuning.LoadFromDataTable(ResidentTable);
		return;
	}

	TuningDataTableLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		TArray<FSoftObjectPath>{ TuningTablePath },
		FStreamableDelegate::CreateUObject(this, &UT66PlayerExperienceSubSystem::HandleTuningDataTableLoaded));
	if (!TuningDataTableLoadHandle.IsValid())
	{
		UE_LOG(LogT66PlayerExperience, Error, TEXT("Failed to queue async load for player experience DataTable at '%s'."), T66PlayerExperienceDataTablePath);
	}
}

void UT66PlayerExperienceSubSystem::HandleTuningDataTableLoaded()
{
	TuningDataTableLoadHandle.Reset();

	const FSoftObjectPath TuningTablePath(T66PlayerExperienceDataTablePath);
	UDataTable* PlayerExperienceDataTable = Cast<UDataTable>(TuningTablePath.ResolveObject());
	if (!PlayerExperienceDataTable)
	{
		UE_LOG(LogT66PlayerExperience, Error, TEXT("Failed to resolve player experience DataTable after async load at '%s'."), T66PlayerExperienceDataTablePath);
		return;
	}

	if (PlayerExperienceDataTable->GetRowStruct() != FT66PlayerExperienceDifficultyTuning::StaticStruct())
	{
		UE_LOG(LogT66PlayerExperience, Error, TEXT("PlayerExperience DataTable '%s' has row struct '%s', expected '%s'."),
			*PlayerExperienceDataTable->GetPathName(),
			PlayerExperienceDataTable->GetRowStruct() ? *PlayerExperienceDataTable->GetRowStruct()->GetName() : TEXT("<null>"),
			*FT66PlayerExperienceDifficultyTuning::StaticStruct()->GetName());
		return;
	}

	bTuningLoaded = CachedTuning.LoadFromDataTable(PlayerExperienceDataTable);
}

bool UT66PlayerExperienceSubSystem::IsTuningReady(const TCHAR* Caller) const
{
	if (bTuningLoaded)
	{
		return true;
	}

	if (!bWarnedTuningUnavailable)
	{
		bWarnedTuningUnavailable = true;
		UE_LOG(LogT66PlayerExperience, Warning, TEXT("PlayerExperience tuning requested by %s before DataTable '%s' was available; returning empty tuning."), Caller ? Caller : TEXT("<unknown>"), T66PlayerExperienceDataTablePath);
	}
	return false;
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyIndex(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(static_cast<int32>(Difficulty), 0, 999);
}

const FT66PlayerExperienceDifficultyTuning& UT66PlayerExperienceSubSystem::GetDifficultyTuning(const ET66Difficulty Difficulty) const
{
	IsTuningReady(TEXT("GetDifficultyTuning"));
	return CachedTuning.Get(Difficulty);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyStartStage(const ET66Difficulty Difficulty) const
{
	if (!IsTuningReady(TEXT("GetDifficultyStartStage")))
	{
		return 0;
	}
	return FMath::Clamp(GetDifficultyTuning(Difficulty).StartStage, 1, 20);
}

int32 UT66PlayerExperienceSubSystem::GetDifficultyEndStage(const ET66Difficulty Difficulty) const
{
	if (!IsTuningReady(TEXT("GetDifficultyEndStage")))
	{
		return 0;
	}
	const FT66PlayerExperienceDifficultyTuning& Tuning = GetDifficultyTuning(Difficulty);
	const int32 StartStage = FMath::Clamp(Tuning.StartStage, 1, 20);
	return FMath::Clamp(Tuning.EndStage, StartStage, 20);
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

float UT66PlayerExperienceSubSystem::GetDifficultyShopStealSuccessChanceOnTimingHitBase(const ET66Difficulty Difficulty) const
{
	return FMath::Clamp(GetDifficultyTuning(Difficulty).ShopStealSuccessChanceOnTimingHitBase, 0.f, 1.f);
}
