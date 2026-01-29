// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/T66DataTypes.h"
#include "T66RunSaveGame.generated.h"

UCLASS(BlueprintType)
class T66_API UT66RunSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName HeroID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66BodyType HeroBodyType = ET66BodyType::TypeA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName CompanionID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66PartySize PartySize = ET66PartySize::Solo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString MapName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FTransform PlayerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString LastPlayedUtc;
};
