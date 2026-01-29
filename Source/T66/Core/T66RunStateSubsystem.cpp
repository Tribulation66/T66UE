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
	AddLogEntry(FString::Printf(TEXT("Picked up %s"), *ItemID.ToString()));
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
		AddLogEntry(FString::Printf(TEXT("Sold %s (unknown value)"), *ItemID.ToString()));
		LogAdded.Broadcast();
		return true;
	}

	CurrentGold += ItemData.SellValueGold;
	Inventory.RemoveAt(0);
	AddLogEntry(FString::Printf(TEXT("Sold %s for %d gold"), *ItemID.ToString(), ItemData.SellValueGold));
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
	LastDamageTime = -9999.f;
	bHUDPanelsVisible = true;
	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	PanelVisibilityChanged.Broadcast();
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
