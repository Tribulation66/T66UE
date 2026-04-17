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
class UWorld;
struct FStreamableHandle;
class SBorder;

enum class ET66HeroPortraitVariant : uint8
{
	Low,
	Half,
	Full,
	Invincible
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

	/** Mini lobby selected hero row name. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	FName MiniSelectedHeroID;

	/** Mini lobby selected companion row name. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	FName MiniSelectedCompanionID;

	/** Mini lobby selected difficulty row name. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	FName MiniSelectedDifficultyID;

	/** Mini lobby selected idol loadout. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	TArray<FName> MiniSelectedIdolIDs;

	/** True when the current mini frontend flow came from loading a save. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	bool bMiniLoadFlow = false;

	/** True when the current mini frontend flow is in intermission/shop state. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	bool bMiniIntermissionFlow = false;

	/** Host-authored mini intermission state revision mirrored through lobby profiles. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	int32 MiniIntermissionStateRevision = 0;

	/** Serialized mini intermission state mirrored through lobby profiles. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	FString MiniIntermissionStateJson;

	/** Client-authored mini intermission request revision mirrored through lobby profiles. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	int32 MiniIntermissionRequestRevision = 0;

	/** Serialized mini intermission request mirrored through lobby profiles. */
	UPROPERTY(BlueprintReadWrite, Category = "Mini")
	FString MiniIntermissionRequestJson;

	/** If true, the next GameplayLevel load should spawn into the Stage Catch Up platform first. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bStageCatchUpPending = false;

	/** Run-level random seed (set when entering tribulation). Used for stage effects, NPC shuffle, world interactables. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	int32 RunSeed = 0;

	/** Active main gameplay terrain layout for the current run. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66MainMapLayoutVariant CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;

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

	/** True when the next tower stage should begin with a high-altitude drop into the start room. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bPendingTowerStageDropIntro = false;

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

	/** Frontend screen to show immediately after the next frontend-level BeginPlay. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66ScreenType PendingFrontendScreen = ET66ScreenType::None;

	/** True while a loaded save is being inspected in paused preview mode. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	bool bSaveSlotPreviewMode = false;

	/** Restore the save-slot browser state after returning from preview mode. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	bool bRestoreSaveSlotsState = false;

	/** Party-size filter to restore when returning from save preview mode. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	ET66PartySize PendingSaveSlotsPartyFilter = ET66PartySize::Solo;

	/** Page index to restore when returning from save preview mode. */
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	int32 PendingSaveSlotsPage = 0;

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

	/** Deterministic item roll using the caller-provided stream. */
	FName GetRandomItemIDFromStream(FRandomStream& Stream);

	/** Deterministic rarity-aware item roll using the caller-provided stream. */
	FName GetRandomItemIDForLootRarityFromStream(ET66Rarity LootRarity, FRandomStream& Stream);

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

	/** Persist the current hero/companion defaults into the profile save. */
	void PersistRememberedSelectionDefaults();

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

	/** Warm main-menu / hero-selection portraits plus the currently selected preview visuals asynchronously. */
	void PrimeHeroSelectionAssetsAsync();

	/** Warm the full hero/companion preview visual library asynchronously after the selection screen is already visible. */
	void PrimeHeroSelectionPreviewVisualsAsync();

	/** Apply the finalized tower layout to the current run state. */
	void ApplyConfiguredMainMapLayoutVariant();

	/** Frontend map package path. */
	static FName GetFrontendLevelName();
	/** Lab map package path. */
	static FName GetLabLevelName();
	/** Main gameplay map package path. */
	static FName GetGameplayLevelName();
	/** Entry map package path for a brand-new Tribulation run. */
	static FName GetTribulationEntryLevelName();
	/** Standalone coliseum gameplay map package path. */
	static FName GetColiseumLevelName();
	/** Standalone tutorial gameplay map package path. */
	static FName GetTutorialLevelName();

private:
	void PrimeCoreDataTablesAsync();
	void HandleCoreDataTablesLoaded();
	void HandleHeroSelectionAssetsLoaded();
	void HandleHeroSelectionPreviewVisualsLoaded();
	bool QueueGameplayVisualAssetPreload();
	void RestoreRememberedSelectionDefaults();

	bool bCoreDataTablesLoadRequested = false;
	bool bCoreDataTablesLoaded = false;
	TSharedPtr<FStreamableHandle> CoreDataTablesLoadHandle;

	bool bHeroSelectionAssetsLoadRequested = false;
	bool bHeroSelectionAssetsLoaded = false;
	TSharedPtr<FStreamableHandle> HeroSelectionAssetsLoadHandle;

	bool bHeroSelectionPreviewVisualsLoadRequested = false;
	bool bHeroSelectionPreviewVisualsLoaded = false;
	TSharedPtr<FStreamableHandle> HeroSelectionPreviewVisualsLoadHandle;
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

	// Gameplay asset pre-load tracking.
	bool bGameplayAssetsPreloadInFlight = false;
	bool bGameplayVisualAssetsPhaseQueued = false;
	TSharedPtr<FStreamableHandle> GameplayAssetsPreloadHandle;
	TFunction<void()> GameplayAssetsPreloadCallback;
	void HandleGameplayAssetsPreloaded();
	TSharedPtr<SBorder> PersistentGameplayTransitionCurtain;
};
