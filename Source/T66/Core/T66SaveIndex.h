// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66SaveIndex.generated.h"

/** Metadata for one save slot (used in index only) */
USTRUCT(BlueprintType)
struct T66_API FT66SaveSlotMeta
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bOccupied = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString LastPlayedUtc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString HeroDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString MapName;
};

/**
 * Index of all save slots (metadata only).
 * Stored as T66_SaveIndex. Slot files: T66_Slot_00 .. T66_Slot_08.
 */
UCLASS(BlueprintType)
class T66_API UT66SaveIndex : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxSlots = 9;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SaveSlotMeta> SlotMeta;
};
