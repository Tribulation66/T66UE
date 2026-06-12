// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Core/T66DailyClimbTypes.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66RunTypes.h"
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
enum class ET66RunModifierKind : uint8
{
	None UMETA(DisplayName = "None"),
	Challenge UMETA(DisplayName = "Challenge"),
	Mod UMETA(DisplayName = "Mod")
};

USTRUCT(BlueprintType)
struct T66_API FT66CustomHeroBuildConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Hero")
	bool bConfigured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Hero")
	FName WeaponSourceHeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Hero")
	FName VisualSourceHeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Hero")
	ET66BodyType BodyType = ET66BodyType::Chad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Hero")
	FT66HeroStatBlock Stats;
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

	/** Reference to the Pets DataTable. Rows are keyed by source boss ID. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> PetsDataTable;

	/** Reference to the Items DataTable (v0: 3 placeholder items) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> ItemsDataTable;

	/** Reference to the Idols DataTable (idol levels 1..10). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> IdolsDataTable;

	/** Reference to the Weapons DataTable (auto-attack branch upgrades). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> WeaponsDataTable;

	/** Reference to combat VFX bindings (weapon/idol row -> production VFX). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> CombatVFXBindingsDataTable;

	/** Reference to the Bosses DataTable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossesDataTable;

	/** Reference to the boss attack ownership DataTable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossAttacksDataTable;

	/** Reference to boss attack behavior/visual definitions keyed by AttackID. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossAttackDefinitionsDataTable;

	/** Reference to authored persistent boss hazard definitions keyed by HazardID. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossHazardDefinitionsDataTable;

	/** Reference to the boss movement pattern DataTable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossMovementPatternsDataTable;

	/** Reference to the Stages DataTable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> StagesDataTable;

	/** Reference to the Enemies DataTable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> EnemiesDataTable;

	/** Reference to negative status effect data applied by enemies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> StatusEffectsDataTable;

	/** Reference to boss encounter metadata. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossEncountersDataTable;

	/** Reference to boss encounter member rows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> BossEncounterMembersDataTable;

	/** Reference to the NPCs DataTable (Gambler/Saint/Ouroboros) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> NPCsDataTable;

	/** Reference to the Loan Shark DataTable (debt collector NPC tuning) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> LoanSharkDataTable;

	/** Reference to unique, authored enemies that do not participate in regular waves. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> UniqueEnemiesDataTable;

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

	/** Selected active pet row/source boss ID. NAME_None = no pet. */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	FName SelectedPetID;

	/** Selected difficulty */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66Difficulty SelectedDifficulty = ET66Difficulty::Easy;

	/** Rules/backend axis for the next or active run. */
	UPROPERTY(BlueprintReadWrite, Category = "Run")
	ET66RunMode SelectedRunMode = ET66RunMode::Regular;

	/** Content/layout axis for the next or active run. */
	UPROPERTY(BlueprintReadWrite, Category = "Run")
	ET66RunCategory SelectedRunCategory = ET66RunCategory::Tower;

	/** Selected run modifier kind. None means no active challenge/mod. */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66RunModifierKind SelectedRunModifierKind = ET66RunModifierKind::None;

	/** Stable run modifier id/name used by the frontend selection. */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	FName SelectedRunModifierID = NAME_None;

	/** Run-level random seed (set when entering tribulation). Used for stage effects, NPC shuffle, world interactables. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	int32 RunSeed = 0;

	/** Most recently fetched backend-authored Daily Descent challenge. */
	UPROPERTY(BlueprintReadWrite, Category = "Daily Descent")
	FT66DailyClimbChallengeData CachedDailyClimbChallenge;

	/** Active Daily Descent run context for the current gameplay session. */
	UPROPERTY(BlueprintReadWrite, Category = "Daily Descent")
	FT66DailyClimbChallengeData ActiveDailyClimbChallenge;

	/** Active main gameplay terrain layout for the current run. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66MainMapLayoutVariant CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;

	/** Legacy terrain theme selector kept for compatibility. Main gameplay now always uses T66MainMapTerrain. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66MapTheme MapTheme = ET66MapTheme::Farm;

	/** Selected body type for hero */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66BodyType SelectedHeroBodyType = ET66BodyType::Chad;

	/** Selected body type for companion */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	ET66BodyType SelectedCompanionBodyType = ET66BodyType::Chad;

	/** Selected hero skin ID (e.g. Default, Beachgoer). Synced from profile when entering hero selection. */
	UPROPERTY(BlueprintReadWrite, Category = "Selection")
	FName SelectedHeroSkinID = FName(TEXT("Default"));

	/** Runtime custom hero build. Kept in GameInstance for the first implementation pass. */
	UPROPERTY(BlueprintReadWrite, Category = "Selection|Custom Hero")
	FT66CustomHeroBuildConfig CustomHeroBuild;

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

	/** Frontend screen to show immediately after the next frontend-level BeginPlay. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66ScreenType PendingFrontendScreen = ET66ScreenType::None;

	/** Optional modal to open after PendingFrontendScreen is shown by direct-entry automation. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	ET66ScreenType PendingDirectEntryModal = ET66ScreenType::None;

	/** True when frontend startup should immediately transition to gameplay through a direct-entry request. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	bool bPendingDirectGameplayEntry = false;

	/** Human-readable source of the pending direct gameplay entry request, used for diagnostics. */
	UPROPERTY(BlueprintReadWrite, Category = "Flow")
	FString PendingDirectGameplayEntrySource;

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

	/** Get the loaded Pets DataTable (loads if necessary, optional during Foundation seam work). */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetPetsDataTable();

	/** Get the loaded Items DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetItemsDataTable();

	/** Get the loaded Idols DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetIdolsDataTable();

	/** Get the loaded Weapons DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetWeaponsDataTable();

	/** Get the loaded Combat VFX Bindings DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetCombatVFXBindingsDataTable();

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

	/** Build-aware candidate weight for item offers/drops. Returns a safe baseline when no build signal exists. */
	float GetSmartLootItemTemplateWeight(FName ItemID);

	/** Deterministic build-aware item roll from a caller-provided candidate pool. */
	FName GetSmartLootItemIDFromPoolFromStream(const TArray<FName>& CandidateItemIDs, FRandomStream& Stream);

	/** Get the loaded Bosses DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossesDataTable();

	/** Get the loaded boss attack ownership DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossAttacksDataTable();

	/** Get the loaded boss attack definition DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossAttackDefinitionsDataTable();

	/** Get the loaded boss hazard definition DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossHazardDefinitionsDataTable();

	/** Get the loaded boss movement pattern DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossMovementPatternsDataTable();

	/** Get the loaded Stages DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetStagesDataTable();

	/** Get the loaded Enemies DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetEnemiesDataTable();

	/** Get the loaded Status Effects DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetStatusEffectsDataTable();

	/** Get the loaded Boss Encounters DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossEncountersDataTable();

	/** Get the loaded Boss Encounter Members DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetBossEncounterMembersDataTable();

	/** Get the loaded NPCs DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetNPCsDataTable();

	/** Get the loaded Loan Shark DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetLoanSharkDataTable();

	/** Get the loaded Unique Enemies DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetUniqueEnemiesDataTable();

	/** Get the loaded Character Visuals DataTable (loads if necessary) */
	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetCharacterVisualsDataTable();

	/** Get item data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetItemData(FName ItemID, FItemData& OutItemData);

	/** Get idol data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetIdolData(FName IdolID, FIdolData& OutIdolData);

	/** Get weapon data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetWeaponData(FName WeaponID, FWeaponData& OutWeaponData);

	/** Get combat VFX binding by source/type/category. Returns false if not found. */
	bool GetCombatVFXBindingData(ET66CombatVFXBindingSourceType SourceType, FName SourceID, ET66AttackCategory AttackCategory, FT66CombatVFXBindingData& OutBindingData);

	/** Get boss data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetBossData(FName BossID, FBossData& OutBossData);

	/** Get authored attack ownership rows for a boss. */
	void GetBossAttackOwnershipRows(FName BossID, TArray<FT66BossAttackOwnershipData>& OutRows);

	/** Get enabled attack definition rows for an attack identity and current phase, sorted by SequenceIndex. */
	void GetBossAttackDefinitionRows(FName AttackID, int32 Phase, TArray<FT66BossAttackDefinitionData>& OutRows);

	/** Get authored persistent hazard definition by HazardID. */
	bool GetBossHazardDefinitionData(FName HazardID, FT66BossHazardDefinitionData& OutDefinition);

	/** Get authored movement pattern rows for a movement profile. */
	void GetBossMovementPatternRows(FName MovementProfileID, TArray<FT66BossMovementPatternData>& OutRows);

	/** Get stage data by stage number. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetStageData(int32 StageNumber, FStageData& OutStageData);

	/** Get enemy data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetEnemyData(FName EnemyID, FT66EnemyData& OutEnemyData);

	/** Get negative status effect data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetStatusEffectData(FName StatusEffectID, FT66StatusEffectData& OutStatusEffectData);

	/** Get boss encounter data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetBossEncounterData(FName BossEncounterID, FT66BossEncounterData& OutEncounterData);

	/** Get all member rows for a boss encounter, sorted by MemberIndex. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	void GetBossEncounterMemberData(FName BossEncounterID, TArray<FT66BossEncounterMemberData>& OutMembers);

	/** Get NPC data by ID (row name). Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetNPCData(FName NPCID, FT66NPCData& OutNPCData);

	/** Get loan shark tuning data. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetLoanSharkData(FName LoanSharkID, FLoanSharkData& OutData);

	/** Get unique enemy tuning data. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetUniqueEnemyData(FName UniqueEnemyID, FUniqueEnemyData& OutData);

	/** Get hero data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetHeroData(FName HeroID, FHeroData& OutHeroData);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection|Custom Hero")
	static FName GetCustomHeroID();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection|Custom Hero")
	static bool IsCustomHeroID(FName HeroID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection|Custom Hero")
	bool IsCustomHeroConfigured() const { return CustomHeroBuild.bConfigured; }

	UFUNCTION(BlueprintCallable, Category = "Selection|Custom Hero")
	void ConfigureCustomHero(FName WeaponSourceHeroID, FName VisualSourceHeroID, ET66BodyType BodyType, const FT66HeroStatBlock& Stats);

	UFUNCTION(BlueprintCallable, Category = "Selection|Custom Hero")
	FName ResolveCustomHeroWeaponSourceHeroID(FName HeroID) const;

	UFUNCTION(BlueprintCallable, Category = "Selection|Custom Hero")
	FName ResolveCustomHeroVisualSourceHeroID(FName HeroID) const;

	UFUNCTION(BlueprintCallable, Category = "Selection|Custom Hero")
	ET66BodyType ResolveCustomHeroBodyType(FName HeroID, ET66BodyType FallbackBodyType) const;

	/** Get companion data by ID. Returns false if not found. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetCompanionData(FName CompanionID, FCompanionData& OutCompanionData);

	/** Get pet data by pet/source boss ID. Synthesizes a fallback from Bosses when DT_Pets is absent. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetPetData(FName PetID, FPetData& OutPetData);

	/** Resolve a pet ID from a defeated boss ID. Rows are keyed by boss ID for the temporary data layer. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	FName ResolvePetIDForBossID(FName BossID);

	/** Get all hero IDs from the DataTable */
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FName> GetAllHeroIDs();

	/** Get hero IDs currently playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	TArray<FName> GetPlayableHeroIDs();

	/** Returns true when the hero is playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	bool IsHeroPlayable(FName HeroID) const;

	/** Resolve to the requested hero if playable, otherwise the first playable hero. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	FName ResolvePlayableHeroID(FName HeroID);

	/** Get difficulty choices currently playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	TArray<ET66Difficulty> GetPlayableDifficulties() const;

	/** Get difficulty choices that should remain visible in selection UI. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	TArray<ET66Difficulty> GetVisibleDifficulties() const;

	/** Returns true when the difficulty is playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	bool IsDifficultyPlayable(ET66Difficulty Difficulty) const;

	/** Resolve to the requested difficulty if playable, otherwise the first playable difficulty. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	ET66Difficulty ResolvePlayableDifficulty(ET66Difficulty Difficulty) const;

	/** Returns true when the run category is playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	bool IsRunCategoryPlayable(ET66RunCategory RunCategory) const;

	/** Resolve to the requested run category if playable, otherwise the default tower run. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	ET66RunCategory ResolvePlayableRunCategory(ET66RunCategory RunCategory) const;

	/** Returns true when the Lab Collector is playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	bool IsCollectorPlayable() const;

	/** Get all companion IDs from the DataTable */
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FName> GetAllCompanionIDs();

	/** Get companion IDs currently playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	TArray<FName> GetPlayableCompanionIDs();

	/** Get all pet IDs from DT_Pets or, until that table exists, from DT_Bosses. */
	UFUNCTION(BlueprintCallable, Category = "Data")
	TArray<FName> GetAllPetIDs();

	/** Returns true when the companion is playable for this release variant. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	bool IsCompanionPlayable(FName CompanionID) const;

	/** Resolve to the requested companion if playable, otherwise no companion. */
	UFUNCTION(BlueprintCallable, Category = "Release")
	FName ResolvePlayableCompanionID(FName CompanionID) const;

	/** Get the currently selected hero data */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool GetSelectedHeroData(FHeroData& OutHeroData);

	/** Get the currently selected companion data (returns false if no companion selected) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool GetSelectedCompanionData(FCompanionData& OutCompanionData);

	/** Get the currently selected pet data (returns false if no pet selected). */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool GetSelectedPetData(FPetData& OutPetData);

	/** Resolve a hero portrait for a body type + portrait state (low / half / full). */
	TSoftObjectPtr<UTexture2D> ResolveHeroPortrait(FName HeroID, ET66BodyType BodyType, ET66HeroPortraitVariant Variant) const;

	/** Resolve a hero portrait from already-loaded hero data for a body type + portrait state. */
	TSoftObjectPtr<UTexture2D> ResolveHeroPortrait(const FHeroData& HeroData, ET66BodyType BodyType, ET66HeroPortraitVariant Variant) const;

	// ============================================
	// Hero Stats (Foundational + Level-up gains)
	// ============================================

	/**
	 * Get hero stat tuning (base stats and fixed per-level gains) for a hero.
	 * This is used both by the run-state leveling system and by the hero selection UI.
	 */
	bool GetHeroStatTuning(FName HeroID, FT66HeroStatBlock& OutBaseStats, FT66HeroPerLevelStatGains& OutPerLevelGains) const;

	// ============================================
	// Selection Helpers
	// ============================================

	/** Clear all selections (reset to defaults) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearSelections();

	UFUNCTION(BlueprintCallable, Category = "Daily Descent")
	void CacheDailyClimbChallenge(const FT66DailyClimbChallengeData& Challenge) { CachedDailyClimbChallenge = Challenge; }

	UFUNCTION(BlueprintCallable, Category = "Daily Descent")
	void BeginDailyClimbRun(const FT66DailyClimbChallengeData& Challenge);

	UFUNCTION(BlueprintCallable, Category = "Daily Descent")
	void ClearActiveDailyClimbRun();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
	bool IsDailyClimbRun() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
	bool IsOfflineRun() const { return SelectedRunMode == ET66RunMode::Offline; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
	bool IsLabRun() const { return SelectedRunCategory == ET66RunCategory::Lab && IsRunCategoryPlayable(ET66RunCategory::Lab); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
	bool IsTutorialRun() const { return SelectedRunCategory == ET66RunCategory::Tutorial; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Run")
	bool IsTestRoomRun() const { return SelectedRunCategory == ET66RunCategory::TestRoom; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Daily Descent")
	int32 GetDailyClimbIntRuleValue(ET66DailyClimbRuleType RuleType, int32 DefaultValue = 0) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Daily Descent")
	float GetDailyClimbFloatRuleValue(ET66DailyClimbRuleType RuleType, float DefaultValue = 0.0f) const;

	/** Check if a hero is selected */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool HasHeroSelected() const { return !SelectedHeroID.IsNone(); }

	/** Check if a companion is selected (NAME_None means no companion) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool HasCompanionSelected() const { return !SelectedCompanionID.IsNone(); }

	/** Check if a run modifier is selected. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection")
	bool HasSelectedRunModifier() const
	{
		return SelectedRunModifierKind != ET66RunModifierKind::None && !SelectedRunModifierID.IsNone();
	}

	/** Check if the selected run modifier is a challenge. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection")
	bool HasSelectedRunChallenge() const
	{
		return SelectedRunModifierKind == ET66RunModifierKind::Challenge && !SelectedRunModifierID.IsNone();
	}

	/** Check if the selected run modifier is a gameplay mod. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selection")
	bool HasSelectedRunMod() const
	{
		return SelectedRunModifierKind == ET66RunModifierKind::Mod && !SelectedRunModifierID.IsNone();
	}

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
	 * Show a loading screen, pre-load gameplay assets, then locally open GameplayLevel.
	 * Call this instead of raw UGameplayStatics::OpenLevel for local standalone gameplay transitions.
	 * Multiplayer/session travel stays owned by UT66SessionSubsystem so it can preserve
	 * ServerTravel, ClientTravel, and listen-server semantics.
	 */
	void TransitionToGameplayLevel();

	/**
	 * Open the frontend level through the GameInstance-owned travel boundary.
	 * This intentionally preserves raw frontend travel behavior: no loading screen,
	 * no gameplay preload, and no run/session side effects.
	 */
	static void TransitionToFrontendLevel(const UObject* WorldContextObject);

	void MarkPendingDirectGameplayEntry(const FString& Source);
	bool ConsumePendingDirectGameplayEntry(FString& OutSource);
	void ClearPendingDirectGameplayEntry();

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
	/** Main gameplay map package path. */
	static FName GetGameplayLevelName();
	/** Entry map package path for a brand-new Tribulation run. */
	static FName GetTribulationEntryLevelName();

private:
	UDataTable* ResolveCachedDataTable(TObjectPtr<UDataTable>& Cached, const TSoftObjectPtr<UDataTable>& Soft);

	template <typename TRow>
	static bool FindDataRow(UDataTable* DataTable, FName RowID, TRow& OutRow, const TCHAR* Context, bool bRequireValidID = true)
	{
		if (!DataTable || (bRequireValidID && RowID.IsNone()))
		{
			return false;
		}
		if (TRow* FoundRow = DataTable->FindRow<TRow>(RowID, Context))
		{
			OutRow = *FoundRow;
			return true;
		}
		return false;
	}

	void PrimeCoreDataTablesAsync();
	void PrimeCorePresentationAssetsAsync();
	void HandleCoreDataTablesLoaded();
	void HandleCorePresentationAssetsLoaded();
	void HandleHeroSelectionAssetsLoaded();
	void HandleHeroSelectionPreviewVisualsLoaded();
	bool QueueGameplayVisualAssetPreload();
	void PollGameplayVisualPreloadCompletion();
	void FinalizeGameplayAssetsPreload();
	void RestoreRememberedSelectionDefaults();

	bool bCoreDataTablesLoadRequested = false;
	bool bCoreDataTablesLoaded = false;
	TSharedPtr<FStreamableHandle> CoreDataTablesLoadHandle;

	bool bCorePresentationAssetsLoadRequested = false;
	bool bCorePresentationAssetsLoaded = false;
	TSharedPtr<FStreamableHandle> CorePresentationAssetsLoadHandle;

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

	/** Cached loaded Pets DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedPetsDataTable;

	/** Cached loaded Items DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedItemsDataTable;

	/** Cached loaded Idols DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedIdolsDataTable;

	/** Cached loaded Weapons DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedWeaponsDataTable;

	/** Cached loaded Combat VFX Bindings DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedCombatVFXBindingsDataTable;


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

	/** Cached loaded boss attack ownership DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedBossAttacksDataTable;

	/** Cached loaded boss attack definition DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedBossAttackDefinitionsDataTable;

	/** Cached loaded boss hazard definition DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedBossHazardDefinitionsDataTable;

	/** Cached loaded boss movement pattern DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedBossMovementPatternsDataTable;


	/** Cached loaded Stages DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedStagesDataTable;

	/** Cached loaded Enemies DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedEnemiesDataTable;

	/** Cached loaded Status Effects DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedStatusEffectsDataTable;

	/** Cached loaded Boss Encounters DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedBossEncountersDataTable;

	/** Cached loaded Boss Encounter Members DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedBossEncounterMembersDataTable;


	/** Cached loaded NPCs DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedNPCsDataTable;

	/** Cached loaded Loan Shark DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedLoanSharkDataTable;

	/** Cached loaded Unique Enemies DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedUniqueEnemiesDataTable;

	/** Cached loaded Character Visuals DataTable */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedCharacterVisualsDataTable;

	// Gameplay asset pre-load tracking.
	bool bGameplayAssetsPreloadInFlight = false;
	bool bGameplayPreloadWaitingOnCoreTables = false;
	bool bGameplayVisualAssetsPhaseQueued = false;
	int32 GameplayVisualPreloadPollRetriesRemaining = 0;
	TSharedPtr<FStreamableHandle> GameplayAssetsPreloadHandle;
	TFunction<void()> GameplayAssetsPreloadCallback;
	FTimerHandle GameplayVisualPreloadPollTimerHandle;
	void HandleGameplayAssetsPreloaded();
	TSharedPtr<SBorder> PersistentGameplayTransitionCurtain;
};

