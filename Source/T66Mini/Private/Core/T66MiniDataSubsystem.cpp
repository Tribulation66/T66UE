// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66CsvUtil.h"

#include "Engine/Texture2D.h"

namespace
{
	FORCEINLINE TArray<TMap<FString, FString>> T66MiniLoadCsvRows(const FString& AbsolutePath) { return T66CsvUtil::LoadCsvRows(AbsolutePath); }
	FORCEINLINE FString T66MiniGetValue(const TMap<FString, FString>& Row, const TCHAR* Key) { return T66CsvUtil::GetValue(Row, Key); }
	FORCEINLINE float T66MiniToFloat(const FString& Value, const float DefaultValue = 0.f) { return T66CsvUtil::ToFloat(Value, DefaultValue); }
	FORCEINLINE int32 T66MiniToInt(const FString& Value, const int32 DefaultValue = 0) { return T66CsvUtil::ToInt(Value, DefaultValue); }
	FORCEINLINE bool T66MiniToBool(const FString& Value, const bool bDefaultValue = false) { return T66CsvUtil::ToBool(Value, bDefaultValue); }
	FORCEINLINE FLinearColor T66MiniToColor(const FString& Value, const FLinearColor& DefaultColor) { return T66CsvUtil::ToColor(Value, DefaultColor); }
	FORCEINLINE FName T66MiniToName(const TMap<FString, FString>& Row, const TCHAR* Key) { return T66CsvUtil::ToName(Row, Key); }
	FORCEINLINE TArray<FName> T66MiniSplitNameList(const FString& Value) { return T66CsvUtil::SplitNameList(Value); }

	ET66MiniEnemyBehaviorProfile T66MiniToBehaviorProfile(const FString& Value)
	{
		if (Value.Equals(TEXT("HumanoidBalanced"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::HumanoidBalanced;
		}
		if (Value.Equals(TEXT("RoostPress"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::RoostPress;
		}
		if (Value.Equals(TEXT("CowBruiser"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::CowBruiser;
		}
		if (Value.Equals(TEXT("GoatCharger"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::GoatCharger;
		}
		if (Value.Equals(TEXT("PigEnrage"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::PigEnrage;
		}
		if (Value.Equals(TEXT("Sharpshooter"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::Sharpshooter;
		}
		if (Value.Equals(TEXT("Juggernaut"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::Juggernaut;
		}
		if (Value.Equals(TEXT("Duelist"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyBehaviorProfile::Duelist;
		}

		return ET66MiniEnemyBehaviorProfile::Balanced;
	}

	ET66MiniEnemyFamily T66MiniToEnemyFamily(const FString& Value, const ET66MiniEnemyBehaviorProfile BehaviorProfile, const bool bIsBoss)
	{
		if (bIsBoss)
		{
			return ET66MiniEnemyFamily::Boss;
		}

		if (Value.Equals(TEXT("Ranged"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyFamily::Ranged;
		}
		if (Value.Equals(TEXT("Rushing"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyFamily::Rushing;
		}
		if (Value.Equals(TEXT("Boss"), ESearchCase::IgnoreCase))
		{
			return ET66MiniEnemyFamily::Boss;
		}

		if (BehaviorProfile == ET66MiniEnemyBehaviorProfile::Sharpshooter)
		{
			return ET66MiniEnemyFamily::Ranged;
		}
		if (BehaviorProfile == ET66MiniEnemyBehaviorProfile::GoatCharger || BehaviorProfile == ET66MiniEnemyBehaviorProfile::Duelist)
		{
			return ET66MiniEnemyFamily::Rushing;
		}

		return ET66MiniEnemyFamily::Melee;
	}

	ET66UltimateType T66MiniToUltimateType(const FString& Value)
	{
		if (Value.Equals(TEXT("SpearStorm"), ESearchCase::IgnoreCase)) return ET66UltimateType::SpearStorm;
		if (Value.Equals(TEXT("MeteorStrike"), ESearchCase::IgnoreCase)) return ET66UltimateType::MeteorStrike;
		if (Value.Equals(TEXT("ChainLightning"), ESearchCase::IgnoreCase)) return ET66UltimateType::ChainLightning;
		if (Value.Equals(TEXT("PlagueCloud"), ESearchCase::IgnoreCase)) return ET66UltimateType::PlagueCloud;
		if (Value.Equals(TEXT("PrecisionStrike"), ESearchCase::IgnoreCase)) return ET66UltimateType::PrecisionStrike;
		if (Value.Equals(TEXT("FanTheHammer"), ESearchCase::IgnoreCase)) return ET66UltimateType::FanTheHammer;
		if (Value.Equals(TEXT("Deadeye"), ESearchCase::IgnoreCase)) return ET66UltimateType::Deadeye;
		if (Value.Equals(TEXT("Discharge"), ESearchCase::IgnoreCase)) return ET66UltimateType::Discharge;
		if (Value.Equals(TEXT("Juiced"), ESearchCase::IgnoreCase)) return ET66UltimateType::Juiced;
		if (Value.Equals(TEXT("DeathSpiral"), ESearchCase::IgnoreCase)) return ET66UltimateType::DeathSpiral;
		if (Value.Equals(TEXT("Shockwave"), ESearchCase::IgnoreCase)) return ET66UltimateType::Shockwave;
		if (Value.Equals(TEXT("TidalWave"), ESearchCase::IgnoreCase)) return ET66UltimateType::TidalWave;
		if (Value.Equals(TEXT("GoldRush"), ESearchCase::IgnoreCase)) return ET66UltimateType::GoldRush;
		if (Value.Equals(TEXT("MiasmaBomb"), ESearchCase::IgnoreCase)) return ET66UltimateType::MiasmaBomb;
		if (Value.Equals(TEXT("RabidFrenzy"), ESearchCase::IgnoreCase)) return ET66UltimateType::RabidFrenzy;
		if (Value.Equals(TEXT("Blizzard"), ESearchCase::IgnoreCase)) return ET66UltimateType::Blizzard;
		if (Value.Equals(TEXT("ScopedSniper"), ESearchCase::IgnoreCase)) return ET66UltimateType::ScopedSniper;
		return ET66UltimateType::None;
	}

	ET66PassiveType T66MiniToPassiveType(const FString& Value)
	{
		if (Value.Equals(TEXT("IronWill"), ESearchCase::IgnoreCase)) return ET66PassiveType::IronWill;
		if (Value.Equals(TEXT("RallyingBlow"), ESearchCase::IgnoreCase)) return ET66PassiveType::RallyingBlow;
		if (Value.Equals(TEXT("ArcaneAmplification"), ESearchCase::IgnoreCase)) return ET66PassiveType::ArcaneAmplification;
		if (Value.Equals(TEXT("MarksmanFocus"), ESearchCase::IgnoreCase)) return ET66PassiveType::MarksmanFocus;
		if (Value.Equals(TEXT("ToxinStacking"), ESearchCase::IgnoreCase)) return ET66PassiveType::ToxinStacking;
		if (Value.Equals(TEXT("QuickDraw"), ESearchCase::IgnoreCase)) return ET66PassiveType::QuickDraw;
		if (Value.Equals(TEXT("Headshot"), ESearchCase::IgnoreCase)) return ET66PassiveType::Headshot;
		if (Value.Equals(TEXT("StaticCharge"), ESearchCase::IgnoreCase)) return ET66PassiveType::StaticCharge;
		if (Value.Equals(TEXT("Overclock"), ESearchCase::IgnoreCase)) return ET66PassiveType::Overclock;
		if (Value.Equals(TEXT("ChaosTheory"), ESearchCase::IgnoreCase)) return ET66PassiveType::ChaosTheory;
		if (Value.Equals(TEXT("Endurance"), ESearchCase::IgnoreCase)) return ET66PassiveType::Endurance;
		if (Value.Equals(TEXT("BrawlersFury"), ESearchCase::IgnoreCase)) return ET66PassiveType::BrawlersFury;
		if (Value.Equals(TEXT("Unflinching"), ESearchCase::IgnoreCase)) return ET66PassiveType::Unflinching;
		if (Value.Equals(TEXT("TreasureHunter"), ESearchCase::IgnoreCase)) return ET66PassiveType::TreasureHunter;
		if (Value.Equals(TEXT("Evasive"), ESearchCase::IgnoreCase)) return ET66PassiveType::Evasive;
		if (Value.Equals(TEXT("Frostbite"), ESearchCase::IgnoreCase)) return ET66PassiveType::Frostbite;
		return ET66PassiveType::None;
	}

	ET66MiniInteractableType T66MiniToInteractableType(const FString& Value)
	{
		if (Value.Equals(TEXT("Fountain"), ESearchCase::IgnoreCase))
		{
			return ET66MiniInteractableType::Fountain;
		}
		if (Value.Equals(TEXT("LootCrate"), ESearchCase::IgnoreCase))
		{
			return ET66MiniInteractableType::LootCrate;
		}
		if (Value.Equals(TEXT("QuickReviveMachine"), ESearchCase::IgnoreCase))
		{
			return ET66MiniInteractableType::QuickReviveMachine;
		}

		return ET66MiniInteractableType::TreasureChest;
	}

	FString T66MiniBossProfileToVisualID(const FString& BossProfile)
	{
		if (BossProfile.Equals(TEXT("Sharpshooter"), ESearchCase::IgnoreCase))
		{
			return TEXT("Roost");
		}
		if (BossProfile.Equals(TEXT("Juggernaut"), ESearchCase::IgnoreCase))
		{
			return TEXT("Cow");
		}
		if (BossProfile.Equals(TEXT("Duelist"), ESearchCase::IgnoreCase))
		{
			return TEXT("Goat");
		}

		return TEXT("Pig");
	}

	FORCEINLINE FString T66MiniMiniDataPath(const TCHAR* FileName) { return T66CsvUtil::ProjectContentPath(TEXT("Mini/Data"), FileName); }
}

void UT66MiniDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadData();
}

void UT66MiniDataSubsystem::ReloadData()
{
	LoadHeroes();
	LoadIdols();
	LoadCompanions();
	LoadDifficulties();
	LoadStages();
	LoadEnemies();
	LoadBosses();
	LoadWaves();
	LoadInteractables();
	LoadItems();
	LoadRuntimeTuning();
	LoadCircusGames();
}

const FT66MiniHeroDefinition* UT66MiniDataSubsystem::FindHero(const FName HeroID) const
{
	return Heroes.FindByPredicate([HeroID](const FT66MiniHeroDefinition& Definition)
	{
		return Definition.HeroID == HeroID;
	});
}

const FT66MiniIdolDefinition* UT66MiniDataSubsystem::FindIdol(const FName IdolID) const
{
	return Idols.FindByPredicate([IdolID](const FT66MiniIdolDefinition& Definition)
	{
		return Definition.IdolID == IdolID;
	});
}

const FT66MiniCompanionDefinition* UT66MiniDataSubsystem::FindCompanion(const FName CompanionID) const
{
	return Companions.FindByPredicate([CompanionID](const FT66MiniCompanionDefinition& Definition)
	{
		return Definition.CompanionID == CompanionID;
	});
}

const FT66MiniDifficultyDefinition* UT66MiniDataSubsystem::FindDifficulty(const FName DifficultyID) const
{
	return Difficulties.FindByPredicate([DifficultyID](const FT66MiniDifficultyDefinition& Definition)
	{
		return Definition.DifficultyID == DifficultyID;
	});
}

const FT66MiniStageDefinition* UT66MiniDataSubsystem::FindStage(const FName StageID) const
{
	return Stages.FindByPredicate([StageID](const FT66MiniStageDefinition& Definition)
	{
		return Definition.StageID == StageID;
	});
}

const FT66MiniStageDefinition* UT66MiniDataSubsystem::FindStage(const FName DifficultyID, const int32 StageIndex) const
{
	return Stages.FindByPredicate([DifficultyID, StageIndex](const FT66MiniStageDefinition& Definition)
	{
		return Definition.DifficultyID == DifficultyID && Definition.StageIndex == StageIndex;
	});
}

const FT66MiniStageDefinition* UT66MiniDataSubsystem::FindStageForWave(const FName DifficultyID, const int32 WaveIndex) const
{
	return Stages.FindByPredicate([DifficultyID, WaveIndex](const FT66MiniStageDefinition& Definition)
	{
		return Definition.DifficultyID == DifficultyID && Definition.WaveIndex == WaveIndex;
	});
}

int32 UT66MiniDataSubsystem::GetMaxStageIndexForDifficulty(const FName DifficultyID) const
{
	int32 MaxStageIndex = 0;
	for (const FT66MiniStageDefinition& Definition : Stages)
	{
		if (Definition.DifficultyID == DifficultyID)
		{
			MaxStageIndex = FMath::Max(MaxStageIndex, Definition.StageIndex);
		}
	}
	return MaxStageIndex;
}

const FT66MiniEnemyDefinition* UT66MiniDataSubsystem::FindEnemy(const FName EnemyID) const
{
	return Enemies.FindByPredicate([EnemyID](const FT66MiniEnemyDefinition& Definition)
	{
		return Definition.EnemyID == EnemyID;
	});
}

const FT66MiniBossDefinition* UT66MiniDataSubsystem::FindBoss(const FName BossID) const
{
	return Bosses.FindByPredicate([BossID](const FT66MiniBossDefinition& Definition)
	{
		return Definition.BossID == BossID;
	});
}

const FT66MiniWaveDefinition* UT66MiniDataSubsystem::FindWave(const FName DifficultyID, const int32 WaveIndex) const
{
	return Waves.FindByPredicate([DifficultyID, WaveIndex](const FT66MiniWaveDefinition& Definition)
	{
		return Definition.DifficultyID == DifficultyID && Definition.WaveIndex == WaveIndex;
	});
}

const FT66MiniInteractableDefinition* UT66MiniDataSubsystem::FindInteractable(const FName InteractableID) const
{
	return Interactables.FindByPredicate([InteractableID](const FT66MiniInteractableDefinition& Definition)
	{
		return Definition.InteractableID == InteractableID;
	});
}

const FT66MiniItemDefinition* UT66MiniDataSubsystem::FindItem(const FName ItemID) const
{
	return Items.FindByPredicate([ItemID](const FT66MiniItemDefinition& Definition)
	{
		return Definition.ItemID == ItemID;
	});
}

const FT66MiniCircusGameDefinition* UT66MiniDataSubsystem::FindCircusGame(const FName GameID) const
{
	return CircusGames.FindByPredicate([GameID](const FT66MiniCircusGameDefinition& Definition)
	{
		return Definition.GameID == GameID;
	});
}

float UT66MiniDataSubsystem::FindRuntimeTuningValue(const FName TuningKey, const float DefaultValue) const
{
	if (const float* FoundValue = RuntimeTuningValues.Find(TuningKey))
	{
		return *FoundValue;
	}
	return DefaultValue;
}

int32 UT66MiniDataSubsystem::FindRuntimeTuningInt(const FName TuningKey, const int32 DefaultValue) const
{
	return FMath::RoundToInt(FindRuntimeTuningValue(TuningKey, static_cast<float>(DefaultValue)));
}

void UT66MiniDataSubsystem::LoadHeroes()
{
	Heroes.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Heroes.csv"))))
	{
		FT66MiniHeroDefinition Definition;
		Definition.HeroID = FName(*T66MiniGetValue(Row, TEXT("HeroID")));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.PrimaryCategory = T66MiniGetValue(Row, TEXT("PrimaryCategory"));
		Definition.PlaceholderColor = T66MiniToColor(T66MiniGetValue(Row, TEXT("PlaceholderColor")), FLinearColor(0.24f, 0.28f, 0.36f, 1.0f));
		Definition.PortraitPath = T66MiniGetValue(Row, TEXT("Portrait"));
		Definition.PortraitFullPath = T66MiniGetValue(Row, TEXT("PortraitFull"));
		Definition.BaseDamage = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseDamage")));
		Definition.BaseAttackSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseAttackSpeed")));
		Definition.BaseArmor = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseArmor")));
		Definition.BaseLuck = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseLuck")));
		Definition.BaseSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseSpeed")));
		Definition.UltimateType = T66MiniToUltimateType(T66MiniGetValue(Row, TEXT("UltimateType")));
		Definition.PassiveType = T66MiniToPassiveType(T66MiniGetValue(Row, TEXT("PassiveType")));
		Definition.bUnlockedByDefault = T66MiniToBool(T66MiniGetValue(Row, TEXT("bUnlockedByDefault")), true);

		if (Definition.HeroID != NAME_None)
		{
			Heroes.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadIdols()
{
	Idols.Reset();
	const FString CsvPath = T66MiniMiniDataPath(TEXT("T66Mini_Idols.csv"));

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(CsvPath))
	{
		FT66MiniIdolDefinition Definition;
		Definition.IdolID = FName(*T66MiniGetValue(Row, TEXT("IdolID")));
		Definition.Category = T66MiniGetValue(Row, TEXT("Category"));
		Definition.IconPath = T66MiniGetValue(Row, TEXT("Icon"));
		Definition.MaxLevel = T66MiniToInt(T66MiniGetValue(Row, TEXT("MaxLevel")), 1);
		Definition.BaseDamage = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseDamage")));
		Definition.DamagePerLevel = T66MiniToFloat(T66MiniGetValue(Row, TEXT("DamagePerLevel")));
		Definition.BaseProperty = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseProperty")));
		Definition.PropertyPerLevel = T66MiniToFloat(T66MiniGetValue(Row, TEXT("PropertyPerLevel")));

		if (Definition.IdolID != NAME_None)
		{
			Idols.Add(MoveTemp(Definition));
		}
	}

	if (Idols.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("T66MiniDataSubsystem: failed to load mini idol rows from '%s'. Mini idols must be authored in data."), *CsvPath);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("T66MiniDataSubsystem: loaded %d mini idols from '%s'."), Idols.Num(), *CsvPath);
	}
}

void UT66MiniDataSubsystem::LoadCompanions()
{
	Companions.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Companions.csv"))))
	{
		FT66MiniCompanionDefinition Definition;
		Definition.CompanionID = T66MiniToName(Row, TEXT("CompanionID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.VisualID = T66MiniGetValue(Row, TEXT("VisualID"));
		Definition.PlaceholderColor = T66MiniToColor(T66MiniGetValue(Row, TEXT("PlaceholderColor")), FLinearColor(0.48f, 0.38f, 0.22f, 1.0f));
		Definition.BaseHealingPerSecond = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseHealingPerSecond")), 5.0f);
		Definition.UnlockStageRequirement = T66MiniToInt(T66MiniGetValue(Row, TEXT("UnlockStageRequirement")), 0);
		Definition.FollowOffsetX = T66MiniToFloat(T66MiniGetValue(Row, TEXT("FollowOffsetX")), -145.0f);
		Definition.FollowOffsetY = T66MiniToFloat(T66MiniGetValue(Row, TEXT("FollowOffsetY")), 110.0f);
		Definition.bUnlockedByDefault = T66MiniToBool(T66MiniGetValue(Row, TEXT("bUnlockedByDefault")), false);

		if (Definition.VisualID.IsEmpty())
		{
			Definition.VisualID = Definition.DisplayName;
		}

		if (Definition.CompanionID != NAME_None)
		{
			Companions.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadDifficulties()
{
	Difficulties.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Difficulties.csv"))))
	{
		FT66MiniDifficultyDefinition Definition;
		Definition.DifficultyID = T66MiniToName(Row, TEXT("DifficultyID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.AccentColor = T66MiniToColor(T66MiniGetValue(Row, TEXT("AccentColor")), FLinearColor(0.34f, 0.44f, 0.60f, 1.0f));
		Definition.HealthScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("HealthScalar")), 1.0f);
		Definition.DamageScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("DamageScalar")), 1.0f);
		Definition.SpeedScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpeedScalar")), 1.0f);
		Definition.SpawnRateScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnRateScalar")), 1.0f);
		Definition.BossScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BossScalar")), 1.0f);
		Definition.InteractableInterval = T66MiniToFloat(T66MiniGetValue(Row, TEXT("InteractableInterval")), 18.0f);
		Definition.StageClearChadCoupons = FMath::Max(0, T66MiniToInt(T66MiniGetValue(Row, TEXT("StageClearChadCoupons")), 0));

		if (Definition.DifficultyID != NAME_None)
		{
			Difficulties.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadStages()
{
	Stages.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Stages.csv"))))
	{
		FT66MiniStageDefinition Definition;
		Definition.StageID = T66MiniToName(Row, TEXT("StageID"));
		Definition.DifficultyID = T66MiniToName(Row, TEXT("DifficultyID"));
		Definition.StageIndex = T66MiniToInt(T66MiniGetValue(Row, TEXT("StageIndex")), 1);
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.WaveIndex = T66MiniToInt(T66MiniGetValue(Row, TEXT("WaveIndex")), Definition.StageIndex);
		Definition.BossID = T66MiniToName(Row, TEXT("BossID"));
		Definition.ClearGoldReward = FMath::Max(0, T66MiniToInt(T66MiniGetValue(Row, TEXT("ClearGoldReward")), 0));
		Definition.ClearMaterialReward = FMath::Max(0, T66MiniToInt(T66MiniGetValue(Row, TEXT("ClearMaterialReward")), 0));
		Definition.ClearChadCoupons = FMath::Max(0, T66MiniToInt(T66MiniGetValue(Row, TEXT("ClearChadCoupons")), 0));
		Definition.NextStageID = T66MiniToName(Row, TEXT("NextStageID"));

		if (Definition.StageID != NAME_None && Definition.DifficultyID != NAME_None && Definition.StageIndex > 0)
		{
			Stages.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadEnemies()
{
	Enemies.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Enemies.csv"))))
	{
		FT66MiniEnemyDefinition Definition;
		Definition.EnemyID = T66MiniToName(Row, TEXT("EnemyID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.VisualID = T66MiniGetValue(Row, TEXT("VisualID"));
		Definition.BehaviorProfile = T66MiniToBehaviorProfile(T66MiniGetValue(Row, TEXT("BehaviorProfile")));
		Definition.Family = T66MiniToEnemyFamily(T66MiniGetValue(Row, TEXT("Family")), Definition.BehaviorProfile, false);
		Definition.BaseHealth = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseHealth")), 30.0f);
		Definition.BaseSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseSpeed")), 260.0f);
		Definition.BaseTouchDamage = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseTouchDamage")), 8.0f);
		Definition.BaseMaterials = T66MiniToInt(T66MiniGetValue(Row, TEXT("BaseMaterials")), 4);
		Definition.BaseExperience = T66MiniToFloat(T66MiniGetValue(Row, TEXT("BaseExperience")), 5.0f);
		Definition.SpawnWeight = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnWeight")), 1.0f);
		Definition.FireIntervalSeconds = T66MiniToFloat(T66MiniGetValue(Row, TEXT("FireIntervalSeconds")), Definition.Family == ET66MiniEnemyFamily::Ranged ? 1.7f : 2.2f);
		Definition.ProjectileSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("ProjectileSpeed")), 980.0f);
		Definition.ProjectileDamage = T66MiniToFloat(T66MiniGetValue(Row, TEXT("ProjectileDamage")), Definition.BaseTouchDamage * 0.9f);
		Definition.PreferredRange = T66MiniToFloat(T66MiniGetValue(Row, TEXT("PreferredRange")), Definition.Family == ET66MiniEnemyFamily::Ranged ? 900.0f : 220.0f);

		if (Definition.EnemyID != NAME_None)
		{
			Enemies.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadBosses()
{
	Bosses.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Bosses.csv"))))
	{
		FT66MiniBossDefinition Definition;
		const FString BossProfile = T66MiniGetValue(Row, TEXT("BossPartProfile"));
		Definition.BossID = T66MiniToName(Row, TEXT("BossID"));
		Definition.DisplayName = Definition.BossID.ToString().Replace(TEXT("_"), TEXT(" "));
		Definition.VisualID = T66MiniBossProfileToVisualID(BossProfile);
		Definition.BehaviorProfile = T66MiniToBehaviorProfile(BossProfile);
		Definition.Family = ET66MiniEnemyFamily::Boss;
		Definition.MaxHealth = T66MiniToFloat(T66MiniGetValue(Row, TEXT("MaxHP")), 1500.0f);
		Definition.MoveSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("MoveSpeed")), 360.0f);
		Definition.TouchDamage = FMath::Max(16.0f, T66MiniToFloat(T66MiniGetValue(Row, TEXT("ProjectileDamageHearts")), 1.0f) * 14.0f);
		Definition.MaterialReward = FMath::Max(20, T66MiniToInt(T66MiniGetValue(Row, TEXT("PointValue")), 500) / 40);
		Definition.ExperienceReward = FMath::Max(18.0f, T66MiniToFloat(T66MiniGetValue(Row, TEXT("PointValue")), 500.0f) / 55.0f);
		Definition.TelegraphSeconds = 1.15f;
		Definition.FireIntervalSeconds = T66MiniToFloat(T66MiniGetValue(Row, TEXT("FireIntervalSeconds")), 1.8f);
		Definition.ProjectileSpeed = T66MiniToFloat(T66MiniGetValue(Row, TEXT("ProjectileSpeed")), 1000.0f);
		Definition.ProjectileDamage = FMath::Max(18.0f, T66MiniToFloat(T66MiniGetValue(Row, TEXT("ProjectileDamageHearts")), 1.0f) * 16.0f);

		if (Definition.BossID != NAME_None)
		{
			Bosses.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadWaves()
{
	Waves.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Waves.csv"))))
	{
		FT66MiniWaveDefinition Definition;
		Definition.DifficultyID = T66MiniToName(Row, TEXT("DifficultyID"));
		Definition.WaveIndex = T66MiniToInt(T66MiniGetValue(Row, TEXT("WaveIndex")), 1);
		Definition.DurationSeconds = T66MiniToFloat(T66MiniGetValue(Row, TEXT("DurationSeconds")), 60.0f);
		Definition.BossID = T66MiniToName(Row, TEXT("BossID"));
		Definition.EnemyIDs = T66MiniSplitNameList(T66MiniGetValue(Row, TEXT("EnemyIDs")));
		Definition.SpawnInterval = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnInterval")), 1.2f);
		Definition.InteractableInterval = T66MiniToFloat(T66MiniGetValue(Row, TEXT("InteractableInterval")), 18.0f);
		Definition.EnemyHealthScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("EnemyHealthScalar")), 1.0f);
		Definition.EnemyDamageScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("EnemyDamageScalar")), 1.0f);
		Definition.EnemySpeedScalar = T66MiniToFloat(T66MiniGetValue(Row, TEXT("EnemySpeedScalar")), 1.0f);

		if (Definition.DifficultyID != NAME_None && Definition.WaveIndex > 0)
		{
			Waves.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadInteractables()
{
	Interactables.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Interactables.csv"))))
	{
		FT66MiniInteractableDefinition Definition;
		Definition.InteractableID = T66MiniToName(Row, TEXT("InteractableID"));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Type = T66MiniToInteractableType(T66MiniGetValue(Row, TEXT("Type")));
		Definition.VisualID = T66MiniGetValue(Row, TEXT("VisualID"));
		Definition.LifetimeSeconds = T66MiniToFloat(T66MiniGetValue(Row, TEXT("LifetimeSeconds")), 12.0f);
		Definition.SpawnWeight = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SpawnWeight")), 1.0f);
		Definition.MaterialReward = T66MiniToInt(T66MiniGetValue(Row, TEXT("MaterialReward")), 0);
		Definition.GoldReward = T66MiniToInt(T66MiniGetValue(Row, TEXT("GoldReward")), 0);
		Definition.ExperienceReward = T66MiniToFloat(T66MiniGetValue(Row, TEXT("ExperienceReward")), 0.0f);
		Definition.HealAmount = T66MiniToFloat(T66MiniGetValue(Row, TEXT("HealAmount")), 0.0f);
		Definition.bRequiresManualInteract = T66MiniToBool(T66MiniGetValue(Row, TEXT("bRequiresManualInteract")), Definition.Type == ET66MiniInteractableType::QuickReviveMachine);

		if (Definition.InteractableID != NAME_None)
		{
			Interactables.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadItems()
{
	Items.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_Items.csv"))))
	{
		FT66MiniItemDefinition Definition;
		Definition.ItemID = FName(*T66MiniGetValue(Row, TEXT("ItemID")));
		Definition.IconPath = T66MiniGetValue(Row, TEXT("Icon"));
		Definition.PrimaryStatType = T66MiniGetValue(Row, TEXT("PrimaryStatType"));
		Definition.SecondaryStatType = T66MiniGetValue(Row, TEXT("SecondaryStatType"));
		Definition.BaseBuyGold = T66MiniToInt(T66MiniGetValue(Row, TEXT("BaseBuyGold")));
		Definition.BaseSellGold = T66MiniToInt(T66MiniGetValue(Row, TEXT("BaseSellGold")));

		if (Definition.ItemID != NAME_None)
		{
			Items.Add(MoveTemp(Definition));
		}
	}
}

void UT66MiniDataSubsystem::LoadRuntimeTuning()
{
	RuntimeTuningEntries.Reset();
	RuntimeTuningValues.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_RuntimeTuning.csv"))))
	{
		FT66MiniRuntimeTuningEntry Entry;
		Entry.TuningKey = FName(*T66MiniGetValue(Row, TEXT("TuningKey")));
		Entry.Value = T66MiniToFloat(T66MiniGetValue(Row, TEXT("Value")));

		if (Entry.TuningKey != NAME_None)
		{
			RuntimeTuningValues.Add(Entry.TuningKey, Entry.Value);
			RuntimeTuningEntries.Add(MoveTemp(Entry));
		}
	}
}

void UT66MiniDataSubsystem::LoadCircusGames()
{
	CircusGames.Reset();

	for (const TMap<FString, FString>& Row : T66MiniLoadCsvRows(T66MiniMiniDataPath(TEXT("T66Mini_CircusGames.csv"))))
	{
		FT66MiniCircusGameDefinition Definition;
		Definition.GameID = FName(*T66MiniGetValue(Row, TEXT("GameID")));
		Definition.DisplayName = T66MiniGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66MiniGetValue(Row, TEXT("Description"));
		Definition.IconPath = T66MiniGetValue(Row, TEXT("Icon"));
		Definition.Bet = T66MiniToInt(T66MiniGetValue(Row, TEXT("Bet")));
		Definition.SuccessChance = T66MiniToFloat(T66MiniGetValue(Row, TEXT("SuccessChance")));
		Definition.PushChance = T66MiniToFloat(T66MiniGetValue(Row, TEXT("PushChance")));
		Definition.PayoutMultiplier = T66MiniToFloat(T66MiniGetValue(Row, TEXT("PayoutMultiplier")));
		Definition.FlatPayout = T66MiniToInt(T66MiniGetValue(Row, TEXT("FlatPayout")));
		Definition.MinGoldPayout = T66MiniToInt(T66MiniGetValue(Row, TEXT("MinGoldPayout")));
		Definition.MaxGoldPayout = T66MiniToInt(T66MiniGetValue(Row, TEXT("MaxGoldPayout")));
		Definition.AngerAdd = T66MiniToFloat(T66MiniGetValue(Row, TEXT("AngerAdd")));
		Definition.ItemRewardChance = T66MiniToFloat(T66MiniGetValue(Row, TEXT("ItemRewardChance")));

		if (Definition.GameID != NAME_None)
		{
			CircusGames.Add(MoveTemp(Definition));
		}
	}
}
