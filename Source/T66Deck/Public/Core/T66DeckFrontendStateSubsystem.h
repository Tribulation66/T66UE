// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66DeckFrontendStateSubsystem.generated.h"

UCLASS()
class T66DECK_API UT66DeckFrontendStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ResetRunSetup();
	void BeginNewRun();
	void BeginDailyRun(FName DifficultyID, const FString& ChallengeId, int32 Seed);
	void SelectHero(FName HeroID);
	void SelectDifficulty(FName DifficultyID);
	void SelectStartingDeck(FName StartingDeckID);
	void SelectCurrentMapNode(FName NodeID);

	FName GetSelectedHeroID() const { return SelectedHeroID; }
	FName GetSelectedDifficultyID() const { return SelectedDifficultyID; }
	FName GetSelectedStartingDeckID() const { return SelectedStartingDeckID; }
	FName GetCurrentMapNodeID() const { return CurrentMapNodeID; }
	bool IsDailyRun() const { return bDailyRun; }
	const FString& GetDailyChallengeId() const { return DailyChallengeId; }
	int32 GetDailySeed() const { return DailySeed; }

	bool HasSelectedHero() const { return SelectedHeroID != NAME_None; }
	bool HasSelectedDifficulty() const { return SelectedDifficultyID != NAME_None; }
	bool HasSelectedStartingDeck() const { return SelectedStartingDeckID != NAME_None; }

private:
	UPROPERTY()
	FName SelectedHeroID = NAME_None;

	UPROPERTY()
	FName SelectedDifficultyID = NAME_None;

	UPROPERTY()
	FName SelectedStartingDeckID = NAME_None;

	UPROPERTY()
	FName CurrentMapNodeID = NAME_None;

	UPROPERTY()
	bool bDailyRun = false;

	UPROPERTY()
	FString DailyChallengeId;

	UPROPERTY()
	int32 DailySeed = 0;
};
