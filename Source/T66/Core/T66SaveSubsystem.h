// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66SaveIndex.h"
#include "T66SaveSubsystem.generated.h"

class UT66RunSaveGame;

UCLASS()
class T66_API UT66SaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString SaveIndexSlotName;
	static const FString SlotNamePrefix;
	static const int32 MaxSlots = 9;

	UFUNCTION(BlueprintCallable, Category = "Save")
	int32 FindFirstEmptySlot() const;

	/** If all slots are full, pick the oldest occupied slot (by LastPlayedUtc). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	int32 FindOldestOccupiedSlot() const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DoesSlotExist(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveToSlot(int32 SlotIndex, UT66RunSaveGame* SaveGameObject);

	UFUNCTION(BlueprintCallable, Category = "Save")
	UT66RunSaveGame* LoadFromSlot(int32 SlotIndex);

	void UpdateIndexOnSave(int32 SlotIndex, const FString& HeroDisplayName, const FString& MapName, const FString& LastPlayedUtc);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool GetSlotMeta(int32 SlotIndex, bool& bOutOccupied, FString& OutLastPlayedUtc, FString& OutHeroDisplayName, FString& OutMapName) const;

private:
	FString GetSlotName(int32 SlotIndex) const;
	UT66SaveIndex* LoadOrCreateIndex() const;
	bool SaveIndex(UT66SaveIndex* Index) const;
};
