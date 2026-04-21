// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66Rarity.h"
#include "Templates/Function.h"
#include "T66RunStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHeartsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGoldChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLogAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPanelVisibilityChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoreChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageTimerChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpeedRunTimerChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDebtChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDifficultyChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGamblerAngerChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIdolsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHeroProgressChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSurvivalChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickReviveChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVendorChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatusEffectsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialHintChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialSubtitleChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialInputChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDevCheatsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCowardiceGatesTakenChanged);

class AActor;
class FSubsystemCollectionBase;
struct FT66SavedRunSnapshot;

/** Single DOT instance on one target (idol DOT damage over time). */
struct FT66DotInstance
{
	float RemainingDuration = 0.f;
	float TickInterval = 0.5f;
	float DamagePerTick = 0.f;
	double NextTickTime = 0.0;
	FName SourceIdolID = NAME_None;
};

/** DOT key allows multiple named DOT sources to coexist on the same target. */
struct FT66DotKey
{
	TWeakObjectPtr<AActor> Target;
	FName SourceIdolID = NAME_None;

	bool operator==(const FT66DotKey& Other) const
	{
		return Target == Other.Target && SourceIdolID == Other.SourceIdolID;
	}
};

FORCEINLINE uint32 GetTypeHash(const FT66DotKey& Key)
{
	return HashCombine(GetTypeHash(Key.Target.Get()), GetTypeHash(Key.SourceIdolID));
}

/** Result of a vendor steal attempt (used by UI feedback). */
enum class ET66VendorStealOutcome : uint8
{
	None = 0,
	Miss,
	Failed,
	InventoryFull,
	Success,
};

/**
 * Authoritative run state: health, gold, inventory, event log, HUD panel visibility.
 * Survives death; reset on Restart / Main Menu.
 */
UCLASS()
class T66_API UT66RunStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static constexpr int32 DefaultMaxHearts = 5;
	/** HP per heart by tier: red=20, blue=25, green=31.25, purple=39.0625, gold=48.828125. */
	static constexpr float HPPerRedHeart = 20.f;
	static constexpr float HeartTierScale = 1.25f;
	static constexpr int32 MaxHeartTier = 4;
	/** Default max HP at run start (5 red hearts). */
	static constexpr float DefaultMaxHP = 100.f;
	static constexpr float DefaultInvulnDurationSeconds = 0.75f;
	static constexpr int32 DefaultStartGold = 100;
	static constexpr int32 MaxInventorySlots = 20;
	static constexpr int32 MaxEquippedIdolSlots = UT66IdolManagerSubsystem::MaxEquippedIdolSlots;
	static constexpr int32 DefaultHeroLevel = 1;
	static constexpr int32 MaxHeroLevel = 99;
	static constexpr int32 MaxHeroStatValue = 99;
	static constexpr int32 HeroStatTenthsScale = 10;
	static constexpr int32 DefaultXPToLevel = 100;
	static constexpr float UltimateCooldownSeconds = 30.f;
	static constexpr int32 UltimateDamage = 200;
	static constexpr float SurvivalChargePerHeart = 0.20f; // 5 hearts of damage -> full
	static constexpr float LastStandDurationSeconds = 10.f;
	static constexpr float QuickReviveDownedDurationSeconds = 3.f;
	static constexpr int32 VendorAngerThresholdGold = 100;
	static constexpr int32 VendorDisplaySlotCount = 5;
	static constexpr int32 BuybackDisplaySlotCount = 5;
	// Safety: keep logs bounded so low-end machines never accumulate unbounded memory / UI work.
	static constexpr int32 MaxEventLogEntries = 400;
	static constexpr int32 MaxStructuredEventLogEntries = 800;
	static constexpr int32 MaxAntiCheatLuckEvents = 512;
	static constexpr int32 MaxAntiCheatHitCheckEvents = 1024;
	static constexpr int32 MaxAntiCheatGamblerEvents = 256;
	static constexpr int32 AntiCheatPressureWindowSeconds = 5;
	static constexpr int32 AntiCheatEvasionBucketCount = 5;

	static int32 ClampHeroStatValue(const int32 Value)
	{
		return FMath::Clamp(Value, 1, MaxHeroStatValue);
	}

	static int32 ClampHeroStatTenths(const int32 ValueTenths)
	{
		return FMath::Clamp(ValueTenths, HeroStatTenthsScale, MaxHeroStatValue * HeroStatTenthsScale);
	}

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnHeartsChanged HeartsChanged;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnGoldChanged GoldChanged;

	/** Debt changed (used for HUD "Owe" display and loan shark logic). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnDebtChanged DebtChanged;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnInventoryChanged InventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnLogAdded LogAdded;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnPanelVisibilityChanged PanelVisibilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnPlayerDied OnPlayerDied;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnScoreChanged ScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnStageTimerChanged StageTimerChanged;

	/** Speedrun elapsed time tracker (current stage segment; starts when stage timer becomes active). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnSpeedRunTimerChanged SpeedRunTimerChanged;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnStageChanged StageChanged;

	/** Boss UI state: active + HP changed (boss health bar). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnBossChanged BossChanged;

	/** Difficulty changed (for HUD + enemy scaling). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnDifficultyChanged DifficultyChanged;

	/** Cowardice gates taken count changed (for HUD clown icons). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnCowardiceGatesTakenChanged CowardiceGatesTakenChanged;

	/** Gambler anger meter changed (0..1). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnGamblerAngerChanged GamblerAngerChanged;

	/** Compatibility delegate: idol state now lives in UT66IdolManagerSubsystem. */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnIdolsChanged IdolsChanged;

	/** Hero XP/Level changed (used for HUD + derived stats). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnHeroProgressChanged HeroProgressChanged;

	/** Ultimate cooldown changed (used for HUD). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnUltimateChanged UltimateChanged;

	/** Survival charge / last-stand state changed (used for HUD + derived stats). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnSurvivalChanged SurvivalChanged;

	/** Quick revive token / downed state changed. */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnQuickReviveChanged QuickReviveChanged;

	/** Vendor shop / anger changed (used by Vendor overlay). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnVendorChanged VendorChanged;

	/** Hero status effects changed (burn/chill/curse). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnStatusEffectsChanged StatusEffectsChanged;

	/** Tutorial hint text changed (above crosshair). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnTutorialHintChanged TutorialHintChanged;

	/** Tutorial subtitle text changed (guide dialogue). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnTutorialSubtitleChanged TutorialSubtitleChanged;

	/** Tutorial input progress changed (move/jump). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnTutorialInputChanged TutorialInputChanged;

	/** Dev-only toggles (immortality/power) changed. */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnDevCheatsChanged DevCheatsChanged;

	/** Current HP (numerical). Hearts are visual only. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetCurrentHP() const { return CurrentHP; }

	/** Max HP (numerical). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetMaxHP() const { return MaxHP; }

	/** Actual HP restored by the current companion during this run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetCompanionHealingDoneThisRun() const { return CompanionHealingDoneThisRun; }

	/** Highest heart display tier across the 5 slots (0=red, 1=blue, 2=green, 3=purple, 4=gold). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetHeartDisplayTier() const;

	/** Display tier for one of the 5 heart slots (0=red, 1=blue, 2=green, 3=purple, 4=gold). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetHeartSlotTier(int32 SlotIndex) const;

	/** Fill amount for one of the 5 heart slots (0..1). SlotIndex 0..4. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetHeartSlotFill(int32 SlotIndex) const;

	/** Legacy: effective heart count (CurrentHP / 20) for compatibility. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentHearts() const { return FMath::RoundToInt(CurrentHP / HPPerRedHeart); }

	/** Legacy: effective max heart count (MaxHP / 20). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetMaxHearts() const { return FMath::RoundToInt(MaxHP / HPPerRedHeart); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentGold() const { return CurrentGold; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentDebt() const { return CurrentDebt; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetInventorySellValueTotal() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetNetWorth() const;

	/** Remaining additional gold the player can borrow before hitting their Net Worth-backed cap. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetRemainingBorrowCapacity() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool CanBorrowGold(int32 Amount) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetDifficultyTier() const { return DifficultyTier; }

	/** Difficulty as "Skulls" (integer; increments by 1 per totem interaction). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetDifficultySkulls() const { return DifficultySkulls; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetDifficultyScalarTier() const;

	/** Difficulty scalar applied to enemy/boss HP/damage/count/score (tier-based, per spec). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetDifficultyScalar() const;

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void IncreaseDifficultyTier();

	/** Add skulls directly. Updates cached tier and broadcasts DifficultyChanged. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddDifficultySkulls(int32 DeltaSkulls);

	/** Upgrade N heart slots by one tier and also grant the added HP immediately (clamped). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddMaxHearts(int32 DeltaHearts);

	/** World Interactables: difficulty totems activated this run (for visual stack growth). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetTotemsActivatedCount() const { return TotemsActivatedCount; }

	/** Register an activated difficulty totem. Returns the new activation index (1-based). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	int32 RegisterTotemActivated();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	float GetGamblerAnger01() const { return GamblerAnger01; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Casino")
	float GetCasinoAnger01() const { return GamblerAnger01; }

	/** Add anger using the shared casino threshold (100 anger-gold = full bar). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Casino")
	float AddCasinoAngerFromGold(int32 AngerGold);

	/** Add anger based on bet size: +min(1, Bet/100). Returns new anger. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	float AddGamblerAngerFromBet(int32 BetGold);

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ResetGamblerAnger();

	/** Bosses skipped via Cowardice Gate; owed bosses are repaid together on the difficulty's final stage. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FName>& GetOwedBossIDs() const { return OwedBossIDs; }

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddOwedBoss(FName BossID);

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void RemoveFirstOwedBoss();

	/** Clear all owed bosses and reset the current difficulty segment's cowardice count. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ClearOwedBosses();

	/** Cowardice gates taken this difficulty segment. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCowardiceGatesTaken() const { return CowardiceGatesTakenCount; }

	/** Add one (call when player confirms Cowardice Gate). Broadcasts CowardiceGatesTakenChanged. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddCowardiceGateTaken();

	/** Add gold (e.g. gambler win). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddGold(int32 Amount);

	/** Spend gold if possible (used by in-run gambling/shop costs). Returns true if spent. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool TrySpendGold(int32 Amount);

	/** Borrow gold: increases gold AND debt by Amount if still within the current remaining borrow capacity. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool BorrowGold(int32 Amount);

	/** Pay down debt using current gold. Pays up to Amount (clamped by gold and debt). Returns amount actually paid. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	int32 PayDebt(int32 Amount);

	/** Mark whether a LoanShark should spawn this stage once the timer starts (set when leaving previous stage with debt). */
	void SetLoanSharkPending(bool bPending) { bLoanSharkPending = bPending; }
	bool GetLoanSharkPending() const { return bLoanSharkPending; }

	// ============================================
	// Dev toggles (HUD switches)
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Dev")
	bool GetDevImmortalityEnabled() const { return bDevImmortality; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Dev")
	bool GetDevPowerEnabled() const { return bDevPower; }

	UFUNCTION(BlueprintCallable, Category = "RunState|Dev")
	void ToggleDevImmortality();

	UFUNCTION(BlueprintCallable, Category = "RunState|Dev")
	void ToggleDevPower();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FT66InventorySlot>& GetInventorySlots() const { return InventorySlots; }

	/** Legacy: returns template IDs only (for backward compat with UI that expects FName array). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	TArray<FName> GetInventory() const;

	/** Compatibility facade: idol state is owned by UT66IdolManagerSubsystem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FName>& GetEquippedIdols() const;

	/** Idol rarity tiers: black, red, yellow, white. */
	static constexpr int32 MaxIdolLevel = UT66IdolManagerSubsystem::MaxIdolLevel;

	/** Equipped idol rarity tier for a specific slot (0 empty, 1 black .. 4 white). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetEquippedIdolLevelInSlot(int32 SlotIndex) const;

	/** Equipped idol rarity for a specific slot. Empty/invalid slots fall back to Black. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	ET66ItemRarity GetEquippedIdolRarityInSlot(int32 SlotIndex) const;

	/** Raw idol tier values aligned with EquippedIdolIDs (0 empty, 1 black .. 4 white). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<uint8>& GetEquippedIdolTierValues() const;

	/** Convert between stored idol tier values and the shared item rarity enum. */
	static ET66ItemRarity IdolTierValueToRarity(int32 TierValue);
	static int32 IdolRarityToTierValue(ET66ItemRarity Rarity);

	/** Equip an idol into a specific slot (0..5). Returns true if changed. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool EquipIdolInSlot(int32 SlotIndex, FName IdolID);

	/** Equip an idol into the first empty slot. Returns true if equipped. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool EquipIdolFirstEmpty(FName IdolID);

	/**
	 * Idol altar selection:
	 * - if idol is already equipped, increase its rarity up to White
	 * - otherwise, equip into first empty slot at Black rarity
	 */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SelectIdolFromAltar(FName IdolID);

	/** Clear all equipped idols. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ClearEquippedIdols();

	/** List of all idol IDs (authoritative roster for the altar UI). */
	static const TArray<FName>& GetAllIdolIDs();

	/** Default tint color for an idol (used for UI + enemy status visuals). */
	static FLinearColor GetIdolColor(FName IdolID);

	/** Apply damage-over-time to a target (from idol DOT). Duration and TickInterval in seconds; damage applied each tick. */
	void ApplyDOT(AActor* Target, float Duration, float TickInterval, float DamagePerTick, FName SourceIdolID);

	/** Set callback to apply one tick of DOT damage (Combat sets this so RunState stays agnostic). Signature: (Target, DamageAmount, SourceIdolID). */
	void SetDOTDamageApplier(TFunction<void(AActor*, int32, FName)> InApplier);

	/** Tick DOTs (called by timer). Applies tick damage via delegate and removes expired. */
	void TickDOT();

	// ============================================================
	// Idol stock (shop-style altar: 3 offered idols + reroll)
	// ============================================================

	static constexpr int32 IdolStockSlotCount = UT66IdolManagerSubsystem::IdolStockSlotCount;

	/** Ensure idol stock has been generated for the current stage. */
	void EnsureIdolStock();

	/** Reroll the idol stock (regenerate 3 random idols). */
	void RerollIdolStock();

	/** Returns the current idol stock IDs (3 slots). */
	const TArray<FName>& GetIdolStockIDs() const;

	/** Returns the current idol stock tier value (0 empty, 1 black .. 4 white). */
	int32 GetIdolStockTierValue(int32 SlotIndex) const;

	/** Returns the current idol stock rarity. Empty/invalid slots fall back to Black. */
	ET66ItemRarity GetIdolStockRarityInSlot(int32 SlotIndex) const;

	/** Select (equip/level-up) the idol at the given stock slot. Returns true on success. */
	bool SelectIdolFromStock(int32 SlotIndex);

	/** Whether a stock slot has already been selected this visit. */
	bool IsIdolStockSlotSelected(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FString>& GetEventLog() const { return EventLog; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool GetHUDPanelsVisible() const { return bHUDPanelsVisible; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool GetStageTimerActive() const { return bStageTimerActive; }

	/** Stage timer duration in seconds (countdown from this; frozen until start gate). */
	static constexpr float StageTimerDurationSeconds = 600.f; // 10:00

	/** Seconds remaining on stage timer. When inactive, stays at full duration (frozen). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetStageTimerSecondsRemaining() const { return StageTimerSecondsRemaining; }

	/**
	 * Speedrun elapsed time for the live stage segment.
	 * This is now independently startable so GameMode can begin timing at stage entry rather than
	 * coupling it to stage-timer activation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetSpeedRunElapsedSeconds() const { return SpeedRunElapsedSeconds; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|SpeedRun")
	bool IsSpeedRunTimerActive() const { return bSpeedRunStarted; }

	/** Full active time in seconds across the current difficulty's cleared stages plus the current stage. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetCurrentRunElapsedSeconds() const;

	/** Per-stage cumulative pacing checkpoints for the current difficulty segment. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|SpeedRun")
	const TArray<FT66StagePacingPoint>& GetStagePacingPoints() const { return StagePacingPoints; }

	/** Final cached run duration in seconds once the run ends. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetFinalRunElapsedSeconds() const { return bRunEnded ? FinalRunElapsedSeconds : GetCurrentRunElapsedSeconds(); }

	/** True once the run has ended and the run summary path has been entered. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool HasRunEnded() const { return bRunEnded; }

	/** True when the run ended because the selected difficulty was fully cleared. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool DidRunEndInVictory() const { return bRunEnded && bRunEndedAsVictory; }

	/** Reserved for future backend-backed stage PBs; currently false because stage pacing is summary-only. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|SpeedRun")
	bool DidThisRunSetNewPersonalBestTime() const { return bThisRunSetNewPersonalBestSpeedRunTime; }

	/** Call every frame from GameMode Tick to count down when timer is active. */
	void TickStageTimer(float DeltaTime);

	/** Call every frame from GameMode Tick to update speedrun timer. */
	void TickSpeedRunTimer(float DeltaTime);

	/** Explicitly starts or resumes the stage-segment speedrun timer. */
	UFUNCTION(BlueprintCallable, Category = "RunState|SpeedRun")
	void StartSpeedRunTimer(bool bResetElapsed = true);

	/** Stops the speedrun timer while optionally preserving the elapsed value for UI/readback. */
	UFUNCTION(BlueprintCallable, Category = "RunState|SpeedRun")
	void StopSpeedRunTimer(bool bKeepElapsed = true);

	/** Resets the speedrun timer to 0 and leaves it inactive until StartSpeedRunTimer is called. */
	UFUNCTION(BlueprintCallable, Category = "RunState|SpeedRun")
	void ResetSpeedRunTimer();

	/** Call every frame from GameMode Tick to tick ultimate + last-stand timers. */
	void TickHeroTimers(float DeltaTime);

	/** Mark the run as ended and cache the final active run duration. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void MarkRunEnded(bool bWasFullClear);

	/** Export the resumable subset of run state used by chained-difficulty save and quit. */
	void ExportSavedRunSnapshot(FT66SavedRunSnapshot& OutSnapshot) const;

	/** Restore a previously exported chained-difficulty save snapshot. */
	void ImportSavedRunSnapshot(const FT66SavedRunSnapshot& Snapshot);

	/** Reset timer to full duration and freeze (e.g. when entering next stage so start gate starts it again). */
	void ResetStageTimerToFull();

	/** Clear the current difficulty's cumulative timer + pacing checkpoints before starting the next difficulty. */
	void ResetDifficultyPacing();

	/** Clear the current difficulty segment's score without wiping the rest of the run build/state. */
	void ResetDifficultyScore();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentScore() const { return CurrentScore; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool GetBossActive() const { return bBossActive; }

	/** Active boss identifier (NAME_None when no boss is active). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	FName GetActiveBossID() const { return ActiveBossID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetBossCurrentHP() const { return BossCurrentHP; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetBossMaxHP() const { return BossMaxHP; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FT66BossPartSnapshot>& GetBossPartSnapshots() const { return BossPartSnapshots; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FRunEvent>& GetStructuredEventLog() const { return StructuredEventLog; }

	/** Set current stage (1–23). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void SetCurrentStage(int32 Stage);

	/** Called when a boss awakens. Sets boss UI visible and initializes HP. */
	void SetBossActive(int32 InMaxHP);

	/** Called when a boss awakens. Sets boss UI visible, stores boss ID, and initializes HP. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void SetBossActiveWithId(FName InBossID, int32 InMaxHP);

	/** Called when a multipart boss awakens or its part state changes. */
	void SetBossActiveWithParts(FName InBossID, const TArray<FT66BossPartSnapshot>& InBossParts);

	/** Difficulty scaling: update boss max HP while preserving current HP percent. */
	void RescaleBossMaxHPPreservePercent(int32 NewMaxHP);

	/** Hide boss UI / clear boss HP (e.g. on boss death or stage transition). */
	void SetBossInactive();

	/** Apply damage to boss; returns true if boss died (HP reached 0). */
	bool ApplyBossDamage(int32 Damage);

	/** Reset boss state for new stage (frozen timer still applies separately). */
	void ResetBossState();

	/** Start/stop stage timer (e.g. when crossing gate). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void SetStageTimerActive(bool bActive);

	/** Add to score (e.g. on enemy kill). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddScore(int32 Points);

	/** Add structured event for provenance (also appends string to EventLog for UI). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddStructuredEvent(ET66RunEventType EventType, const FString& Payload);

	/** Apply numerical damage (HP). Optional Attacker: if set and reflect > 0, reflect fraction back and optionally Crush (OHKO). Returns true if damage was applied (not blocked by i-frames). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool ApplyDamage(int32 DamageHP, AActor* Attacker = nullptr);

	/** Heal to full HP. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void HealToFull();

	/** Heal HP (clamped by MaxHP). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void HealHP(float Amount);

	/** Heal HP and attribute the applied amount to the current companion's lifetime total. */
	void HealHPFromCompanion(float Amount);

	/** Legacy: heal hearts (1 heart = 20 HP). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void HealHearts(int32 Hearts);

	/** Apply HP regen (call from Tick; DeltaTime in seconds). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ApplyHpRegen(float DeltaTime);

	/** Instantly kill the player (bypasses i-frames). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void KillPlayer();

	/** Add an item by template ID with auto-generated rarity and rolled value. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddItem(FName ItemID);

	/** Add an item by template ID with an explicit rarity and auto-generated rolled value. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddItemWithRarity(FName ItemID, ET66ItemRarity Rarity);

	/** Add a fully specified item slot (template + rarity + rolled value). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddItemSlot(const FT66InventorySlot& Slot);

	/** Activates the Gambler's Token for the current run (1..5 => 50%..90% sell value). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Items")
	void ApplyGamblersTokenPickup(int32 TokenLevel);

	/** Active Gambler's Token level for this run (0 = inactive). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	int32 GetActiveGamblersTokenLevel() const { return ActiveGamblersTokenLevel; }

	/** Current sell fraction for regular items in this run (0.4 base, up to 1.0 with the token). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Economy")
	float GetCurrentSellFraction() const;

	/** Sell value for a specific inventory slot under the current run modifiers. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Economy")
	int32 GetSellGoldForInventorySlot(const FT66InventorySlot& Slot) const;

	static constexpr int32 MaxGamblersTokenLevel = 5;

	/** Clear inventory only (e.g. Lab "Reset Items"). Recomputes stats and broadcasts. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ClearInventory();

	/** Sell first inventory item. Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SellFirstItem();

	/** Sell a specific inventory item slot (0..InventorySlots.Num-1). Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SellInventoryItemAt(int32 InventoryIndex);

	static constexpr int32 AlchemyCopiesRequired = 3;

	/** True if the item is a valid alchemy target and enough matching copies exist to fuse it. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Casino|Alchemy")
	bool CanAlchemyUpgradeInventoryItemAt(int32 InventoryIndex) const;

	/** Count matching inventory copies usable for alchemy (same item template and rarity). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Casino|Alchemy")
	int32 GetAlchemyMatchingInventoryCount(int32 InventoryIndex) const;

	/** Preview the fused result for a target item. Returns true only when enough matching copies exist. */
	bool GetAlchemyUpgradePreviewAt(int32 InventoryIndex, FT66InventorySlot& OutUpgradedSlot, int32& OutMatchingCount) const;

	/** Consume three matching copies to increase an item by one rarity step. SacrificeIndex is ignored for compatibility. */
	bool TryAlchemyUpgradeInventoryItems(int32 TargetIndex, int32 SacrificeIndex, FT66InventorySlot& OutUpgradedSlot);

	static ET66ItemRarity GetNextItemRarity(ET66ItemRarity Rarity);

	/** True if there is space in the inventory (max 20). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	bool HasInventorySpace() const { return true; }

	// ============================================
	// Hero Level / Stats
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero")
	int32 GetHeroLevel() const { return HeroLevel; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero")
	int32 GetHeroXP() const { return HeroXP; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero")
	int32 GetHeroXPToNextLevel() const { return XPToNextLevel; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero")
	float GetHeroXP01() const { return (XPToNextLevel <= 0) ? 0.f : FMath::Clamp(static_cast<float>(HeroXP) / static_cast<float>(XPToNextLevel), 0.f, 1.f); }

	/** Add XP and auto-level (v0: 100 XP per level). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Hero")
	void AddHeroXP(int32 Amount);

	/** Foundational stat points (Damage/Attack Speed/Attack Scale/Accuracy/Armor/Evasion/Luck + Speed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetSpeedStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetDamageStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetAttackSpeedStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetScaleStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetAccuracyStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetArmorStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetEvasionStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetLuckStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetSeedLuck0To100() const { return (SeedLuck0To100 >= 0) ? FMath::Clamp(SeedLuck0To100, 0, 100) : 50; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	float GetTotalLuckModifierPercent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	float GetEffectiveLuckValue() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetEffectiveLuckBiasStat() const;

	static FText GetSeedLuckAdjectiveText(int32 InSeedLuck0To100);

	// ============================================
	// Category-Specific Stats (base from hero DataTable + item bonuses)
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|Pierce")
	int32 GetPierceDmgStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|Pierce")
	int32 GetPierceAtkSpdStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|Pierce")
	int32 GetPierceAtkScaleStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|Bounce")
	int32 GetBounceDmgStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|Bounce")
	int32 GetBounceAtkSpdStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|Bounce")
	int32 GetBounceAtkScaleStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|AOE")
	int32 GetAoeDmgStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|AOE")
	int32 GetAoeAtkSpdStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|AOE")
	int32 GetAoeAtkScaleStat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|DOT")
	int32 GetDotDmgStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|DOT")
	int32 GetDotAtkSpdStat() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats|DOT")
	int32 GetDotAtkScaleStat() const;

	// ============================================
	// Secondary Stats (from items Line 2 * hero base)
	// ============================================

	/** Get the effective value of a secondary stat (hero base * accumulated item Line 2 multipliers). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetSecondaryStatValue(ET66SecondaryStatType StatType) const;

	/** Get the raw hero-base value of a secondary stat before primary-stat scaling and Line 2 multipliers. */
	float GetSecondaryStatBaselineValue(ET66SecondaryStatType StatType) const;

	/** Aggro multiplier (base * taunt items). Higher = enemies target this hero more. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetAggroMultiplier() const;

	/** HP regen per second (base * items). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetHpRegenPerSecond() const;

	/** Movement speed secondary multiplier (1.0 = no bonus; items can add). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetMovementSpeedSecondaryMultiplier() const;

	/** Crate reward rarity multiplier (1.0 = no bonus; higher shifts crate rewards upward). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetLootCrateRewardMultiplier() const;

	/** Lucky extra-upgrade chance when using the Alchemy tab (0..1). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetAlchemyLuckyUpgradeChance01() const;

	/** Crit chance (0..1). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetCritChance01() const;

	/** Crit damage multiplier (e.g. 1.5 = 50% bonus). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetCritDamageMultiplier() const;

	/** Life steal fraction (0..1). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetLifeStealFraction() const;

	/** Reflect damage fraction (0..1). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetReflectDamageFraction() const;

	/** Crush OHKO chance (0..1). Only rolls when reflect fires. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetCrushChance01() const;

	/** Assassinate OHKO chance (0..1). Only rolls when dodge succeeds. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetAssassinateChance01() const;

	/** Invisibility (confusion) proc chance on hit (0..1). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetInvisibilityChance01() const;

	/** Counter attack damage fraction on dodge. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetCounterAttackFraction() const;

	/** Close range bonus threshold = 10% of attack range (UU). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetCloseRangeThreshold() const;

	/** Long range bonus threshold = 90% of attack range (UU). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetLongRangeThreshold() const;

	/** Close range damage multiplier. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetCloseRangeDamageMultiplier() const;

	/** Long range damage multiplier. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetLongRangeDamageMultiplier() const;

	// ============================================
	// Power Crystals (earned from bosses; credited to the wallet when the live run summary opens)
	// ============================================

	/** Power Crystals earned in the current run segment from boss kills. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|PowerUp")
	int32 GetPowerCrystalsEarnedThisRun() const { return PowerCrystalsEarnedThisRun; }

	UFUNCTION(BlueprintCallable, Category = "RunState|PowerUp")
	void AddPowerCrystalsEarnedThisRun(int32 Amount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|PowerUp")
	int32 GetPendingPowerCrystalsForWallet() const { return FMath::Max(0, PowerCrystalsEarnedThisRun - PowerCrystalsGrantedToWalletThisRun); }

	UFUNCTION(BlueprintCallable, Category = "RunState|PowerUp")
	void MarkPendingPowerCrystalsGrantedToWallet();

	UFUNCTION(BlueprintCallable, Category = "RunState|PowerUp")
	void MarkPendingPowerCrystalsSuppressedForWallet();

	UFUNCTION(BlueprintCallable, Category = "RunState|PowerUp")
	bool ShouldSuppressPendingPowerCrystalsForWallet();

	UFUNCTION(BlueprintCallable, Category = "RunState|PowerUp")
	void ActivatePendingSingleUseBuffsForRunStart();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Finale")
	bool HasPendingDifficultyClearSummary() const { return bPendingDifficultyClearSummary; }

	UFUNCTION(BlueprintCallable, Category = "RunState|Finale")
	void SetPendingDifficultyClearSummary(bool bPending);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Finale")
	bool IsSaintBlessingActive() const { return bSaintBlessingActive; }

	UFUNCTION(BlueprintCallable, Category = "RunState|Finale")
	void SetSaintBlessingActive(bool bActive);

	/** Temporarily upgrades the current inventory + equipped idols to White rarity until EndSaintBlessingEmpowerment is called. */
	void BeginSaintBlessingEmpowerment();

	/** Restores the inventory + equipped idols that were active before BeginSaintBlessingEmpowerment. */
	void EndSaintBlessingEmpowerment();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Finale")
	float GetFinalSurvivalEnemyScalar() const { return FMath::Max(1.f, FinalSurvivalEnemyScalar); }

	UFUNCTION(BlueprintCallable, Category = "RunState|Finale")
	void SetFinalSurvivalEnemyScalar(float Scalar);

	// ============================================
	// Luck Rating (aggregated Luck/RNG outcomes)
	// ============================================

	/** Record a luck-affected quantity roll outcome, normalized within [Min..Max]. */
	void RecordLuckQuantityRoll(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue, int32 RunDrawIndex = INDEX_NONE, int32 PreDrawSeed = 0);

	/** Record a float-biased luck roll that was rounded into an integer reward. */
	void RecordLuckQuantityFloatRollRounded(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue, float ReplayMinValue, float ReplayMaxValue, int32 RunDrawIndex = INDEX_NONE, int32 PreDrawSeed = 0);

	/** Record a luck-affected boolean outcome (false=0, true=1). */
	void RecordLuckQuantityBool(FName Category, bool bSucceeded, float ExpectedChance01 = -1.f, int32 RunDrawIndex = INDEX_NONE, int32 PreDrawSeed = 0);

	/** Record a luck-affected quality outcome (rarity tier). */
	void RecordLuckQualityRarity(FName Category, ET66Rarity Rarity, int32 RunDrawIndex = INDEX_NONE, int32 PreDrawSeed = 0, const FT66RarityWeights* ReplayWeights = nullptr);

	/** Final run Luck Rating (0..100), derived from recorded quantity + quality outcomes. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckRating0To100() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckRatingQuantity0To100() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckRatingQuality0To100() const;

	/** Total quantity-roll samples contributing to the current Luck Rating. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckQuantitySampleCount() const;

	/** Total quality-roll samples contributing to the current Luck Rating. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckQualitySampleCount() const;

	/** Category-level quantity accumulators used for backend anti-cheat review. */
	void GetLuckQuantityAccumulators(TArray<FT66SavedLuckAccumulator>& OutAccumulators) const;

	/** Category-level quality accumulators used for backend anti-cheat review. */
	void GetLuckQualityAccumulators(TArray<FT66SavedLuckAccumulator>& OutAccumulators) const;

	/** Per-roll luck telemetry used for post-run anti-cheat review. */
	void GetAntiCheatLuckEvents(TArray<FT66AntiCheatLuckEvent>& OutEvents) const;

	/** Per-hit dodge/evasion telemetry used for post-run anti-cheat review. */
	void GetAntiCheatHitCheckEvents(TArray<FT66AntiCheatHitCheckEvent>& OutEvents) const;

	/** Aggregate evasion-bucket telemetry used when sampled hit checks are truncated. */
	void GetAntiCheatEvasionBuckets(TArray<FT66AntiCheatEvasionBucketSummary>& OutBuckets) const;

	/** Aggregate 5-second combat-pressure summary used for post-run anti-cheat review. */
	void GetAntiCheatPressureWindowSummary(FT66AntiCheatPressureWindowSummary& OutSummary) const;

	/** Per-game gambler summary totals used for post-run anti-cheat review. */
	void GetAntiCheatGamblerSummaries(TArray<FT66AntiCheatGamblerGameSummary>& OutSummaries) const;

	/** Per-round gambler telemetry used for post-run anti-cheat review. */
	void GetAntiCheatGamblerEvents(TArray<FT66AntiCheatGamblerEvent>& OutEvents) const;

	bool AreAntiCheatLuckEventsTruncated() const { return bAntiCheatLuckEventsTruncated; }
	bool AreAntiCheatHitCheckEventsTruncated() const { return bAntiCheatHitCheckEventsTruncated; }
	bool AreAntiCheatGamblerEventsTruncated() const { return bAntiCheatGamblerEventsTruncated; }

	/** Number of incoming hit checks that evaluated dodge/evasion this run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetIncomingHitCheckCount() const { return AntiCheatIncomingHitChecks; }

	/** Number of hit checks that actually applied damage this run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetDamageTakenHitCount() const { return AntiCheatDamageTakenHitCount; }

	/** Number of successful dodges this run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetDodgeCount() const { return AntiCheatDodgeCount; }

	/** Longest observed consecutive dodge streak this run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetMaxConsecutiveDodges() const { return AntiCheatMaxConsecutiveDodges; }

	/** Sum of dodge chances sampled across incoming hit checks this run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	float GetTotalEvasionChanceAccumulated() const { return AntiCheatTotalEvasionChance; }

	// ============================================
	// Tutorial (HUD hint + first-time onboarding)
	// ============================================

	/** Set the on-screen tutorial hint (shown above crosshair). */
	void SetTutorialHint(const FText& InLine1, const FText& InLine2);

	/** Clear the on-screen tutorial hint. */
	void ClearTutorialHint();

	FText GetTutorialHintLine1() const { return TutorialHintLine1; }
	FText GetTutorialHintLine2() const { return TutorialHintLine2; }
	bool IsTutorialHintVisible() const { return bTutorialHintVisible; }

	/** Set the tutorial subtitle line (guide dialogue). */
	void SetTutorialSubtitle(const FText& InSpeaker, const FText& InText);

	/** Clear the tutorial subtitle line. */
	void ClearTutorialSubtitle();

	FText GetTutorialSubtitleSpeaker() const { return TutorialSubtitleSpeaker; }
	FText GetTutorialSubtitleText() const { return TutorialSubtitleText; }
	bool IsTutorialSubtitleVisible() const { return bTutorialSubtitleVisible; }

	/** Mark that movement input has been used at least once (for tutorial flow). */
	void NotifyTutorialMoveInput();

	/** Mark that jump input has been used at least once (for tutorial flow). */
	void NotifyTutorialJumpInput();

	/** Mark that camera look input has been used at least once (for tutorial flow). */
	void NotifyTutorialLookInput();

	/** Mark that the player has manually locked a target at least once. */
	void NotifyTutorialAttackLockInput();

	/** Mark that the player has activated their ultimate at least once. */
	void NotifyTutorialUltimateUsed();

	bool HasSeenTutorialMoveInput() const { return bTutorialMoveInputSeen; }
	bool HasSeenTutorialJumpInput() const { return bTutorialJumpInputSeen; }
	bool HasSeenTutorialLookInput() const { return bTutorialLookInputSeen; }
	bool HasSeenTutorialAttackLockInput() const { return bTutorialAttackLockInputSeen; }
	bool HasSeenTutorialUltimateUsed() const { return bTutorialUltimateUsedSeen; }

	/** Reset tutorial input flags (used when beginning tutorial). */
	void ResetTutorialInputFlags();

	/** Derived multipliers from hero stats (separate from item multipliers). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroMoveSpeedMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroDamageMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroAttackSpeedMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroScaleMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroAccuracyMultiplier() const;

	/** Damage reduction percent (0..1) applied to incoming hits (v0: 1% per level). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetArmorReduction01() const;

	/** Dodge chance percent (0..1) (v0: 1% per level). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetEvasionChance01() const;

	/** Chance for untargeted auto-attacks to prefer enemy head hit zones. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetAccuracyChance01() const;

	// ============================================
	// Vendor (Shop + Anger + Stealing)
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	int32 GetVendorAngerGold() const { return VendorAngerGold; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	float GetVendorAnger01() const { return FMath::Clamp(static_cast<float>(VendorAngerGold) / static_cast<float>(VendorAngerThresholdGold), 0.f, 1.f); }

	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	void ResetVendorForStage();

	/** Reset only vendor anger (used when VendorBoss is defeated). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	void ResetVendorAnger();

	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	void EnsureVendorStockForCurrentStage();

	/** Force a reroll of the current stage's vendor stock (resets sold flags). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	void RerollVendorStockForCurrentStage();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	const TArray<FName>& GetVendorStockItemIDs() const { return VendorStockItemIDs; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	const TArray<FT66InventorySlot>& GetVendorStockSlots() const { return VendorStockSlots; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	bool IsVendorStockSlotSold(int32 Index) const;

	/** Attempt to buy a vendor slot; returns true if purchased. */
	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	bool TryBuyVendorStockSlot(int32 Index);

	/** Attempt to steal a vendor slot (grants item if success). Always increases anger based on attempt outcome. */
	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	bool ResolveVendorStealAttempt(int32 Index, bool bTimingHit, bool bRngSuccess);

	ET66VendorStealOutcome GetLastVendorStealOutcome() const { return LastVendorStealOutcome; }

	/** True if vendor anger has reached threshold and a boss should spawn. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	bool IsVendorAngryEnoughToBoss() const { return VendorAngerGold >= VendorAngerThresholdGold; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	bool HasBoughtFromVendorThisStage() const { return bVendorBoughtSomethingThisStage; }

	// ============================================
	// Buyback (Vendor + Gambler: items sold this run, repurchase at sell price)
	// ============================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuybackChanged);
	UPROPERTY(BlueprintAssignable, Category = "RunState|Buyback")
	FOnBuybackChanged BuybackChanged;

	/** Refresh the buyback display from the pool (call when opening buyback tab or after buyback/reroll). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Buyback")
	void GenerateBuybackDisplay();

	/** Cycle to the next page of buyback items (same layout as shop + reroll). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Buyback")
	void RerollBuybackDisplay();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Buyback")
	int32 GetBuybackPoolSize() const { return BuybackPool.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Buyback")
	const TArray<FT66InventorySlot>& GetBuybackDisplaySlots() const { return BuybackDisplaySlots; }

	/** Attempt to buy back a display slot. Price = sell price of that item. Returns true if purchased. */
	UFUNCTION(BlueprintCallable, Category = "RunState|Buyback")
	bool TryBuybackSlot(int32 DisplayIndex);

	// ============================================
	// Stage Catch Up (Difficulty start)
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|StageCatchUp")
	bool IsInStageCatchUp() const { return bInStageCatchUp; }

	UFUNCTION(BlueprintCallable, Category = "RunState|StageCatchUp")
	void SetInStageCatchUp(bool bInCatchUp);

	// ============================================
	// Ultimate
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Ultimate")
	bool IsUltimateReady() const { return UltimateCooldownRemainingSeconds <= 0.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Ultimate")
	float GetUltimateCooldownRemainingSeconds() const { return FMath::Max(0.f, UltimateCooldownRemainingSeconds); }

	/** Consume ultimate if ready; returns true if it was activated. */
	UFUNCTION(BlueprintCallable, Category = "RunState|Hero|Ultimate")
	bool TryActivateUltimate();

	// ============================================
	// Hero Passive
	// ============================================

	ET66PassiveType GetPassiveType() const { return PassiveType; }

	/** Call when an enemy is killed by hero damage (for Rallying Blow / ChaosTheory). */
	void NotifyEnemyKilledByHero();

	/** Rallying Blow: multiplier for attack speed (1.0 + 0.15 * stacks, max 3 stacks, 3s duration). */
	float GetRallyAttackSpeedMultiplier() const;

	/** Rallying Blow: current stack count (0..3). For HUD stack badge. */
	int32 GetRallyStacks() const { return RallyStacks; }

	/** True if the actor has at least one active DOT from RunState. */
	bool HasActiveDOT(AActor* Target) const;

	/** Toxin Stacking: damage multiplier when target has active DOT (1.15 if any, else 1.0). */
	float GetToxinStackingDamageMultiplier(AActor* Target) const;

	// --- New passive queries ---

	/** QuickDraw: returns damage multiplier for next shot (2.0 if idle for 2s+, else 1.0). */
	float GetQuickDrawDamageMultiplier() const;
	void NotifyAttackFired();

	/** Overclock: returns true if this attack should fire twice. Caller should call NotifyAttackFired(). */
	bool ShouldOverclockDouble() const;

	/** ChaosTheory: bonus bounce count from kill stacks. */
	int32 GetChaosTheoryBonusBounceCount() const;

	/** Endurance: returns atk speed mult (2.0 if below 30% HP, else 1.0). */
	float GetEnduranceAttackSpeedMultiplier() const;
	/** Endurance: returns damage mult (1.25 if below 30% HP, else 1.0). */
	float GetEnduranceDamageMultiplier() const;

	/** BrawlersFury: returns damage mult (1.3 for 3s after taking damage, else 1.0). */
	float GetBrawlersFuryDamageMultiplier() const;

	/** Unflinching: true if this hero has Unflinching passive (15% damage reduction + confusion immune). */
	bool HasUnflinching() const { return PassiveType == ET66PassiveType::Unflinching; }

	/** TreasureHunter: gold multiplier for enemy kills (1.25 if active, else 1.0). */
	float GetTreasureHunterGoldMultiplier() const;

	/** Evasive: mark that next attack should apply bonus DOT. */
	void NotifyEvasionProc();
	/** Evasive: consume and return true if next attack should apply bonus DOT. */
	bool ConsumeEvasiveBonusDOT();

	/** Frostbite: true if this hero has Frostbite passive (DOT slows enemies). */
	bool HasFrostbite() const { return PassiveType == ET66PassiveType::Frostbite; }

	// ============================================
	// Survival / Last Stand
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Survival")
	float GetSurvivalCharge01() const { return SurvivalCharge01; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Survival")
	bool IsInLastStand() const { return bInLastStand; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Survival")
	float GetLastStandSecondsRemaining() const { return FMath::Max(0.f, LastStandSecondsRemaining); }

	/** Additional multiplier during last stand (v0: 2x speed + 2x attack speed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Survival")
	float GetLastStandMoveSpeedMultiplier() const { return bInLastStand ? 2.f : 1.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Survival")
	float GetLastStandAttackSpeedMultiplier() const { return bInLastStand ? 2.f : 1.f; }

	// ============================================
	// Quick Revive
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|QuickRevive")
	bool HasQuickReviveCharge() const { return bQuickReviveChargeReady; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|QuickRevive")
	bool IsInQuickReviveDownedState() const { return bInQuickReviveDowned; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|QuickRevive")
	float GetQuickReviveDownedSecondsRemaining() const { return FMath::Max(0.f, QuickReviveDownedSecondsRemaining); }

	UFUNCTION(BlueprintCallable, Category = "RunState|Hero|QuickRevive")
	bool GrantQuickReviveCharge();

	UFUNCTION(BlueprintCallable, Category = "RunState|Hero|QuickRevive")
	void ClearQuickReviveCharge();

	// ============================================
	// Stage effects (ground tiles)
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|StageEffects")
	float GetStageMoveSpeedMultiplier() const { return StageMoveSpeedMultiplier; }

	/** Temporary speed boost from stage tiles. */
	UFUNCTION(BlueprintCallable, Category = "RunState|StageEffects")
	void ApplyStageSpeedBoost(float MoveSpeedMultiplier, float DurationSeconds);

	// ============================================
	// Hero status effects (Unique enemy debuffs)
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Status")
	bool HasStatusBurn() const { return StatusBurnSecondsRemaining > 0.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Status")
	bool HasStatusChill() const { return StatusChillSecondsRemaining > 0.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Status")
	bool HasStatusCurse() const { return StatusCurseSecondsRemaining > 0.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Status")
	float GetStatusMoveSpeedMultiplier() const { return StatusChillSecondsRemaining > 0.f ? StatusChillMoveSpeedMultiplier : 1.f; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Status")
	float GetStatusCurseAlpha01() const { return HasStatusCurse() ? 1.f : 0.f; }

	UFUNCTION(BlueprintCallable, Category = "RunState|Status")
	void ApplyStatusBurn(float DurationSeconds, float DamagePerSecond);

	UFUNCTION(BlueprintCallable, Category = "RunState|Status")
	void ApplyStatusChill(float DurationSeconds, float MoveSpeedMultiplier);

	UFUNCTION(BlueprintCallable, Category = "RunState|Status")
	void ApplyStatusCurse(float DurationSeconds);

	/** Damage that bypasses evasion/armor (used by status effects). */
	UFUNCTION(BlueprintCallable, Category = "RunState|Status")
	bool ApplyTrueDamage(int32 DamageHP);

	/** Item-derived move speed multiplier (separate from hero level). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items|Derived")
	float GetItemMoveSpeedMultiplier() const { return ItemMoveSpeedMultiplier; }

	// ============================================
	// Items -> Derived combat tuning (event-driven)
	// ============================================

	/** Sum of item "PowerGivenPercent" across inventory. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	float GetItemPowerGivenPercent() const { return ItemPowerGivenPercent; }

	/** Multiplier applied to hero auto-attack damage. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	float GetItemDamageMultiplier() const { return ItemDamageMultiplier; }

	/** Multiplier applied to hero auto-attack rate (higher = faster). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	float GetItemAttackSpeedMultiplier() const { return ItemAttackSpeedMultiplier; }

	/** Multiplier applied to dash cooldown (lower = more frequent dash). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	float GetDashCooldownMultiplier() const { return DashCooldownMultiplier; }

	/** Multiplier applied to projectile visual scale (cosmetic). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	float GetItemScaleMultiplier() const { return ItemScaleMultiplier; }

	/** Hero base attack range from DataTable (before Scale multipliers). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero")
	float GetHeroBaseAttackRange() const { return HeroBaseAttackRange; }

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ResetForNewRun();

	FName ConsumeDeferredRunStartItemId();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ToggleHUDPanels();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddLogEntry(const FString& Entry);

	void RecordAntiCheatGamblerRound(
		ET66AntiCheatGamblerGameType GameType,
		int32 BetGold,
		int32 PayoutGold,
		bool bCheatAttempted,
		bool bCheatSucceeded,
		bool bWin,
		bool bDraw,
		int32 PlayerChoice = INDEX_NONE,
		int32 OpponentChoice = INDEX_NONE,
		int32 OutcomeValue = 0,
		int32 OutcomeSecondaryValue = 0,
		int32 SelectedMask = 0,
		int32 ResolvedMask = 0,
		int32 PathBits = 0,
		int32 ShufflePreDrawSeed = 0,
		int32 ShuffleStartDrawIndex = INDEX_NONE,
		int32 OutcomePreDrawSeed = 0,
		int32 OutcomeDrawIndex = INDEX_NONE,
		float OutcomeExpectedChance01 = -1.f,
		const FString& ActionSequence = FString());

private:
	struct FT66LuckAccumulator
	{
		double Sum01 = 0.0;
		int32 Count = 0;

		void Add01(float V01)
		{
			Sum01 += static_cast<double>(FMath::Clamp(V01, 0.f, 1.f));
			// Note: keep this UHT-friendly (no digit separators).
			Count = FMath::Clamp(Count + 1, 0, 1000000);
		}

		float Avg01() const
		{
			return (Count > 0) ? static_cast<float>(Sum01 / static_cast<double>(Count)) : 0.5f;
		}
	};

	float ComputeLuckRatingQuantity01() const;
	float ComputeLuckRatingQuality01() const;
	float GetRunElapsedSecondsForAntiCheatEvent() const;
	void RecordAntiCheatLuckEvent(ET66AntiCheatLuckEventType EventType, FName Category, float Value01, int32 RawValue, int32 RawMin, int32 RawMax, int32 RunDrawIndex = INDEX_NONE, int32 PreDrawSeed = 0, float ExpectedChance01 = -1.f, const FT66RarityWeights* ReplayWeights = nullptr, const FT66FloatRange* ReplayFloatRange = nullptr);
	void RecordAntiCheatHitCheckEvent(float EvasionChance01, bool bDodged, bool bDamageApplied);
	FT66AntiCheatGamblerGameSummary& FindOrAddAntiCheatGamblerSummary(ET66AntiCheatGamblerGameType GameType);
	void InitializeAntiCheatEvasionBuckets();
	static int32 GetAntiCheatEvasionBucketIndex(float EvasionChance01);
	static float GetAntiCheatEvasionBucketMinChance01(int32 BucketIndex);
	static float GetAntiCheatEvasionBucketMaxChance01(int32 BucketIndex);
	void ResetAntiCheatPressureTracking();
	FName DeferredRunStartItemId = NAME_None;
	void FinalizeCurrentAntiCheatPressureWindow();
	void RecordAntiCheatPressureHitCheck(float RunElapsedSeconds, float EvasionChance01, bool bDodged, bool bDamageApplied);
	FT66AntiCheatPressureWindowSummary BuildAntiCheatPressureWindowSummarySnapshot() const;

	void ResetLuckRatingTracking();
	float GetSingleUseLuckModifierPercent() const;
	UFUNCTION()
	void HandleIdolStateChanged();

	UT66IdolManagerSubsystem* GetIdolManager() const;

	void TrimLogsIfNeeded();
	bool HasStagePacingPoint(int32 Stage) const;
	void RecordStagePacingPoint(int32 Stage, float CumulativeElapsedSeconds);
	void RecomputeItemDerivedStats();
	void HandleLethalDamage(AActor* Attacker);
	void EnterLastStand();
	void EndLastStandAndDie();
	void EnterQuickReviveDowned();
	void EndQuickReviveDownedAndRevive();
	void EndQuickReviveDownedAndDie();
	void InitializeHeroStatTuningForSelectedHero();
	void InitializeHeroStatsForNewRun();
	void ApplyOneHeroLevelUp();
	void RefreshPermanentBuffBonusesFromProfile();
	static int32 WholeStatToTenths(int32 WholeValue);
	static int32 TenthsToDisplayStat(int32 ValueTenths);
	static float TenthsToFloatStat(int32 ValueTenths);
	int32 GetPrecisePrimaryStatTenths(ET66HeroStatType StatType) const;
	int32 GetItemPrimaryStatTenths(ET66HeroStatType StatType) const;
	int32 GetPermanentPrimaryBuffTenths(ET66HeroStatType StatType) const;
	int32 GetSecondaryStatBonusTenths(ET66SecondaryStatType StatType) const;
	float GetSecondaryStatBonusValue(ET66SecondaryStatType StatType) const;
	int32 GetCategoryBaseStatTenths(ET66SecondaryStatType StatType) const;
	int32 GetCategoryTotalStatTenths(ET66SecondaryStatType StatType) const;
	void SyncLegacyHeroStatsFromPrecise();
	void ClearPersistentSecondaryStatBonuses();
	void AddPersistentSecondaryStatBonusTenths(ET66SecondaryStatType StatType, int32 DeltaTenths);
	void AddItemSecondaryStatBonusTenths(ET66SecondaryStatType StatType, int32 DeltaTenths);
	int32 RollHeroPrimaryGainTenthsBiased(const FT66HeroStatGainRange& Range, FName Category);
	void ApplyPrimaryGainToSecondaryBonuses(ET66HeroStatType PrimaryStatType, int32 PrimaryGainTenths, TMap<ET66SecondaryStatType, int32>& TargetBonuses, int32 SeedSalt = 0) const;
	static bool IsBossDamageSource(const AActor* Attacker);
	static float GetHPForHeartTier(int32 Tier);
	float GetHeartSlotCapacity(int32 SlotIndex) const;
	float GetTotalHeartCapacity() const;
	void ResetHeartSlotTiers();
	void SyncMaxHPToHeartTiers();
	void RebuildHeartSlotTiersFromMaxHP();
	int32 FindUpgradeableHeartSlot() const;

	UPROPERTY()
	float CurrentHP = DefaultMaxHP;

	UPROPERTY()
	float MaxHP = DefaultMaxHP;

	UPROPERTY()
	float CompanionHealingDoneThisRun = 0.f;

	UPROPERTY()
	TArray<uint8> HeartSlotTiers;

	UPROPERTY()
	int32 CurrentGold = 0;

	UPROPERTY()
	int32 CurrentDebt = 0;

	UPROPERTY()
	int32 DifficultyTier = 0;

	/** Difficulty in skulls (integer). */
	UPROPERTY()
	int32 DifficultySkulls = 0;

	UPROPERTY()
	int32 TotemsActivatedCount = 0;

	UPROPERTY()
	float GamblerAnger01 = 0.f;

	UPROPERTY()
	TArray<FName> OwedBossIDs;

	/** Cowardice gates taken this segment (resets when ClearOwedBosses is called after the owed-boss payoff stage). */
	UPROPERTY()
	int32 CowardiceGatesTakenCount = 0;

	/** Item inventory using the new slot-based system (template + rarity + rolled value). */
	UPROPERTY()
	TArray<FT66InventorySlot> InventorySlots;

	/** Active Gambler's Token level for this run. It behaves like a special run upgrade, not a regular slot item. */
	int32 ActiveGamblersTokenLevel = 0;

	/** Active DOTs (idol): one per target+source pair so distinct idols do not overwrite each other. */
	TMap<FT66DotKey, FT66DotInstance> ActiveDOTs;

	TFunction<void(AActor*, int32, FName)> DOTDamageApplier;
	FTimerHandle DOTTimerHandle;
	static constexpr float DOTTickRateSeconds = 0.2f;

	UPROPERTY()
	TArray<FString> EventLog;

	UPROPERTY()
	TArray<FRunEvent> StructuredEventLog;

	UPROPERTY()
	TArray<FT66StagePacingPoint> StagePacingPoints;

	UPROPERTY()
	int32 CurrentStage = 1;

	UPROPERTY()
	bool bStageTimerActive = false;

	UPROPERTY()
	float StageTimerSecondsRemaining = StageTimerDurationSeconds;

	/** Last integer second we broadcasted (throttle StageTimerChanged to once per second). */
	int32 LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);

	// Speedrun timer (counts up from start gate; stage-local segment only).
	UPROPERTY()
	float SpeedRunElapsedSeconds = 0.f;

	UPROPERTY()
	float SpeedRunStartWorldSeconds = 0.f;

	UPROPERTY()
	bool bSpeedRunStarted = false;

	/** Last integer second we broadcasted (throttle SpeedRunTimerChanged to once per second). */
	int32 LastBroadcastSpeedRunSecond = -1;

	// Run Summary banner: set true if any completed stage submission set a new personal best time.
	bool bThisRunSetNewPersonalBestSpeedRunTime = false;

	/** Accumulated active time from all completed stages in the current difficulty segment. */
	UPROPERTY()
	float CompletedStageActiveSeconds = 0.f;

	/** Cached final active run duration once the run ends. */
	UPROPERTY()
	float FinalRunElapsedSeconds = 0.f;

	/** True once death/victory has finalized the run. */
	UPROPERTY()
	bool bRunEnded = false;

	/** True if the finalized run ended on a full clear instead of a death. */
	UPROPERTY()
	bool bRunEndedAsVictory = false;

	/** True when the player should be shown the between-difficulties clear summary instead of ending the run. */
	UPROPERTY()
	bool bPendingDifficultyClearSummary = false;

	/** Saint blessing used during the post-boss survival extraction. */
	UPROPERTY()
	bool bSaintBlessingActive = false;

	/** Exact inventory state before the temporary Saint blessing upgrade was applied. */
	UPROPERTY()
	TArray<FT66InventorySlot> SaintBlessingInventorySnapshot;

	/** Exact idol IDs before the temporary Saint blessing upgrade was applied. */
	UPROPERTY()
	TArray<FName> SaintBlessingEquippedIdolsSnapshot;

	/** Exact idol tiers before the temporary Saint blessing upgrade was applied. */
	UPROPERTY()
	TArray<uint8> SaintBlessingEquippedIdolTiersSnapshot;

	UPROPERTY()
	bool bSaintBlessingLoadoutSnapshotValid = false;

	/** Extra scalar applied only to the final-survival enemy escalation. */
	UPROPERTY()
	float FinalSurvivalEnemyScalar = 1.f;

	UPROPERTY()
	int32 CurrentScore = 0;

	UPROPERTY()
	bool bHUDPanelsVisible = true;

	// Tutorial hint text (HUD, above crosshair).
	UPROPERTY()
	bool bTutorialHintVisible = false;

	UPROPERTY()
	FText TutorialHintLine1;

	UPROPERTY()
	FText TutorialHintLine2;

	UPROPERTY()
	bool bTutorialSubtitleVisible = false;

	UPROPERTY()
	FText TutorialSubtitleSpeaker;

	UPROPERTY()
	FText TutorialSubtitleText;

	// Tutorial input flags (first-time onboarding).
	UPROPERTY()
	bool bTutorialMoveInputSeen = false;

	UPROPERTY()
	bool bTutorialJumpInputSeen = false;

	UPROPERTY()
	bool bTutorialLookInputSeen = false;

	UPROPERTY()
	bool bTutorialAttackLockInputSeen = false;

	UPROPERTY()
	bool bTutorialUltimateUsedSeen = false;

	// Dev toggles (debug/test).
	UPROPERTY()
	bool bDevImmortality = false;

	UPROPERTY()
	bool bDevPower = false;

	UPROPERTY()
	bool bBossActive = false;

	UPROPERTY()
	FName ActiveBossID = NAME_None;

	UPROPERTY()
	int32 BossMaxHP = 100;

	UPROPERTY()
	int32 BossCurrentHP = 0;

	UPROPERTY()
	TArray<FT66BossPartSnapshot> BossPartSnapshots;

	/** If true, spawn LoanShark when the timer starts in this stage (only when debt was carried from previous stage). */
	bool bLoanSharkPending = false;

	float LastDamageTime = -9999.f;

	int32 PowerCrystalsEarnedThisRun = 0;
	int32 PowerCrystalsGrantedToWalletThisRun = 0;
	int32 SeedLuck0To100 = -1;

	// ============================================
	// Derived combat tuning from Inventory (recomputed on InventoryChanged)
	// ============================================

	/** Flat stat bonuses from inventory items (main stat line only, v1). */
	FT66HeroStatBonuses ItemStatBonuses = FT66HeroStatBonuses{};

	/** Permanent buff bonuses from the buff subsystem, refreshed at run start. */
	FT66HeroStatBonuses PermanentBuffStatBonuses = FT66HeroStatBonuses{};

	/** Single-use secondary multipliers purchased in the shop and consumed at the start of the current run. */
	TMap<ET66SecondaryStatType, float> SingleUseSecondaryMultipliers;

	float ItemPowerGivenPercent = 0.f;
	float BonusDamagePercent = 0.f;
	float BonusAttackSpeedPercent = 0.f;
	float DashCooldownReductionPercent = 0.f;

	float ItemDamageMultiplier = 1.f;
	float ItemAttackSpeedMultiplier = 1.f;
	float DashCooldownMultiplier = 1.f;
	float ItemScaleMultiplier = 1.f;

	// Extra item-derived modifiers beyond base "Power"
	float ItemMoveSpeedMultiplier = 1.f;
	float ItemArmorBonus01 = 0.f;
	float ItemEvasionBonus01 = 0.f;
	int32 ItemBonusLuckFlat = 0;

	// ============================================
	// Hero XP / Level + timers
	// ============================================

	int32 HeroLevel = DefaultHeroLevel;
	int32 HeroXP = 0;
	int32 XPToNextLevel = DefaultXPToLevel;

	/** Legacy whole-number mirror of the precise hero stats, kept for compatibility and snapshots. */
	FT66HeroStatBlock HeroStats = FT66HeroStatBlock{};

	/** Authoritative foundational hero stats stored in fixed-point tenths. */
	FT66HeroPreciseStatBlock HeroPreciseStats = FT66HeroPreciseStatBlock{};

	/** Current hero's per-level gain ranges sourced from hero tuning. */
	FT66HeroPerLevelStatGains HeroPerLevelGains = FT66HeroPerLevelStatGains{};

	/** Category-specific base stats (loaded from Heroes DataTable, not leveled). */
	int32 BasePierceDmg = 1; int32 BasePierceAtkSpd = 1; int32 BasePierceAtkScale = 1;
	int32 BaseBounceDmg = 1; int32 BaseBounceAtkSpd = 1; int32 BaseBounceAtkScale = 1;
	int32 BaseAoeDmg = 1;    int32 BaseAoeAtkSpd = 1;    int32 BaseAoeAtkScale = 1;
	int32 BaseDotDmg = 1;    int32 BaseDotAtkSpd = 1;    int32 BaseDotAtkScale = 1;

	// ============================================
	// Hero secondary base stats (loaded from Heroes DataTable, multiplied by items)
	// ============================================
	float HeroBaseCritDamage = 1.5f;
	float HeroBaseCritChance = 0.05f;
	float HeroBaseCloseRangeDmg = 1.0f;
	float HeroBaseLongRangeDmg = 1.0f;
	float HeroBaseTaunt = 1.0f;
	float HeroBaseReflectDmg = 0.0f;
	float HeroBaseHpRegen = 0.0f;
	float HeroBaseCrushChance = 0.01f;
	float HeroBaseInvisChance = 0.01f;
	float HeroBaseCounterAttack = 0.0f;
	float HeroBaseLifeSteal = 0.0f;
	float HeroBaseAssassinateChance = 0.01f;
	float HeroBaseCheatChance = 0.05f;
	float HeroBaseStealChance = 0.05f;
	float HeroBaseAttackRange = 1000.f;
	float HeroBaseAccuracy = 0.15f;
	ET66AttackCategory HeroPrimaryAttackCategory = ET66AttackCategory::Pierce;

	// ============================================
	// Accumulated secondary stat multipliers from items (product of Line 2 multipliers)
	// Reset and recomputed in RecomputeItemDerivedStats()
	// ============================================
	TMap<ET66SecondaryStatType, float> SecondaryMultipliers;
	TMap<ET66SecondaryStatType, int32> PersistentSecondaryStatBonusTenths;
	TMap<ET66SecondaryStatType, int32> ItemSecondaryStatBonusTenths;
	FT66HeroPreciseStatBlock ItemPrimaryStatBonusesPrecise = FT66HeroPreciseStatBlock{};

	/** Run-persistent RNG for hero stat gains (so stage reloads don't reshuffle). */
	FRandomStream HeroStatRng;

	float UltimateCooldownRemainingSeconds = 0.f;
	int32 LastBroadcastUltimateSecond = 0;

	ET66PassiveType PassiveType = ET66PassiveType::None;
	int32 RallyStacks = 0;
	double RallyTimerEndWorldTime = 0.0;

	// QuickDraw
	double LastAttackFireWorldTime = -9999.0;

	// Overclock
	int32 OverclockAttackCounter = 0;

	// ChaosTheory
	int32 ChaosTheoryBounceStacks = 0;
	double ChaosTheoryTimerEndWorldTime = 0.0;

	// BrawlersFury
	double BrawlersFuryEndWorldTime = 0.0;

	// Evasive
	bool bEvasiveNextAttackBonusDOT = false;

	float SurvivalCharge01 = 0.f;
	bool bInLastStand = false;
	float LastStandSecondsRemaining = 0.f;
	int32 LastBroadcastLastStandSecond = 0;
	bool bQuickReviveChargeReady = false;
	bool bInQuickReviveDowned = false;
	float QuickReviveDownedSecondsRemaining = 0.f;
	int32 LastBroadcastQuickReviveSecond = 0;

	// ============================================
	// Vendor shop state (per-stage)
	// ============================================

	int32 VendorAngerGold = 0;
	int32 VendorStockStage = 0;
	int32 VendorStockRerollStage = 0;
	int32 VendorStockRerollCounter = 0;
	TArray<FName> VendorStockItemIDs;
	TArray<FT66InventorySlot> VendorStockSlots;
	TArray<bool> VendorStockSold;

	TArray<FT66InventorySlot> BuybackPool;
	TArray<FT66InventorySlot> BuybackDisplaySlots;
	int32 BuybackDisplayPage = 0;

	/** Smart reroll: times each item has been shown this vendor session (reset on stage change). */
	TMap<FName, int32> VendorSeenCounts;
	bool bVendorBoughtSomethingThisStage = false;
	ET66VendorStealOutcome LastVendorStealOutcome = ET66VendorStealOutcome::None;

	// Aggregated tracking for Luck Rating (per run).
	TMap<FName, FT66LuckAccumulator> LuckQuantityByCategory;
	TMap<FName, FT66LuckAccumulator> LuckQualityByCategory;
	TArray<FT66AntiCheatLuckEvent> AntiCheatLuckEvents;
	bool bAntiCheatLuckEventsTruncated = false;
	TArray<FT66AntiCheatHitCheckEvent> AntiCheatHitCheckEvents;
	bool bAntiCheatHitCheckEventsTruncated = false;
	TArray<FT66AntiCheatGamblerGameSummary> AntiCheatGamblerSummaries;
	TArray<FT66AntiCheatGamblerEvent> AntiCheatGamblerEvents;
	bool bAntiCheatGamblerEventsTruncated = false;

	// Aggregated telemetry for backend anti-cheat heuristics (per run).
	int32 AntiCheatIncomingHitChecks = 0;
	int32 AntiCheatDamageTakenHitCount = 0;
	int32 AntiCheatDodgeCount = 0;
	int32 AntiCheatCurrentConsecutiveDodges = 0;
	int32 AntiCheatMaxConsecutiveDodges = 0;
	float AntiCheatTotalEvasionChance = 0.f;
	TArray<FT66AntiCheatEvasionBucketSummary> AntiCheatEvasionBuckets;
	FT66AntiCheatPressureWindowSummary AntiCheatPressureWindowSummary;
	int32 AntiCheatCurrentPressureWindowIndex = INDEX_NONE;
	int32 AntiCheatCurrentPressureHitChecks = 0;
	int32 AntiCheatCurrentPressureDodges = 0;
	int32 AntiCheatCurrentPressureDamageApplied = 0;
	float AntiCheatCurrentPressureExpectedDodges = 0.f;

	bool bInStageCatchUp = false;

	// Stage effects
	float StageMoveSpeedMultiplier = 1.f;
	float StageMoveSpeedSecondsRemaining = 0.f;

	// Status effects
	float StatusBurnSecondsRemaining = 0.f;
	float StatusBurnDamagePerSecond = 0.f;
	float StatusBurnAccumDamage = 0.f;

	float StatusChillSecondsRemaining = 0.f;
	float StatusChillMoveSpeedMultiplier = 1.f;

	float StatusCurseSecondsRemaining = 0.f;
	float InvulnDurationSeconds = DefaultInvulnDurationSeconds;
};
