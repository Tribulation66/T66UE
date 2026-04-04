// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunSaveGame.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "UI/T66UITypes.h"
#include "T66GameInstance.generated.h"

class UDataTable;
class UTexture2D;
struct FStreamableHandle;
class SBorder;

enum class ET66HeroPortraitVariant : uint8
{
	Low,
	Half,
	Full
};

UENUM(BlueprintType)
enum class ET66ColiseumFlowMode : uint8
{
	None UMETA(DisplayName = "None"),
	OwedBosses UMETA(DisplayName = "Owed Bosses"),
	FinalDifficultyBoss UMETA(DisplayName = "Final Difficulty Boss")
};

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

	/** If true, the next GameplayLevel load should spawn into the Stage Catch Up platform first. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bStageCatchUpPending = false;

	/** Run-level random seed (set when entering tribulation). Used for stage effects, NPC shuffle, world interactables. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	int32 RunSeed = 0;

	/** Legacy terrain theme selector kept for compatibility. Main gameplay now always uses T66MainMapTerrain. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66MapTheme MapTheme = ET66MapTheme::Farm;

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

	/** When true, load into gameplay level frozen; show LOAD button overlay; clicking LOAD unfreezes. Set by Save Slots Preview. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	bool bLoadAsPreview = false;

	/** Full run snapshot to restore on the next gameplay load. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	FT66SavedRunSnapshot PendingLoadedRunSnapshot;

	/** True when PendingLoadedRunSnapshot should be imported on the next gameplay BeginPlay. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	bool bApplyLoadedRunSnapshot = false;

	/** Owner ID of the current run/save context. */
	UPROPERTY(BlueprintReadWrite, Category = "Party")
	FString CurrentRunOwnerPlayerId;

	/** Owner display name of the current run/save context. */
	UPROPERTY(BlueprintReadWrite, Category = "Party")
	FString CurrentRunOwnerDisplayName;

	/** Party member IDs attached to the current run/save context. */
	UPROPERTY(BlueprintReadWrite, Category = "Party")
	TArray<FString> CurrentRunPartyMemberIds;

	/** Party member display names attached to the current run/save context. */
	UPROPERTY(BlueprintReadWrite, Category = "Party")
	TArray<FString> CurrentRunPartyMemberDisplayNames;

	/** True if Main Menu chose New Game, false if Load Game (used by Party Size Picker) */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bIsNewGameFlow = true;

	/** True when advancing to next stage (reload level, keep progress). GameMode skips ResetForNewRun. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bIsStageTransition = false;

	/** When true, this run was started via "Retry level"; do not submit to leaderboard. Cleared when starting a fresh run. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bRunIneligibleForLeaderboard = false;

	/**
	 * If true, treat the current map as Coliseum behavior (Coliseum is embedded inside GameplayLevel).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bForceColiseumMode = false;

	/** Distinguishes the owed-boss Coliseum route from the selected-difficulty final Coliseum route. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66ColiseumFlowMode ColiseumFlowMode = ET66ColiseumFlowMode::None;

	/** If true, the current level is The Lab (practice room). No rewards, no run save; exit returns to Hero Selection. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bIsLabLevel = false;

	/** When true, Hero Selection was opened from the co-op Lobby; hide Enter the Tribulation and Back returns to Lobby. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bHeroSelectionFromLobby = false;

	/** Frontend screen to show immediately after the next frontend-level BeginPlay. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66ScreenType PendingFrontendScreen = ET66ScreenType::None;

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

	/** Resolve a hero portrait for a body type + portrait state (low / half / full). */
	TSoftObjectPtr<UTexture2D> ResolveHeroPortrait(FName HeroID, ET66BodyType BodyType, ET66HeroPortraitVariant Variant) const;

	/** Resolve a hero portrait from already-loaded hero data for a body type + portrait state. */
	TSoftObjectPtr<UTexture2D> ResolveHeroPortrait(const FHeroData& HeroData, ET66BodyType BodyType, ET66HeroPortraitVariant Variant) const;

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

	/**
	 * Pre-load gameplay assets (engine meshes, ground materials) that would otherwise
	 * hitch the game thread during GameplayLevel BeginPlay. Call before OpenLevel.
	 * Invokes OnComplete once all loads have finished (or immediately if nothing to load).
	 */
	void PreloadGameplayAssets(TFunction<void()> OnComplete);

	/** True while a PreloadGameplayAssets batch is in flight. */
	bool IsPreloadingGameplayAssets() const { return bGameplayAssetsPreloadInFlight; }

	/**
	 * Show a loading screen, pre-load gameplay assets, then open GameplayLevel.
	 * Call this instead of raw UGameplayStatics::OpenLevel for gameplay transitions.
	 */
	void TransitionToGameplayLevel();

	/** Cover the viewport with a persistent blackout that survives level transitions. */
	void ShowPersistentGameplayTransitionCurtain();

	/** Remove the persistent gameplay transition blackout, if present. */
	void HidePersistentGameplayTransitionCurtain();

	/** Warm hero/companion selection portraits + visuals asynchronously so first open does not hitch. */
	void PrimeHeroSelectionAssetsAsync();

	/** When true, Enter the Tribulation opens the demo map instead of GameplayLevel. Flip in T66GameInstance.cpp. */
	static bool UseDemoMapForTribulation();
	/** Main gameplay map asset name. */
	static FName GetGameplayLevelName();
	/** Entry map for a brand-new Tribulation run. Temporary rule: always route through tutorial first. */
	static FName GetTribulationEntryLevelName();
	/** Standalone coliseum gameplay map asset name. */
	static FName GetColiseumLevelName();
	/** Standalone tutorial gameplay map asset name. */
	static FName GetTutorialLevelName();
	/** Level name when UseDemoMapForTribulation() is true (e.g. Map_Summer). Used by TransitionToGameplayLevel and IsGameplayLevel. */
	static FName GetDemoMapLevelNameForTribulation();

private:
	void PrimeCoreDataTablesAsync();
	void HandleCoreDataTablesLoaded();
	void HandleHeroSelectionAssetsLoaded();

	bool bCoreDataTablesLoadRequested = false;
	bool bCoreDataTablesLoaded = false;
	TSharedPtr<FStreamableHandle> CoreDataTablesLoadHandle;

	bool bHeroSelectionAssetsLoadRequested = false;
	bool bHeroSelectionAssetsLoaded = false;
	TSharedPtr<FStreamableHandle> HeroSelectionAssetsLoadHandle;
	TArray<FName> GameplayPreloadVisualIDs;

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

	bool bIdolsDataTableCsvOverrideAttempted = false;

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

	bool bBossesDataTableCsvOverrideAttempted = false;

	/** Cached loaded Stages DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedStagesDataTable;

	bool bStagesDataTableCsvOverrideAttempted = false;

	/** Cached loaded House NPCs DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedHouseNPCsDataTable;

	/** Cached loaded Loan Shark DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedLoanSharkDataTable;

	/** Cached loaded Character Visuals DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedCharacterVisualsDataTable;

	// Gameplay asset pre-load tracking.
	bool bGameplayAssetsPreloadInFlight = false;
	TSharedPtr<FStreamableHandle> GameplayAssetsPreloadHandle;
	TFunction<void()> GameplayAssetsPreloadCallback;
	void HandleGameplayAssetsPreloaded();
	TSharedPtr<SBorder> PersistentGameplayTransitionCurtain;
};
