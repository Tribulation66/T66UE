// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Core/T66Rarity.h"
#include "T66IdolManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIdolStateChanged);

/**
 * Central owner for idol progression, altar stock, and idol-related difficulty rules.
 */
UCLASS()
class T66_API UT66IdolManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxEquippedIdolSlots = 4;
	static constexpr int32 MaxIdolLevel = 4;
	static constexpr int32 IdolStockSlotCount = 16;

	UPROPERTY(BlueprintAssignable, Category = "Idols")
	FOnIdolStateChanged IdolStateChanged;

	const TArray<FName>& GetEquippedIdols() const { return EquippedIdolIDs; }
	const TArray<uint8>& GetEquippedIdolTierValues() const { return EquippedIdolLevels; }

	int32 GetEquippedIdolLevelInSlot(int32 SlotIndex) const;
	ET66ItemRarity GetEquippedIdolRarityInSlot(int32 SlotIndex) const;

	static ET66ItemRarity IdolTierValueToRarity(int32 TierValue);
	static int32 IdolRarityToTierValue(ET66ItemRarity Rarity);
	static const TArray<FName>& GetAllIdolIDs();
	static FLinearColor GetIdolColor(FName IdolID);

	bool EquipIdolInSlot(int32 SlotIndex, FName IdolID);
	bool EquipIdolFirstEmpty(FName IdolID);
	bool SelectIdolFromAltar(FName IdolID);
	void ClearEquippedIdols();

	void EnsureIdolStock();
	void RerollIdolStock();
	const TArray<FName>& GetIdolStockIDs() const { return IdolStockIDs; }
	int32 GetIdolStockTierValue(int32 SlotIndex) const;
	ET66ItemRarity GetIdolStockRarityInSlot(int32 SlotIndex) const;
	bool SelectIdolFromStock(int32 SlotIndex);
	bool IsIdolStockSlotSelected(int32 SlotIndex) const;
	bool SellEquippedIdolInSlot(int32 SlotIndex);
	void RestoreState(const TArray<FName>& InEquippedIdols, const TArray<uint8>& InEquippedIdolTiers, ET66Difficulty Difficulty, int32 InRemainingCatchUpIdolPicks);

	void ResetForNewRun(ET66Difficulty Difficulty);
	void HandleStageChanged(int32 NewStage);

	int32 GetCatchUpIdolPickCountForDifficulty(ET66Difficulty Difficulty) const;
	int32 GetRemainingCatchUpIdolPicks() const { return RemainingCatchUpIdolPicks; }
	bool HasCatchUpIdolPicksRemaining() const { return RemainingCatchUpIdolPicks > 0; }
	bool ConsumeCatchUpIdolPick();
	void SetRemainingCatchUpIdolPicks(int32 NewRemainingCatchUpIdolPicks);

	ET66Difficulty GetCurrentDifficulty() const { return CurrentDifficulty; }

private:
	void NormalizeEquippedArrays();
	void ClearIdolStock();
	void BroadcastIdolStateChanged();
	bool ApplyStockOfferToEquipped(int32 SlotIndex);
	int32 GetCurrentStage() const;
	int32 GetDifficultyStartStage(ET66Difficulty Difficulty) const;
	int32 GetDifficultyEndStage(ET66Difficulty Difficulty) const;

	UPROPERTY()
	TArray<FName> EquippedIdolIDs;

	UPROPERTY()
	TArray<uint8> EquippedIdolLevels;

	UPROPERTY()
	TArray<FName> IdolStockIDs;

	UPROPERTY()
	TArray<uint8> IdolStockTierValues;

	UPROPERTY()
	TArray<bool> IdolStockSelected;

	UPROPERTY()
	int32 IdolStockStage = INDEX_NONE;

	UPROPERTY()
	ET66Difficulty CurrentDifficulty = ET66Difficulty::Easy;

	UPROPERTY()
	int32 RemainingCatchUpIdolPicks = 0;
};
