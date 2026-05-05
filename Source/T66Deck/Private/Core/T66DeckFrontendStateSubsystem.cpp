// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66DeckFrontendStateSubsystem.h"

void UT66DeckFrontendStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetRunSetup();
}

void UT66DeckFrontendStateSubsystem::ResetRunSetup()
{
	SelectedHeroID = NAME_None;
	SelectedDifficultyID = NAME_None;
	SelectedStartingDeckID = NAME_None;
	CurrentMapNodeID = NAME_None;
	bDailyRun = false;
	DailyChallengeId.Reset();
	DailySeed = 0;
}

void UT66DeckFrontendStateSubsystem::BeginNewRun()
{
	ResetRunSetup();
}

void UT66DeckFrontendStateSubsystem::BeginDailyRun(const FName DifficultyID, const FString& ChallengeId, const int32 Seed)
{
	ResetRunSetup();
	SelectedDifficultyID = DifficultyID;
	bDailyRun = true;
	DailyChallengeId = ChallengeId;
	DailySeed = Seed;
}

void UT66DeckFrontendStateSubsystem::SelectHero(const FName HeroID)
{
	SelectedHeroID = HeroID;
}

void UT66DeckFrontendStateSubsystem::SelectDifficulty(const FName DifficultyID)
{
	SelectedDifficultyID = DifficultyID;
}

void UT66DeckFrontendStateSubsystem::SelectStartingDeck(const FName StartingDeckID)
{
	SelectedStartingDeckID = StartingDeckID;
}

void UT66DeckFrontendStateSubsystem::SelectCurrentMapNode(const FName NodeID)
{
	CurrentMapNodeID = NodeID;
}
