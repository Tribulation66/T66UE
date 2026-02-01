// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVendorChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatusEffectsChanged);

/**
 * Authoritative run state: health, gold, inventory, event log, HUD panel visibility.
 * Survives death; reset on Restart / Main Menu.
 */
UCLASS()
class T66_API UT66RunStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 DefaultMaxHearts = 5;
	static constexpr float DefaultInvulnDurationSeconds = 0.75f;
	static constexpr int32 DefaultStartGold = 100;
	static constexpr int32 MaxInventorySlots = 20;
	static constexpr int32 MaxEquippedIdolSlots = 6;
	static constexpr int32 DefaultHeroLevel = 1;
	static constexpr int32 DefaultXPToLevel = 100;
	static constexpr float UltimateCooldownSeconds = 30.f;
	static constexpr int32 UltimateDamage = 200;
	static constexpr float SurvivalChargePerHeart = 0.20f; // 5 hearts of damage -> full
	static constexpr float LastStandDurationSeconds = 10.f;
	static constexpr int32 VendorAngerThresholdGold = 100;
	// Safety: keep logs bounded so low-end machines never accumulate unbounded memory / UI work.
	static constexpr int32 MaxEventLogEntries = 400;
	static constexpr int32 MaxStructuredEventLogEntries = 800;

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

	/** Speedrun elapsed time (per-stage; starts when stage timer becomes active). */
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

	/** Gambler anger meter changed (0..1). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnGamblerAngerChanged GamblerAngerChanged;

	/** Equipped idols changed (slot 0..2). */
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

	/** Vendor shop / anger changed (used by Vendor overlay). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnVendorChanged VendorChanged;

	/** Hero status effects changed (burn/chill/curse). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnStatusEffectsChanged StatusEffectsChanged;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentHearts() const { return CurrentHearts; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetMaxHearts() const { return MaxHearts; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentGold() const { return CurrentGold; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentDebt() const { return CurrentDebt; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetDifficultyTier() const { return DifficultyTier; }

	/** Difficulty as "Skulls" (can be fractional, e.g. 0.5). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetDifficultySkulls() const { return DifficultySkulls; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetDifficultyMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void IncreaseDifficultyTier();

	/** Add skulls directly (can be fractional). Updates DifficultyTier cache for enemy scaling. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddDifficultySkulls(float DeltaSkulls);

	/** Increase max hearts and also grant the new hearts immediately (clamped). */
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

	/** Add anger based on bet size: +min(1, Bet/100). Returns new anger. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	float AddGamblerAngerFromBet(int32 BetGold);

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ResetGamblerAnger();

	/** Bosses skipped via Cowardice Gate (must be cleared in Coliseum before checkpoint stages). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FName>& GetOwedBossIDs() const { return OwedBossIDs; }

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddOwedBoss(FName BossID);

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void RemoveFirstOwedBoss();

	/** Clear all owed bosses (used after a Coliseum clear). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ClearOwedBosses();

	/** Add gold (e.g. gambler win). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddGold(int32 Amount);

	/** Spend gold if possible (used by in-run gambling/shop costs). Returns true if spent. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool TrySpendGold(int32 Amount);

	/** Borrow gold: increases gold AND debt by Amount (no limit for now). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void BorrowGold(int32 Amount);

	/** Pay down debt using current gold. Pays up to Amount (clamped by gold and debt). Returns amount actually paid. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	int32 PayDebt(int32 Amount);

	/** Mark whether a LoanShark should spawn this stage once the timer starts (set when leaving previous stage with debt). */
	void SetLoanSharkPending(bool bPending) { bLoanSharkPending = bPending; }
	bool GetLoanSharkPending() const { return bLoanSharkPending; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FName>& GetInventory() const { return Inventory; }

	/** Equipped idol slots (size=6). NAME_None means empty slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FName>& GetEquippedIdols() const { return EquippedIdolIDs; }

	/** Equip an idol into a specific slot (0..5). Returns true if changed. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool EquipIdolInSlot(int32 SlotIndex, FName IdolID);

	/** Equip an idol into the first empty slot. Returns true if equipped. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool EquipIdolFirstEmpty(FName IdolID);

	/** Clear all equipped idols. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ClearEquippedIdols();

	/** List of all idol IDs (authoritative roster for the altar UI). */
	static const TArray<FName>& GetAllIdolIDs();

	/** Default tint color for an idol (used for UI + enemy status visuals). */
	static FLinearColor GetIdolColor(FName IdolID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FString>& GetEventLog() const { return EventLog; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool GetHUDPanelsVisible() const { return bHUDPanelsVisible; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool GetStageTimerActive() const { return bStageTimerActive; }

	/** Stage timer duration in seconds (countdown from this; frozen until start gate). */
	static constexpr float StageTimerDurationSeconds = 360.f; // 6:00

	/** Seconds remaining on stage timer. When inactive, stays at full duration (frozen). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetStageTimerSecondsRemaining() const { return StageTimerSecondsRemaining; }

	/**
	 * Speedrun elapsed time (per-stage).
	 * Starts counting when the stage timer becomes active (i.e., after leaving the start area / crossing the start gate).
	 * Resets to 0 when the stage timer is reset/frozen for the next stage.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetSpeedRunElapsedSeconds() const { return SpeedRunElapsedSeconds; }

	/** Call every frame from GameMode Tick to count down when timer is active. */
	void TickStageTimer(float DeltaTime);

	/** Call every frame from GameMode Tick to update speedrun timer. */
	void TickSpeedRunTimer(float DeltaTime);

	/** Call every frame from GameMode Tick to tick ultimate + last-stand timers. */
	void TickHeroTimers(float DeltaTime);

	/** Reset timer to full duration and freeze (e.g. when entering next stage so start gate starts it again). */
	void ResetStageTimerToFull();

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
	const TArray<FRunEvent>& GetStructuredEventLog() const { return StructuredEventLog; }

	/** Set current stage (1–66). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void SetCurrentStage(int32 Stage);

	/** Called when a boss awakens. Sets boss UI visible and initializes HP. */
	void SetBossActive(int32 InMaxHP);

	/** Called when a boss awakens. Sets boss UI visible, stores boss ID, and initializes HP. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void SetBossActiveWithId(FName InBossID, int32 InMaxHP);

	/** Hide boss UI / clear boss HP (e.g. on boss death or stage transition). */
	void SetBossInactive();

	/** Apply damage to boss; returns true if boss died (HP reached 0). */
	bool ApplyBossDamage(int32 Damage);

	/** Reset boss state for new stage (frozen timer still applies separately). */
	void ResetBossState();

	/** Start/stop stage timer (e.g. when crossing gate). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void SetStageTimerActive(bool bActive);

	/** Add to Bounty score (e.g. on enemy kill). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddScore(int32 Points);

	/** Add structured event for provenance (also appends string to EventLog for UI). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddStructuredEvent(ET66RunEventType EventType, const FString& Payload);

	/** Apply damage (1 heart). Returns true if damage was applied (not blocked by i-frames). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool ApplyDamage(int32 Hearts = 1);

	/** Heal to full hearts. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void HealToFull();

	/** Heal a number of hearts (clamped by MaxHearts). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void HealHearts(int32 Hearts);

	/** Instantly kill the player (bypasses i-frames). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void KillPlayer();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddItem(FName ItemID);

	/** Sell first inventory item. Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SellFirstItem();

	/** Sell a specific inventory item slot (0..Inventory.Num-1). Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SellInventoryItemAt(int32 InventoryIndex);

	/** True if there is space in the inventory (max 20). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	bool HasInventorySpace() const { return Inventory.Num() < MaxInventorySlots; }

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

	/** Foundational stat points (Damage/Attack Speed/Attack Size/Armor/Evasion/Luck + Speed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetSpeedStat() const { return FMath::Max(1, HeroStats.Speed); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetDamageStat() const { return FMath::Max(1, HeroStats.Damage + FMath::Max(0, ItemStatBonuses.Damage)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetAttackSpeedStat() const { return FMath::Max(1, HeroStats.AttackSpeed + FMath::Max(0, ItemStatBonuses.AttackSpeed)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetScaleStat() const { return FMath::Max(1, HeroStats.AttackSize + FMath::Max(0, ItemStatBonuses.AttackSize)); } // Attack Size (aka Scale)

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetArmorStat() const { return FMath::Max(1, HeroStats.Armor + FMath::Max(0, ItemStatBonuses.Armor)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetEvasionStat() const { return FMath::Max(1, HeroStats.Evasion + FMath::Max(0, ItemStatBonuses.Evasion)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetLuckStat() const { return FMath::Max(1, HeroStats.Luck + FMath::Max(0, ItemStatBonuses.Luck)); }

	/** Derived multipliers from hero stats (separate from item multipliers). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroMoveSpeedMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroDamageMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroAttackSpeedMultiplier() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetHeroScaleMultiplier() const;

	/** Damage reduction percent (0..1) applied to incoming hits (v0: 1% per level). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetArmorReduction01() const;

	/** Dodge chance percent (0..1) (v0: 1% per level). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Derived")
	float GetEvasionChance01() const;

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
	bool IsVendorStockSlotSold(int32 Index) const;

	/** Attempt to buy a vendor slot; returns true if purchased. */
	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	bool TryBuyVendorStockSlot(int32 Index);

	/** Attempt to steal a vendor slot (grants item if success). Always increases anger based on attempt outcome. */
	UFUNCTION(BlueprintCallable, Category = "RunState|Vendor")
	bool ResolveVendorStealAttempt(int32 Index, bool bTimingHit, bool bRngSuccess);

	/** True if vendor anger has reached threshold and a boss should spawn. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	bool IsVendorAngryEnoughToBoss() const { return VendorAngerGold >= VendorAngerThresholdGold; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Vendor")
	bool HasBoughtFromVendorThisStage() const { return bVendorBoughtSomethingThisStage; }

	// ============================================
	// Stage Boost (Difficulty start)
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|StageBoost")
	bool IsInStageBoost() const { return bInStageBoost; }

	UFUNCTION(BlueprintCallable, Category = "RunState|StageBoost")
	void SetInStageBoost(bool bInBoost);

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
	bool ApplyTrueDamage(int32 Hearts);

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

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ResetForNewRun();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ToggleHUDPanels();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddLogEntry(const FString& Entry);

private:
	void TrimLogsIfNeeded();
	void RecomputeItemDerivedStats();
	void EnterLastStand();
	void EndLastStandAndDie();
	void InitializeHeroStatTuningForSelectedHero();
	void InitializeHeroStatsForNewRun();
	void ApplyOneHeroLevelUp();

	UPROPERTY()
	int32 CurrentHearts = DefaultMaxHearts;

	UPROPERTY()
	int32 MaxHearts = DefaultMaxHearts;

	UPROPERTY()
	int32 CurrentGold = 0;

	UPROPERTY()
	int32 CurrentDebt = 0;

	UPROPERTY()
	int32 DifficultyTier = 0;

	/** Difficulty in skulls (can be fractional). */
	UPROPERTY()
	float DifficultySkulls = 0.f;

	UPROPERTY()
	int32 TotemsActivatedCount = 0;

	UPROPERTY()
	float GamblerAnger01 = 0.f;

	UPROPERTY()
	TArray<FName> OwedBossIDs;

	UPROPERTY()
	TArray<FName> Inventory;

	UPROPERTY()
	TArray<FName> EquippedIdolIDs;

	UPROPERTY()
	TArray<FString> EventLog;

	UPROPERTY()
	TArray<FRunEvent> StructuredEventLog;

	UPROPERTY()
	int32 CurrentStage = 1;

	UPROPERTY()
	bool bStageTimerActive = false;

	UPROPERTY()
	float StageTimerSecondsRemaining = StageTimerDurationSeconds;

	/** Last integer second we broadcasted (throttle StageTimerChanged to once per second). */
	int32 LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);

	// Speedrun timer (counts up from start gate; resets each stage).
	UPROPERTY()
	float SpeedRunElapsedSeconds = 0.f;

	UPROPERTY()
	float SpeedRunStartWorldSeconds = 0.f;

	UPROPERTY()
	bool bSpeedRunStarted = false;

	/** Last integer second we broadcasted (throttle SpeedRunTimerChanged to once per second). */
	int32 LastBroadcastSpeedRunSecond = -1;

	UPROPERTY()
	int32 CurrentScore = 0;

	UPROPERTY()
	bool bHUDPanelsVisible = true;

	UPROPERTY()
	bool bBossActive = false;

	UPROPERTY()
	FName ActiveBossID = NAME_None;

	UPROPERTY()
	int32 BossMaxHP = 100;

	UPROPERTY()
	int32 BossCurrentHP = 0;

	/** If true, spawn LoanShark when the timer starts in this stage (only when debt was carried from previous stage). */
	bool bLoanSharkPending = false;

	float LastDamageTime = -9999.f;

	// ============================================
	// Derived combat tuning from Inventory (recomputed on InventoryChanged)
	// ============================================

	/** Flat stat bonuses from inventory items (main stat line only, v1). */
	FT66HeroStatBonuses ItemStatBonuses = FT66HeroStatBonuses{};

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

	/** Current hero's foundational stat points (base + level-ups). */
	FT66HeroStatBlock HeroStats = FT66HeroStatBlock{};

	/** Current hero's per-level gain ranges (Speed is always +1 per level). */
	FT66HeroPerLevelStatGains HeroPerLevelGains = FT66HeroPerLevelStatGains{};

	/** Run-persistent RNG for hero stat gains (so stage reloads don't reshuffle). */
	FRandomStream HeroStatRng;

	float UltimateCooldownRemainingSeconds = 0.f;
	int32 LastBroadcastUltimateSecond = 0;

	float SurvivalCharge01 = 0.f;
	bool bInLastStand = false;
	float LastStandSecondsRemaining = 0.f;
	int32 LastBroadcastLastStandSecond = 0;

	// ============================================
	// Vendor shop state (per-stage)
	// ============================================

	int32 VendorAngerGold = 0;
	int32 VendorStockStage = 0;
	int32 VendorStockRerollStage = 0;
	int32 VendorStockRerollCounter = 0;
	TArray<FName> VendorStockItemIDs;
	TArray<bool> VendorStockSold;
	bool bVendorBoughtSomethingThisStage = false;

	bool bInStageBoost = false;

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
