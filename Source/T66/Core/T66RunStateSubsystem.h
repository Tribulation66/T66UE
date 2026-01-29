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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDebtChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDifficultyChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGamblerAngerChanged);

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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetDifficultyMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void IncreaseDifficultyTier();

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

	/** Call every frame from GameMode Tick to count down when timer is active. */
	void TickStageTimer(float DeltaTime);

	/** Reset timer to full duration and freeze (e.g. when entering next stage so start gate starts it again). */
	void ResetStageTimerToFull();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentScore() const { return CurrentScore; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	bool GetBossActive() const { return bBossActive; }

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

	/** Instantly kill the player (bypasses i-frames). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void KillPlayer();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddItem(FName ItemID);

	/** Sell first inventory item. Returns true if sold. */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	bool SellFirstItem();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ResetForNewRun();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void ToggleHUDPanels();

	UFUNCTION(BlueprintCallable, Category = "RunState")
	void AddLogEntry(const FString& Entry);

private:
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

	UPROPERTY()
	float GamblerAnger01 = 0.f;

	UPROPERTY()
	TArray<FName> OwedBossIDs;

	UPROPERTY()
	TArray<FName> Inventory;

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

	UPROPERTY()
	int32 CurrentScore = 0;

	UPROPERTY()
	bool bHUDPanelsVisible = true;

	UPROPERTY()
	bool bBossActive = false;

	UPROPERTY()
	int32 BossMaxHP = 100;

	UPROPERTY()
	int32 BossCurrentHP = 0;

	/** If true, spawn LoanShark when the timer starts in this stage (only when debt was carried from previous stage). */
	bool bLoanSharkPending = false;

	float LastDamageTime = -9999.f;
	float InvulnDurationSeconds = DefaultInvulnDurationSeconds;
};
