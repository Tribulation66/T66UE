// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66DeckDataSubsystem.h"
#include "Core/T66CsvUtil.h"

namespace
{
	ET66DeckCardType T66DeckToCardType(const FString& Value)
	{
		if (Value.Equals(TEXT("Skill"), ESearchCase::IgnoreCase)) return ET66DeckCardType::Skill;
		if (Value.Equals(TEXT("Power"), ESearchCase::IgnoreCase)) return ET66DeckCardType::Power;
		if (Value.Equals(TEXT("Curse"), ESearchCase::IgnoreCase)) return ET66DeckCardType::Curse;
		return ET66DeckCardType::Attack;
	}

	ET66DeckNodeType T66DeckToNodeType(const FString& Value)
	{
		if (Value.Equals(TEXT("Elite"), ESearchCase::IgnoreCase)) return ET66DeckNodeType::Elite;
		if (Value.Equals(TEXT("Boss"), ESearchCase::IgnoreCase)) return ET66DeckNodeType::Boss;
		if (Value.Equals(TEXT("Rest"), ESearchCase::IgnoreCase)) return ET66DeckNodeType::Rest;
		if (Value.Equals(TEXT("Shop"), ESearchCase::IgnoreCase)) return ET66DeckNodeType::Shop;
		if (Value.Equals(TEXT("Event"), ESearchCase::IgnoreCase)) return ET66DeckNodeType::Event;
		if (Value.Equals(TEXT("Treasure"), ESearchCase::IgnoreCase)) return ET66DeckNodeType::Treasure;
		return ET66DeckNodeType::Combat;
	}

	FORCEINLINE TArray<TMap<FString, FString>> T66DeckLoadCsvRows(const FString& AbsolutePath) { return T66CsvUtil::LoadCsvRows(AbsolutePath, TEXT("T66DeckDataSubsystem")); }
	FORCEINLINE FString T66DeckGetValue(const TMap<FString, FString>& Row, const TCHAR* Key) { return T66CsvUtil::GetValue(Row, Key); }
	FORCEINLINE int32 T66DeckToInt(const FString& Value, const int32 DefaultValue = 0) { return T66CsvUtil::ToInt(Value, DefaultValue); }
	FORCEINLINE FName T66DeckToName(const TMap<FString, FString>& Row, const TCHAR* Key) { return T66CsvUtil::ToName(Row, Key); }
	FORCEINLINE FLinearColor T66DeckToColor(const FString& Value, const FLinearColor& DefaultColor) { return T66CsvUtil::ToColor(Value, DefaultColor); }
	FORCEINLINE TArray<FName> T66DeckSplitNameList(const FString& Value) { return T66CsvUtil::SplitNameList(Value); }
	FORCEINLINE FString T66DeckDataPath(const TCHAR* FileName) { return T66CsvUtil::ProjectContentPath(TEXT("Deck/Data"), FileName); }
}

void UT66DeckDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadData();
}

void UT66DeckDataSubsystem::ReloadData()
{
	LoadHeroes();
	LoadCompanions();
	LoadCards();
	LoadRelics();
	LoadEnemies();
	LoadItems();
	LoadEncounters();
	LoadStartingDecks();
	LoadStages();
	LoadTuning();
}

const FT66DeckHeroDefinition* UT66DeckDataSubsystem::FindHero(const FName HeroID) const { return Heroes.FindByPredicate([HeroID](const FT66DeckHeroDefinition& D) { return D.HeroID == HeroID; }); }
const FT66DeckCompanionDefinition* UT66DeckDataSubsystem::FindCompanion(const FName CompanionID) const { return Companions.FindByPredicate([CompanionID](const FT66DeckCompanionDefinition& D) { return D.CompanionID == CompanionID; }); }
const FT66DeckCardDefinition* UT66DeckDataSubsystem::FindCard(const FName CardID) const { return Cards.FindByPredicate([CardID](const FT66DeckCardDefinition& D) { return D.CardID == CardID; }); }
const FT66DeckRelicDefinition* UT66DeckDataSubsystem::FindRelic(const FName RelicID) const { return Relics.FindByPredicate([RelicID](const FT66DeckRelicDefinition& D) { return D.RelicID == RelicID; }); }
const FT66DeckEnemyDefinition* UT66DeckDataSubsystem::FindEnemy(const FName EnemyID) const { return Enemies.FindByPredicate([EnemyID](const FT66DeckEnemyDefinition& D) { return D.EnemyID == EnemyID; }); }
const FT66DeckItemDefinition* UT66DeckDataSubsystem::FindItem(const FName ItemID) const { return Items.FindByPredicate([ItemID](const FT66DeckItemDefinition& D) { return D.ItemID == ItemID; }); }
const FT66DeckEncounterDefinition* UT66DeckDataSubsystem::FindEncounter(const FName EncounterID) const { return Encounters.FindByPredicate([EncounterID](const FT66DeckEncounterDefinition& D) { return D.EncounterID == EncounterID; }); }
const FT66DeckStartingDeckDefinition* UT66DeckDataSubsystem::FindStartingDeck(const FName StartingDeckID) const { return StartingDecks.FindByPredicate([StartingDeckID](const FT66DeckStartingDeckDefinition& D) { return D.StartingDeckID == StartingDeckID; }); }
const FT66DeckStageDefinition* UT66DeckDataSubsystem::FindStage(const FName StageID) const { return Stages.FindByPredicate([StageID](const FT66DeckStageDefinition& D) { return D.StageID == StageID; }); }
const FT66DeckStageDefinition* UT66DeckDataSubsystem::FindStageForFloor(const int32 FloorIndex) const { return Stages.FindByPredicate([FloorIndex](const FT66DeckStageDefinition& D) { return FloorIndex >= D.FirstFloor && FloorIndex <= D.BossFloor; }); }

TArray<const FT66DeckEncounterDefinition*> UT66DeckDataSubsystem::GetEncountersForFloor(const int32 FloorIndex) const
{
	TArray<const FT66DeckEncounterDefinition*> MatchingEncounters;
	if (const FT66DeckStageDefinition* Stage = FindStageForFloor(FloorIndex))
	{
		if (FloorIndex == Stage->BossFloor && !Stage->BossEncounterID.IsNone())
		{
			if (const FT66DeckEncounterDefinition* BossEncounter = FindEncounter(Stage->BossEncounterID))
			{
				MatchingEncounters.Add(BossEncounter);
				return MatchingEncounters;
			}
		}
	}

	for (const FT66DeckEncounterDefinition& Encounter : Encounters)
	{
		if (FloorIndex >= Encounter.MinFloor && FloorIndex <= Encounter.MaxFloor)
		{
			MatchingEncounters.Add(&Encounter);
		}
	}
	return MatchingEncounters;
}

void UT66DeckDataSubsystem::LoadHeroes()
{
	Heroes.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Heroes.csv"))))
	{
		FT66DeckHeroDefinition Definition;
		Definition.HeroID = T66DeckToName(Row, TEXT("HeroID"));
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66DeckGetValue(Row, TEXT("Description"));
		Definition.AccentColor = T66DeckToColor(T66DeckGetValue(Row, TEXT("AccentColor")), FLinearColor(0.72f, 0.50f, 0.24f, 1.0f));
		Definition.StartingHealth = T66DeckToInt(T66DeckGetValue(Row, TEXT("StartingHealth")), 80);
		Definition.StartingGold = T66DeckToInt(T66DeckGetValue(Row, TEXT("StartingGold")), 0);
		if (Definition.HeroID != NAME_None)
		{
			Heroes.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadCompanions()
{
	Companions.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Companions.csv"))))
	{
		FT66DeckCompanionDefinition Definition;
		Definition.CompanionID = T66DeckToName(Row, TEXT("CompanionID"));
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66DeckGetValue(Row, TEXT("Description"));
		Definition.AccentColor = T66DeckToColor(T66DeckGetValue(Row, TEXT("AccentColor")), FLinearColor(0.34f, 0.62f, 0.82f, 1.0f));
		Definition.StartingBlock = T66DeckToInt(T66DeckGetValue(Row, TEXT("StartingBlock")), 0);
		Definition.PassiveDamageBonus = T66DeckToInt(T66DeckGetValue(Row, TEXT("PassiveDamageBonus")), 0);
		if (Definition.CompanionID != NAME_None)
		{
			Companions.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadCards()
{
	Cards.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Cards.csv"))))
	{
		FT66DeckCardDefinition Definition;
		Definition.CardID = T66DeckToName(Row, TEXT("CardID"));
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.RulesText = T66DeckGetValue(Row, TEXT("RulesText"));
		Definition.CardType = T66DeckToCardType(T66DeckGetValue(Row, TEXT("CardType")));
		Definition.EnergyCost = T66DeckToInt(T66DeckGetValue(Row, TEXT("EnergyCost")), 1);
		Definition.ArchetypeID = T66DeckToName(Row, TEXT("ArchetypeID"));
		Definition.Damage = T66DeckToInt(T66DeckGetValue(Row, TEXT("Damage")), 0);
		Definition.Block = T66DeckToInt(T66DeckGetValue(Row, TEXT("Block")), 0);
		Definition.RarityID = T66DeckToName(Row, TEXT("RarityID"));
		Definition.AccentColor = T66DeckToColor(T66DeckGetValue(Row, TEXT("AccentColor")), FLinearColor(0.45f, 0.25f, 0.86f, 1.0f));
		if (Definition.CardID != NAME_None)
		{
			Cards.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadRelics()
{
	Relics.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Relics.csv"))))
	{
		FT66DeckRelicDefinition Definition;
		Definition.RelicID = T66DeckToName(Row, TEXT("RelicID"));
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.RulesText = T66DeckGetValue(Row, TEXT("RulesText"));
		Definition.RarityID = T66DeckToName(Row, TEXT("RarityID"));
		if (Definition.RelicID != NAME_None)
		{
			Relics.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadEnemies()
{
	Enemies.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Enemies.csv"))))
	{
		FT66DeckEnemyDefinition Definition;
		Definition.EnemyID = T66DeckToName(Row, TEXT("EnemyID"));
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.IntentLabel = T66DeckGetValue(Row, TEXT("IntentLabel"));
		Definition.AccentColor = T66DeckToColor(T66DeckGetValue(Row, TEXT("AccentColor")), FLinearColor(0.72f, 0.20f, 0.18f, 1.0f));
		Definition.BaseHealth = T66DeckToInt(T66DeckGetValue(Row, TEXT("BaseHealth")), 40);
		Definition.HealthPerFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("HealthPerFloor")), 8);
		Definition.BaseIntentDamage = T66DeckToInt(T66DeckGetValue(Row, TEXT("BaseIntentDamage")), 6);
		Definition.IntentDamagePerFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("IntentDamagePerFloor")), 2);
		Definition.GoldReward = T66DeckToInt(T66DeckGetValue(Row, TEXT("GoldReward")), 10);
		if (Definition.EnemyID != NAME_None)
		{
			Enemies.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadItems()
{
	Items.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Items.csv"))))
	{
		FT66DeckItemDefinition Definition;
		Definition.ItemID = T66DeckToName(Row, TEXT("ItemID"));
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.RulesText = T66DeckGetValue(Row, TEXT("RulesText"));
		Definition.RarityID = T66DeckToName(Row, TEXT("RarityID"));
		Definition.AccentColor = T66DeckToColor(T66DeckGetValue(Row, TEXT("AccentColor")), FLinearColor(0.66f, 0.48f, 0.22f, 1.0f));
		Definition.BonusMaxHealth = T66DeckToInt(T66DeckGetValue(Row, TEXT("BonusMaxHealth")), 0);
		Definition.BonusStartingBlock = T66DeckToInt(T66DeckGetValue(Row, TEXT("BonusStartingBlock")), 0);
		if (Definition.ItemID != NAME_None)
		{
			Items.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadEncounters()
{
	Encounters.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Encounters.csv"))))
	{
		FT66DeckEncounterDefinition Definition;
		Definition.EncounterID = T66DeckToName(Row, TEXT("EncounterID"));
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.NodeType = T66DeckToNodeType(T66DeckGetValue(Row, TEXT("NodeType")));
		Definition.ActIndex = T66DeckToInt(T66DeckGetValue(Row, TEXT("ActIndex")), 1);
		Definition.EnemyIDs = T66DeckSplitNameList(T66DeckGetValue(Row, TEXT("EnemyIDs")));
		Definition.RewardCardIDs = T66DeckSplitNameList(T66DeckGetValue(Row, TEXT("RewardCardIDs")));
		Definition.RewardItemIDs = T66DeckSplitNameList(T66DeckGetValue(Row, TEXT("RewardItemIDs")));
		Definition.MinFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("MinFloor")), 1);
		Definition.MaxFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("MaxFloor")), 999);
		Definition.GoldRewardBonus = T66DeckToInt(T66DeckGetValue(Row, TEXT("GoldRewardBonus")), 0);
		if (Definition.EncounterID != NAME_None)
		{
			Encounters.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadStartingDecks()
{
	StartingDecks.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_StartingDecks.csv"))))
	{
		FT66DeckStartingDeckDefinition Definition;
		Definition.StartingDeckID = T66DeckToName(Row, TEXT("StartingDeckID"));
		Definition.HeroID = T66DeckToName(Row, TEXT("HeroID"));
		Definition.CardIDs = T66DeckSplitNameList(T66DeckGetValue(Row, TEXT("CardIDs")));
		if (Definition.StartingDeckID != NAME_None)
		{
			StartingDecks.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadStages()
{
	Stages.Reset();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Stages.csv"))))
	{
		FT66DeckStageDefinition Definition;
		Definition.StageID = T66DeckToName(Row, TEXT("StageID"));
		Definition.ActIndex = T66DeckToInt(T66DeckGetValue(Row, TEXT("ActIndex")), 1);
		Definition.StageIndex = T66DeckToInt(T66DeckGetValue(Row, TEXT("StageIndex")), 1);
		Definition.DisplayName = T66DeckGetValue(Row, TEXT("DisplayName"));
		Definition.FirstFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("FirstFloor")), 1);
		Definition.BossFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("BossFloor")), Definition.FirstFloor);
		Definition.BossEncounterID = T66DeckToName(Row, TEXT("BossEncounterID"));
		Definition.ClearGoldReward = FMath::Max(0, T66DeckToInt(T66DeckGetValue(Row, TEXT("ClearGoldReward")), 0));
		Definition.NextStageID = T66DeckToName(Row, TEXT("NextStageID"));
		if (Definition.StageID != NAME_None && Definition.StageIndex > 0)
		{
			Stages.Add(MoveTemp(Definition));
		}
	}
}

void UT66DeckDataSubsystem::LoadTuning()
{
	Tuning = FT66DeckTuningDefinition();
	for (const TMap<FString, FString>& Row : T66DeckLoadCsvRows(T66DeckDataPath(TEXT("T66Deck_Tuning.csv"))))
	{
		FT66DeckTuningDefinition Definition;
		Definition.TuningID = T66DeckToName(Row, TEXT("TuningID"));
		Definition.StartingHeroID = T66DeckToName(Row, TEXT("StartingHeroID"));
		Definition.StartingCompanionID = T66DeckToName(Row, TEXT("StartingCompanionID"));
		Definition.StartingDeckID = T66DeckToName(Row, TEXT("StartingDeckID"));
		Definition.DifficultyID = T66DeckToName(Row, TEXT("DifficultyID"));
		Definition.StartingEnergy = T66DeckToInt(T66DeckGetValue(Row, TEXT("StartingEnergy")), 3);
		Definition.HandSize = T66DeckToInt(T66DeckGetValue(Row, TEXT("HandSize")), 5);
		Definition.MapChoicesPerFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("MapChoicesPerFloor")), 3);
		Definition.StartingFloor = T66DeckToInt(T66DeckGetValue(Row, TEXT("StartingFloor")), 1);
		if (Definition.TuningID != NAME_None)
		{
			Tuning = MoveTemp(Definition);
			return;
		}
	}
}
