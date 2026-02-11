// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Core/T66Rarity.h"
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialHintChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialInputChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDevCheatsChanged);

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

	/** Tutorial hint text changed (above crosshair). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnTutorialHintChanged TutorialHintChanged;

	/** Tutorial input progress changed (move/jump). */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnTutorialInputChanged TutorialInputChanged;

	/** Dev-only toggles (immortality/power) changed. */
	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnDevCheatsChanged DevCheatsChanged;

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

	/** Equipped idol slots (size=6). NAME_None means empty slot. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FName>& GetEquippedIdols() const { return EquippedIdolIDs; }

	/** Idol max level (level-up by selecting same idol again). */
	static constexpr int32 MaxIdolLevel = 10;

	/** Equipped idol level for a specific slot (0..5). Returns 0 when empty. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetEquippedIdolLevelInSlot(int32 SlotIndex) const;

	/** Equip an idol into a specific slot (0..5). Returns true if changed. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool EquipIdolInSlot(int32 SlotIndex, FName IdolID);

	/** Equip an idol into the first empty slot. Returns true if equipped. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool EquipIdolFirstEmpty(FName IdolID);

	/**
	 * Idol altar selection:
	 * - if idol is already equipped, increase its level up to MaxIdolLevel
	 * - otherwise, equip into first empty slot at level 1
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

	// ============================================================
	// Idol stock (shop-style altar: 3 offered idols + reroll)
	// ============================================================

	static constexpr int32 IdolStockSlotCount = 3;

	/** Ensure idol stock has been generated for the current stage. */
	void EnsureIdolStock();

	/** Reroll the idol stock (regenerate 3 random idols). */
	void RerollIdolStock();

	/** Returns the current idol stock IDs (3 slots). */
	const TArray<FName>& GetIdolStockIDs() const { return IdolStockIDs; }

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

	/** True if this run has produced a new personal-best speed run time for any completed stage (difficulty/party scoped). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|SpeedRun")
	bool DidThisRunSetNewPersonalBestTime() const { return bThisRunSetNewPersonalBestSpeedRunTime; }

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

	/** Add an item by template ID with auto-generated rarity and rolled value. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddItem(FName ItemID);

	/** Add a fully specified item slot (template + rarity + rolled value). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddItemSlot(const FT66InventorySlot& Slot);

	/** Clear inventory only (e.g. Lab "Reset Items"). Recomputes stats and broadcasts. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ClearInventory();

	/** Sell first inventory item. Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SellFirstItem();

	/** Sell a specific inventory item slot (0..InventorySlots.Num-1). Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SellInventoryItemAt(int32 InventoryIndex);

	/** True if there is space in the inventory (max 20). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Items")
	bool HasInventorySpace() const { return InventorySlots.Num() < MaxInventorySlots; }

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

	/** Foundational stat points (Damage/Attack Speed/Attack Scale/Armor/Evasion/Luck + Speed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetSpeedStat() const { return FMath::Max(1, HeroStats.Speed); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetDamageStat() const { return FMath::Max(1, HeroStats.Damage + FMath::Max(0, ItemStatBonuses.Damage) + FMath::Max(0, PowerupStatBonuses.Damage)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetAttackSpeedStat() const { return FMath::Max(1, HeroStats.AttackSpeed + FMath::Max(0, ItemStatBonuses.AttackSpeed) + FMath::Max(0, PowerupStatBonuses.AttackSpeed)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetScaleStat() const { return FMath::Max(1, HeroStats.AttackScale + FMath::Max(0, ItemStatBonuses.AttackScale) + FMath::Max(0, PowerupStatBonuses.AttackScale)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetArmorStat() const { return FMath::Max(1, HeroStats.Armor + FMath::Max(0, ItemStatBonuses.Armor) + FMath::Max(0, PowerupStatBonuses.Armor)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetEvasionStat() const { return FMath::Max(1, HeroStats.Evasion + FMath::Max(0, ItemStatBonuses.Evasion) + FMath::Max(0, PowerupStatBonuses.Evasion)); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Stats")
	int32 GetLuckStat() const { return FMath::Max(1, HeroStats.Luck + FMath::Max(0, ItemStatBonuses.Luck) + FMath::Max(0, PowerupStatBonuses.Luck)); }

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

	/** Aggro multiplier (base * taunt items). Higher = enemies target this hero more. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetAggroMultiplier() const;

	/** HP regen per second (base * items). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Hero|Secondary")
	float GetHpRegenPerSecond() const;

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
	// Power Crystals (earned this run; persisted at run end)
	// ============================================

	/** Power Crystals earned this run (from boss kills). Added to persistent balance when run ends. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|PowerUp")
	int32 GetPowerCrystalsEarnedThisRun() const { return PowerCrystalsEarnedThisRun; }

	UFUNCTION(BlueprintCallable, Category = "RunState|PowerUp")
	void AddPowerCrystalsEarnedThisRun(int32 Amount);

	// ============================================
	// Luck Rating (aggregated Luck/RNG outcomes)
	// ============================================

	/** Record a luck-affected quantity roll outcome, normalized within [Min..Max]. */
	void RecordLuckQuantityRoll(FName Category, int32 RolledValue, int32 MinValue, int32 MaxValue);

	/** Record a luck-affected boolean outcome (false=0, true=1). */
	void RecordLuckQuantityBool(FName Category, bool bSucceeded);

	/** Record a luck-affected quality outcome (rarity tier). */
	void RecordLuckQualityRarity(FName Category, ET66Rarity Rarity);

	/** Final run Luck Rating (0..100), derived from recorded quantity + quality outcomes. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckRating0To100() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckRatingQuantity0To100() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetLuckRatingQuality0To100() const;

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

	/** Mark that movement input has been used at least once (for tutorial flow). */
	void NotifyTutorialMoveInput();

	/** Mark that jump input has been used at least once (for tutorial flow). */
	void NotifyTutorialJumpInput();

	bool HasSeenTutorialMoveInput() const { return bTutorialMoveInputSeen; }
	bool HasSeenTutorialJumpInput() const { return bTutorialJumpInputSeen; }

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

	void ResetLuckRatingTracking();

	void TrimLogsIfNeeded();
	void RecomputeItemDerivedStats();
	void EnterLastStand();
	void EndLastStandAndDie();
	void InitializeHeroStatTuningForSelectedHero();
	void InitializeHeroStatsForNewRun();
	void ApplyOneHeroLevelUp();
	void RefreshPowerupBonusesFromProfile();

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

	/** Difficulty in skulls (integer). */
	UPROPERTY()
	int32 DifficultySkulls = 0;

	UPROPERTY()
	int32 TotemsActivatedCount = 0;

	UPROPERTY()
	float GamblerAnger01 = 0.f;

	UPROPERTY()
	TArray<FName> OwedBossIDs;

	/** Item inventory using the new slot-based system (template + rarity + rolled value). */
	UPROPERTY()
	TArray<FT66InventorySlot> InventorySlots;

	UPROPERTY()
	TArray<FName> EquippedIdolIDs;

	UPROPERTY()
	TArray<uint8> EquippedIdolLevels;

	/** Idol stock: 3 offered idols (shop-style altar). */
	TArray<FName> IdolStockIDs;

	/** Whether each stock slot has been selected this visit. */
	TArray<bool> IdolStockSelected;

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

	// Run Summary banner: set true if any completed stage submission set a new personal best time.
	bool bThisRunSetNewPersonalBestSpeedRunTime = false;

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

	// Tutorial input flags (first-time onboarding).
	UPROPERTY()
	bool bTutorialMoveInputSeen = false;

	UPROPERTY()
	bool bTutorialJumpInputSeen = false;

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

	/** If true, spawn LoanShark when the timer starts in this stage (only when debt was carried from previous stage). */
	bool bLoanSharkPending = false;

	float LastDamageTime = -9999.f;

	int32 PowerCrystalsEarnedThisRun = 0;

	// ============================================
	// Derived combat tuning from Inventory (recomputed on InventoryChanged)
	// ============================================

	/** Flat stat bonuses from inventory items (main stat line only, v1). */
	FT66HeroStatBonuses ItemStatBonuses = FT66HeroStatBonuses{};

	/** Power-up slice bonuses (from PowerUpSubsystem; refreshed at run start). */
	FT66HeroStatBonuses PowerupStatBonuses = FT66HeroStatBonuses{};

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

	// ============================================
	// Accumulated secondary stat multipliers from items (product of Line 2 multipliers)
	// Reset and recomputed in RecomputeItemDerivedStats()
	// ============================================
	TMap<ET66SecondaryStatType, float> SecondaryMultipliers;

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
	TArray<FT66InventorySlot> VendorStockSlots;
	TArray<bool> VendorStockSold;
	bool bVendorBoughtSomethingThisStage = false;
	ET66VendorStealOutcome LastVendorStealOutcome = ET66VendorStealOutcome::None;

	// Aggregated tracking for Luck Rating (per run).
	TMap<FName, FT66LuckAccumulator> LuckQuantityByCategory;
	TMap<FName, FT66LuckAccumulator> LuckQualityByCategory;

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
