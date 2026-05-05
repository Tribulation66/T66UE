// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66MiniDataTypes.h"
#include "GameFramework/SaveGame.h"
#include "T66MiniProfileSaveGame.generated.h"

UCLASS()
class T66MINI_API UT66MiniProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> UnlockedHeroIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> UnlockedDifficultyIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> UnlockedCompanionIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TMap<FName, int32> CompanionUnityStagesClearedByID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 CompletedRunCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 FailedRunCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 BestWaveReached = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 BestStageIndexReached = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float BestClearSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 LifetimeMaterialsCollected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 LifetimeGoldCollected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 TotalStagesCleared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName FavoriteHeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FT66MiniRunSummary LastRunSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FString LastUpdatedUtc;
};
