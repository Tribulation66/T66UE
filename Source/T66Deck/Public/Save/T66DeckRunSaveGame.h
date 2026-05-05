// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DeckDataTypes.h"
#include "GameFramework/SaveGame.h"
#include "T66DeckRunSaveGame.generated.h"

UCLASS()
class T66DECK_API UT66DeckRunSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 SaveSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName CompanionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName StartingDeckID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 ActIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 FloorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName CurrentStageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 StageIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 HighestStageIndexCleared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 CurrentHealth = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 MaxHealth = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 PlayerBlock = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 Energy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName CurrentNodeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FName CurrentEnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 EnemyHealth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 EnemyMaxHealth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 EnemyIntent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	TArray<FName> DrawPileCardIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	TArray<FName> RewardCardIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	TArray<FName> RewardItemIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	TArray<FName> RelicIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	TArray<FT66DeckRunNode> RunMapNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 SavedViewMode = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	bool bRunDefeated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FString LastUpdatedUtc;
};
