// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DeckDataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66DeckDataSubsystem.generated.h"

UCLASS()
class T66DECK_API UT66DeckDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Deck")
	void ReloadData();

	const TArray<FT66DeckHeroDefinition>& GetHeroes() const { return Heroes; }
	const TArray<FT66DeckCompanionDefinition>& GetCompanions() const { return Companions; }
	const TArray<FT66DeckCardDefinition>& GetCards() const { return Cards; }
	const TArray<FT66DeckRelicDefinition>& GetRelics() const { return Relics; }
	const TArray<FT66DeckEnemyDefinition>& GetEnemies() const { return Enemies; }
	const TArray<FT66DeckItemDefinition>& GetItems() const { return Items; }
	const TArray<FT66DeckEncounterDefinition>& GetEncounters() const { return Encounters; }
	const TArray<FT66DeckStartingDeckDefinition>& GetStartingDecks() const { return StartingDecks; }
	const TArray<FT66DeckStageDefinition>& GetStages() const { return Stages; }
	const FT66DeckTuningDefinition& GetTuning() const { return Tuning; }

	const FT66DeckHeroDefinition* FindHero(FName HeroID) const;
	const FT66DeckCompanionDefinition* FindCompanion(FName CompanionID) const;
	const FT66DeckCardDefinition* FindCard(FName CardID) const;
	const FT66DeckRelicDefinition* FindRelic(FName RelicID) const;
	const FT66DeckEnemyDefinition* FindEnemy(FName EnemyID) const;
	const FT66DeckItemDefinition* FindItem(FName ItemID) const;
	const FT66DeckEncounterDefinition* FindEncounter(FName EncounterID) const;
	const FT66DeckStartingDeckDefinition* FindStartingDeck(FName StartingDeckID) const;
	const FT66DeckStageDefinition* FindStage(FName StageID) const;
	const FT66DeckStageDefinition* FindStageForFloor(int32 FloorIndex) const;
	TArray<const FT66DeckEncounterDefinition*> GetEncountersForFloor(int32 FloorIndex) const;

private:
	void LoadHeroes();
	void LoadCompanions();
	void LoadCards();
	void LoadRelics();
	void LoadEnemies();
	void LoadItems();
	void LoadEncounters();
	void LoadStartingDecks();
	void LoadStages();
	void LoadTuning();

	TArray<FT66DeckHeroDefinition> Heroes;
	TArray<FT66DeckCompanionDefinition> Companions;
	TArray<FT66DeckCardDefinition> Cards;
	TArray<FT66DeckRelicDefinition> Relics;
	TArray<FT66DeckEnemyDefinition> Enemies;
	TArray<FT66DeckItemDefinition> Items;
	TArray<FT66DeckEncounterDefinition> Encounters;
	TArray<FT66DeckStartingDeckDefinition> StartingDecks;
	TArray<FT66DeckStageDefinition> Stages;
	FT66DeckTuningDefinition Tuning;
};
