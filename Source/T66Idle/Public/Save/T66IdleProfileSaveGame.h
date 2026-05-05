// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66IdleDataTypes.h"
#include "GameFramework/SaveGame.h"
#include "T66IdleProfileSaveGame.generated.h"

UCLASS()
class T66IDLE_API UT66IdleProfileSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	FT66IdleProfileSnapshot Snapshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	TArray<FName> UnlockedHeroIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	TArray<FName> UnlockedCompanionIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	TArray<FName> OwnedItemIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle")
	TArray<FName> OwnedIdolIDs;
};
