// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PlayerExperienceSubSystem.h"

#include "Misc/ConfigCacheIni.h"

namespace
{
	static constexpr const TCHAR* T66PlayerExperienceConfigSection = TEXT("/Script/T66.T66PlayerExperienceSubSystem");

	static FString GetT66PlayerExperienceConfigFilename()
	{
		FString ConfigFilename;
		FConfigCacheIni::LoadGlobalIniFile(ConfigFilename, TEXT("T66PlayerExperience"));
		return ConfigFilename;
	}

	template <typename StructType>
	static void LoadStructValue(const FString& ConfigFilename, const TCHAR* Key, StructType& Value)
	{
		if (!GConfig)
		{
			return;
		}

		FString RawValue;
		if (!GConfig->GetString(T66PlayerExperienceConfigSection, Key, RawValue, ConfigFilename) || RawValue.IsEmpty())
		{
			return;
		}

		UScriptStruct* StructTypeInfo = StructType::StaticStruct();
		if (!StructTypeInfo)
		{
			return;
		}

		StructTypeInfo->ImportText(*RawValue, &Value, nullptr, PPF_None, GLog, StructTypeInfo->GetName());
	}

	static FT66RarityWeights MakeRarityWeights(const float Black, const float Red, const float Yellow, const float White)
	{
		FT66RarityWeights Weights;
		Weights.Black = Black;
		Weights.Red = Red;
		Weights.Yellow = Yellow;
		Weights.White = White;
		return Weights;
	}

	static FT66PlayerExperienceDifficultyTuning MakeDefaultDifficultyTuning(const int32 DifficultyIndex)
	{
		FT66PlayerExperienceDifficultyTuning Tuning;
		static constexpr int32 DefaultStartStages[] = { 1, 6, 11, 16, 21 };
		const int32 SafeIndex = FMath::Clamp(DifficultyIndex, 0, UE_ARRAY_COUNT(DefaultStartStages) - 1);
		Tuning.StartStage = DefaultStartStages[SafeIndex];
		Tuning.StartGoldBonus = 200 * DifficultyIndex;
		Tuning.StartLootBags = 2 + (DifficultyIndex * 2);
		Tuning.StartHeroBonusLevels = DifficultyIndex * 10;
		Tuning.bSpawnSupportVendorAtRunStart = DifficultyIndex > 0;
		Tuning.bSupportVendorAllowSteal = false;
		Tuning.EnemyLootBagDropChanceBase = 0.10f;
		Tuning.EnemyLootBagCountOnDrop = { 1, 1 };
		Tuning.EnemyLootBagRarityWeights = MakeRarityWeights(0.80f, 0.15f, 0.045f, 0.005f);
		Tuning.CatchUpLootBagRarityWeights = MakeRarityWeights(0.70f, 0.20f, 0.09f, 0.01f);
		Tuning.ChestsPerStage = { 4, 10 };
		Tuning.ChestRarityWeights = MakeRarityWeights(0.70f, 0.20f, 0.09f, 0.01f);
		Tuning.ChestGoldRangeByRarity.Black = { 35, 75 };
		Tuning.ChestGoldRangeByRarity.Red = { 100, 180 };
		Tuning.ChestGoldRangeByRarity.Yellow = { 220, 380 };
		Tuning.ChestGoldRangeByRarity.White = { 450, 750 };
		Tuning.ChestMimicChance = 0.20f;
		Tuning.WheelsPerStage = { 5, 11 };
		Tuning.WheelRarityWeights = MakeRarityWeights(0.80f, 0.15f, 0.045f, 0.005f);
		Tuning.WheelGoldRangeBlack = { 25.f, 100.f };
		Tuning.WheelGoldRangeRed = { 100.f, 300.f };
		Tuning.WheelGoldRangeYellow = { 200.f, 600.f };
		Tuning.WheelGoldRangeWhite = { 500.f, 1500.f };
		Tuning.CratesPerStage = { 3, 6 };
		Tuning.CrateRarityWeights = MakeRarityWeights(0.70f, 0.20f, 0.08f, 0.02f);
		Tuning.GamblerCheatSuccessChanceBase = 0.40f;
		Tuning.VendorStealSuccessChanceOnTimingHitBase = 0.65f;
		return Tuning;
	}
}

FT66PlayerExperienceTuningTable::FT66PlayerExperienceTuningTable()
	: Easy(MakeDefaultDifficultyTuning(0))
	, Medium(MakeDefaultDifficultyTuning(1))
	, Hard(MakeDefaultDifficultyTuning(2))
	, VeryHard(MakeDefaultDifficultyTuning(3))
	, Impossible(MakeDefaultDifficultyTuning(4))
{
}

void FT66PlayerExperienceTuningTable::LoadFromConfig()
{
	const FString ConfigFilename = GetT66PlayerExperienceConfigFilename();
	LoadStructValue(ConfigFilename, TEXT("Easy"), Easy);
	LoadStructValue(ConfigFilename, TEXT("Medium"), Medium);
	LoadStructValue(ConfigFilename, TEXT("Hard"), Hard);
	LoadStructValue(ConfigFilename, TEXT("VeryHard"), VeryHard);
	LoadStructValue(ConfigFilename, TEXT("Impossible"), Impossible);
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
	CachedTuning.LoadFromConfig();
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
	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return 5;
	case ET66Difficulty::Medium: return 10;
	case ET66Difficulty::Hard: return 15;
	case ET66Difficulty::VeryHard: return 20;
	case ET66Difficulty::Impossible: return 23;
	default: return 5;
	}
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
