// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TDDataSubsystem.h"
#include "Core/T66CsvUtil.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

namespace
{
	FORCEINLINE TArray<TMap<FString, FString>> T66TDLoadCsvRows(const FString& AbsolutePath) { return T66CsvUtil::LoadCsvRows(AbsolutePath); }
	FORCEINLINE FString T66TDGetValue(const TMap<FString, FString>& Row, const TCHAR* Key) { return T66CsvUtil::GetValue(Row, Key); }
	FORCEINLINE float T66TDToFloat(const FString& Value, const float DefaultValue = 0.f) { return T66CsvUtil::ToFloat(Value, DefaultValue); }
	FORCEINLINE int32 T66TDToInt(const FString& Value, const int32 DefaultValue = 0) { return T66CsvUtil::ToInt(Value, DefaultValue); }
	FORCEINLINE bool T66TDToBool(const FString& Value, const bool bDefaultValue = false) { return T66CsvUtil::ToBool(Value, bDefaultValue); }
	FORCEINLINE FLinearColor T66TDToColor(const FString& Value, const FLinearColor& DefaultColor) { return T66CsvUtil::ToColor(Value, DefaultColor); }
	FORCEINLINE TArray<FName> T66TDSplitNameList(const FString& Value) { return T66CsvUtil::SplitNameList(Value); }
	FORCEINLINE FString T66TDDataPath(const TCHAR* FileName) { return T66CsvUtil::ProjectContentPath(TEXT("TD/Data"), FileName); }
}

void UT66TDDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadData();
}

void UT66TDDataSubsystem::ReloadData()
{
	LoadHeroes();
	LoadHeroCombatDefinitions();
	LoadEnemyArchetypes();
	LoadBattleTuning();
	LoadThemeModifierRules();
	LoadDifficulties();
	LoadMaps();
	LoadStages();
	LoadLayouts();
}

const FT66TDHeroDefinition* UT66TDDataSubsystem::FindHero(const FName HeroID) const { return Heroes.FindByPredicate([HeroID](const FT66TDHeroDefinition& D) { return D.HeroID == HeroID; }); }
const FT66TDHeroCombatDefinition* UT66TDDataSubsystem::FindHeroCombatDefinition(const FName HeroID) const { return HeroCombatDefinitions.FindByPredicate([HeroID](const FT66TDHeroCombatDefinition& D) { return D.HeroID == HeroID; }); }
const FT66TDEnemyArchetypeDefinition* UT66TDDataSubsystem::FindEnemyArchetype(const FName EnemyID) const { return EnemyArchetypes.FindByPredicate([EnemyID](const FT66TDEnemyArchetypeDefinition& D) { return D.EnemyID == EnemyID; }); }
const FT66TDDifficultyDefinition* UT66TDDataSubsystem::FindDifficulty(const FName DifficultyID) const { return Difficulties.FindByPredicate([DifficultyID](const FT66TDDifficultyDefinition& D) { return D.DifficultyID == DifficultyID; }); }
const FT66TDMapDefinition* UT66TDDataSubsystem::FindMap(const FName MapID) const { return Maps.FindByPredicate([MapID](const FT66TDMapDefinition& D) { return D.MapID == MapID; }); }
const FT66TDStageDefinition* UT66TDDataSubsystem::FindStage(const FName StageID) const { return Stages.FindByPredicate([StageID](const FT66TDStageDefinition& D) { return D.StageID == StageID; }); }
const FT66TDStageDefinition* UT66TDDataSubsystem::FindStageForMap(const FName MapID) const { return Stages.FindByPredicate([MapID](const FT66TDStageDefinition& D) { return D.MapID == MapID; }); }
const FT66TDStageDefinition* UT66TDDataSubsystem::FindStage(const FName DifficultyID, const int32 StageIndex) const { return Stages.FindByPredicate([DifficultyID, StageIndex](const FT66TDStageDefinition& D) { return D.DifficultyID == DifficultyID && D.StageIndex == StageIndex; }); }
const FT66TDMapLayoutDefinition* UT66TDDataSubsystem::FindLayout(const FName MapID) const { return Layouts.Find(MapID); }

TArray<const FT66TDMapDefinition*> UT66TDDataSubsystem::GetMapsForDifficulty(const FName DifficultyID) const
{
	TArray<const FT66TDMapDefinition*> MatchingMaps;
	for (const FT66TDMapDefinition& Map : Maps)
	{
		if (Map.DifficultyID == DifficultyID)
		{
			MatchingMaps.Add(&Map);
		}
	}
	return MatchingMaps;
}

TArray<const FT66TDStageDefinition*> UT66TDDataSubsystem::GetStagesForDifficulty(const FName DifficultyID) const
{
	TArray<const FT66TDStageDefinition*> MatchingStages;
	for (const FT66TDStageDefinition& Stage : Stages)
	{
		if (Stage.DifficultyID == DifficultyID)
		{
			MatchingStages.Add(&Stage);
		}
	}
	return MatchingStages;
}

void UT66TDDataSubsystem::LoadHeroes()
{
	Heroes.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_Heroes.csv"))))
	{
		FT66TDHeroDefinition Definition;
		Definition.HeroID = FName(*T66TDGetValue(Row, TEXT("HeroID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66TDGetValue(Row, TEXT("Description"));
		Definition.RoleLabel = T66TDGetValue(Row, TEXT("RoleLabel"));
		Definition.Signature = T66TDGetValue(Row, TEXT("Signature"));
		Definition.UltimateLabel = T66TDGetValue(Row, TEXT("UltimateLabel"));
		Definition.PassiveLabel = T66TDGetValue(Row, TEXT("PassiveLabel"));
		Definition.PlaceholderColor = T66TDToColor(T66TDGetValue(Row, TEXT("PlaceholderColor")), FLinearColor(0.26f, 0.30f, 0.36f, 1.0f));

		if (Definition.HeroID != NAME_None)
		{
			Heroes.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadHeroCombatDefinitions()
{
	HeroCombatDefinitions.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_HeroCombat.csv"))))
	{
		FT66TDHeroCombatDefinition Definition;
		Definition.HeroID = FName(*T66TDGetValue(Row, TEXT("HeroID")));
		Definition.Cost = T66TDToInt(T66TDGetValue(Row, TEXT("Cost")));
		Definition.Damage = T66TDToFloat(T66TDGetValue(Row, TEXT("Damage")));
		Definition.Range = T66TDToFloat(T66TDGetValue(Row, TEXT("Range")));
		Definition.FireInterval = T66TDToFloat(T66TDGetValue(Row, TEXT("FireInterval")));
		Definition.ChainBounces = T66TDToInt(T66TDGetValue(Row, TEXT("ChainBounces")));
		Definition.ChainRadius = T66TDToFloat(T66TDGetValue(Row, TEXT("ChainRadius")));
		Definition.SplashRadius = T66TDToFloat(T66TDGetValue(Row, TEXT("SplashRadius")));
		Definition.DotDamagePerSecond = T66TDToFloat(T66TDGetValue(Row, TEXT("DotDamagePerSecond")));
		Definition.DotDuration = T66TDToFloat(T66TDGetValue(Row, TEXT("DotDuration")));
		Definition.SlowMultiplier = T66TDToFloat(T66TDGetValue(Row, TEXT("SlowMultiplier")), 1.0f);
		Definition.SlowDuration = T66TDToFloat(T66TDGetValue(Row, TEXT("SlowDuration")));
		Definition.BossDamageMultiplier = T66TDToFloat(T66TDGetValue(Row, TEXT("BossDamageMultiplier")), 1.0f);
		Definition.FlatArmorPierce = T66TDToFloat(T66TDGetValue(Row, TEXT("FlatArmorPierce")));
		Definition.ShieldDamageMultiplier = T66TDToFloat(T66TDGetValue(Row, TEXT("ShieldDamageMultiplier")), 1.0f);
		Definition.bCanTargetHidden = T66TDToBool(T66TDGetValue(Row, TEXT("bCanTargetHidden")));
		Definition.bPrioritizeBoss = T66TDToBool(T66TDGetValue(Row, TEXT("bPrioritizeBoss")));
		Definition.VolleyShots = T66TDToInt(T66TDGetValue(Row, TEXT("VolleyShots")), 1);
		Definition.BonusMaterialsOnKill = T66TDToInt(T66TDGetValue(Row, TEXT("BonusMaterialsOnKill")));
		Definition.CombatLabel = T66TDGetValue(Row, TEXT("CombatLabel"));

		if (Definition.HeroID != NAME_None)
		{
			HeroCombatDefinitions.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadEnemyArchetypes()
{
	EnemyArchetypes.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_EnemyArchetypes.csv"))))
	{
		FT66TDEnemyArchetypeDefinition Definition;
		Definition.EnemyID = FName(*T66TDGetValue(Row, TEXT("EnemyID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.VisualID = T66TDGetValue(Row, TEXT("VisualID"));
		Definition.BaseHealth = T66TDToFloat(T66TDGetValue(Row, TEXT("BaseHealth")));
		Definition.BaseSpeed = T66TDToFloat(T66TDGetValue(Row, TEXT("BaseSpeed")));
		Definition.LeakDamage = T66TDToInt(T66TDGetValue(Row, TEXT("LeakDamage")));
		Definition.Bounty = T66TDToInt(T66TDGetValue(Row, TEXT("Bounty")));
		Definition.Radius = T66TDToFloat(T66TDGetValue(Row, TEXT("Radius")));
		Definition.Tint = T66TDToColor(T66TDGetValue(Row, TEXT("Tint")), FLinearColor::White);

		if (Definition.EnemyID != NAME_None)
		{
			EnemyArchetypes.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadBattleTuning()
{
	BattleTuningValues.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_BattleTuning.csv"))))
	{
		const FName TuningKey(*T66TDGetValue(Row, TEXT("TuningKey")));
		if (TuningKey != NAME_None)
		{
			BattleTuningValues.Add(TuningKey, T66TDToFloat(T66TDGetValue(Row, TEXT("Value"))));
		}
	}
}

void UT66TDDataSubsystem::LoadThemeModifierRules()
{
	ThemeModifierRules.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_ThemeModifierRules.csv"))))
	{
		FT66TDThemeModifierRule Rule;
		Rule.ThemeLabel = T66TDGetValue(Row, TEXT("ThemeLabel"));
		Rule.BossVisualID = T66TDGetValue(Row, TEXT("BossVisualID"));
		Rule.BossModifierMask = T66TDToInt(T66TDGetValue(Row, TEXT("BossModifierMask")));
		Rule.HiddenMinWave = T66TDToInt(T66TDGetValue(Row, TEXT("HiddenMinWave")));
		Rule.ShieldMinWave = T66TDToInt(T66TDGetValue(Row, TEXT("ShieldMinWave")));
		Rule.bEnableRegen = T66TDToBool(T66TDGetValue(Row, TEXT("bEnableRegen")));
		Rule.bShieldGoat = T66TDToBool(T66TDGetValue(Row, TEXT("bShieldGoat")));
		Rule.ArmorChanceBonus = T66TDToFloat(T66TDGetValue(Row, TEXT("ArmorChanceBonus")));
		Rule.HiddenChanceBonus = T66TDToFloat(T66TDGetValue(Row, TEXT("HiddenChanceBonus")));
		Rule.RegenChanceBonus = T66TDToFloat(T66TDGetValue(Row, TEXT("RegenChanceBonus")));
		Rule.ShieldChanceBonus = T66TDToFloat(T66TDGetValue(Row, TEXT("ShieldChanceBonus")));

		if (!Rule.ThemeLabel.IsEmpty())
		{
			ThemeModifierRules.Add(MoveTemp(Rule));
		}
	}
}

void UT66TDDataSubsystem::LoadDifficulties()
{
	Difficulties.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_Difficulties.csv"))))
	{
		FT66TDDifficultyDefinition Definition;
		Definition.DifficultyID = FName(*T66TDGetValue(Row, TEXT("DifficultyID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66TDGetValue(Row, TEXT("Description"));
		Definition.AccentColor = T66TDToColor(T66TDGetValue(Row, TEXT("AccentColor")), FLinearColor(0.32f, 0.44f, 0.68f, 1.0f));
		Definition.MapCount = T66TDToInt(T66TDGetValue(Row, TEXT("MapCount")), 4);
		Definition.EnemyHealthScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("EnemyHealthScalar")), 1.0f);
		Definition.EnemySpeedScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("EnemySpeedScalar")), 1.0f);
		Definition.BossScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("BossScalar")), 1.0f);
		Definition.RewardScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("RewardScalar")), 1.0f);
		Definition.StageClearChadCoupons = FMath::Max(0, T66TDToInt(T66TDGetValue(Row, TEXT("StageClearChadCoupons")), 0));

		if (Definition.DifficultyID != NAME_None)
		{
			Difficulties.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadMaps()
{
	Maps.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_Maps.csv"))))
	{
		FT66TDMapDefinition Definition;
		Definition.MapID = FName(*T66TDGetValue(Row, TEXT("MapID")));
		Definition.DifficultyID = FName(*T66TDGetValue(Row, TEXT("DifficultyID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.ThemeLabel = T66TDGetValue(Row, TEXT("ThemeLabel"));
		Definition.Description = T66TDGetValue(Row, TEXT("Description"));
		Definition.BackgroundRelativePath = T66TDGetValue(Row, TEXT("BackgroundRelativePath"));
		Definition.LaneCount = T66TDToInt(T66TDGetValue(Row, TEXT("LaneCount")), 1);
		Definition.BuildPadCount = T66TDToInt(T66TDGetValue(Row, TEXT("BuildPadCount")), 12);
		Definition.BossWave = T66TDToInt(T66TDGetValue(Row, TEXT("BossWave")), 15);
		Definition.EnemyNotes = T66TDGetValue(Row, TEXT("EnemyNotes"));
		Definition.FeaturedHeroIDs = T66TDSplitNameList(T66TDGetValue(Row, TEXT("FeaturedHeroIDs")));

		if (Definition.MapID != NAME_None && Definition.DifficultyID != NAME_None)
		{
			Maps.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadStages()
{
	Stages.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_Stages.csv"))))
	{
		FT66TDStageDefinition Definition;
		Definition.StageID = FName(*T66TDGetValue(Row, TEXT("StageID")));
		Definition.DifficultyID = FName(*T66TDGetValue(Row, TEXT("DifficultyID")));
		Definition.StageIndex = T66TDToInt(T66TDGetValue(Row, TEXT("StageIndex")), 1);
		Definition.MapID = FName(*T66TDGetValue(Row, TEXT("MapID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66TDGetValue(Row, TEXT("Description"));
		Definition.BossWave = FMath::Max(1, T66TDToInt(T66TDGetValue(Row, TEXT("BossWave")), 15));
		Definition.StartingGold = FMath::Max(0, T66TDToInt(T66TDGetValue(Row, TEXT("StartingGold")), 0));
		Definition.StartingMaterials = FMath::Max(0, T66TDToInt(T66TDGetValue(Row, TEXT("StartingMaterials")), 0));
		Definition.ClearGoldReward = FMath::Max(0, T66TDToInt(T66TDGetValue(Row, TEXT("ClearGoldReward")), 0));
		Definition.ClearMaterialReward = FMath::Max(0, T66TDToInt(T66TDGetValue(Row, TEXT("ClearMaterialReward")), 0));
		Definition.ClearChadCoupons = FMath::Max(0, T66TDToInt(T66TDGetValue(Row, TEXT("ClearChadCoupons")), 0));
		Definition.NextStageID = FName(*T66TDGetValue(Row, TEXT("NextStageID")));

		if (Definition.StageID != NAME_None && Definition.DifficultyID != NAME_None && Definition.MapID != NAME_None && Definition.StageIndex > 0)
		{
			Stages.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadLayouts()
{
	Layouts.Reset();

	FString RawText;
	if (!FFileHelper::LoadFileToString(RawText, *T66TDDataPath(TEXT("T66TD_Layouts.json"))))
	{
		return;
	}

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawText);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return;
	}

	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : RootObject->Values)
	{
		const TSharedPtr<FJsonObject>* LayoutObject = nullptr;
		if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(LayoutObject) || !LayoutObject || !LayoutObject->IsValid())
		{
			continue;
		}

		FT66TDMapLayoutDefinition Layout;
		Layout.MapID = FName(*Pair.Key);

		const TArray<TSharedPtr<FJsonValue>>* PathsArray = nullptr;
		if ((*LayoutObject)->TryGetArrayField(TEXT("paths"), PathsArray) && PathsArray)
		{
			for (const TSharedPtr<FJsonValue>& PathValue : *PathsArray)
			{
				const TArray<TSharedPtr<FJsonValue>>* PathPointsArray = nullptr;
				if (!PathValue.IsValid() || !PathValue->TryGetArray(PathPointsArray) || !PathPointsArray)
				{
					continue;
				}

				TArray<FVector2D> PathPoints;
				for (const TSharedPtr<FJsonValue>& PointValue : *PathPointsArray)
				{
					const TSharedPtr<FJsonObject>* PointObject = nullptr;
					if (!PointValue.IsValid() || !PointValue->TryGetObject(PointObject) || !PointObject || !PointObject->IsValid())
					{
						continue;
					}

					PathPoints.Add(FVector2D(
						(*PointObject)->GetNumberField(TEXT("x")),
						(*PointObject)->GetNumberField(TEXT("y"))));
				}

				if (PathPoints.Num() >= 2)
				{
					Layout.Paths.Add(MoveTemp(PathPoints));
				}
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* PadsArray = nullptr;
		if ((*LayoutObject)->TryGetArrayField(TEXT("pads"), PadsArray) && PadsArray)
		{
			for (const TSharedPtr<FJsonValue>& PadValue : *PadsArray)
			{
				const TSharedPtr<FJsonObject>* PadObject = nullptr;
				if (!PadValue.IsValid() || !PadValue->TryGetObject(PadObject) || !PadObject || !PadObject->IsValid())
				{
					continue;
				}

				FT66TDMapPadDefinition Pad;
				Pad.PositionNormalized = FVector2D(
					(*PadObject)->GetNumberField(TEXT("x")),
					(*PadObject)->GetNumberField(TEXT("y")));
				Pad.RadiusNormalized = (*PadObject)->GetNumberField(TEXT("radius"));
				Layout.Pads.Add(Pad);
			}
		}

		if (Layout.MapID != NAME_None && Layout.Paths.Num() > 0 && Layout.Pads.Num() > 0)
		{
			Layouts.Add(Layout.MapID, MoveTemp(Layout));
		}
	}
}
