// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"

bool UT66RunStateSubsystem::ApplyDamage(int32 Hearts)
{
	if (Hearts <= 0) return false;

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	if (Now - LastDamageTime < InvulnDurationSeconds)
	{
		return false;
	}

	LastDamageTime = Now;
	CurrentHearts = FMath::Max(0, CurrentHearts - Hearts);
	HeartsChanged.Broadcast();

	if (CurrentHearts <= 0)
	{
		OnPlayerDied.Broadcast();
	}
	return true;
}

void UT66RunStateSubsystem::AddItem(FName ItemID)
{
	if (ItemID.IsNone()) return;
	Inventory.Add(ItemID);
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *ItemID.ToString()));
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

bool UT66RunStateSubsystem::SellFirstItem()
{
	if (Inventory.Num() == 0) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	FName ItemID = Inventory[0];
	FItemData ItemData;
	if (!GI->GetItemData(ItemID, ItemData))
	{
		Inventory.RemoveAt(0);
		InventoryChanged.Broadcast();
		AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=unknown,Source=Vendor,ItemID=%s"), *ItemID.ToString()));
		LogAdded.Broadcast();
		return true;
	}

	CurrentGold += ItemData.SellValueGold;
	Inventory.RemoveAt(0);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Vendor,ItemID=%s"), ItemData.SellValueGold, *ItemID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

void UT66RunStateSubsystem::ResetForNewRun()
{
	CurrentHearts = MaxHearts;
	CurrentGold = 0;
	Inventory.Empty();
	EventLog.Empty();
	StructuredEventLog.Empty();
	CurrentStage = 1;
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	CurrentScore = 0;
	LastDamageTime = -9999.f;
	bHUDPanelsVisible = true;
	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	PanelVisibilityChanged.Broadcast();
	ScoreChanged.Broadcast();
	StageTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::SetCurrentStage(int32 Stage)
{
	const int32 NewStage = FMath::Clamp(Stage, 1, 66);
	if (CurrentStage == NewStage) return;
	CurrentStage = NewStage;
	StageChanged.Broadcast();
}

void UT66RunStateSubsystem::SetStageTimerActive(bool bActive)
{
	if (bStageTimerActive == bActive) return;
	bStageTimerActive = bActive;
	StageTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::TickStageTimer(float DeltaTime)
{
	if (!bStageTimerActive || StageTimerSecondsRemaining <= 0.f) return;
	StageTimerSecondsRemaining = FMath::Max(0.f, StageTimerSecondsRemaining - DeltaTime);
	const int32 CurrentSecond = FMath::FloorToInt(StageTimerSecondsRemaining);
	if (CurrentSecond != LastBroadcastStageTimerSecond)
	{
		LastBroadcastStageTimerSecond = CurrentSecond;
		StageTimerChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::ResetStageTimerToFull()
{
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	StageTimerChanged.Broadcast();
}

void UT66RunStateSubsystem::AddScore(int32 Points)
{
	if (Points <= 0) return;
	CurrentScore += Points;
	ScoreChanged.Broadcast();
}

void UT66RunStateSubsystem::AddStructuredEvent(ET66RunEventType EventType, const FString& Payload)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	const float Timestamp = World ? static_cast<float>(World->GetTimeSeconds()) : 0.f;
	StructuredEventLog.Add(FRunEvent(EventType, Timestamp, Payload));
	// Append human-readable line to EventLog for Run Summary scroll
	FString Short;
	switch (EventType)
	{
		case ET66RunEventType::StageEntered:   Short = FString::Printf(TEXT("Stage %s"), *Payload); break;
		case ET66RunEventType::ItemAcquired:   Short = FString::Printf(TEXT("Picked up %s"), *Payload); break;
		case ET66RunEventType::GoldGained:    Short = FString::Printf(TEXT("Gold: %s"), *Payload); break;
		case ET66RunEventType::EnemyKilled:   Short = FString::Printf(TEXT("Kill +%s"), *Payload); break;
		default: Short = Payload; break;
	}
	EventLog.Add(Short);
}

void UT66RunStateSubsystem::ToggleHUDPanels()
{
	bHUDPanelsVisible = !bHUDPanelsVisible;
	PanelVisibilityChanged.Broadcast();
}

void UT66RunStateSubsystem::AddLogEntry(const FString& Entry)
{
	EventLog.Add(Entry);
}
