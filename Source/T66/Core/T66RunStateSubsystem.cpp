// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"

const TArray<FName>& UT66RunStateSubsystem::GetAllIdolIDs()
{
	// Stable IDs (contract): keep these names stable forever.
	static const TArray<FName> Idols = {
		FName(TEXT("Idol_Frost")),
		FName(TEXT("Idol_Shock")),
		FName(TEXT("Idol_Glue")),
		FName(TEXT("Idol_Silence")),
		FName(TEXT("Idol_Mark")),
		FName(TEXT("Idol_Pierce")),
		FName(TEXT("Idol_Split")),
		FName(TEXT("Idol_Knockback")),
		FName(TEXT("Idol_Ricochet")),
		FName(TEXT("Idol_Hex")),
		FName(TEXT("Idol_Fire")),
		FName(TEXT("Idol_Lifesteal")),
	};
	return Idols;
}

FLinearColor UT66RunStateSubsystem::GetIdolColor(FName IdolID)
{
	if (IdolID == FName(TEXT("Idol_Frost"))) return FLinearColor(0.35f, 0.75f, 1.0f, 1.f);
	if (IdolID == FName(TEXT("Idol_Shock"))) return FLinearColor(0.70f, 0.25f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Glue"))) return FLinearColor(0.20f, 0.85f, 0.35f, 1.f);
	if (IdolID == FName(TEXT("Idol_Silence"))) return FLinearColor(0.55f, 0.55f, 0.75f, 1.f);
	if (IdolID == FName(TEXT("Idol_Mark"))) return FLinearColor(0.95f, 0.80f, 0.20f, 1.f);
	if (IdolID == FName(TEXT("Idol_Pierce"))) return FLinearColor(0.92f, 0.92f, 0.98f, 1.f);
	if (IdolID == FName(TEXT("Idol_Split"))) return FLinearColor(0.95f, 0.20f, 0.20f, 1.f);
	if (IdolID == FName(TEXT("Idol_Knockback"))) return FLinearColor(0.95f, 0.55f, 0.15f, 1.f);
	if (IdolID == FName(TEXT("Idol_Ricochet"))) return FLinearColor(0.20f, 0.95f, 0.95f, 1.f);
	if (IdolID == FName(TEXT("Idol_Hex"))) return FLinearColor(0.10f, 0.10f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Fire"))) return FLinearColor(0.95f, 0.25f, 0.10f, 1.f);
	if (IdolID == FName(TEXT("Idol_Lifesteal"))) return FLinearColor(0.75f, 0.10f, 0.25f, 1.f);
	return FLinearColor(0.25f, 0.25f, 0.28f, 1.f);
}

bool UT66RunStateSubsystem::EquipIdolInSlot(int32 SlotIndex, FName IdolID)
{
	if (SlotIndex < 0 || SlotIndex >= MaxEquippedIdolSlots) return false;
	if (IdolID.IsNone()) return false;

	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}

	if (EquippedIdolIDs[SlotIndex] == IdolID) return false;
	EquippedIdolIDs[SlotIndex] = IdolID;
	IdolsChanged.Broadcast();
	return true;
}

bool UT66RunStateSubsystem::EquipIdolFirstEmpty(FName IdolID)
{
	if (IdolID.IsNone()) return false;
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
	}

	for (int32 i = 0; i < EquippedIdolIDs.Num(); ++i)
	{
		if (EquippedIdolIDs[i].IsNone())
		{
			EquippedIdolIDs[i] = IdolID;
			IdolsChanged.Broadcast();
			return true;
		}
	}
	return false;
}

void UT66RunStateSubsystem::ClearEquippedIdols()
{
	if (EquippedIdolIDs.Num() != MaxEquippedIdolSlots)
	{
		EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
		IdolsChanged.Broadcast();
		return;
	}

	bool bAny = false;
	for (FName& N : EquippedIdolIDs)
	{
		if (!N.IsNone())
		{
			N = NAME_None;
			bAny = true;
		}
	}
	if (bAny)
	{
		IdolsChanged.Broadcast();
	}
}

void UT66RunStateSubsystem::TrimLogsIfNeeded()
{
	if (EventLog.Num() > MaxEventLogEntries)
	{
		const int32 RemoveCount = EventLog.Num() - MaxEventLogEntries;
		EventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
	if (StructuredEventLog.Num() > MaxStructuredEventLogEntries)
	{
		const int32 RemoveCount = StructuredEventLog.Num() - MaxStructuredEventLogEntries;
		StructuredEventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
}

float UT66RunStateSubsystem::GetDifficultyMultiplier() const
{
	// Linear scaling: Tier 0 => 1x, Tier 1 => 2x, Tier 2 => 3x, ...
	return 1.f + static_cast<float>(FMath::Max(0, DifficultyTier));
}

void UT66RunStateSubsystem::IncreaseDifficultyTier()
{
	AddDifficultySkulls(1.0f);
}

void UT66RunStateSubsystem::AddDifficultySkulls(float DeltaSkulls)
{
	if (DeltaSkulls == 0.f) return;
	DifficultySkulls = FMath::Clamp(DifficultySkulls + DeltaSkulls, 0.f, 9999.f);

	// Cache integer tier for existing enemy scaling logic (tier 0 -> 1x, tier 1 -> 2x, ...).
	DifficultyTier = FMath::Clamp(FMath::FloorToInt(DifficultySkulls), 0, 999);
	DifficultyChanged.Broadcast();
}

void UT66RunStateSubsystem::AddMaxHearts(int32 DeltaHearts)
{
	if (DeltaHearts <= 0) return;
	MaxHearts = FMath::Clamp(MaxHearts + DeltaHearts, 1, 9999);
	CurrentHearts = FMath::Clamp(CurrentHearts + DeltaHearts, 0, MaxHearts);
	HeartsChanged.Broadcast();
}

int32 UT66RunStateSubsystem::RegisterTotemActivated()
{
	TotemsActivatedCount = FMath::Clamp(TotemsActivatedCount + 1, 0, 999);
	return TotemsActivatedCount;
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
	if (Inventory.Num() >= MaxInventorySlots)
	{
		AddLogEntry(TEXT("Inventory full."));
		LogAdded.Broadcast();
		return;
	}
	Inventory.Add(ItemID);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *ItemID.ToString()));
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}

void UT66RunStateSubsystem::HealToFull()
{
	CurrentHearts = MaxHearts;
	HeartsChanged.Broadcast();
}

void UT66RunStateSubsystem::HealHearts(int32 Hearts)
{
	if (Hearts <= 0) return;
	const int32 NewHearts = FMath::Clamp(CurrentHearts + Hearts, 0, MaxHearts);
	if (NewHearts == CurrentHearts) return;
	CurrentHearts = NewHearts;
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
		RecomputeItemDerivedStats();
		InventoryChanged.Broadcast();
		AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=unknown,Source=Vendor,ItemID=%s"), *ItemID.ToString()));
		LogAdded.Broadcast();
		return true;
	}

	CurrentGold += ItemData.SellValueGold;
	Inventory.RemoveAt(0);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Vendor,ItemID=%s"), ItemData.SellValueGold, *ItemID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}

void UT66RunStateSubsystem::RecomputeItemDerivedStats()
{
	ItemPowerGivenPercent = 0.f;
	BonusDamagePercent = 0.f;
	BonusAttackSpeedPercent = 0.f;
	DashCooldownReductionPercent = 0.f;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	for (const FName& ItemID : Inventory)
	{
		if (ItemID.IsNone()) continue;

		FItemData D;
		if (!GI || !GI->GetItemData(ItemID, D))
		{
			// Safe fallback: keep the old v0 feel even if the DT row is missing.
			if (ItemID == TEXT("Item_01")) { ItemPowerGivenPercent += 5.f; BonusDamagePercent += 10.f; }
			else if (ItemID == TEXT("Item_02")) { ItemPowerGivenPercent += 7.5f; BonusAttackSpeedPercent += 15.f; }
			else if (ItemID == TEXT("Item_03")) { ItemPowerGivenPercent += 10.f; DashCooldownReductionPercent += 25.f; }
			continue;
		}

		ItemPowerGivenPercent += D.PowerGivenPercent;
		switch (D.EffectType)
		{
			case ET66ItemEffectType::BonusDamagePct: BonusDamagePercent += D.EffectMagnitude; break;
			case ET66ItemEffectType::BonusAttackSpeedPct: BonusAttackSpeedPercent += D.EffectMagnitude; break;
			case ET66ItemEffectType::DashCooldownReductionPct: DashCooldownReductionPercent += D.EffectMagnitude; break;
			default: break;
		}
	}

	const float PowerMult = 1.f + (ItemPowerGivenPercent / 100.f);
	const float DamageMult = 1.f + (BonusDamagePercent / 100.f);
	const float AttackSpeedMult = 1.f + (BonusAttackSpeedPercent / 100.f);
	const float DashCdMult = 1.f - (DashCooldownReductionPercent / 100.f);

	ItemDamageMultiplier = FMath::Clamp(PowerMult * DamageMult, 0.1f, 100.f);
	ItemAttackSpeedMultiplier = FMath::Clamp(PowerMult * AttackSpeedMult, 0.1f, 100.f);
	DashCooldownMultiplier = FMath::Clamp(DashCdMult, 0.05f, 1.f);
	ItemScaleMultiplier = FMath::Clamp(PowerMult, 0.1f, 10.f);
}

void UT66RunStateSubsystem::ResetForNewRun()
{
	MaxHearts = DefaultMaxHearts;
	CurrentHearts = MaxHearts;
	CurrentGold = DefaultStartGold;
	CurrentDebt = 0;
	bLoanSharkPending = false;
	DifficultyTier = 0;
	DifficultySkulls = 0.f;
	TotemsActivatedCount = 0;
	GamblerAnger01 = 0.f;
	OwedBossIDs.Empty();
	Inventory.Empty();
	RecomputeItemDerivedStats();
	EquippedIdolIDs.Init(NAME_None, MaxEquippedIdolSlots);
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
	IdolsChanged.Broadcast();
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
	FString Short = Payload;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	switch (EventType)
	{
		case ET66RunEventType::StageEntered:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_StageFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Stage %s"), *Payload);
			break;
		case ET66RunEventType::ItemAcquired:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_PickedUpFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Picked up %s"), *Payload);
			break;
		case ET66RunEventType::GoldGained:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_GoldFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Gold: %s"), *Payload);
			break;
		case ET66RunEventType::EnemyKilled:
			if (Loc) Short = FText::Format(Loc->GetText_RunLog_KillFormat(), FText::FromString(Payload)).ToString();
			else Short = FString::Printf(TEXT("Kill +%s"), *Payload);
			break;
		default:
			break;
	}
	EventLog.Add(Short);
	TrimLogsIfNeeded();
}

void UT66RunStateSubsystem::ToggleHUDPanels()
{
	bHUDPanelsVisible = !bHUDPanelsVisible;
	PanelVisibilityChanged.Broadcast();
}

void UT66RunStateSubsystem::AddLogEntry(const FString& Entry)
{
	EventLog.Add(Entry);
	TrimLogsIfNeeded();
}
