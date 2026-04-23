// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

float UT66RunStateSubsystem::AddGamblerAngerFromBet(int32 BetGold)
{
	return AddCasinoAngerFromGold(BetGold);
}


float UT66RunStateSubsystem::AddCasinoAngerFromGold(int32 AngerGold)
{
	const float Delta = FMath::Clamp(static_cast<float>(FMath::Max(0, AngerGold)) / static_cast<float>(VendorAngerThresholdGold), 0.f, 1.f);
	const float NewAnger01 = FMath::Clamp(GamblerAnger01 + Delta, 0.f, 1.f);
	const int32 NewVendorAngerGold = FMath::RoundToInt(NewAnger01 * static_cast<float>(VendorAngerThresholdGold));
	if (FMath::IsNearlyEqual(NewAnger01, GamblerAnger01) && VendorAngerGold == NewVendorAngerGold)
	{
		return GamblerAnger01;
	}

	GamblerAnger01 = NewAnger01;
	VendorAngerGold = NewVendorAngerGold;
	GamblerAngerChanged.Broadcast();
	VendorChanged.Broadcast();
	return GamblerAnger01;
}


void UT66RunStateSubsystem::ResetGamblerAnger()
{
	if (FMath::IsNearlyZero(GamblerAnger01) && VendorAngerGold == 0) return;
	GamblerAnger01 = 0.f;
	VendorAngerGold = 0;
	GamblerAngerChanged.Broadcast();
	VendorChanged.Broadcast();
}


void UT66RunStateSubsystem::ResetVendorForStage()
{
	VendorAngerGold = 0;
	VendorStockStage = 0;
	VendorStockItemIDs.Reset();
	VendorStockSlots.Reset();
	VendorStockSold.Reset();
	bVendorBoughtSomethingThisStage = false;
	VendorChanged.Broadcast();
}


void UT66RunStateSubsystem::ResetVendorAnger()
{
	ResetGamblerAnger();
}


void UT66RunStateSubsystem::EnsureVendorStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 23);
	if (VendorStockStage == Stage && VendorStockItemIDs.Num() > 0 && VendorStockSold.Num() == VendorStockItemIDs.Num())
	{
		return;
	}

	// Reset reroll counter and seen-counts when stage changes.
	if (VendorStockRerollStage != Stage)
	{
		VendorStockRerollStage = Stage;
		VendorStockRerollCounter = 0;
		VendorSeenCounts.Reset();
	}

	VendorStockStage = Stage;
	VendorStockItemIDs.Reset();
	VendorStockSold.Reset();
	VendorStockSlots.Reset();

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI)
	{
		// Fallback: keep deterministic placeholder behavior with the live slot count.
		const FT66InventorySlot FallbackStock[] =
		{
			FT66InventorySlot(FName(TEXT("Item_AoeDamage")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_BounceScale")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_DamageReduction")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_CritDamage")), ET66ItemRarity::Red, 5),
			FT66InventorySlot(FName(TEXT("Item_Accuracy")), ET66ItemRarity::Yellow, 8),
		};
		for (const FT66InventorySlot& Slot : FallbackStock)
		{
			VendorStockSlots.Add(Slot);
			VendorStockItemIDs.Add(Slot.ItemTemplateID);
		}
		VendorStockSold.Init(false, VendorStockSlots.Num());
		VendorChanged.Broadcast();
		return;
	}

	// Build pool of all template IDs from the Items DataTable.
	UDataTable* ItemsDT = GI->GetItemsDataTable();
	TArray<FName> TemplatePool;
	if (ItemsDT)
	{
		for (const FName ItemID : ItemsDT->GetRowNames())
		{
			FItemData ItemData;
			if (!GI->GetItemData(ItemID, ItemData))
			{
				continue;
			}

			if (!T66_IsGamblersTokenItem(ItemID) && T66IsLiveSecondaryStatType(ItemData.SecondaryStatType))
			{
				TemplatePool.Add(ItemID);
			}
		}
	}

	{
		FItemData AccuracyItemData;
		const FName AccuracyItemID(TEXT("Item_Accuracy"));
		if (GI->GetItemData(AccuracyItemID, AccuracyItemData)
			&& T66IsLiveSecondaryStatType(AccuracyItemData.SecondaryStatType)
			&& !TemplatePool.Contains(AccuracyItemID))
		{
			TemplatePool.Add(AccuracyItemID);
		}
	}

	if (TemplatePool.Num() == 0)
	{
		TemplatePool =
		{
			FName(TEXT("Item_AoeDamage")),
			FName(TEXT("Item_BounceScale")),
			FName(TEXT("Item_DamageReduction")),
			FName(TEXT("Item_CritDamage")),
			FName(TEXT("Item_Accuracy"))
		};
	}

	// Seed: per-stage and per-reroll, plus run seed so the first shop display is randomized each run.
	int32 Seed = Stage * 777 + 13 + VendorStockRerollCounter * 10007;
	if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
	{
		Seed ^= RngSub->GetRunSeed();
	}
	FRandomStream Rng(Seed);

	// Smart reroll: weight = 1/(1 + SeenCount*Decay), floor 0.05. Build weights and pick unique cards.
	constexpr float DecayFactor = 2.0f;
	constexpr float WeightFloor = 0.05f;
	TArray<float> Weights;
	Weights.SetNumUninitialized(TemplatePool.Num());
	for (int32 TemplateIndex = 0; TemplateIndex < TemplatePool.Num(); ++TemplateIndex)
	{
		const int32 Seen = VendorSeenCounts.FindRef(TemplatePool[TemplateIndex]);
		Weights[TemplateIndex] = FMath::Max(WeightFloor, 1.0f / (1.0f + static_cast<float>(Seen) * DecayFactor));
	}

	const ET66ItemRarity SlotRarities[VendorDisplaySlotCount] =
	{
		ET66ItemRarity::Black,
		ET66ItemRarity::Black,
		ET66ItemRarity::Black,
		ET66ItemRarity::Red,
		ET66ItemRarity::Yellow
	};

	for (int32 SlotIndex = 0; SlotIndex < VendorDisplaySlotCount; ++SlotIndex)
	{
		float TotalWeight = 0.f;
		for (int32 TemplateIndex = 0; TemplateIndex < TemplatePool.Num(); ++TemplateIndex)
		{
			if (!VendorStockItemIDs.Contains(TemplatePool[TemplateIndex]))
			{
				TotalWeight += Weights[TemplateIndex];
			}
		}
		if (TotalWeight <= 0.f)
		{
			TotalWeight = 1.f;
		}

		FName Chosen = NAME_None;
		float Roll = Rng.FRand() * TotalWeight;
		for (int32 TemplateIndex = 0; TemplateIndex < TemplatePool.Num(); ++TemplateIndex)
		{
			if (VendorStockItemIDs.Contains(TemplatePool[TemplateIndex]))
			{
				continue;
			}
			Roll -= Weights[TemplateIndex];
			if (Roll <= 0.f)
			{
				Chosen = TemplatePool[TemplateIndex];
				break;
			}
		}
		if (Chosen.IsNone())
		{
			for (int32 TemplateIndex = 0; TemplateIndex < TemplatePool.Num(); ++TemplateIndex)
			{
				if (!VendorStockItemIDs.Contains(TemplatePool[TemplateIndex]))
				{
					Chosen = TemplatePool[TemplateIndex];
					break;
				}
			}
		}
		if (Chosen.IsNone())
		{
			Chosen = TemplatePool[0];
		}

		VendorSeenCounts.FindOrAdd(Chosen)++;

		const ET66ItemRarity Rarity = SlotRarities[SlotIndex];
		int32 RollMin = 1;
		int32 RollMax = 3;
		FItemData::GetLine1RollRange(Rarity, RollMin, RollMax);
		const int32 Rolled = Rng.RandRange(RollMin, RollMax);

		VendorStockSlots.Add(FT66InventorySlot(Chosen, Rarity, Rolled));
		VendorStockItemIDs.Add(Chosen);
	}

	VendorStockSold.Init(false, VendorStockSlots.Num());
	VendorChanged.Broadcast();
}


void UT66RunStateSubsystem::RerollVendorStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 23);
	if (VendorStockRerollStage != Stage)
	{
		VendorStockRerollStage = Stage;
		VendorStockRerollCounter = 0;
	}
	VendorStockRerollCounter = FMath::Clamp(VendorStockRerollCounter + 1, 0, 9999);

	// Force regeneration even if the stage didn't change.
	VendorStockStage = 0;
	VendorStockItemIDs.Reset();
	VendorStockSold.Reset();
	EnsureVendorStockForCurrentStage();
	// EnsureVendorStockForCurrentStage broadcasts VendorChanged.
}


bool UT66RunStateSubsystem::IsVendorStockSlotSold(int32 Index) const
{
	if (Index < 0 || Index >= VendorStockSold.Num()) return true;
	return VendorStockSold[Index];
}


bool UT66RunStateSubsystem::TryBuyVendorStockSlot(int32 Index)
{
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockSlots.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;
	if (!HasInventorySpace()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& Slot = VendorStockSlots[Index];
	if (!GI || !GI->GetItemData(Slot.ItemTemplateID, D)) return false;
	const int32 BuyPrice = D.GetBuyGoldForRarity(Slot.Rarity);
	if (BuyPrice <= 0) return false;
	if (!TrySpendGold(BuyPrice)) return false;

	AddItemSlot(Slot);
	VendorStockSold[Index] = true;
	bVendorBoughtSomethingThisStage = true;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorPurchase=%s"), *Slot.ItemTemplateID.ToString()));
	VendorChanged.Broadcast();
	return true;
}


bool UT66RunStateSubsystem::ResolveVendorStealAttempt(int32 Index, bool bTimingHit, bool bRngSuccess)
{
	(void)bRngSuccess;
	EnsureVendorStockForCurrentStage();
	if (Index < 0 || Index >= VendorStockSlots.Num()) return false;
	if (IsVendorStockSlotSold(Index)) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& StealSlot = VendorStockSlots[Index];
	if (!GI || !GI->GetItemData(StealSlot.ItemTemplateID, D)) return false;
	const int32 BuyPrice = D.GetBuyGoldForRarity(StealSlot.Rarity);
	if (BuyPrice <= 0) return false;

	// Determine success via player-experience tuning and central luck bias.
	UT66PlayerExperienceSubSystem* PlayerExperience = nullptr;
	UT66RngSubsystem* RngSub = nullptr;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		PlayerExperience = GameInstance->GetSubsystem<UT66PlayerExperienceSubSystem>();
		RngSub = GameInstance->GetSubsystem<UT66RngSubsystem>();
		if (RngSub)
		{
			RngSub->UpdateLuckStat(GetEffectiveLuckBiasStat());
		}
	}

	float BaseChance = 0.f;
	if (bTimingHit)
	{
		const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
		BaseChance = PlayerExperience
			? PlayerExperience->GetDifficultyVendorStealSuccessChanceOnTimingHitBase(Difficulty)
			: 0.65f;
	}
	BaseChance = FMath::Clamp(BaseChance, 0.f, 1.f);

	bool bSuccess = false;
	float AppliedChance = 0.f;
	int32 DrawIndex = INDEX_NONE;
	int32 PreDrawSeed = 0;
	if (bTimingHit && BaseChance > 0.f)
	{
		const float Chance = RngSub ? RngSub->BiasChance01(BaseChance) : BaseChance;
		AppliedChance = Chance;
		FRandomStream Local(FPlatformTime::Cycles());
		FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : Local;
		if (RngSub)
		{
			bSuccess = RngSub->RollChance01(Chance);
			DrawIndex = RngSub->GetLastRunDrawIndex();
			PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
		}
		else
		{
			bSuccess = (Stream.GetFraction() < Chance);
		}
	}

	LastVendorStealOutcome = ET66VendorStealOutcome::None;
	if (!bTimingHit)
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::Miss;
	}
	else if (!bSuccess)
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::Failed;
	}
	else if (!HasInventorySpace())
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::InventoryFull;
	}
	else
	{
		LastVendorStealOutcome = ET66VendorStealOutcome::Success;
	}

	bool bGranted = false;
	if (LastVendorStealOutcome == ET66VendorStealOutcome::Success)
	{
		AddItemSlot(StealSlot);
		VendorStockSold[Index] = true;
		AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("VendorSteal=%s"), *StealSlot.ItemTemplateID.ToString()));
		bGranted = true;
		// Success: no anger increase.
	}
	else
	{
		// Failure: anger increases and the item is not granted.
		AddCasinoAngerFromGold(BuyPrice);
	}

	// Luck Rating tracking (quantity): vendor steal success means item granted with no anger increase.
	RecordLuckQuantityBool(
		FName(TEXT("VendorStealSuccess")),
		(LastVendorStealOutcome == ET66VendorStealOutcome::Success),
		AppliedChance,
		DrawIndex,
		PreDrawSeed);

	if (LastVendorStealOutcome == ET66VendorStealOutcome::Success)
	{
		VendorChanged.Broadcast();
	}
	return bGranted;
}


void UT66RunStateSubsystem::GenerateBuybackDisplay()
{
	BuybackDisplaySlots.SetNum(BuybackDisplaySlotCount);
	const int32 PoolNum = BuybackPool.Num();
	const int32 MaxPage = PoolNum > 0 ? FMath::Max(0, (PoolNum + BuybackDisplaySlotCount - 1) / BuybackDisplaySlotCount - 1) : 0;
	BuybackDisplayPage = FMath::Clamp(BuybackDisplayPage, 0, MaxPage);
	const int32 Start = BuybackDisplayPage * BuybackDisplaySlotCount;
	for (int32 i = 0; i < BuybackDisplaySlotCount; ++i)
	{
		const int32 Idx = Start + i;
		if (Idx < PoolNum)
		{
			BuybackDisplaySlots[i] = BuybackPool[Idx];
		}
		else
		{
			BuybackDisplaySlots[i] = FT66InventorySlot();
		}
	}
	BuybackChanged.Broadcast();
}


void UT66RunStateSubsystem::RerollBuybackDisplay()
{
	const int32 PoolNum = BuybackPool.Num();
	const int32 MaxPage = PoolNum > 0 ? FMath::Max(0, (PoolNum + BuybackDisplaySlotCount - 1) / BuybackDisplaySlotCount - 1) : 0;
	if (MaxPage > 0)
	{
		BuybackDisplayPage = (BuybackDisplayPage + 1) % (MaxPage + 1);
	}
	GenerateBuybackDisplay();
}


bool UT66RunStateSubsystem::TryBuybackSlot(int32 DisplayIndex)
{
	if (DisplayIndex < 0 || DisplayIndex >= BuybackDisplaySlotCount) return false;
	if (!HasInventorySpace()) return false;

	const int32 PoolIndex = BuybackDisplayPage * BuybackDisplaySlotCount + DisplayIndex;
	if (PoolIndex < 0 || PoolIndex >= BuybackPool.Num()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI) return false;

	const FT66InventorySlot Slot = BuybackPool[PoolIndex];
	if (!Slot.IsValid()) return false;

	FItemData ItemData;
	int32 BuyPrice = 0;
	if (GI->GetItemData(Slot.ItemTemplateID, ItemData))
	{
		BuyPrice = GetSellGoldForInventorySlot(Slot);
	}
	if (BuyPrice <= 0) BuyPrice = 1;
	if (CurrentGold < BuyPrice) return false;

	CurrentGold -= BuyPrice;
	BuybackPool.RemoveAt(PoolIndex);
	AddItemSlot(Slot);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=-%d,Source=Buyback,ItemID=%s"), BuyPrice, *Slot.ItemTemplateID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	GenerateBuybackDisplay();
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


int32 UT66RunStateSubsystem::GetInventorySellValueTotal() const
{
	int32 TotalSellValue = 0;
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		TotalSellValue += GetSellGoldForInventorySlot(Slot);
	}

	return TotalSellValue;
}


int32 UT66RunStateSubsystem::GetNetWorth() const
{
	return CurrentGold + GetInventorySellValueTotal() - CurrentDebt;
}


int32 UT66RunStateSubsystem::GetRemainingBorrowCapacity() const
{
	return FMath::Max(0, GetNetWorth() - CurrentDebt);
}


bool UT66RunStateSubsystem::CanBorrowGold(int32 Amount) const
{
	return Amount > 0 && Amount <= GetRemainingBorrowCapacity();
}


bool UT66RunStateSubsystem::BorrowGold(int32 Amount)
{
	if (!CanBorrowGold(Amount))
	{
		return false;
	}

	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	CurrentDebt = FMath::Max(0, CurrentDebt + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Borrow"), Amount));
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
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


TArray<FName> UT66RunStateSubsystem::GetInventory() const
{
	TArray<FName> Result;
	Result.Reserve(InventorySlots.Num());
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		Result.Add(Slot.ItemTemplateID);
	}
	return Result;
}


void UT66RunStateSubsystem::AddItem(FName ItemID)
{
	if (T66_IsGamblersTokenItem(ItemID))
	{
		ApplyGamblersTokenPickup(1);
		return;
	}

	AddItemWithRarity(ItemID, ET66ItemRarity::Black);
}


void UT66RunStateSubsystem::AddItemWithRarity(FName ItemID, ET66ItemRarity Rarity)
{
	if (ItemID.IsNone()) return;
	if (T66_IsGamblersTokenItem(ItemID))
	{
		ApplyGamblersTokenPickup(1);
		return;
	}

	// Auto-generate the Line 1 roll for the provided rarity.
	int32 RolledMin = 1, RolledMax = 3;
	FItemData::GetLine1RollRange(Rarity, RolledMin, RolledMax);
	UT66RngSubsystem* RngSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
	FRandomStream Local(FPlatformTime::Cycles());
	FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : Local;
	const int32 RolledValue = Stream.RandRange(RolledMin, RolledMax);
	const int32 RollSeed = Stream.RandRange(1, MAX_int32 / 4);

	FT66InventorySlot Slot(ItemID, Rarity, RolledValue, 0.f, 0, RollSeed);
	AddItemSlot(Slot);
}


void UT66RunStateSubsystem::AddItemSlot(const FT66InventorySlot& Slot)
{
	if (!Slot.IsValid()) return;
	if (T66_IsGamblersTokenItem(Slot.ItemTemplateID))
	{
		ApplyGamblersTokenPickup(Slot.Line1RolledValue);
		return;
	}

	FT66InventorySlot NormalizedSlot = Slot;
	if (NormalizedSlot.Line1RolledValue <= 0)
	{
		int32 RolledMin = 1;
		int32 RolledMax = 1;
		FItemData::GetLine1RollRange(NormalizedSlot.Rarity, RolledMin, RolledMax);
		NormalizedSlot.Line1RolledValue = RolledMax;
	}
	if (NormalizedSlot.RollSeed == 0)
	{
		NormalizedSlot.RollSeed = T66_GetDefaultInventoryRollSeed();
	}

	InventorySlots.Add(NormalizedSlot);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=LootBag"), *NormalizedSlot.ItemTemplateID.ToString()));
	// Lab unlock: mark item as unlocked for The Lab (any run type including Lab).
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->AddLabUnlockedItem(NormalizedSlot.ItemTemplateID);
		}
	}
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}


void UT66RunStateSubsystem::ApplyGamblersTokenPickup(int32 TokenLevel)
{
	const int32 ClampedLevel = FMath::Clamp(TokenLevel, 1, MaxGamblersTokenLevel);
	if (ClampedLevel <= 0 || ActiveGamblersTokenLevel >= ClampedLevel)
	{
		return;
	}

	ActiveGamblersTokenLevel = ClampedLevel;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=GamblerToken,Level=%d"), *T66GamblersTokenItemID.ToString(), ActiveGamblersTokenLevel));

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->AddLabUnlockedItem(T66GamblersTokenItemID);
		}
	}

	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}


float UT66RunStateSubsystem::GetCurrentSellFraction() const
{
	return T66_GetSellFractionForTokenLevel(ActiveGamblersTokenLevel);
}


int32 UT66RunStateSubsystem::GetSellGoldForInventorySlot(const FT66InventorySlot& Slot) const
{
	if (!Slot.IsValid() || T66_IsGamblersTokenItem(Slot.ItemTemplateID))
	{
		return 0;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI)
	{
		return 0;
	}

	FItemData ItemData;
	if (!GI->GetItemData(Slot.ItemTemplateID, ItemData))
	{
		return 0;
	}

	const int32 BuyGold = ItemData.GetBuyGoldForRarity(Slot.Rarity);
	return FMath::Max(0, FMath::RoundToInt(static_cast<float>(BuyGold) * GetCurrentSellFraction()));
}


void UT66RunStateSubsystem::ClearInventory()
{
	InventorySlots.Empty();
	ActiveGamblersTokenLevel = 0;
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();
}


bool UT66RunStateSubsystem::SellFirstItem()
{
	if (InventorySlots.Num() == 0) return false;
	return SellInventoryItemAt(0);
}


bool UT66RunStateSubsystem::SellInventoryItemAt(int32 InventoryIndex)
{
	if (InventoryIndex < 0 || InventoryIndex >= InventorySlots.Num()) return false;

	const FT66InventorySlot Slot = InventorySlots[InventoryIndex];
	const int32 SellGold = GetSellGoldForInventorySlot(Slot);

	CurrentGold += SellGold;
	BuybackPool.Add(Slot);
	InventorySlots.RemoveAt(InventoryIndex);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Vendor,ItemID=%s"), SellGold, *Slot.ItemTemplateID.ToString()));
	GoldChanged.Broadcast();
	InventoryChanged.Broadcast();
	BuybackChanged.Broadcast();
	LogAdded.Broadcast();
	return true;
}


ET66ItemRarity UT66RunStateSubsystem::GetNextItemRarity(ET66ItemRarity Rarity)
{
	switch (Rarity)
	{
	case ET66ItemRarity::Black:  return ET66ItemRarity::Red;
	case ET66ItemRarity::Red:    return ET66ItemRarity::Yellow;
	case ET66ItemRarity::Yellow: return ET66ItemRarity::White;
	case ET66ItemRarity::White:  return ET66ItemRarity::White;
	default:                     return ET66ItemRarity::Black;
	}
}


bool UT66RunStateSubsystem::CanAlchemyUpgradeInventoryItemAt(const int32 InventoryIndex) const
{
	FT66InventorySlot PreviewSlot;
	int32 MatchingCount = 0;
	return GetAlchemyUpgradePreviewAt(InventoryIndex, PreviewSlot, MatchingCount);
}


int32 UT66RunStateSubsystem::GetAlchemyMatchingInventoryCount(const int32 InventoryIndex) const
{
	if (!InventorySlots.IsValidIndex(InventoryIndex))
	{
		return 0;
	}

	const UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	const FT66InventorySlot& TargetSlot = InventorySlots[InventoryIndex];
	if (!T66_IsAlchemyEligibleSlot(TargetSlot, GI))
	{
		return 0;
	}

	int32 MatchingCount = 0;
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		if (T66_IsAlchemyMatch(TargetSlot, Slot))
		{
			++MatchingCount;
		}
	}

	return MatchingCount;
}


bool UT66RunStateSubsystem::GetAlchemyUpgradePreviewAt(const int32 InventoryIndex, FT66InventorySlot& OutUpgradedSlot, int32& OutMatchingCount) const
{
	OutUpgradedSlot = FT66InventorySlot();
	OutMatchingCount = GetAlchemyMatchingInventoryCount(InventoryIndex);
	if (!InventorySlots.IsValidIndex(InventoryIndex))
	{
		return false;
	}

	const UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	const FT66InventorySlot& TargetSlot = InventorySlots[InventoryIndex];
	if (!T66_IsAlchemyEligibleSlot(TargetSlot, GI) || OutMatchingCount < AlchemyCopiesRequired)
	{
		return false;
	}

	const TArray<int32> SourceIndices = T66_GatherAlchemySourceIndices(InventorySlots, InventoryIndex);
	if (SourceIndices.Num() < AlchemyCopiesRequired)
	{
		return false;
	}

	TArray<FT66InventorySlot> SourceSlots;
	SourceSlots.Reserve(SourceIndices.Num());
	for (const int32 SourceIndex : SourceIndices)
	{
		SourceSlots.Add(InventorySlots[SourceIndex]);
	}

	OutUpgradedSlot = T66_BuildAlchemyUpgradeSlot(TargetSlot, SourceSlots);
	return true;
}


bool UT66RunStateSubsystem::TryAlchemyUpgradeInventoryItems(int32 TargetIndex, int32 SacrificeIndex, FT66InventorySlot& OutUpgradedSlot)
{
	OutUpgradedSlot = FT66InventorySlot();
	(void)SacrificeIndex;

	if (TargetIndex < 0 || TargetIndex >= InventorySlots.Num())
	{
		return false;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	const FT66InventorySlot OriginalTargetSlot = InventorySlots[TargetIndex];
	if (!OriginalTargetSlot.IsValid())
	{
		return false;
	}

	int32 MatchingCount = 0;
	FT66InventorySlot UpgradedSlot;
	if (!GetAlchemyUpgradePreviewAt(TargetIndex, UpgradedSlot, MatchingCount))
	{
		return false;
	}

	const TArray<int32> SourceIndices = T66_GatherAlchemySourceIndices(InventorySlots, TargetIndex);
	if (SourceIndices.Num() < AlchemyCopiesRequired)
	{
		return false;
	}

	const float LuckyAlchemyChance = GetAlchemyLuckyUpgradeChance01();
	if (LuckyAlchemyChance > 0.f)
	{
		UT66RngSubsystem* RngSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
		const bool bLuckyAlchemy = RngSub
			? RngSub->RollChance01(LuckyAlchemyChance)
			: (FMath::FRand() < LuckyAlchemyChance);
		RecordLuckQuantityBool(
			FName(TEXT("LuckyAlchemySuccess")),
			bLuckyAlchemy,
			LuckyAlchemyChance,
			RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
			RngSub ? RngSub->GetLastRunPreDrawSeed() : 0);
		if (bLuckyAlchemy)
		{
			T66_ApplyLuckyAlchemyBonus(UpgradedSlot);
		}
	}

	int32 AngerGold = 0;
	if (GI)
	{
		for (const int32 SourceIndex : SourceIndices)
		{
			if (!InventorySlots.IsValidIndex(SourceIndex))
			{
				continue;
			}

			FItemData SourceItemData;
			if (GI->GetItemData(InventorySlots[SourceIndex].ItemTemplateID, SourceItemData))
			{
				AngerGold += FMath::Max(1, SourceItemData.GetBuyGoldForRarity(InventorySlots[SourceIndex].Rarity));
			}
		}
	}

	int32 InsertIndex = TargetIndex;
	for (const int32 SourceIndex : SourceIndices)
	{
		if (SourceIndex < TargetIndex)
		{
			--InsertIndex;
		}
	}

	TArray<int32> RemovalIndices = SourceIndices;
	RemovalIndices.Sort([](const int32 A, const int32 B) { return A > B; });
	for (const int32 RemovalIndex : RemovalIndices)
	{
		InventorySlots.RemoveAt(RemovalIndex);
	}
	InsertIndex = FMath::Clamp(InsertIndex, 0, InventorySlots.Num());
	InventorySlots.Insert(UpgradedSlot, InsertIndex);

	RecomputeItemDerivedStats();
	AddCasinoAngerFromGold(AngerGold);
	AddStructuredEvent(
		ET66RunEventType::ItemAcquired,
		FString::Printf(
			TEXT("ItemID=%s,Source=Alchemy,Copies=%d,FromRarity=%d,ToRarity=%d"),
			*UpgradedSlot.ItemTemplateID.ToString(),
			AlchemyCopiesRequired,
			static_cast<int32>(OriginalTargetSlot.Rarity),
			static_cast<int32>(UpgradedSlot.Rarity)));
	InventoryChanged.Broadcast();
	LogAdded.Broadcast();

	OutUpgradedSlot = UpgradedSlot;
	return true;
}


void UT66RunStateSubsystem::RecomputeItemDerivedStats()
{
	// Reset all accumulators.
	ItemStatBonuses = FT66HeroStatBonuses{};
	ItemPrimaryStatBonusesPrecise = FT66HeroPreciseStatBlock{};
	ItemPowerGivenPercent = 0.f;
	BonusDamagePercent = 0.f;
	BonusAttackSpeedPercent = 0.f;
	DashCooldownReductionPercent = 0.f;
	ItemDamageMultiplier = 1.f;
	ItemAttackSpeedMultiplier = 1.f;
	DashCooldownMultiplier = 1.f;
	ItemScaleMultiplier = 1.f;
	ItemMoveSpeedMultiplier = 1.f;
	ItemArmorBonus01 = 0.f;
	ItemEvasionBonus01 = 0.f;
	ItemBonusLuckFlat = 0;
	SecondaryMultipliers.Reset();
	ItemSecondaryStatBonusTenths.Reset();

	auto AddPrimaryBonusTenths = [&](ET66HeroStatType Type, int32 DeltaTenths)
	{
		const int32 V = FMath::Clamp(DeltaTenths, 0, MaxHeroStatValue * HeroStatTenthsScale);
		if (V <= 0)
		{
			return;
		}

		switch (Type)
		{
		case ET66HeroStatType::Damage:      ItemPrimaryStatBonusesPrecise.DamageTenths += V; break;
		case ET66HeroStatType::AttackSpeed: ItemPrimaryStatBonusesPrecise.AttackSpeedTenths += V; break;
		case ET66HeroStatType::AttackScale: ItemPrimaryStatBonusesPrecise.AttackScaleTenths += V; break;
		case ET66HeroStatType::Accuracy:    ItemPrimaryStatBonusesPrecise.AccuracyTenths += V; break;
		case ET66HeroStatType::Armor:       ItemPrimaryStatBonusesPrecise.ArmorTenths += V; break;
		case ET66HeroStatType::Evasion:     ItemPrimaryStatBonusesPrecise.EvasionTenths += V; break;
		case ET66HeroStatType::Luck:        ItemPrimaryStatBonusesPrecise.LuckTenths += V; break;
		case ET66HeroStatType::Speed:       ItemPrimaryStatBonusesPrecise.SpeedTenths += V; break;
		default: break;
		}
	};

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		if (!Slot.IsValid()) continue;

		FItemData D;
		const bool bHasRow = (GI && GI->GetItemData(Slot.ItemTemplateID, D));
		if (!bHasRow) continue;
		if (T66_IsGamblersTokenItem(Slot.ItemTemplateID) || D.SecondaryStatType == ET66SecondaryStatType::GamblerToken)
		{
			continue;
		}

		// Line 1: Additive flat bonus to primary stat.
		const int32 PrimaryGainTenths = WholeStatToTenths(FMath::Max(0, Slot.Line1RolledValue));
		AddPrimaryBonusTenths(D.PrimaryStatType, PrimaryGainTenths);
		ApplyPrimaryGainToSecondaryBonuses(
			D.PrimaryStatType,
			PrimaryGainTenths,
			ItemSecondaryStatBonusTenths,
			T66_CombineInventorySeed(Slot, static_cast<int32>(D.PrimaryStatType)));

		// Line 2: flat secondary stat bonus on the item's tagged secondary.
		if (D.SecondaryStatType != ET66SecondaryStatType::None)
		{
			AddItemSecondaryStatBonusTenths(D.SecondaryStatType, WholeStatToTenths(Slot.GetSecondaryStatBonusValue()));
		}
	}

	ItemStatBonuses.Damage = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.DamageTenths);
	ItemStatBonuses.AttackSpeed = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.AttackSpeedTenths);
	ItemStatBonuses.AttackScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.AttackScaleTenths);
	ItemStatBonuses.Accuracy = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.AccuracyTenths);
	ItemStatBonuses.Armor = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.ArmorTenths);
	ItemStatBonuses.Evasion = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.EvasionTenths);
	ItemStatBonuses.Luck = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemPrimaryStatBonusesPrecise.LuckTenths);

	ItemStatBonuses.PierceDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::PierceDamage));
	ItemStatBonuses.PierceAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::PierceSpeed));
	ItemStatBonuses.PierceAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::PierceScale));
	ItemStatBonuses.BounceDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::BounceDamage));
	ItemStatBonuses.BounceAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::BounceSpeed));
	ItemStatBonuses.BounceAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::BounceScale));
	ItemStatBonuses.AoeDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::AoeDamage));
	ItemStatBonuses.AoeAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::AoeSpeed));
	ItemStatBonuses.AoeAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::AoeScale));
	ItemStatBonuses.DotDmg = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::DotDamage));
	ItemStatBonuses.DotAtkSpd = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::DotSpeed));
	ItemStatBonuses.DotAtkScale = FT66HeroPreciseStatBlock::TenthsToDisplayStat(ItemSecondaryStatBonusTenths.FindRef(ET66SecondaryStatType::DotScale));
}
