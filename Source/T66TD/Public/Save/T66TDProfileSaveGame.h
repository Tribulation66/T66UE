// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66TDProfileSaveGame.generated.h"

UCLASS()
class T66TD_API UT66TDProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	TMap<FName, int32> BestStageIndexByDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 CompletedStageCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 FailedBattleCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 BestScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 LifetimeGoldEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	int32 LifetimeMaterialsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FName LastStageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FName LastDifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TD")
	FString LastUpdatedUtc;
};
