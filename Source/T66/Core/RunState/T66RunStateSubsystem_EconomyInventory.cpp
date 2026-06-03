// Copyright Tribulation 66. All Rights Reserved.

#include "Core/RunState/T66RunStateSubsystem_Private.h"

#include "Gameplay/T66WorldSystemsAPI.h"
#include "HAL/IConsoleManager.h"

using namespace T66RunStatePrivate;

DEFINE_LOG_CATEGORY_STATIC(LogT66ShopProof, Log, All);

namespace
{
	const TCHAR* T66_GetShopRarityName(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:
			return TEXT("Black");
		case ET66ItemRarity::Red:
			return TEXT("Red");
		case ET66ItemRarity::Yellow:
			return TEXT("Yellow");
		case ET66ItemRarity::White:
			return TEXT("White");
		default:
			return TEXT("Unknown");
		}
	}

	const TCHAR* T66_GetGoldTransactionSourceName(const ET66GoldTransactionSource Source)
	{
		switch (Source)
		{
		case ET66GoldTransactionSource::MobLootSale:
			return TEXT("MobLootSale");
		case ET66GoldTransactionSource::Gambler:
		default:
			return TEXT("Gambler");
		}
	}

	const TCHAR* T66_GetMobLootCollectorTypeName(const ET66MobLootCollectorType CollectorType)
	{
		switch (CollectorType)
		{
		case ET66MobLootCollectorType::Player:
			return TEXT("Player");
		case ET66MobLootCollectorType::Pet:
			return TEXT("Pet");
		case ET66MobLootCollectorType::System:
		default:
			return TEXT("System");
		}
	}

	ET66ItemRarity T66_RollShopSlotRarity(FRandomStream& Rng)
	{
		return UT66RunStateSubsystem::RollShopSlotRarity(Rng);
	}

	void T66RunShopWeightedOddsProofCommand(const TArray<FString>& Args, UWorld* World)
	{
		(void)World;
		const int32 Samples = Args.Num() > 0 ? FMath::Max(1000, FCString::Atoi(*Args[0])) : 100000;
		const int32 Seed = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 660066;
		FRandomStream Rng(Seed);
		int32 BlackCount = 0;
		int32 RedCount = 0;
		int32 YellowCount = 0;
		int32 WhiteCount = 0;

		for (int32 Index = 0; Index < Samples; ++Index)
		{
			switch (T66_RollShopSlotRarity(Rng))
			{
			case ET66ItemRarity::Black:
				++BlackCount;
				break;
			case ET66ItemRarity::Red:
				++RedCount;
				break;
			case ET66ItemRarity::Yellow:
				++YellowCount;
				break;
			case ET66ItemRarity::White:
				++WhiteCount;
				break;
			default:
				break;
			}
		}

		auto Ratio = [Samples](const int32 Count)
		{
			return Samples > 0 ? static_cast<float>(Count) / static_cast<float>(Samples) : 0.f;
		};

		const float BlackRatio = Ratio(BlackCount);
		const float RedRatio = Ratio(RedCount);
		const float YellowRatio = Ratio(YellowCount);
		const float WhiteRatio = Ratio(WhiteCount);
		const float ExpectedBlack = UT66RunStateSubsystem::ShopRarityWeightBlack / UT66RunStateSubsystem::ShopRarityWeightTotal;
		const float ExpectedRed = UT66RunStateSubsystem::ShopRarityWeightRed / UT66RunStateSubsystem::ShopRarityWeightTotal;
		const float ExpectedYellow = UT66RunStateSubsystem::ShopRarityWeightYellow / UT66RunStateSubsystem::ShopRarityWeightTotal;
		const float ExpectedWhite = UT66RunStateSubsystem::ShopRarityWeightWhite / UT66RunStateSubsystem::ShopRarityWeightTotal;
		const bool bPass =
			FMath::Abs(BlackRatio - ExpectedBlack) <= 0.015f &&
			FMath::Abs(RedRatio - ExpectedRed) <= 0.015f &&
			FMath::Abs(YellowRatio - ExpectedYellow) <= 0.004f &&
			FMath::Abs(WhiteRatio - ExpectedWhite) <= 0.002f;

		if (bPass)
		{
			UE_LOG(
				LogT66ShopProof,
				Display,
				TEXT("[T66Proof][ShopWeightedOdds] Result=PASS Samples=%d Seed=%d Black=%d Ratio=%.5f Expected=%.5f Red=%d Ratio=%.5f Expected=%.5f Yellow=%d Ratio=%.5f Expected=%.5f White=%d Ratio=%.5f Expected=%.5f Weights=%.1f/%.1f/%.1f/%.1f"),
				Samples,
				Seed,
				BlackCount,
				BlackRatio,
				ExpectedBlack,
				RedCount,
				RedRatio,
				ExpectedRed,
				YellowCount,
				YellowRatio,
				ExpectedYellow,
				WhiteCount,
				WhiteRatio,
				ExpectedWhite,
				UT66RunStateSubsystem::ShopRarityWeightBlack,
				UT66RunStateSubsystem::ShopRarityWeightRed,
				UT66RunStateSubsystem::ShopRarityWeightYellow,
				UT66RunStateSubsystem::ShopRarityWeightWhite);
		}
		else
		{
			UE_LOG(
				LogT66ShopProof,
				Warning,
				TEXT("[T66Proof][ShopWeightedOdds] Result=FAIL Samples=%d Seed=%d Black=%d Ratio=%.5f Expected=%.5f Red=%d Ratio=%.5f Expected=%.5f Yellow=%d Ratio=%.5f Expected=%.5f White=%d Ratio=%.5f Expected=%.5f Weights=%.1f/%.1f/%.1f/%.1f"),
				Samples,
				Seed,
				BlackCount,
				BlackRatio,
				ExpectedBlack,
				RedCount,
				RedRatio,
				ExpectedRed,
				YellowCount,
				YellowRatio,
				ExpectedYellow,
				WhiteCount,
				WhiteRatio,
				ExpectedWhite,
				UT66RunStateSubsystem::ShopRarityWeightBlack,
				UT66RunStateSubsystem::ShopRarityWeightRed,
				UT66RunStateSubsystem::ShopRarityWeightYellow,
				UT66RunStateSubsystem::ShopRarityWeightWhite);
		}
	}

	FAutoConsoleCommandWithWorldAndArgs T66ShopWeightedOddsProofCommand(
		TEXT("T66.Proof.ShopWeightedOdds"),
		TEXT("Runs a deterministic shop rarity weighted-odds proof. Args: <Samples> <Seed>."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66RunShopWeightedOddsProofCommand));
}

const FName UT66RunStateSubsystem::BackroomsQuickReviveItemID(TEXT("Item_BackroomsQuickRevive"));
const FName UT66RunStateSubsystem::KromerItemID(TEXT("Item_Kromer"));
const FName UT66RunStateSubsystem::MobLootItemID(TEXT("Item_MobLoot"));

ET66ItemRarity UT66RunStateSubsystem::RollShopSlotRarity(FRandomStream& Rng)
{
	float Roll = Rng.FRandRange(0.f, ShopRarityWeightTotal);
	Roll -= ShopRarityWeightBlack;
	if (Roll <= 0.f)
	{
		return ET66ItemRarity::Black;
	}

	Roll -= ShopRarityWeightRed;
	if (Roll <= 0.f)
	{
		return ET66ItemRarity::Red;
	}

	Roll -= ShopRarityWeightYellow;
	if (Roll <= 0.f)
	{
		return ET66ItemRarity::Yellow;
	}

	return ET66ItemRarity::White;
}

void UT66RunStateSubsystem::ResetShopForStage()
{
	ShopStockStage = 0;
	ShopStockItemIDs.Reset();
	ShopStockSlots.Reset();
	ShopStockSold.Reset();
	bBoughtFromShopThisStage = false;
	LastShopStealOutcome = ET66ShopStealOutcome::None;
	ShopAngerState = FT66ShopAngerState();
	ShopChanged.Broadcast();
}


void UT66RunStateSubsystem::EnsureShopStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 20);
	if (ShopStockStage == Stage && ShopStockItemIDs.Num() > 0 && ShopStockSold.Num() == ShopStockItemIDs.Num())
	{
		return;
	}

	// Reset reroll counter and seen-counts when stage changes.
	if (ShopStockRerollStage != Stage)
	{
		ShopStockRerollStage = Stage;
		ShopStockRerollCounter = 0;
		ShopSeenCounts.Reset();
	}

	ShopStockStage = Stage;
	ShopStockItemIDs.Reset();
	ShopStockSold.Reset();
	ShopStockSlots.Reset();

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!GI)
	{
		// Fallback: keep deterministic placeholder behavior with the live slot count.
		const FT66InventorySlot FallbackStock[] =
		{
			FT66InventorySlot(FName(TEXT("Item_AoeDamage")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_BounceScale")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_DamageReduction")), ET66ItemRarity::Black, 2),
			FT66InventorySlot(FName(TEXT("Item_Headshot")), ET66ItemRarity::Red, 5),
		};
		for (const FT66InventorySlot& Slot : FallbackStock)
		{
			ShopStockSlots.Add(Slot);
			ShopStockItemIDs.Add(Slot.ItemTemplateID);
		}
		ShopStockSold.Init(false, ShopStockSlots.Num());
		ShopChanged.Broadcast();
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

			if (!T66_IsRewardOnlySpecialItem(ItemID) && T66IsLiveSecondaryStatType(ItemData.SecondaryStatType))
			{
				TemplatePool.Add(ItemID);
			}
		}
	}

	{
		FItemData ExecuteItemData;
		const FName ExecuteItemID(TEXT("Item_Execute"));
		if (GI->GetItemData(ExecuteItemID, ExecuteItemData)
			&& T66IsLiveSecondaryStatType(ExecuteItemData.SecondaryStatType)
			&& !TemplatePool.Contains(ExecuteItemID))
		{
			TemplatePool.Add(ExecuteItemID);
		}
	}

	if (TemplatePool.Num() == 0)
	{
		TemplatePool =
		{
			FName(TEXT("Item_AoeDamage")),
			FName(TEXT("Item_BounceScale")),
			FName(TEXT("Item_DamageReduction")),
			FName(TEXT("Item_Headshot")),
			FName(TEXT("Item_Execute"))
		};
	}

	// Seed: per-stage and per-reroll, plus run seed so the first shop display is randomized each run.
	int32 Seed = Stage * 777 + 13 + ShopStockRerollCounter * 10007;
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
		const int32 Seen = ShopSeenCounts.FindRef(TemplatePool[TemplateIndex]);
		Weights[TemplateIndex] = FMath::Max(WeightFloor, 1.0f / (1.0f + static_cast<float>(Seen) * DecayFactor));
	}

	for (int32 SlotIndex = 0; SlotIndex < ShopDisplaySlotCount; ++SlotIndex)
	{
		float TotalWeight = 0.f;
		for (int32 TemplateIndex = 0; TemplateIndex < TemplatePool.Num(); ++TemplateIndex)
		{
			if (!ShopStockItemIDs.Contains(TemplatePool[TemplateIndex]))
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
			if (ShopStockItemIDs.Contains(TemplatePool[TemplateIndex]))
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
				if (!ShopStockItemIDs.Contains(TemplatePool[TemplateIndex]))
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

		ShopSeenCounts.FindOrAdd(Chosen)++;

		const ET66ItemRarity Rarity = T66_RollShopSlotRarity(Rng);
		int32 RollMin = 1;
		int32 RollMax = 3;
		FItemData::GetLine1RollRange(Rarity, RollMin, RollMax);
		const int32 Rolled = Rng.RandRange(RollMin, RollMax);

		ShopStockSlots.Add(FT66InventorySlot(Chosen, Rarity, Rolled));
		ShopStockItemIDs.Add(Chosen);
		UE_LOG(
			LogT66ShopProof,
			Log,
			TEXT("[T66Proof][ShopWeightedSlot] Stage=%d Reroll=%d Slot=%d Item=%s Rarity=%s Seed=%d Weights=%.1f/%.1f/%.1f/%.1f"),
			Stage,
			ShopStockRerollCounter,
			SlotIndex,
			*Chosen.ToString(),
			T66_GetShopRarityName(Rarity),
			Seed,
			ShopRarityWeightBlack,
			ShopRarityWeightRed,
			ShopRarityWeightYellow,
			ShopRarityWeightWhite);
	}

	ShopStockSold.Init(false, ShopStockSlots.Num());
	ShopChanged.Broadcast();
}


void UT66RunStateSubsystem::RerollShopStockForCurrentStage()
{
	const int32 Stage = FMath::Clamp(CurrentStage, 1, 20);
	if (ShopStockRerollStage != Stage)
	{
		ShopStockRerollStage = Stage;
		ShopStockRerollCounter = 0;
	}
	ShopStockRerollCounter = FMath::Clamp(ShopStockRerollCounter + 1, 0, 9999);

	// Force regeneration even if the stage didn't change.
	ShopStockStage = 0;
	ShopStockItemIDs.Reset();
	ShopStockSold.Reset();
	EnsureShopStockForCurrentStage();
	// EnsureShopStockForCurrentStage broadcasts ShopChanged.
}


bool UT66RunStateSubsystem::IsShopStockSlotSold(int32 Index) const
{
	if (Index < 0 || Index >= ShopStockSold.Num()) return true;
	return ShopStockSold[Index];
}


bool UT66RunStateSubsystem::TryBuyShopStockSlot(int32 Index)
{
	EnsureShopStockForCurrentStage();
	if (Index < 0 || Index >= ShopStockSlots.Num()) return false;
	if (IsShopStockSlotSold(Index)) return false;
	if (!HasInventorySpace()) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& Slot = ShopStockSlots[Index];
	if (!GI || !GI->GetItemData(Slot.ItemTemplateID, D)) return false;
	const int32 BuyPrice = GetBuyGoldForShopStockSlot(Index);
	if (BuyPrice <= 0) return false;
	if (!TrySpendGold(BuyPrice)) return false;

	AddItemSlot(Slot);
	ShopStockSold[Index] = true;
	bBoughtFromShopThisStage = true;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ShopPurchase=%s"), *Slot.ItemTemplateID.ToString()));
	ShopChanged.Broadcast();
	return true;
}


bool UT66RunStateSubsystem::ResolveShopStealAttempt(int32 Index, bool bTimingHit, bool bRngSuccess)
{
	(void)bRngSuccess;
	ShopAngerState.bLastAttemptTriggeredVendorBoss = false;
	EnsureShopStockForCurrentStage();
	if (Index < 0 || Index >= ShopStockSlots.Num()) return false;
	if (IsShopStockSlotSold(Index)) return false;

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& StealSlot = ShopStockSlots[Index];
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
			? PlayerExperience->GetDifficultyShopStealSuccessChanceOnTimingHitBase(Difficulty)
			: 0.65f;
	}
	BaseChance = FMath::Clamp(BaseChance, 0.f, 1.f);

	bool bSuccess = false;
	float AppliedChance = 0.f;
	int32 DrawIndex = INDEX_NONE;
	int32 PreDrawSeed = 0;
	if (bTimingHit && BaseChance > 0.f)
	{
		const float BiasedChance = RngSub ? RngSub->BiasChance01(BaseChance) : BaseChance;
		const float Chance = FMath::Clamp(BiasedChance + GetStealingLuckChanceBonus01(), 0.f, 0.95f);
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

	LastShopStealOutcome = ET66ShopStealOutcome::None;
	if (!bTimingHit)
	{
		LastShopStealOutcome = ET66ShopStealOutcome::Miss;
	}
	else if (!bSuccess)
	{
		LastShopStealOutcome = ET66ShopStealOutcome::Failed;
	}
	else if (!HasInventorySpace())
	{
		LastShopStealOutcome = ET66ShopStealOutcome::InventoryFull;
	}
	else
	{
		LastShopStealOutcome = ET66ShopStealOutcome::Success;
	}

	ShopAngerState.StealAttemptCount = FMath::Max(0, ShopAngerState.StealAttemptCount) + 1;
	ShopAngerState.LastAttemptSlotIndex = Index;
	ShopAngerState.LastAttemptBuyValue = BuyPrice;
	ShopAngerState.bLastAttemptTriggeredVendorBoss = ShopAngerState.bTriggerVendorBossOnAnyAttempt;

	bool bGranted = false;
	if (LastShopStealOutcome == ET66ShopStealOutcome::Success)
	{
		AddItemSlot(StealSlot);
		ShopStockSold[Index] = true;
		AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ShopSteal=%s"), *StealSlot.ItemTemplateID.ToString()));
		bGranted = true;
	}

	RecordLuckQuantityBool(
		FName(TEXT("ShopStealSuccess")),
		(LastShopStealOutcome == ET66ShopStealOutcome::Success),
		AppliedChance,
		DrawIndex,
		PreDrawSeed);

	if (LastShopStealOutcome == ET66ShopStealOutcome::Success || ShopAngerState.bLastAttemptTriggeredVendorBoss)
	{
		ShopChanged.Broadcast();
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
	if (T66_IsRewardOnlySpecialItem(Slot.ItemTemplateID)) return false;

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
	AddGold(Amount, ET66GoldTransactionSource::Gambler);
}


void UT66RunStateSubsystem::AddGold(int32 Amount, const ET66GoldTransactionSource Source)
{
	if (Amount == 0) return;
	CurrentGold = FMath::Max(0, CurrentGold + Amount);
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=%s"), Amount, T66_GetGoldTransactionSourceName(Source)));
	GoldChanged.Broadcast();
	LogAdded.Broadcast();
}


int32 UT66RunStateSubsystem::AddCollectedMobLootFromCollection(
	const FT66MobLootCollectResult& Result,
	const FT66MobLootCollectorRef& Collector)
{
	const int32 Amount = FMath::Max(0, Result.GoldValueCollected);
	if (Amount <= 0)
	{
		return 0;
	}

	const int32 Before = CollectedMobLootStack;
	CollectedMobLootStack = FMath::Clamp(CollectedMobLootStack + Amount, 0, MaxCollectedMobLootStack);
	const int32 Added = CollectedMobLootStack - Before;
	if (Added > 0)
	{
		MobLootDropsCollectedThisRun = FMath::Clamp(MobLootDropsCollectedThisRun + FMath::Max(0, Result.DropsCollected), 0, MAX_int32);
		MobLootQuantityCollectedThisRun = FMath::Clamp(MobLootQuantityCollectedThisRun + Added, 0, MAX_int32);
		MobLootGoldValueCollectedThisRun = FMath::Clamp(MobLootGoldValueCollectedThisRun + Added, 0, MAX_int32);
		switch (Collector.CollectorType)
		{
		case ET66MobLootCollectorType::Pet:
			MobLootQuantityCollectedByPetThisRun = FMath::Clamp(MobLootQuantityCollectedByPetThisRun + Added, 0, MAX_int32);
			MobLootDropsCollectedByPetThisRun = FMath::Clamp(MobLootDropsCollectedByPetThisRun + FMath::Max(0, Result.DropsCollected), 0, MAX_int32);
			break;
		case ET66MobLootCollectorType::Player:
			MobLootQuantityCollectedByPlayerThisRun = FMath::Clamp(MobLootQuantityCollectedByPlayerThisRun + Added, 0, MAX_int32);
			break;
		case ET66MobLootCollectorType::System:
		default:
			break;
		}
	}
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(
		TEXT("ItemID=MobLoot,Source=MobLootCollection,Collector=%s,Amount=%d,Stack=%d,Cap=%d"),
		T66_GetMobLootCollectorTypeName(Collector.CollectorType),
		Added,
		CollectedMobLootStack,
		MaxCollectedMobLootStack));
	InventoryChanged.Broadcast();
	ShopChanged.Broadcast();
	LogAdded.Broadcast();
	return Added;
}


bool UT66RunStateSubsystem::SellCollectedMobLoot()
{
	if (CollectedMobLootStack <= 0)
	{
		return false;
	}

	const int32 SellGold = CollectedMobLootStack;
	CollectedMobLootStack = 0;
	MobLootQuantitySoldThisRun = FMath::Clamp(MobLootQuantitySoldThisRun + SellGold, 0, MAX_int32);
	MobLootSaleGoldThisRun = FMath::Clamp(MobLootSaleGoldThisRun + SellGold, 0, MAX_int32);
	AddStructuredEvent(ET66RunEventType::ItemConsumed, FString::Printf(TEXT("ItemID=MobLoot,Source=MobLootSale,Amount=%d"), SellGold));
	AddGold(SellGold, ET66GoldTransactionSource::MobLootSale);
	InventoryChanged.Broadcast();
	ShopChanged.Broadcast();
	return true;
}


#if !UE_BUILD_SHIPPING
void UT66RunStateSubsystem::SetCollectedMobLootStackForAutomation(const int32 Amount)
{
	CollectedMobLootStack = FMath::Clamp(Amount, 0, MaxCollectedMobLootStack);
	InventoryChanged.Broadcast();
	ShopChanged.Broadcast();
}
#endif


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
	if (T66_IsRetiredRemovedItemID(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Items] Skipping retired item ID %s."), *ItemID.ToString());
		return;
	}
	if (T66_IsVendorTokenItem(ItemID))
	{
		ApplyVendorTokenPickup(1);
		return;
	}

	AddItemWithRarity(ItemID, ET66ItemRarity::Black);
}


void UT66RunStateSubsystem::AddItemWithRarity(FName ItemID, ET66ItemRarity Rarity)
{
	if (ItemID.IsNone()) return;
	if (T66_IsRetiredRemovedItemID(ItemID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Items] Skipping retired item ID %s."), *ItemID.ToString());
		return;
	}
	if (T66_IsVendorTokenItem(ItemID))
	{
		ApplyVendorTokenPickup(1);
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
	if (T66_IsRetiredRemovedItemID(Slot.ItemTemplateID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Items] Skipping retired inventory slot item ID %s."), *Slot.ItemTemplateID.ToString());
		return;
	}
	if (T66_IsVendorTokenItem(Slot.ItemTemplateID))
	{
		ApplyVendorTokenPickup(Slot.Line1RolledValue);
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
	if (!T66_IsRewardOnlySpecialItem(NormalizedSlot.ItemTemplateID))
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
		{
			if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				Achieve->AddLabUnlockedItem(NormalizedSlot.ItemTemplateID);
			}
		}
	}
	InventoryChanged.Broadcast();
	if (T66_IsBackroomsQuickReviveItem(NormalizedSlot.ItemTemplateID))
	{
		QuickReviveChanged.Broadcast();
	}
	LogAdded.Broadcast();
}

bool UT66RunStateSubsystem::HasBackroomsQuickReviveItem() const
{
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		if (Slot.IsValid() && T66_IsBackroomsQuickReviveItem(Slot.ItemTemplateID))
		{
			return true;
		}
	}

	return false;
}

bool UT66RunStateSubsystem::ConsumeBackroomsQuickReviveItem()
{
	for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
	{
		if (InventorySlots[Index].IsValid() && T66_IsBackroomsQuickReviveItem(InventorySlots[Index].ItemTemplateID))
		{
			InventorySlots.RemoveAt(Index);
			RecomputeItemDerivedStats();
			AddStructuredEvent(ET66RunEventType::ItemConsumed, TEXT("ItemID=Item_BackroomsQuickRevive,Source=QuickReviveConsumed"));
			InventoryChanged.Broadcast();
			QuickReviveChanged.Broadcast();
			LogAdded.Broadcast();
			return true;
		}
	}

	return false;
}

bool UT66RunStateSubsystem::HasKromerItem() const
{
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		if (Slot.IsValid() && T66_IsKromerItem(Slot.ItemTemplateID))
		{
			return true;
		}
	}

	return false;
}

bool UT66RunStateSubsystem::ConsumeKromerItem()
{
	for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
	{
		if (InventorySlots[Index].IsValid() && T66_IsKromerItem(InventorySlots[Index].ItemTemplateID))
		{
			InventorySlots.RemoveAt(Index);
			RecomputeItemDerivedStats();
			AddStructuredEvent(ET66RunEventType::ItemConsumed, TEXT("ItemID=Item_Kromer,Source=SaintSecretEnding"));
			InventoryChanged.Broadcast();
			LogAdded.Broadcast();
			return true;
		}
	}

	return false;
}

void UT66RunStateSubsystem::SnapshotAndClearInventoryForBackrooms(TArray<FT66InventorySlot>& OutSnapshot)
{
	OutSnapshot = InventorySlots;
	InventorySlots.Empty();
	ActiveVendorTokenStacks = 0;
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();
	QuickReviveChanged.Broadcast();
}

void UT66RunStateSubsystem::RestoreInventoryFromBackroomsSnapshot(const TArray<FT66InventorySlot>& Snapshot)
{
	InventorySlots = Snapshot;
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();
	QuickReviveChanged.Broadcast();
}


int32 UT66RunStateSubsystem::GetBuyGoldForShopStockSlot(int32 Index) const
{
	if (Index < 0 || Index >= ShopStockSlots.Num())
	{
		return 0;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	FItemData D;
	const FT66InventorySlot& Slot = ShopStockSlots[Index];
	if (!GI || !GI->GetItemData(Slot.ItemTemplateID, D))
	{
		return 0;
	}

	const int32 BaseBuyPrice = D.GetBuyGoldForRarity(Slot.Rarity);
	if (BaseBuyPrice <= 0)
	{
		return 0;
	}

	const float DiscountedPrice = static_cast<float>(BaseBuyPrice) * FMath::Max(0.f, 1.f - GetCurrentBuyDiscountFraction());
	return FMath::Max(0, FMath::RoundToInt(DiscountedPrice));
}


void UT66RunStateSubsystem::ApplyVendorTokenPickup(int32 TokenStacks)
{
	const int32 StacksToAdd = FMath::Clamp(TokenStacks, 1, MaxVendorTokenStacks);
	const int32 PreviousStacks = T66_ClampVendorTokenStackCount(ActiveVendorTokenStacks);
	const int32 NewStacks = T66_ClampVendorTokenStackCount(PreviousStacks + StacksToAdd);
	if (NewStacks <= PreviousStacks)
	{
		return;
	}

	ActiveVendorTokenStacks = NewStacks;
	AddStructuredEvent(ET66RunEventType::ItemAcquired, FString::Printf(TEXT("ItemID=%s,Source=VendorToken,Stacks=%d"), *T66VendorTokenItemID.ToString(), ActiveVendorTokenStacks));

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->AddLabUnlockedItem(T66VendorTokenItemID);
		}
	}

	InventoryChanged.Broadcast();
	LogAdded.Broadcast();
}


float UT66RunStateSubsystem::GetCurrentSellFraction() const
{
	return T66_GetSellFractionForVendorTokenStacks(ActiveVendorTokenStacks);
}


float UT66RunStateSubsystem::GetCurrentBuyDiscountFraction() const
{
	return T66_GetBuyDiscountFractionForVendorTokenStacks(ActiveVendorTokenStacks);
}


int32 UT66RunStateSubsystem::GetSellGoldForInventorySlot(const FT66InventorySlot& Slot) const
{
	if (!Slot.IsValid() || T66_IsRewardOnlySpecialItem(Slot.ItemTemplateID))
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
	ActiveVendorTokenStacks = 0;
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
	if (T66_IsRewardOnlySpecialItem(Slot.ItemTemplateID))
	{
		return false;
	}

	const int32 SellGold = GetSellGoldForInventorySlot(Slot);

	CurrentGold += SellGold;
	BuybackPool.Add(Slot);
	InventorySlots.RemoveAt(InventoryIndex);
	RecomputeItemDerivedStats();
	AddStructuredEvent(ET66RunEventType::GoldGained, FString::Printf(TEXT("Amount=%d,Source=Shop,ItemID=%s"), SellGold, *Slot.ItemTemplateID.ToString()));
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

	UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance());
	for (const FT66InventorySlot& Slot : InventorySlots)
	{
		if (!Slot.IsValid()) continue;

		FItemData D;
		const bool bHasRow = (GI && GI->GetItemData(Slot.ItemTemplateID, D));
		if (!bHasRow) continue;
		if (T66_IsRewardOnlySpecialItem(Slot.ItemTemplateID)
			|| D.SecondaryStatType == ET66SecondaryStatType::VendorToken)
		{
			continue;
		}
		if (D.PrimaryStatType == ET66HeroStatType::Special)
		{
			continue;
		}

		// Items now only apply their secondary line. Primary growth comes from level-up and diplomas.
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
