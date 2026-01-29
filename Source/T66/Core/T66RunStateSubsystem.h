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

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnHeartsChanged HeartsChanged;

	UPROPERTY(BlueprintAssignable, Category = "RunState")
	FOnGoldChanged GoldChanged;

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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentHearts() const { return CurrentHearts; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetMaxHearts() const { return MaxHearts; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentGold() const { return CurrentGold; }

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
	static constexpr float StageTimerDurationSeconds = 60.f;

	/** Seconds remaining on stage timer. When inactive, stays at 60 (frozen). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	float GetStageTimerSecondsRemaining() const { return StageTimerSecondsRemaining; }

	/** Call every frame from GameMode Tick to count down when timer is active. */
	void TickStageTimer(float DeltaTime);

	/** Reset timer to 60 and freeze (e.g. when entering next stage so start gate starts it again). */
	void ResetStageTimerToFull();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	int32 GetCurrentScore() const { return CurrentScore; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState")
	const TArray<FRunEvent>& GetStructuredEventLog() const { return StructuredEventLog; }

	/** Set current stage (1–66). */
	UFUNCTION(BlueprintCallable, Category = "RunState")
	void SetCurrentStage(int32 Stage);

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
	int32 LastBroadcastStageTimerSecond = 60;

	UPROPERTY()
	int32 CurrentScore = 0;

	UPROPERTY()
	bool bHUDPanelsVisible = true;

	float LastDamageTime = -9999.f;
	float InvulnDurationSeconds = DefaultInvulnDurationSeconds;
};
