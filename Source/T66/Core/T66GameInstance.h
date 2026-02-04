// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Core/T66Rarity.h"
#include "T66GameInstance.generated.h"

class UDataTable;
struct FStreamableHandle;

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

	/** Returns true once core DataTables have finished async preloading. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool AreCoreDataTablesLoaded() const { return bCoreDataTablesLoaded; }

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

	/** Reference to the Idols DataTable (idol levels 1..10). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> IdolsDataTable;

	/** Reference to the Bosses DataTable (v0: placeholder boss) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossesDataTable;

	/** Reference to the Stages DataTable (v0: placeholder stages) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> StagesDataTable;

	/** Reference to the House NPCs DataTable (Vendor/Gambler/Saint/Ouroboros) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> HouseNPCsDataTable;

	/** Reference to the Loan Shark DataTable (debt collector NPC tuning) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> LoanSharkDataTable;

	/** Reference to the Character Visuals DataTable (ID -> SkeletalMesh + optional looping anim + transform). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> CharacterVisualsDataTable;

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

	/** If true, the next GameplayLevel load should spawn into the Stage Boost platform first. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bStageBoostPending = false;

	/** Seed for procedural hills terrain. Set when entering tribulation; used when GameplayLevel loads. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	int32 ProceduralTerrainSeed = 0;

	/** Selected body type for hero */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66BodyType SelectedHeroBodyType = ET66BodyType::TypeA;

	/** Selected body type for companion */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66BodyType SelectedCompanionBodyType = ET66BodyType::TypeA;

	/** Selected hero skin ID (e.g. Default, Beachgoer). Synced from profile when entering hero selection. */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	FName SelectedHeroSkinID = FName(TEXT("Default"));

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

	/**
	 * If true, treat the current map as Coliseum behavior (Coliseum is embedded inside GameplayLevel).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bForceColiseumMode = false;

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

	/** Get the loaded Idols DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetIdolsDataTable();

	/** Get a random item ID from the Items DataTable (cached; never returns NAME_None). */
	UFUNCTION(BlueprintCallable, Category = "Data")
	FName GetRandomItemID();

	/** Get a random item ID from the Items DataTable filtered by Loot Bag rarity. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	FName GetRandomItemIDForLootRarity(ET66Rarity LootRarity);

	/** Get the loaded Bosses DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossesDataTable();

	/** Get the loaded Stages DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetStagesDataTable();

	/** Get the loaded House NPCs DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetHouseNPCsDataTable();

	/** Get the loaded Loan Shark DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetLoanSharkDataTable();

	/** Get the loaded Character Visuals DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetCharacterVisualsDataTable();

	/** Get item data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetItemData(FName ItemID, FItemData& OutItemData);

	/** Get idol data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetIdolData(FName IdolID, FIdolData& OutIdolData);

	/** Get boss data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetBossData(FName BossID, FBossData& OutBossData);

	/** Get stage data by stage number. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetStageData(int32 StageNumber, FStageData& OutStageData);

	/** Get house NPC data by ID (row name). Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetHouseNPCData(FName NPCID, FHouseNPCData& OutNPCData);

	/** Get loan shark tuning data. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetLoanSharkData(FName LoanSharkID, FLoanSharkData& OutData);

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
	// Hero Stats (Foundational + Level-up gains)
	// ============================================

	/**
	 * Get hero stat tuning (base stats and per-level gain ranges) for a hero.
	 * This is used both by the run-state leveling system and by the hero selection UI.
	 */
	bool GetHeroStatTuning(FName HeroID, FT66HeroStatBlock& OutBaseStats, FT66HeroPerLevelStatGains& OutPerLevelGains) const;

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
	void PrimeCoreDataTablesAsync();
	void HandleCoreDataTablesLoaded();

	bool bCoreDataTablesLoadRequested = false;
	bool bCoreDataTablesLoaded = false;
	TSharedPtr<FStreamableHandle> CoreDataTablesLoadHandle;

	void EnsureCachedItemIDs();
	void EnsureCachedItemIDsByRarity();

	/** Cached loaded Hero DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedHeroDataTable;

	/** Cached loaded Companion DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedCompanionDataTable;

	/** Cached loaded Items DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedItemsDataTable;

	/** Cached loaded Idols DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedIdolsDataTable;

	/** Cached item row names (built once per runtime session). */
	UPROPERTY(Transient)
	TArray<FName> CachedItemIDs;

	bool bCachedItemIDsInitialized = false;

	/** Cached item IDs per rarity (built once per runtime session). */
	UPROPERTY(Transient)
	TArray<FName> CachedItemIDs_Black;

	UPROPERTY(Transient)
	TArray<FName> CachedItemIDs_Red;

	UPROPERTY(Transient)
	TArray<FName> CachedItemIDs_Yellow;

	UPROPERTY(Transient)
	TArray<FName> CachedItemIDs_White;

	bool bCachedItemIDsByRarityInitialized = false;

	/** Cached loaded Bosses DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedBossesDataTable;

	/** Cached loaded Stages DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedStagesDataTable;

	/** Cached loaded House NPCs DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedHouseNPCsDataTable;

	/** Cached loaded Loan Shark DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedLoanSharkDataTable;

	/** Cached loaded Character Visuals DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedCharacterVisualsDataTable;
};
