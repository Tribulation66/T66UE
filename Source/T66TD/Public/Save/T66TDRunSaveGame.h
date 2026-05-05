// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66TDRunSaveGame.generated.h"

UCLASS()
class T66TD_API UT66TDRunSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 SaveSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FName StageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 StageIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FName MapID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 CurrentWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 Hearts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 Materials = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 LastScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	bool bStageCleared = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	bool bDailyRun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FString DailyChallengeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 DailySeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FString LastUpdatedUtc;
};
