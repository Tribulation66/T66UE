// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SaveMigration.h"
#include "Data/T66DataTypes.h"
#include "Core/PlayerExperience/T66PlayerExperienceTypes.h"
#include "GameFramework/SaveGame.h"
#include "Core/T66RunIntegrityTypes.h"
#include "T66LeaderboardRunSummarySaveGame.generated.h"

/**
 * Snapshot of a run intended to be opened from a leaderboard entry.
 * This is the "beginning of the system" for per-leaderboard-run summary files.
 *
 * v0 scope: local best Score run only (offline leaderboard).
 */
UCLASS()
class T66_API UT66LeaderboardRunSummarySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Bump if fields change in a breaking way. */
	UPROPERTY(SaveGame)
	int32 SchemaVersion = T66CurrentRunSummarySchemaVersion;

	/** Backend leaderboard entry UUID when this snapshot came from the online service. */
	UPROPERTY(SaveGame)
	FString EntryId;

	/** SteamID that owns this run summary when sourced from the online backend. */
	UPROPERTY(SaveGame)
	FString OwnerSteamId;

	UPROPERTY(SaveGame)
	FString OwnerDisplayName;

	/** What kind of leaderboard this run belongs to (Score or SpeedRun). */
	UPROPERTY(SaveGame)
	ET66LeaderboardType LeaderboardType = ET66LeaderboardType::Score;

	/** True when this run should be eligible for high-score leaderboard submission. */
	UPROPERTY(SaveGame)
	bool bSubmitHighScoreLeaderboard = true;

	/** True when this run should be eligible for speedrun leaderboard submission. */
	UPROPERTY(SaveGame)
	bool bSubmitSpeedRunLeaderboard = true;

	UPROPERTY(SaveGame)
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(SaveGame)
	ET66PartySize PartySize = ET66PartySize::Solo;

	/** UTC timestamp for provenance/debug. */
	UPROPERTY(SaveGame)
	FDateTime SavedAtUtc = FDateTime::MinValue();

	/** UTC timestamp for when the run ended. */
	UPROPERTY(SaveGame)
	FDateTime RunEndedAtUtc = FDateTime::MinValue();

	/** Active run duration in seconds at the moment the run ended. */
	UPROPERTY(SaveGame)
	float RunDurationSeconds = 0.f;

	/** True when the player fully cleared the selected difficulty. */
	UPROPERTY(SaveGame)
	bool bWasFullClear = false;

	/** True when Speed Run mode was enabled for this run. */
	UPROPERTY(SaveGame)
	bool bWasSpeedRunMode = false;

	// ===== Core run summary =====

	UPROPERTY(SaveGame)
	int32 StageReached = 1;

	UPROPERTY(SaveGame)
	int32 Score = 0;

	/** All-time score rank captured for this run summary, if known. */
	UPROPERTY(SaveGame)
	int32 ScoreRankAllTime = 0;

	/** Weekly score rank captured for this run summary, if known. */
	UPROPERTY(SaveGame)
	int32 ScoreRankWeekly = 0;

	/** All-time speedrun rank captured for this run summary, if known. */
	UPROPERTY(SaveGame)
	int32 SpeedRunRankAllTime = 0;

	/** Weekly speedrun rank captured for this run summary, if known. */
	UPROPERTY(SaveGame)
	int32 SpeedRunRankWeekly = 0;

	// ===== Identity / selection =====

	UPROPERTY(SaveGame)
	FName HeroID = NAME_None;

	UPROPERTY(SaveGame)
	ET66BodyType HeroBodyType = ET66BodyType::Chad;

	UPROPERTY(SaveGame)
	FName CompanionID = NAME_None;

	UPROPERTY(SaveGame)
	ET66BodyType CompanionBodyType = ET66BodyType::Chad;

	// ===== Level / stats =====

	UPROPERTY(SaveGame)
	int32 HeroLevel = 1;

	UPROPERTY(SaveGame)
	int32 HeroMasteryLevel = 1;

	UPROPERTY(SaveGame)
	int32 HeroMasteryXP = 0;

	UPROPERTY(SaveGame)
	int32 DamageStat = 1;

	UPROPERTY(SaveGame)
	int32 AttackSpeedStat = 1;

	UPROPERTY(SaveGame)
	int32 AttackScaleStat = 1;

	UPROPERTY(SaveGame)
	int32 AccuracyStat = 1;

	UPROPERTY(SaveGame)
	int32 ArmorStat = 1;

	UPROPERTY(SaveGame)
	int32 EvasionStat = 1;

	UPROPERTY(SaveGame)
	int32 LuckStat = 1;

	UPROPERTY(SaveGame)
	int32 SpeedStat = 1;

	/** Secondary stat values (for full stats panel in Run Summary). SchemaVersion>=6. Populated for fake snapshots and when saving local best. */
	UPROPERTY(SaveGame)
	TMap<ET66StatType, float> StatValues;

	// ===== Ratings (post-run) =====

	/** Luck Rating (0..100). SchemaVersion>=2. */
	UPROPERTY(SaveGame)
	int32 LuckRating0To100 = -1;

	/** Fixed per-run Seed Luck (0..100). SchemaVersion>=19. */
	UPROPERTY(SaveGame)
	int32 SeedLuck0To100 = -1;

	/** Aggregate modifier percent applied to Seed Luck. SchemaVersion>=19. */
	UPROPERTY(SaveGame)
	float LuckModifierPercent = 0.f;

	/** Effective Seed Luck after modifiers. SchemaVersion>=19. */
	UPROPERTY(SaveGame)
	float EffectiveLuck = 0.f;

	/** Quantity component (0..100). SchemaVersion>=2. */
	UPROPERTY(SaveGame)
	int32 LuckRatingQuantity0To100 = -1;

	/** Quality component (0..100). SchemaVersion>=2. */
	UPROPERTY(SaveGame)
	int32 LuckRatingQuality0To100 = -1;

	/** Skill Rating (0..100). SchemaVersion>=4. */
	UPROPERTY(SaveGame)
	int32 SkillRating0To100 = -1;

	// ===== Anti-cheat telemetry =====

	/** Run-scoped RNG seed used for this summary (debug/provenance). SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	int32 RunSeed = 0;

	/** Total recorded quantity-roll samples contributing to Luck Rating. SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	int32 LuckQuantitySampleCount = 0;

	/** Total recorded quality-roll samples contributing to Luck Rating. SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	int32 LuckQualitySampleCount = 0;

	/** Category-level quantity accumulators used to review Luck Rating. SchemaVersion>=10. */
	UPROPERTY(SaveGame)
	TArray<FT66SavedLuckAccumulator> LuckQuantityAccumulators;

	/** Category-level quality accumulators used to review Luck Rating. SchemaVersion>=10. */
	UPROPERTY(SaveGame)
	TArray<FT66SavedLuckAccumulator> LuckQualityAccumulators;

	/** Sampled per-roll luck telemetry used for anti-cheat review. SchemaVersion>=11. */
	UPROPERTY(SaveGame)
	TArray<FT66AntiCheatLuckEvent> AntiCheatLuckEvents;

	/** True if the per-roll luck telemetry ring buffer truncated older events. SchemaVersion>=11. */
	UPROPERTY(SaveGame)
	bool bAntiCheatLuckEventsTruncated = false;

	/** Sampled per-hit dodge/evasion telemetry used for anti-cheat review. SchemaVersion>=11. */
	UPROPERTY(SaveGame)
	TArray<FT66AntiCheatHitCheckEvent> AntiCheatHitCheckEvents;

	/** True if the per-hit telemetry ring buffer truncated older events. SchemaVersion>=11. */
	UPROPERTY(SaveGame)
	bool bAntiCheatHitCheckEventsTruncated = false;

	/** Number of incoming hit checks that evaluated dodge/evasion. SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	int32 IncomingHitChecks = 0;

	/** Number of those hit checks that actually applied damage. SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	int32 DamageTakenHitCount = 0;

	/** Number of successful dodges observed during the run. SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	int32 DodgeCount = 0;

	/** Longest observed consecutive dodge streak during the run. SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	int32 MaxConsecutiveDodges = 0;

	/** Sum of sampled dodge chances across all incoming hit checks. SchemaVersion>=9. */
	UPROPERTY(SaveGame)
	float TotalEvasionChance = 0.f;

	/** Evasion-chance buckets used to review dodge plausibility even when hit-check samples truncate. SchemaVersion>=13. */
	UPROPERTY(SaveGame)
	TArray<FT66AntiCheatEvasionBucketSummary> AntiCheatEvasionBuckets;

	/** Compact 5-second combat-pressure summary used to review skill/dodge plausibility. SchemaVersion>=13. */
	UPROPERTY(SaveGame)
	FT66AntiCheatPressureWindowSummary AntiCheatPressureWindowSummary;

	/** Per-game gambler summary totals used for post-run anti-cheat review. SchemaVersion>=15. */
	UPROPERTY(SaveGame)
	TArray<FT66AntiCheatGamblerGameSummary> AntiCheatGamblerSummaries;

	/** Per-round gambler telemetry used for post-run anti-cheat review. SchemaVersion>=15. */
	UPROPERTY(SaveGame)
	TArray<FT66AntiCheatGamblerEvent> AntiCheatGamblerEvents;

	/** True if the per-round gambler telemetry ring buffer truncated older events. SchemaVersion>=15. */
	UPROPERTY(SaveGame)
	bool bAntiCheatGamblerEventsTruncated = false;

	/** Build and runtime integrity context captured for this run. SchemaVersion>=16. */
	UPROPERTY(SaveGame)
	FT66RunIntegrityContext IntegrityContext;

	/** Legal spawned score budget versus awarded score for anti-cheat review. */
	UPROPERTY(SaveGame)
	FT66ScoreBudget ScoreBudgetContext;

	// ===== Proof of Run =====

	/** Proof-of-run URL (e.g. YouTube). SchemaVersion>=3. */
	UPROPERTY(SaveGame)
	FString ProofOfRunUrl;

	/** True if ProofOfRunUrl is considered "confirmed" (locks edit by default). SchemaVersion>=3. */
	UPROPERTY(SaveGame)
	bool bProofOfRunLocked = false;

	// ===== Inventory / idols / log =====

	/** Equipped idol IDs in fixed run-summary slots. SchemaVersion>=23 stores four slots. */
	UPROPERTY(SaveGame)
	TArray<FName> EquippedIdols;

	/** Equipped idol tier values aligned with EquippedIdols (0 empty, 1 black .. 4 white). SchemaVersion>=23. */
	UPROPERTY(SaveGame)
	TArray<uint8> EquippedIdolTiers;

	/** Equipped idol elements aligned with EquippedIdols. SchemaVersion>=24. */
	UPROPERTY(SaveGame)
	TArray<ET66IdolElement> EquippedIdolElements;

	/** Equipped idol attack categories aligned with EquippedIdols. SchemaVersion>=24. */
	UPROPERTY(SaveGame)
	TArray<ET66AttackCategory> EquippedIdolCategories;

	UPROPERTY(SaveGame)
	TArray<FName> Inventory;

	/** Rarity-bearing inventory slot snapshot. Keep Inventory for old wire compatibility. SchemaVersion>=24. */
	UPROPERTY(SaveGame)
	TArray<FT66InventorySlot> InventorySlots;

	/** No Idol altar selections accumulated this run. SchemaVersion>=24. */
	UPROPERTY(SaveGame)
	int32 NoIdolSelectionStacks = 0;

	UPROPERTY(SaveGame)
	FT66HeroPreciseStatBlock NoIdolBaseStatBonuses;

	/** Mob Loot quantities are sellable-stack quantities, not active world-drop counts. SchemaVersion>=24. */
	UPROPERTY(SaveGame)
	int32 MobLootDropsCollectedThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootQuantityCollectedThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootGoldValueCollectedThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootQuantityCollectedByPlayerThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootQuantityCollectedByPetThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootDropsCollectedByPetThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootQuantitySoldThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootSaleGoldThisRun = 0;

	UPROPERTY(SaveGame)
	int32 MobLootRemainingStack = 0;

	/** Player-facing gambler outcome copy; anti-cheat arrays are preserved separately. SchemaVersion>=24. */
	UPROPERTY(SaveGame)
	TArray<FT66AntiCheatGamblerGameSummary> GamblerOutcomeSummaries;

	UPROPERTY(SaveGame)
	TArray<FT66AntiCheatGamblerEvent> GamblerOutcomeEvents;

	UPROPERTY(SaveGame)
	bool bGamblerOutcomeEventsTruncated = false;

	UPROPERTY(SaveGame)
	int32 CurrentGold = 0;

	UPROPERTY(SaveGame)
	int32 CurrentDebt = 0;

	UPROPERTY(SaveGame)
	int32 InventorySellValueTotal = 0;

	UPROPERTY(SaveGame)
	int32 NetWorth = 0;

	UPROPERTY(SaveGame)
	int32 ActiveVendorTokenStacks = 0;

	UPROPERTY(SaveGame)
	float CurrentSellFraction = 0.f;

	UPROPERTY(SaveGame)
	int32 ShopStockCount = 0;

	UPROPERTY(SaveGame)
	int32 BuybackPoolSize = 0;

	UPROPERTY(SaveGame)
	FName EquippedWeaponID = NAME_None;

	UPROPERTY(SaveGame)
	ET66AttackCategory EquippedWeaponBranch = ET66AttackCategory::Pierce;

	UPROPERTY(SaveGame)
	ET66WeaponRarity EquippedWeaponRarity = ET66WeaponRarity::Black;

	UPROPERTY(SaveGame)
	FName EquippedWeaponAttackPatternID = NAME_None;

	UPROPERTY(SaveGame)
	int32 EquippedWeaponProjectileCount = 0;

	UPROPERTY(SaveGame)
	float EquippedWeaponSpreadAngleDegrees = 0.f;

	UPROPERTY(SaveGame)
	FName ActivePetID = NAME_None;

	UPROPERTY(SaveGame)
	FName ActivePetSkinID = NAME_None;

	UPROPERTY(SaveGame)
	int32 ActivePetBondStagesCleared = 0;

	UPROPERTY(SaveGame)
	float ActivePetBondMovementSpeedMultiplier = 1.f;

	UPROPERTY(SaveGame)
	int32 PetMobLootQuantityCollectedThisRun = 0;

	UPROPERTY(SaveGame)
	int32 PetMobLootDropsCollectedThisRun = 0;

	UPROPERTY(SaveGame)
	bool bBossActiveAtSummary = false;

	UPROPERTY(SaveGame)
	FName ActiveBossID = NAME_None;

	UPROPERTY(SaveGame)
	int32 BossMaxHP = 0;

	UPROPERTY(SaveGame)
	int32 BossCurrentHP = 0;

	UPROPERTY(SaveGame)
	TArray<FT66BossPartSnapshot> BossParts;

	UPROPERTY(SaveGame)
	TArray<FName> OwedBossIDs;

	UPROPERTY(SaveGame)
	int32 CowardiceGatesTakenCount = 0;

	/** Temporary buff loadout that was active when the run summary snapshot was captured. SchemaVersion>=17. */
	UPROPERTY(SaveGame)
	TArray<ET66StatType> TemporaryBuffSlots;

	UPROPERTY(SaveGame)
	TArray<FString> EventLog;

	/** Damage dealt by source this run (source ID -> total). SchemaVersion>=5. */
	UPROPERTY(SaveGame)
	TMap<FName, int32> DamageBySource;

	/** Damage received by source this run (source ID -> total). SchemaVersion>=22. */
	UPROPERTY(SaveGame)
	TMap<FName, int32> DamageReceivedBySource;

	/** Per-stage cumulative pacing checkpoints captured at the end of each cleared stage. SchemaVersion>=8. */
	UPROPERTY(SaveGame)
	TArray<FT66StagePacingPoint> StagePacingPoints;

	/** Display name for this player (used by Pick the Player modal for fake snapshots; not persisted to slot). */
	UPROPERTY(Transient)
	FString DisplayName;
};

