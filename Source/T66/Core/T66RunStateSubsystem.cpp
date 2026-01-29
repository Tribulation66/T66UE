// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"

float UT66RunStateSubsystem::GetDifficultyMultiplier() const
{
	// Linear scaling: Tier 0 => 1x, Tier 1 => 2x, Tier 2 => 3x, ...
	return 1.f + static_cast<float>(FMath::Max(0, DifficultyTier));
}

void UT66RunStateSubsystem::IncreaseDifficultyTier()
{
	DifficultyTier = FMath::Clamp(DifficultyTier + 1, 0, 999);
	DifficultyChanged.Broadcast();
}

float UT66RunStateSubsystem::AddGamblerAngerFromBet(int32 BetGold)
{
	const float Delta = FMath::Clamp(static_cast<float>(FMath::Max(0, BetGold)) / 100.f, 0.f, 1.f);
	GamblerAnger01 = FMath::Clamp(GamblerAnger01 + Delta, 0.f, 1.f);
	GamblerAngerChanged.Broadcast();
	return GamblerAnger01;
}

void UT66RunStateSubsystem::ResetGamblerAnger()
{
	if (FMath::IsNearlyZero(GamblerAnger01)) return;
	GamblerAnger01 = 0.f;
	GamblerAngerChanged.Broadcast();
}

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

void UT66RunStateSubsystem::AddGold(int32 Amount)
{
	if (Amount == 0) return;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Gambler"), Amount));
	GoldChanged.Broadcast();
	LogAdded.Broadcast();
}

bool UT66RunStateSubsystem::TrySpendGold(int32 Amount)
{
	if (Amount <= 0) return true;
	if (CurrentGold < Amount) return false;

	CurrentGold = FMath::Max(0, CurrentGold - Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=Gambler"), Amount));
	GoldChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

void UT66RunStateSubsystem::AddOwedBoss(FName BossID)
{
	if (BossID.IsNone()) return;
	OwedBossIDs.Add(BossID);
	AddStructuredEvent(ET66RunEventType::StageExited, FString::Printf(TEXT("OwedBoss=%s"), *BossID.ToString()));
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::RemoveFirstOwedBoss()
{
	if (OwedBossIDs.Num() <= 0) return;
	OwedBossIDs.RemoveAt(0);
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::BorrowGold(int32 Amount)
{
	if (Amount <= 0) return;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	CurrentDebt = FMath::Max(0, CurrentDebt + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Borrow"), Amount));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();
}

int32 UT66RunStateSubsystem::PayDebt(int32 Amount)
{
	if (Amount <= 0 || CurrentDebt <= 0 || CurrentGold <= 0) return 0;
	const int32 Pay = FMath::Clamp(Amount, 0, FMath::Min(CurrentDebt, CurrentGold));
	if (Pay <= 0) return 0;

	CurrentGold = FMath::Max(0, CurrentGold - Pay);
	CurrentDebt = FMath::Max(0, CurrentDebt - Pay);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=PayDebt"), Pay));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();

	// If debt is cleared, make sure a pending loan shark won't spawn.
	if (CurrentDebt <= 0)
	{
		bLoanSharkPending = false;
	}
	return Pay;
}

void UT66RunStateSubsystem::AddItem(FName ItemID)
{
	if (ItemID.IsNone()) return;
	Inventory.Add(ItemID);
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *ItemID.ToString()));
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::HealToFull()
{
	CurrentHearts = MaxHearts;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::KillPlayer()
{
	CurrentHearts = 0;
	LastDamageTime = -9999.f;
	HeartsChanged.Broadcast();
	OnPlayerDied.Broadcast();
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
	CurrentGold = DefaultStartGold;
	CurrentDebt = 0;
	bLoanSharkPending = false;
	DifficultyTier = 0;
	GamblerAnger01 = 0.f;
	OwedBossIDs.Empty();
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
	ResetBossState();
	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	DifficultyChanged.Broadcast();
	GamblerAngerChanged.Broadcast();
	InventoryChanged.Broadcast();
	PanelVisibilityChanged.Broadcast();
	ScoreChanged.Broadcast();
	StageTimerChanged.Broadcast();
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::SetCurrentStage(int32 Stage)
{
	const int32 NewStage = FMath::Clamp(Stage, 1, 66);
	if (CurrentStage == NewStage) return;
	CurrentStage = NewStage;
	// Bible: gambler anger resets at end of every stage.
	ResetGamblerAnger();
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

void UT66RunStateSubsystem::SetBossActive(int32 InMaxHP)
{
	bBossActive = true;
	BossMaxHP = FMath::Max(1, InMaxHP);
	BossCurrentHP = BossMaxHP;
	BossChanged.Broadcast();
}

void UT66RunStateSubsystem::SetBossInactive()
{
	bBossActive = false;
	BossCurrentHP = 0;
	BossChanged.Broadcast();
}

bool UT66RunStateSubsystem::ApplyBossDamage(int32 Damage)
{
	if (!bBossActive || Damage <= 0 || BossCurrentHP <= 0) return false;
	BossCurrentHP = FMath::Max(0, BossCurrentHP - Damage);
	BossChanged.Broadcast();
	return BossCurrentHP <= 0;
}

void UT66RunStateSubsystem::ResetBossState()
{
	bBossActive = false;
	BossMaxHP = 100;
	BossCurrentHP = 0;
	BossChanged.Broadcast();
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
