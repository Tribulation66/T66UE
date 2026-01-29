// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/T66DataTypes.h"
#include "T66GameInstance.generated.h"

class UDataTable;

/**
 * Game Instance for Tribulation 66
 * Persists across level loads and holds:
 * - Current UI state / selected screen
 * - Player selections (hero, companion, party size, difficulty)
 * - References to core DataTables
 */
UCLASS()
class T66_API UT66GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UT66GameInstance();

	virtual void Init() override;

	// ============================================
	// DataTable References
	// ============================================

	/** Reference to the Hero DataTable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> HeroDataTable;

	/** Reference to the Companion DataTable */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> CompanionDataTable;

	/** Reference to the Items DataTable (v0: 3 placeholder items) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> ItemsDataTable;

	// ============================================
	// Player Selections (for current run setup)
	// ============================================

	/** Selected party size */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66PartySize SelectedPartySize = ET66PartySize::Solo;

	/** Selected hero row name (from Hero DataTable) */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	FName SelectedHeroID;

	/** Selected companion row name (from Companion DataTable). NAME_None = no companion */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	FName SelectedCompanionID;

	/** Selected difficulty */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66Difficulty SelectedDifficulty = ET66Difficulty::Easy;

	/** Selected body type for hero */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66BodyType SelectedHeroBodyType = ET66BodyType::TypeA;

	/** Selected body type for companion */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66BodyType SelectedCompanionBodyType = ET66BodyType::TypeA;

	// ============================================
	// Save / Load flow
	// ============================================

	/** Current run save slot index (0..N-1), or -1 if none */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 CurrentSaveSlotIndex = -1;

	/** When loading a game, transform to apply to player after spawn; cleared after use */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	FTransform PendingLoadedTransform;

	/** True when PendingLoadedTransform should be applied on next spawn (set by load, cleared after apply) */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	bool bApplyLoadedTransform = false;

	/** True if Main Menu chose New Game, false if Load Game (used by Party Size Picker) */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bIsNewGameFlow = true;

	/** True when advancing to next stage (reload level, keep progress). GameMode skips ResetForNewRun. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bIsStageTransition = false;

	// ============================================
	// DataTable Access Helpers
	// ============================================

	/** Get the loaded Hero DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetHeroDataTable();

	/** Get the loaded Companion DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetCompanionDataTable();

	/** Get the loaded Items DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetItemsDataTable();

	/** Get item data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetItemData(FName ItemID, FItemData& OutItemData);

	/** Get hero data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetHeroData(FName HeroID, FHeroData& OutHeroData);

	/** Get companion data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetCompanionData(FName CompanionID, FCompanionData& OutCompanionData);

	/** Get all hero IDs from the DataTable */
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FName> GetAllHeroIDs();

	/** Get all companion IDs from the DataTable */
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FName> GetAllCompanionIDs();

	/** Get the currently selected hero data */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool GetSelectedHeroData(FHeroData& OutHeroData);

	/** Get the currently selected companion data (returns false if no companion selected) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool GetSelectedCompanionData(FCompanionData& OutCompanionData);

	// ============================================
	// Selection Helpers
	// ============================================

	/** Clear all selections (reset to defaults) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearSelections();

	/** Check if a hero is selected */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool HasHeroSelected() const { return !SelectedHeroID.IsNone(); }

	/** Check if a companion is selected (NAME_None means no companion) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool HasCompanionSelected() const { return !SelectedCompanionID.IsNone(); }

private:
	/** Cached loaded Hero DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedHeroDataTable;

	/** Cached loaded Companion DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedCompanionDataTable;

	/** Cached loaded Items DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedItemsDataTable;
};
