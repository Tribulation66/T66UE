// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66IdleDataSubsystem.h"
#include "Core/T66CsvUtil.h"

namespace
{
	FORCEINLINE TArray<TMap<FString, FString>> T66IdleLoadCsvRows(const FString& AbsolutePath) { return T66CsvUtil::LoadCsvRows(AbsolutePath, TEXT("T66IdleDataSubsystem")); }
	FORCEINLINE FString T66IdleGetValue(const TMap<FString, FString>& Row, const TCHAR* Key) { return T66CsvUtil::GetValue(Row, Key); }
	FORCEINLINE double T66IdleToDouble(const FString& Value, const double DefaultValue = 0.0) { return T66CsvUtil::ToDouble(Value, DefaultValue); }
	FORCEINLINE int32 T66IdleToInt(const FString& Value, const int32 DefaultValue = 0) { return T66CsvUtil::ToInt(Value, DefaultValue); }
	FORCEINLINE FName T66IdleToName(const TMap<FString, FString>& Row, const TCHAR* Key) { return T66CsvUtil::ToName(Row, Key); }
	FORCEINLINE FLinearColor T66IdleToColor(const FString& Value, const FLinearColor& DefaultColor) { return T66CsvUtil::ToColor(Value, DefaultColor); }
	FORCEINLINE TArray<FName> T66IdleSplitNameList(const FString& Value) { return T66CsvUtil::SplitNameList(Value); }
	FORCEINLINE FString T66IdleDataPath(const TCHAR* FileName) { return T66CsvUtil::ProjectContentPath(TEXT("Idle/Data"), FileName); }
}

void UT66IdleDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadData();
}

void UT66IdleDataSubsystem::ReloadData()
{
	LoadHeroes();
	LoadCompanions();
	LoadItems();
	LoadIdols();
	LoadEnemies();
	LoadZones();
	LoadStages();
	LoadTuning();
}

const FT66IdleHeroDefinition* UT66IdleDataSubsystem::FindHero(const FName HeroID) const { return Heroes.FindByPredicate([HeroID](const FT66IdleHeroDefinition& D) { return D.HeroID == HeroID; }); }
const FT66IdleCompanionDefinition* UT66IdleDataSubsystem::FindCompanion(const FName CompanionID) const { return Companions.FindByPredicate([CompanionID](const FT66IdleCompanionDefinition& D) { return D.CompanionID == CompanionID; }); }
const FT66IdleItemDefinition* UT66IdleDataSubsystem::FindItem(const FName ItemID) const { return Items.FindByPredicate([ItemID](const FT66IdleItemDefinition& D) { return D.ItemID == ItemID; }); }
const FT66IdleIdolDefinition* UT66IdleDataSubsystem::FindIdol(const FName IdolID) const { return Idols.FindByPredicate([IdolID](const FT66IdleIdolDefinition& D) { return D.IdolID == IdolID; }); }
const FT66IdleEnemyDefinition* UT66IdleDataSubsystem::FindEnemy(const FName EnemyID) const { return Enemies.FindByPredicate([EnemyID](const FT66IdleEnemyDefinition& D) { return D.EnemyID == EnemyID; }); }
const FT66IdleZoneDefinition* UT66IdleDataSubsystem::FindZone(const FName ZoneID) const { return Zones.FindByPredicate([ZoneID](const FT66IdleZoneDefinition& D) { return D.ZoneID == ZoneID; }); }
const FT66IdleZoneDefinition* UT66IdleDataSubsystem::FindZoneForStage(const int32 Stage) const { return Zones.FindByPredicate([Stage](const FT66IdleZoneDefinition& D) { return Stage >= D.FirstStage && Stage < D.FirstStage + D.StageCount; }); }
const FT66IdleStageDefinition* UT66IdleDataSubsystem::FindStage(const FName StageID) const { return Stages.FindByPredicate([StageID](const FT66IdleStageDefinition& D) { return D.StageID == StageID; }); }
const FT66IdleStageDefinition* UT66IdleDataSubsystem::FindStageByIndex(const int32 StageIndex) const { return Stages.FindByPredicate([StageIndex](const FT66IdleStageDefinition& D) { return D.StageIndex == StageIndex; }); }

void UT66IdleDataSubsystem::LoadHeroes()
{
	Heroes.Reset();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Heroes.csv"))))
	{
		FT66IdleHeroDefinition Definition;
		Definition.HeroID = T66IdleToName(Row, TEXT("HeroID"));
		Definition.DisplayName = T66IdleGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66IdleGetValue(Row, TEXT("Description"));
		Definition.RoleLabel = T66IdleGetValue(Row, TEXT("RoleLabel"));
		Definition.AccentColor = T66IdleToColor(T66IdleGetValue(Row, TEXT("AccentColor")), FLinearColor(0.86f, 0.62f, 0.22f, 1.0f));
		Definition.HireCostGold = T66IdleToDouble(T66IdleGetValue(Row, TEXT("HireCostGold")));
		Definition.TapDamage = T66IdleToDouble(T66IdleGetValue(Row, TEXT("TapDamage")), 1.0);
		Definition.PassiveDamagePerSecond = T66IdleToDouble(T66IdleGetValue(Row, TEXT("PassiveDamagePerSecond")));
		Definition.TapDamagePerLevel = T66IdleToDouble(T66IdleGetValue(Row, TEXT("TapDamagePerLevel")), 1.0);
		Definition.PassiveDamagePerLevel = T66IdleToDouble(T66IdleGetValue(Row, TEXT("PassiveDamagePerLevel")));
		Definition.MaxLevel = T66IdleToInt(T66IdleGetValue(Row, TEXT("MaxLevel")), 1);

		if (Definition.HeroID != NAME_None)
		{
			Heroes.Add(MoveTemp(Definition));
		}
	}
}

void UT66IdleDataSubsystem::LoadCompanions()
{
	Companions.Reset();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Companions.csv"))))
	{
		FT66IdleCompanionDefinition Definition;
		Definition.CompanionID = T66IdleToName(Row, TEXT("CompanionID"));
		Definition.DisplayName = T66IdleGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66IdleGetValue(Row, TEXT("Description"));
		Definition.RoleLabel = T66IdleGetValue(Row, TEXT("RoleLabel"));
		Definition.AccentColor = T66IdleToColor(T66IdleGetValue(Row, TEXT("AccentColor")), FLinearColor(0.42f, 0.72f, 0.86f, 1.0f));
		Definition.HireCostGold = T66IdleToDouble(T66IdleGetValue(Row, TEXT("HireCostGold")));
		Definition.PassiveDamagePerSecond = T66IdleToDouble(T66IdleGetValue(Row, TEXT("PassiveDamagePerSecond")));
		Definition.PassiveDamagePerLevel = T66IdleToDouble(T66IdleGetValue(Row, TEXT("PassiveDamagePerLevel")));
		Definition.MaxLevel = T66IdleToInt(T66IdleGetValue(Row, TEXT("MaxLevel")), 1);

		if (Definition.CompanionID != NAME_None)
		{
			Companions.Add(MoveTemp(Definition));
		}
	}
}

void UT66IdleDataSubsystem::LoadItems()
{
	Items.Reset();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Items.csv"))))
	{
		FT66IdleItemDefinition Definition;
		Definition.ItemID = T66IdleToName(Row, TEXT("ItemID"));
		Definition.DisplayName = T66IdleGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66IdleGetValue(Row, TEXT("Description"));
		Definition.Category = T66IdleGetValue(Row, TEXT("Category"));
		Definition.AccentColor = T66IdleToColor(T66IdleGetValue(Row, TEXT("AccentColor")), FLinearColor(0.70f, 0.54f, 0.22f, 1.0f));
		Definition.BaseCostGold = T66IdleToDouble(T66IdleGetValue(Row, TEXT("BaseCostGold")));
		Definition.TapDamageBonus = T66IdleToDouble(T66IdleGetValue(Row, TEXT("TapDamageBonus")));
		Definition.PassiveDamageBonus = T66IdleToDouble(T66IdleGetValue(Row, TEXT("PassiveDamageBonus")));
		Definition.GoldRewardMultiplier = T66IdleToDouble(T66IdleGetValue(Row, TEXT("GoldRewardMultiplier")), 1.0);
		Definition.MaxLevel = T66IdleToInt(T66IdleGetValue(Row, TEXT("MaxLevel")), 1);

		if (Definition.ItemID != NAME_None)
		{
			Items.Add(MoveTemp(Definition));
		}
	}
}

void UT66IdleDataSubsystem::LoadIdols()
{
	Idols.Reset();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Idols.csv"))))
	{
		FT66IdleIdolDefinition Definition;
		Definition.IdolID = T66IdleToName(Row, TEXT("IdolID"));
		Definition.DisplayName = T66IdleGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66IdleGetValue(Row, TEXT("Description"));
		Definition.Category = T66IdleGetValue(Row, TEXT("Category"));
		Definition.AccentColor = T66IdleToColor(T66IdleGetValue(Row, TEXT("AccentColor")), FLinearColor(0.62f, 0.34f, 0.88f, 1.0f));
		Definition.BaseCostGold = T66IdleToDouble(T66IdleGetValue(Row, TEXT("BaseCostGold")));
		Definition.TapDamageMultiplier = T66IdleToDouble(T66IdleGetValue(Row, TEXT("TapDamageMultiplier")), 1.0);
		Definition.PassiveDamageMultiplier = T66IdleToDouble(T66IdleGetValue(Row, TEXT("PassiveDamageMultiplier")), 1.0);
		Definition.GoldRewardMultiplier = T66IdleToDouble(T66IdleGetValue(Row, TEXT("GoldRewardMultiplier")), 1.0);
		Definition.MaxLevel = T66IdleToInt(T66IdleGetValue(Row, TEXT("MaxLevel")), 1);

		if (Definition.IdolID != NAME_None)
		{
			Idols.Add(MoveTemp(Definition));
		}
	}
}

void UT66IdleDataSubsystem::LoadEnemies()
{
	Enemies.Reset();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Enemies.csv"))))
	{
		FT66IdleEnemyDefinition Definition;
		Definition.EnemyID = T66IdleToName(Row, TEXT("EnemyID"));
		Definition.DisplayName = T66IdleGetValue(Row, TEXT("DisplayName"));
		Definition.ArchetypeLabel = T66IdleGetValue(Row, TEXT("ArchetypeLabel"));
		Definition.AccentColor = T66IdleToColor(T66IdleGetValue(Row, TEXT("AccentColor")), FLinearColor(0.82f, 0.24f, 0.18f, 1.0f));
		Definition.BaseHealth = T66IdleToDouble(T66IdleGetValue(Row, TEXT("BaseHealth")), 10.0);
		Definition.HealthPerStage = T66IdleToDouble(T66IdleGetValue(Row, TEXT("HealthPerStage")), 4.0);
		Definition.BaseGoldReward = T66IdleToDouble(T66IdleGetValue(Row, TEXT("BaseGoldReward")), 1.0);
		Definition.GoldPerStage = T66IdleToDouble(T66IdleGetValue(Row, TEXT("GoldPerStage")), 0.65);

		if (Definition.EnemyID != NAME_None)
		{
			Enemies.Add(MoveTemp(Definition));
		}
	}
}

void UT66IdleDataSubsystem::LoadZones()
{
	Zones.Reset();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Zones.csv"))))
	{
		FT66IdleZoneDefinition Definition;
		Definition.ZoneID = T66IdleToName(Row, TEXT("ZoneID"));
		Definition.DisplayName = T66IdleGetValue(Row, TEXT("DisplayName"));
		Definition.ThemeLabel = T66IdleGetValue(Row, TEXT("ThemeLabel"));
		Definition.FirstStage = T66IdleToInt(T66IdleGetValue(Row, TEXT("FirstStage")), 1);
		Definition.StageCount = T66IdleToInt(T66IdleGetValue(Row, TEXT("StageCount")), 10);
		Definition.BaseEnemyHealth = T66IdleToDouble(T66IdleGetValue(Row, TEXT("BaseEnemyHealth")), 10.0);
		Definition.BaseGoldReward = T66IdleToDouble(T66IdleGetValue(Row, TEXT("BaseGoldReward")), 1.0);
		Definition.EnemyIDs = T66IdleSplitNameList(T66IdleGetValue(Row, TEXT("EnemyIDs")));
		Definition.EnemyHealthScalar = T66IdleToDouble(T66IdleGetValue(Row, TEXT("EnemyHealthScalar")), 1.0);
		Definition.GoldRewardScalar = T66IdleToDouble(T66IdleGetValue(Row, TEXT("GoldRewardScalar")), 1.0);

		if (Definition.ZoneID != NAME_None)
		{
			Zones.Add(MoveTemp(Definition));
		}
	}
}

void UT66IdleDataSubsystem::LoadStages()
{
	Stages.Reset();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Stages.csv"))))
	{
		FT66IdleStageDefinition Definition;
		Definition.StageID = T66IdleToName(Row, TEXT("StageID"));
		Definition.StageIndex = T66IdleToInt(T66IdleGetValue(Row, TEXT("StageIndex")), 1);
		Definition.DisplayName = T66IdleGetValue(Row, TEXT("DisplayName"));
		Definition.ZoneID = T66IdleToName(Row, TEXT("ZoneID"));
		Definition.EnemyID = T66IdleToName(Row, TEXT("EnemyID"));
		Definition.BossEnemyID = T66IdleToName(Row, TEXT("BossEnemyID"));
		Definition.bBossStage = !Definition.BossEnemyID.IsNone()
			|| T66IdleGetValue(Row, TEXT("bBossStage")).Equals(TEXT("true"), ESearchCase::IgnoreCase)
			|| T66IdleGetValue(Row, TEXT("bBossStage")).Equals(TEXT("1"), ESearchCase::IgnoreCase);
		Definition.ClearGoldReward = FMath::Max(0.0, T66IdleToDouble(T66IdleGetValue(Row, TEXT("ClearGoldReward")), 0.0));
		Definition.NextStageID = T66IdleToName(Row, TEXT("NextStageID"));

		if (Definition.StageID != NAME_None && Definition.StageIndex > 0)
		{
			Stages.Add(MoveTemp(Definition));
		}
	}
}

void UT66IdleDataSubsystem::LoadTuning()
{
	Tuning = FT66IdleTuningDefinition();

	for (const TMap<FString, FString>& Row : T66IdleLoadCsvRows(T66IdleDataPath(TEXT("T66Idle_Tuning.csv"))))
	{
		FT66IdleTuningDefinition Definition;
		Definition.TuningID = T66IdleToName(Row, TEXT("TuningID"));
		Definition.StartingHeroID = T66IdleToName(Row, TEXT("StartingHeroID"));
		Definition.StartingStage = T66IdleToInt(T66IdleGetValue(Row, TEXT("StartingStage")), 1);
		Definition.StartingGold = T66IdleToDouble(T66IdleGetValue(Row, TEXT("StartingGold")));
		Definition.StartingTapDamage = T66IdleToDouble(T66IdleGetValue(Row, TEXT("StartingTapDamage")), 1.0);
		Definition.StartingPassiveDamagePerSecond = T66IdleToDouble(T66IdleGetValue(Row, TEXT("StartingPassiveDamagePerSecond")), 0.5);
		Definition.OfflineProgressCapHours = T66IdleToDouble(T66IdleGetValue(Row, TEXT("OfflineProgressCapHours")), 12.0);
		Definition.AutosaveIntervalSeconds = T66IdleToDouble(T66IdleGetValue(Row, TEXT("AutosaveIntervalSeconds")), 10.0);
		Definition.EngineProgressPerPassiveDamageSecond = T66IdleToDouble(T66IdleGetValue(Row, TEXT("EngineProgressPerPassiveDamageSecond")), 0.035);
		Definition.EngineGoldPayoutMultiplier = T66IdleToDouble(T66IdleGetValue(Row, TEXT("EngineGoldPayoutMultiplier")), 1.0);
		Definition.TapUpgradeBaseCost = T66IdleToDouble(T66IdleGetValue(Row, TEXT("TapUpgradeBaseCost")), 12.0);
		Definition.TapUpgradeCostPerDamage = T66IdleToDouble(T66IdleGetValue(Row, TEXT("TapUpgradeCostPerDamage")), 12.0);
		Definition.TapUpgradeDamageStep = T66IdleToDouble(T66IdleGetValue(Row, TEXT("TapUpgradeDamageStep")), 1.0);
		Definition.EngineUpgradeBaseCost = T66IdleToDouble(T66IdleGetValue(Row, TEXT("EngineUpgradeBaseCost")), 18.0);
		Definition.EngineUpgradeCostPerDps = T66IdleToDouble(T66IdleGetValue(Row, TEXT("EngineUpgradeCostPerDps")), 18.0);
		Definition.EngineUpgradeDpsStep = T66IdleToDouble(T66IdleGetValue(Row, TEXT("EngineUpgradeDpsStep")), 0.5);

		if (Definition.TuningID != NAME_None)
		{
			Tuning = MoveTemp(Definition);
			return;
		}
	}
}
