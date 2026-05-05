// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniCircusSubsystem.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/GameInstance.h"
#include "Save/T66MiniRunSaveGame.h"

namespace
{
	int32 T66MiniCountOwnedItemsWithStat(const UT66MiniRunSaveGame* RunSave, const UT66MiniDataSubsystem* DataSubsystem, const FString& StatType)
	{
		if (!RunSave || !DataSubsystem)
		{
			return 0;
		}

		int32 Count = 0;
		for (const FName ItemID : RunSave->OwnedItemIDs)
		{
			if (const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(ItemID))
			{
				if (ItemDefinition->SecondaryStatType == StatType)
				{
					++Count;
				}
			}
		}

		return Count;
	}

	float T66MiniCircusTuning(const UT66MiniDataSubsystem* DataSubsystem, const TCHAR* Key, const float DefaultValue)
	{
		return DataSubsystem ? DataSubsystem->FindRuntimeTuningValue(FName(Key), DefaultValue) : DefaultValue;
	}

	int32 T66MiniCircusTuningInt(const UT66MiniDataSubsystem* DataSubsystem, const TCHAR* Key, const int32 DefaultValue)
	{
		return FMath::RoundToInt(T66MiniCircusTuning(DataSubsystem, Key, static_cast<float>(DefaultValue)));
	}
}

void UT66MiniCircusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetCircusState();
}

void UT66MiniCircusSubsystem::ResetCircusState()
{
	CurrentVendorOfferIDs.Reset();
	LockedVendorOfferIDs.Reset();
	BuybackItemIDs.Reset();
	CircusDebt = 0;
	CircusAnger01 = 0.f;
	VendorRerollCount = 0;
}

void UT66MiniCircusSubsystem::SeedFromRunSave(const UT66MiniRunSaveGame* RunSave)
{
	ResetCircusState();
	if (!RunSave)
	{
		return;
	}

	CurrentVendorOfferIDs = RunSave->CurrentShopOfferIDs;
	LockedVendorOfferIDs = RunSave->LockedShopOfferIDs;
	BuybackItemIDs = RunSave->CircusBuybackItemIDs;
	CircusDebt = FMath::Max(0, RunSave->CircusDebt);
	CircusAnger01 = FMath::Clamp(RunSave->CircusAnger01, 0.f, 1.f);
	VendorRerollCount = FMath::Max(0, RunSave->CircusVendorRerollCount);
}

void UT66MiniCircusSubsystem::WriteToRunSave(UT66MiniRunSaveGame* RunSave) const
{
	if (!RunSave)
	{
		return;
	}

	RunSave->CurrentShopOfferIDs = CurrentVendorOfferIDs;
	RunSave->LockedShopOfferIDs = LockedVendorOfferIDs;
	RunSave->ShopRerollCount = VendorRerollCount;
	RunSave->CircusBuybackItemIDs = BuybackItemIDs;
	RunSave->CircusDebt = CircusDebt;
	RunSave->CircusAnger01 = CircusAnger01;
	RunSave->CircusVendorRerollCount = VendorRerollCount;
}

void UT66MiniCircusSubsystem::PrimeVendorOffers(const UT66MiniDataSubsystem* DataSubsystem)
{
	if (CurrentVendorOfferIDs.Num() == 0)
	{
		GenerateVendorOffers(DataSubsystem, false);
	}
}

void UT66MiniCircusSubsystem::ToggleVendorOfferLock(const FName ItemID)
{
	if (ItemID == NAME_None || !CurrentVendorOfferIDs.Contains(ItemID))
	{
		return;
	}

	if (LockedVendorOfferIDs.Contains(ItemID))
	{
		LockedVendorOfferIDs.Remove(ItemID);
	}
	else
	{
		LockedVendorOfferIDs.Add(ItemID);
	}
}

bool UT66MiniCircusSubsystem::IsVendorOfferLocked(const FName ItemID) const
{
	return LockedVendorOfferIDs.Contains(ItemID);
}

bool UT66MiniCircusSubsystem::TryBuyOffer(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, const FName ItemID, FString& OutResult)
{
	const FT66MiniItemDefinition* ItemDefinition = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!ActiveRun || !ItemDefinition)
	{
		OutResult = TEXT("Mini circus purchase failed because the run state is unavailable.");
		return false;
	}

	if (ActiveRun->Gold < ItemDefinition->BaseBuyGold)
	{
		OutResult = TEXT("Not enough gold for that circus purchase.");
		return false;
	}

	ActiveRun->Gold -= ItemDefinition->BaseBuyGold;
	ActiveRun->OwnedItemIDs.Add(ItemID);
	CurrentVendorOfferIDs.Remove(ItemID);
	LockedVendorOfferIDs.Remove(ItemID);
	GenerateVendorOffers(DataSubsystem, false);
	OutResult = FString::Printf(TEXT("%s purchased from the mini circus."), *ItemID.ToString());
	return true;
}

bool UT66MiniCircusSubsystem::TryStealOffer(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, const FName ItemID, FString& OutResult)
{
	const FT66MiniItemDefinition* ItemDefinition = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!ActiveRun || !ItemDefinition)
	{
		OutResult = TEXT("The steal attempt failed because the mini circus state is unavailable.");
		return false;
	}

	const int32 StealSupport = CalculateStealSupportBonus(ActiveRun, DataSubsystem);
	const float SuccessChance = FMath::Clamp(
		T66MiniCircusTuning(DataSubsystem, TEXT("StealBaseChance"), 0.18f)
			+ (StealSupport * T66MiniCircusTuning(DataSubsystem, TEXT("StealSupportChance"), 0.05f))
			- (CircusAnger01 * T66MiniCircusTuning(DataSubsystem, TEXT("StealAngerPenalty"), 0.22f)),
		T66MiniCircusTuning(DataSubsystem, TEXT("StealMinChance"), 0.08f),
		T66MiniCircusTuning(DataSubsystem, TEXT("StealMaxChance"), 0.72f));
	const bool bSuccess = FMath::FRand() <= SuccessChance;
	AddAnger(bSuccess ? T66MiniCircusTuning(DataSubsystem, TEXT("StealSuccessAnger"), 0.16f) : T66MiniCircusTuning(DataSubsystem, TEXT("StealFailAnger"), 0.30f));

	if (bSuccess)
	{
		ActiveRun->OwnedItemIDs.Add(ItemID);
		CurrentVendorOfferIDs.Remove(ItemID);
		LockedVendorOfferIDs.Remove(ItemID);
		GenerateVendorOffers(DataSubsystem, false);
		OutResult = FString::Printf(TEXT("Steal succeeded. %s was lifted cleanly."), *ItemID.ToString());
	}
	else
	{
		CircusDebt += FMath::Max(
			T66MiniCircusTuningInt(DataSubsystem, TEXT("StealFailDebtMin"), 8),
			ItemDefinition->BaseSellGold + T66MiniCircusTuningInt(DataSubsystem, TEXT("StealFailDebtFlat"), 6));
		OutResult = FString::Printf(TEXT("Steal failed. The circus marked you and added debt."));
	}

	HandleBacklash(ActiveRun, OutResult);
	return bSuccess;
}

bool UT66MiniCircusSubsystem::TrySellOwnedItem(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, const FName ItemID, FString& OutResult)
{
	const FT66MiniItemDefinition* ItemDefinition = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!ActiveRun || !ItemDefinition)
	{
		OutResult = TEXT("Could not sell that item.");
		return false;
	}

	if (ActiveRun->OwnedItemIDs.RemoveSingle(ItemID) <= 0)
	{
		OutResult = TEXT("That item is not currently owned.");
		return false;
	}

	ActiveRun->Gold += ItemDefinition->BaseSellGold;
	BuybackItemIDs.Insert(ItemID, 0);
	OutResult = FString::Printf(TEXT("%s sold for %d gold and moved to buyback."), *ItemID.ToString(), ItemDefinition->BaseSellGold);
	return true;
}

bool UT66MiniCircusSubsystem::TryBuybackItem(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, const FName ItemID, FString& OutResult)
{
	const FT66MiniItemDefinition* ItemDefinition = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!ActiveRun || !ItemDefinition || !BuybackItemIDs.Contains(ItemID))
	{
		OutResult = TEXT("That buyback item is no longer available.");
		return false;
	}

	const int32 BuybackCost = FMath::Max(T66MiniCircusTuningInt(DataSubsystem, TEXT("BuybackMinCost"), 1), ItemDefinition->BaseSellGold);
	if (ActiveRun->Gold < BuybackCost)
	{
		OutResult = TEXT("Not enough gold to buy back that item.");
		return false;
	}

	ActiveRun->Gold -= BuybackCost;
	ActiveRun->OwnedItemIDs.Add(ItemID);
	BuybackItemIDs.RemoveSingle(ItemID);
	OutResult = FString::Printf(TEXT("%s bought back for %d gold."), *ItemID.ToString(), BuybackCost);
	return true;
}

bool UT66MiniCircusSubsystem::TryRerollVendor(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult)
{
	if (!ActiveRun || !DataSubsystem)
	{
		OutResult = TEXT("The mini circus reroll is unavailable.");
		return false;
	}

	const int32 RerollCost = T66MiniCircusTuningInt(DataSubsystem, TEXT("VendorRerollBaseCost"), 12)
		+ (VendorRerollCount * T66MiniCircusTuningInt(DataSubsystem, TEXT("VendorRerollCostPerReroll"), 6));
	if (ActiveRun->Gold < RerollCost)
	{
		OutResult = TEXT("Not enough gold to reroll the circus vendor.");
		return false;
	}

	ActiveRun->Gold -= RerollCost;
	AddAnger(T66MiniCircusTuning(DataSubsystem, TEXT("VendorRerollAnger"), 0.08f));
	GenerateVendorOffers(DataSubsystem, true);
	OutResult = FString::Printf(TEXT("Circus vendor rerolled for %d gold."), RerollCost);
	HandleBacklash(ActiveRun, OutResult);
	return true;
}

bool UT66MiniCircusSubsystem::TryBorrowGold(UT66MiniRunSaveGame* ActiveRun, const int32 Amount, FString& OutResult)
{
	if (!ActiveRun || Amount <= 0)
	{
		OutResult = TEXT("Borrowing is unavailable.");
		return false;
	}

	ActiveRun->Gold += Amount;
	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	CircusDebt += Amount + FMath::Max(
		T66MiniCircusTuningInt(DataSubsystem, TEXT("BorrowDebtFlatMin"), 6),
		Amount / FMath::Max(1, T66MiniCircusTuningInt(DataSubsystem, TEXT("BorrowDebtDivisor"), 4)));
	AddAnger(T66MiniCircusTuning(DataSubsystem, TEXT("BorrowAnger"), 0.05f));
	OutResult = FString::Printf(TEXT("Borrowed %d gold. The circus ledger got heavier."), Amount);
	return true;
}

bool UT66MiniCircusSubsystem::TryPayDebt(UT66MiniRunSaveGame* ActiveRun, const int32 Amount, FString& OutResult)
{
	if (!ActiveRun || CircusDebt <= 0)
	{
		OutResult = TEXT("No circus debt to pay.");
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const int32 Payment = FMath::Min3(FMath::Max(T66MiniCircusTuningInt(DataSubsystem, TEXT("PayDebtMinPayment"), 1), Amount), ActiveRun->Gold, CircusDebt);
	if (Payment <= 0)
	{
		OutResult = TEXT("Not enough gold to pay down debt.");
		return false;
	}

	ActiveRun->Gold -= Payment;
	CircusDebt -= Payment;
	CircusAnger01 = FMath::Max(0.f, CircusAnger01 - T66MiniCircusTuning(DataSubsystem, TEXT("PayDebtAngerReduction"), 0.08f));
	OutResult = FString::Printf(TEXT("Paid %d gold toward circus debt."), Payment);
	return true;
}

bool UT66MiniCircusSubsystem::TryPlayGame(const FName GameID, UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult)
{
	const FT66MiniCircusGameDefinition* GameDefinition = DataSubsystem ? DataSubsystem->FindCircusGame(GameID) : nullptr;
	if (!ActiveRun || !GameDefinition)
	{
		OutResult = TEXT("The mini circus game is unavailable.");
		return false;
	}

	auto SpendBet = [&OutResult, ActiveRun](const int32 Bet) -> bool
	{
		if (ActiveRun->Gold < Bet)
		{
			OutResult = TEXT("Not enough gold to make that bet.");
			return false;
		}

		ActiveRun->Gold -= Bet;
		return true;
	};

	const int32 Bet = FMath::Max(0, GameDefinition->Bet);
	if (GameID == FName(TEXT("CoinFlip")))
	{
		if (!SpendBet(Bet))
		{
			return false;
		}

		AddAnger(GameDefinition->AngerAdd);
		if (FMath::FRand() < GameDefinition->SuccessChance)
		{
			ActiveRun->Gold += FMath::RoundToInt(Bet * FMath::Max(0.f, GameDefinition->PayoutMultiplier));
			OutResult = TEXT("Coin Flip won. The circus paid double.");
		}
		else
		{
			OutResult = TEXT("Coin Flip lost. The circus kept the bet.");
		}
	}
	else if (GameID == FName(TEXT("RockPaperScissors")))
	{
		if (!SpendBet(Bet))
		{
			return false;
		}

		AddAnger(GameDefinition->AngerAdd);
		const float Roll = FMath::FRand();
		if (Roll < GameDefinition->SuccessChance)
		{
			ActiveRun->Gold += GameDefinition->FlatPayout;
			OutResult = TEXT("Rock Paper Scissors won. The demon hand folded.");
		}
		else if (Roll < GameDefinition->SuccessChance + GameDefinition->PushChance)
		{
			ActiveRun->Gold += Bet;
			OutResult = TEXT("Rock Paper Scissors drew. Bet returned.");
		}
		else
		{
			OutResult = TEXT("Rock Paper Scissors lost.");
		}
	}
	else if (GameID == FName(TEXT("BlackJack")))
	{
		if (!SpendBet(Bet))
		{
			return false;
		}

		AddAnger(GameDefinition->AngerAdd);
		const int32 PlayerTotal = FMath::RandRange(15, 23);
		const int32 DealerTotal = FMath::RandRange(14, 22);
		if ((PlayerTotal <= 21 && PlayerTotal > DealerTotal) || DealerTotal > 21)
		{
			ActiveRun->Gold += FMath::RoundToInt(Bet * FMath::Max(0.f, GameDefinition->PayoutMultiplier));
			OutResult = FString::Printf(TEXT("Black Jack won. %d beat %d."), PlayerTotal, DealerTotal);
		}
		else if (PlayerTotal == DealerTotal)
		{
			ActiveRun->Gold += Bet;
			OutResult = FString::Printf(TEXT("Black Jack pushed. %d to %d."), PlayerTotal, DealerTotal);
		}
		else
		{
			OutResult = FString::Printf(TEXT("Black Jack lost. %d against %d."), PlayerTotal, DealerTotal);
		}
	}
	else if (GameID == FName(TEXT("Lottery")))
	{
		if (!SpendBet(Bet))
		{
			return false;
		}

		AddAnger(GameDefinition->AngerAdd);
		if (FMath::FRand() < GameDefinition->SuccessChance)
		{
			ActiveRun->Gold += GameDefinition->FlatPayout;
			OutResult = TEXT("Lottery hit. The house coughed up a jackpot.");
		}
		else
		{
			OutResult = TEXT("Lottery missed.");
		}
	}
	else if (GameID == FName(TEXT("Plinko")))
	{
		if (!SpendBet(Bet))
		{
			return false;
		}

		AddAnger(GameDefinition->AngerAdd);
		const float Roll = FMath::FRand();
		const float Multiplier =
			Roll < T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoMissThreshold"), 0.18f)
			? 0.0f
			: (Roll < T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoLowThreshold"), 0.48f)
				? T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoLowMultiplier"), 0.5f)
				: (Roll < T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoMidThreshold"), 0.78f)
					? T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoMidMultiplier"), 1.2f)
					: (Roll < T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoHighThreshold"), 0.94f)
						? T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoHighMultiplier"), 2.2f)
						: T66MiniCircusTuning(DataSubsystem, TEXT("PlinkoJackpotMultiplier"), 4.5f))));
		const int32 Payout = FMath::RoundToInt(Bet * Multiplier);
		ActiveRun->Gold += Payout;
		OutResult = FString::Printf(TEXT("Plinko returned %d gold."), Payout);
	}
	else if (GameID == FName(TEXT("BoxOpening")))
	{
		if (!SpendBet(Bet))
		{
			return false;
		}

		AddAnger(GameDefinition->AngerAdd);
		if (FMath::FRand() < GameDefinition->ItemRewardChance && DataSubsystem && DataSubsystem->GetItems().Num() > 0)
		{
			const int32 PickedIndex = FMath::RandRange(0, DataSubsystem->GetItems().Num() - 1);
			ActiveRun->OwnedItemIDs.Add(DataSubsystem->GetItems()[PickedIndex].ItemID);
			OutResult = FString::Printf(TEXT("Box Opening paid out %s."), *DataSubsystem->GetItems()[PickedIndex].ItemID.ToString());
		}
		else
		{
			const int32 GoldWon = FMath::RandRange(GameDefinition->MinGoldPayout, GameDefinition->MaxGoldPayout);
			ActiveRun->Gold += GoldWon;
			OutResult = FString::Printf(TEXT("Box Opening spilled %d gold."), GoldWon);
		}
	}
	else
	{
		OutResult = TEXT("That mini circus game is not wired.");
		return false;
	}

	HandleBacklash(ActiveRun, OutResult);
	return true;
}

bool UT66MiniCircusSubsystem::TryAlchemyTransmute(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult)
{
	if (!ActiveRun || !DataSubsystem || ActiveRun->OwnedItemIDs.Num() < 2 || DataSubsystem->GetItems().Num() == 0)
	{
		OutResult = TEXT("Alchemy needs at least two owned items.");
		return false;
	}

	const FName FirstItem = ActiveRun->OwnedItemIDs[0];
	const FName SecondItem = ActiveRun->OwnedItemIDs[1];
	ActiveRun->OwnedItemIDs.RemoveAt(0, 2, EAllowShrinking::No);
	const int32 PickedIndex = FMath::RandRange(0, DataSubsystem->GetItems().Num() - 1);
	const FName RewardItem = DataSubsystem->GetItems()[PickedIndex].ItemID;
	ActiveRun->OwnedItemIDs.Add(RewardItem);
	AddAnger(T66MiniCircusTuning(DataSubsystem, TEXT("AlchemyTransmuteAnger"), 0.06f));
	OutResult = FString::Printf(TEXT("Alchemy fused %s and %s into %s."), *FirstItem.ToString(), *SecondItem.ToString(), *RewardItem.ToString());
	HandleBacklash(ActiveRun, OutResult);
	return true;
}

bool UT66MiniCircusSubsystem::TryAlchemyDissolveOldest(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult)
{
	if (!ActiveRun || !DataSubsystem || ActiveRun->OwnedItemIDs.Num() == 0)
	{
		OutResult = TEXT("No owned items available for alchemy.");
		return false;
	}

	const FName ItemID = ActiveRun->OwnedItemIDs[0];
	const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(ItemID);
	ActiveRun->OwnedItemIDs.RemoveAt(0);
	const int32 Value = ItemDefinition
		? FMath::Max(T66MiniCircusTuningInt(DataSubsystem, TEXT("AlchemyDissolveMinValue"), 4), ItemDefinition->BaseSellGold)
		: T66MiniCircusTuningInt(DataSubsystem, TEXT("AlchemyDissolveFallbackValue"), 8);
	if (CircusDebt > 0)
	{
		const int32 DebtBurn = Value * T66MiniCircusTuningInt(DataSubsystem, TEXT("AlchemyDebtBurnMultiplier"), 2);
		CircusDebt = FMath::Max(0, CircusDebt - DebtBurn);
		OutResult = FString::Printf(TEXT("Alchemy dissolved %s and burned away %d debt."), *ItemID.ToString(), DebtBurn);
	}
	else
	{
		ActiveRun->Gold += Value;
		OutResult = FString::Printf(TEXT("Alchemy dissolved %s into %d gold."), *ItemID.ToString(), Value);
	}

	AddAnger(T66MiniCircusTuning(DataSubsystem, TEXT("AlchemyDissolveAnger"), 0.04f));
	return true;
}

void UT66MiniCircusSubsystem::GenerateVendorOffers(const UT66MiniDataSubsystem* DataSubsystem, const bool bCountAsReroll)
{
	TArray<FName> PreservedLockedOffers;
	for (const FName LockedID : LockedVendorOfferIDs)
	{
		if (CurrentVendorOfferIDs.Contains(LockedID))
		{
			PreservedLockedOffers.Add(LockedID);
		}
	}

	CurrentVendorOfferIDs = PreservedLockedOffers;
	if (!DataSubsystem || DataSubsystem->GetItems().Num() == 0)
	{
		return;
	}

	if (bCountAsReroll)
	{
		++VendorRerollCount;
	}

	TArray<int32> CandidateIndices;
	CandidateIndices.Reserve(DataSubsystem->GetItems().Num());
	for (int32 Index = 0; Index < DataSubsystem->GetItems().Num(); ++Index)
	{
		if (!CurrentVendorOfferIDs.Contains(DataSubsystem->GetItems()[Index].ItemID))
		{
			CandidateIndices.Add(Index);
		}
	}

	FRandomStream Stream(static_cast<int32>(FDateTime::UtcNow().GetTicks() ^ VendorRerollCount ^ CandidateIndices.Num()));
	for (int32 Index = CandidateIndices.Num() - 1; Index > 0; --Index)
	{
		CandidateIndices.Swap(Index, Stream.RandRange(0, Index));
	}

	const int32 OfferCount = FMath::Max(0, T66MiniCircusTuningInt(DataSubsystem, TEXT("VendorOfferCount"), 4));
	while (CurrentVendorOfferIDs.Num() < OfferCount && CandidateIndices.Num() > 0)
	{
		const int32 PickedIndex = CandidateIndices.Pop(EAllowShrinking::No);
		CurrentVendorOfferIDs.Add(DataSubsystem->GetItems()[PickedIndex].ItemID);
	}
}

void UT66MiniCircusSubsystem::AddAnger(const float Amount)
{
	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	CircusAnger01 = FMath::Clamp(CircusAnger01 + Amount, 0.f, T66MiniCircusTuning(DataSubsystem, TEXT("AngerMax"), 1.5f));
}

void UT66MiniCircusSubsystem::HandleBacklash(UT66MiniRunSaveGame* ActiveRun, FString& InOutResult)
{
	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	if (!ActiveRun || CircusAnger01 < T66MiniCircusTuning(DataSubsystem, TEXT("BacklashThreshold"), 1.f))
	{
		return;
	}

	CircusAnger01 = T66MiniCircusTuning(DataSubsystem, TEXT("BacklashResetAnger"), 0.35f);
	CircusDebt += T66MiniCircusTuningInt(DataSubsystem, TEXT("BacklashDebtAdd"), 28);
	const int32 SeizedGold = FMath::Min(T66MiniCircusTuningInt(DataSubsystem, TEXT("BacklashGoldSeizeMax"), 20), ActiveRun->Gold);
	ActiveRun->Gold -= SeizedGold;
	InOutResult += FString::Printf(TEXT(" The circus snapped back, seized %d gold, and added debt."), SeizedGold);
}

int32 UT66MiniCircusSubsystem::CalculateStealSupportBonus(const UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem) const
{
	return T66MiniCountOwnedItemsWithStat(ActiveRun, DataSubsystem, TEXT("Stealing"))
		+ T66MiniCountOwnedItemsWithStat(ActiveRun, DataSubsystem, TEXT("TreasureChest"))
		+ T66MiniCountOwnedItemsWithStat(ActiveRun, DataSubsystem, TEXT("LootCrate"));
}
