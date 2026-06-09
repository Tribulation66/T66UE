// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66RngTuningConfig.h"
#include "Core/T66DailyClimbTypes.h"
#include "Core/T66RunTypes.h"
#include "Core/T66SaveMigration.h"
#include "GameFramework/SaveGame.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66BossPartTypes.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Core/PlayerExperience/T66PlayerExperienceTypes.h"
#include "Core/T66RunIntegrityTypes.h"
#include "T66RunSaveGame.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66SavedLuckAccumulator
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName Category = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float Sum01 = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 Count = 0;
};

UENUM(BlueprintType)
enum class ET66AntiCheatLuckEventType : uint8
{
	QuantityRoll   UMETA(DisplayName = "Quantity Roll"),
	QuantityBool   UMETA(DisplayName = "Quantity Bool"),
	QualityRarity  UMETA(DisplayName = "Quality Rarity")
};

USTRUCT(BlueprintType)
struct T66_API FT66AntiCheatLuckEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66AntiCheatLuckEventType EventType = ET66AntiCheatLuckEventType::QuantityRoll;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName Category = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float TimeSeconds = 0.f;

	/** Fixed per-run Seed Luck (0..100) active when this event was recorded. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SeedLuck0To100 = -1;

	/** Aggregate luck modifier percentage active when this event was recorded. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float LuckModifierPercent = 0.f;

	/** Effective Seed Luck after applying modifiers when this event was recorded. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float EffectiveLuck = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float Value01 = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 RawValue = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 RawMin = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 RawMax = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 RunDrawIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PreDrawSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 LuckStatSnapshot = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float Luck01Snapshot = -1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float ExpectedChance01 = -1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bHasRarityWeights = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66RarityWeights RarityWeights;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bHasFloatReplayRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float FloatReplayMin = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float FloatReplayMax = 0.f;
};

USTRUCT(BlueprintType)
struct T66_API FT66AntiCheatHitCheckEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float TimeSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float EvasionChance01 = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bDodged = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bDamageApplied = false;
};

USTRUCT(BlueprintType)
struct T66_API FT66AntiCheatEvasionBucketSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 BucketIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float MinChance01 = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float MaxChance01 = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 HitChecks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 Dodges = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 DamageApplied = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float ExpectedDodges = 0.f;
};

USTRUCT(BlueprintType)
struct T66_API FT66AntiCheatPressureWindowSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 WindowSeconds = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ActiveWindows = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PressuredWindows4Plus = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PressuredWindows8Plus = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ZeroDamageWindows4Plus = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ZeroDamageWindows8Plus = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 NearPerfectWindows8Plus = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MaxHitChecksInWindow = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MaxDodgesInWindow = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MaxDamageAppliedInWindow = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float MaxExpectedDodgesInWindow = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 TotalHitChecks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 TotalDodges = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 TotalDamageApplied = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float TotalExpectedDodges = 0.f;
};

UENUM(BlueprintType)
enum class ET66AntiCheatGamblerGameType : uint8
{
	CoinFlip           UMETA(DisplayName = "Coin Flip"),
	GuessTheCup        UMETA(DisplayName = "Guess the Cup"),
	PickLongestShortestStick UMETA(DisplayName = "Pick Longest/Shortest Stick"),
	FindJoker          UMETA(DisplayName = "Find Joker")
};

USTRUCT(BlueprintType)
struct T66_API FT66AntiCheatGamblerGameSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66AntiCheatGamblerGameType GameType = ET66AntiCheatGamblerGameType::CoinFlip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 Rounds = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 Wins = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 Losses = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 Draws = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CheatAttempts = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CheatSuccesses = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 TotalBetGold = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 TotalPayoutGold = 0;
};

USTRUCT(BlueprintType)
struct T66_API FT66AntiCheatGamblerEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66AntiCheatGamblerGameType GameType = ET66AntiCheatGamblerGameType::CoinFlip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float TimeSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 BetGold = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PayoutGold = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bCheatAttempted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bCheatSucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bWin = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bDraw = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PlayerChoice = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 OpponentChoice = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 OutcomeValue = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 OutcomeSecondaryValue = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SelectedMask = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ResolvedMask = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PathBits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ShufflePreDrawSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ShuffleStartDrawIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 OutcomePreDrawSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 OutcomeDrawIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float OutcomeExpectedChance01 = -1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString ActionSequence;
};

USTRUCT(BlueprintType)
struct T66_API FT66SavedStatBonusEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66StatType StatType = ET66StatType::None;

	/** Fixed-point tenths of accumulated secondary stat points. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 BonusTenths = 0;
};

USTRUCT(BlueprintType)
struct T66_API FT66SavedRunSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentStage = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float CurrentHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float HeroDamagePercent = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<uint8> HeartSlotTiers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentGold = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentDebt = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 DifficultyTier = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 DifficultySkulls = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 TotemsActivatedCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FName> OwedBossIDs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CowardiceGatesTakenCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66InventorySlot> InventorySlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 ActiveVendorTokenStacks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FString> EventLog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FRunEvent> StructuredEventLog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66StagePacingPoint> StagePacingPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bStageTimerActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float StageTimerSecondsRemaining = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float SpeedRunElapsedSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bSpeedRunStarted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float CompletedStageActiveSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float FinalRunElapsedSeconds = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bRunEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bRunEndedAsVictory = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CurrentScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66ScoreBudget ScoreBudgetContext;

	/** Active in-run hero level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 HeroLevel = 1;

	/** Active in-run hero XP toward the next level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 HeroXP = 0;

	/** Active data-driven XP threshold for the next level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 XPToNextLevel = 0;

	/** Active display primary stats at save time. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66HeroStatBlock HeroStats;

	/** Active precise primary stats at save time. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66HeroPreciseStatBlock HeroPreciseStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 HeroStatRngCurrentSeed = 0;

	/** Persistent secondary gains created by in-run level-ups. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SavedStatBonusEntry> PersistentStatBonusEntries;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PowerCrystalsEarnedThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 PowerCrystalsGrantedToWalletThisRun = 0;

	/** Fixed per-run Seed Luck rolled once at run start. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SeedLuck0To100 = -1;

	/** Aggregate modifier percent applied to Seed Luck for runtime RNG biasing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float LuckModifierPercent = 0.f;

	/** Effective Seed Luck after applying modifiers. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float EffectiveLuck = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float CompanionHealingDoneThisRun = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bBossActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName ActiveBossID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 BossMaxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 BossCurrentHP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66BossPartSnapshot> BossParts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FName> EquippedIdols;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<uint8> EquippedIdolTiers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName EquippedWeaponID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bSaintBlessingActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66HeroPreciseStatBlock SaintBlessingBaseStatBonuses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SavedStatBonusEntry> SaintBlessingStatBonusEntries;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float FinalSurvivalEnemyScalar = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SavedLuckAccumulator> LuckQuantityAccumulators;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SavedLuckAccumulator> LuckQualityAccumulators;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66AntiCheatLuckEvent> AntiCheatLuckEvents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bAntiCheatLuckEventsTruncated = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66AntiCheatHitCheckEvent> AntiCheatHitCheckEvents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bAntiCheatHitCheckEventsTruncated = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatIncomingHitChecks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatDamageTakenHitCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatDodgeCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatCurrentConsecutiveDodges = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatMaxConsecutiveDodges = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float AntiCheatTotalEvasionChance = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66AntiCheatEvasionBucketSummary> AntiCheatEvasionBuckets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66AntiCheatPressureWindowSummary AntiCheatPressureWindowSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66AntiCheatGamblerGameSummary> AntiCheatGamblerSummaries;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66AntiCheatGamblerEvent> AntiCheatGamblerEvents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bAntiCheatGamblerEventsTruncated = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatCurrentPressureWindowIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatCurrentPressureHitChecks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatCurrentPressureDodges = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 AntiCheatCurrentPressureDamageApplied = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float AntiCheatCurrentPressureExpectedDodges = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 NoIdolSelectionStacks = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66HeroPreciseStatBlock NoIdolBaseStatBonuses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float UltimateCharge = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 CollectedMobLootStack = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootDropsCollectedThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootQuantityCollectedThisRun = 0;

	/** Total sell value represented by Mob Loot collected this run. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootGoldValueCollectedThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootQuantityCollectedByPlayerThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootQuantityCollectedByPetThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootDropsCollectedByPetThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootQuantitySoldThisRun = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 MobLootSaleGoldThisRun = 0;
};

USTRUCT(BlueprintType)
struct T66_API FT66SavedPartyPlayerState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString PlayerId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName HeroID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66BodyType HeroBodyType = ET66BodyType::Chad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName HeroSkinID = FName(TEXT("Default"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName CompanionID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66BodyType CompanionBodyType = ET66BodyType::Chad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FTransform PlayerTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bIsPartyHost = false;
};

UCLASS(BlueprintType)
class T66_API UT66RunSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = T66CurrentRunSaveVersion;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName HeroID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66BodyType HeroBodyType = ET66BodyType::Chad;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FName CompanionID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66RunMode RunMode = ET66RunMode::Regular;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66RunCategory RunCategory = ET66RunCategory::Tower;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 RunSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bIsDailyClimbRun = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66DailyClimbChallengeData DailyClimbChallenge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	ET66MainMapLayoutVariant MainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bRunIneligibleForLeaderboard = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66RunIntegrityContext IntegrityContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FT66SavedPartyPlayerState> SavedPartyPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FT66SavedRunSnapshot RunSnapshot;
};
