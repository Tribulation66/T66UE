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
	int32 SaveVersion = 3;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString OwnerPlayerId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString OwnerDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FString> PartyMemberIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FString> PartyMemberDisplayNames;

	/** Stage reached when save was written (for slot display). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 StageReached = 1;

	/** Equipped idol IDs when save was written (for slot display). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FName> EquippedIdols;

	/** Equipped idol tier values aligned with EquippedIdols (0 empty, 1 black .. 4 white). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<uint8> EquippedIdolTiers;
};
