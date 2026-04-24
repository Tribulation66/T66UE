// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GamblerOverlayWidget.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "UI/T66ItemCardTextUtils.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "UI/Style/T66Style.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66PlayerController.h"
#include "Engine/Texture2D.h"

static FString MakeGamblerEconomyInventoryStackKey(const FT66InventorySlot& Slot)
{
	return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
}

static FText BuildGamblerEconomyWagerText(const int32 WagerAmount)
{
	return WagerAmount > 0
		? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
		: FText::GetEmpty();
}

template <typename TNpcType>
static TNpcType* GetRegisteredGamblerEconomyNpc(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNpc : Registry->GetNPCs())
		{
			if (TNpcType* Npc = Cast<TNpcType>(WeakNpc.Get()))
			{
				return Npc;
			}
		}
	}

	return nullptr;
}

static bool HasRegisteredGamblerEconomyBoss(UWorld* World)
{
	if (!World)
	{
		return false;
	}

	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			if (Cast<AT66GamblerBoss>(WeakBoss.Get()) != nullptr)
			{
				return true;
			}
		}
	}

	return false;
}

namespace
{
	static int32 T66BuildGamblerEconomyNumberMask(const TSet<int32>& Numbers)
	{
		int32 Mask = 0;
		for (const int32 Number : Numbers)
		{
			if (Number >= 1 && Number <= 10)
			{
				Mask |= (1 << (Number - 1));
			}
		}
		return Mask;
	}

	static int32 T66BuildGamblerEconomyNumberMask(const TArray<int32>& Numbers)
	{
		int32 Mask = 0;
		for (const int32 Number : Numbers)
		{
			if (Number >= 1 && Number <= 10)
			{
				Mask |= (1 << (Number - 1));
			}
		}
		return Mask;
	}

	static int32 T66GetGamblerEconomyPlinkoPayoutTierFromSlot(const int32 SlotIndex)
	{
		static const int32 Tiers[9] = { 4, 3, 2, 1, 0, 1, 2, 3, 4 };
		return Tiers[FMath::Clamp(SlotIndex, 0, 8)];
	}

	static ET66Rarity T66GamblerEconomyBoxOpeningIndexToRarity(const int32 ColorIndex)
	{
		switch (ColorIndex)
		{
		case 1: return ET66Rarity::Red;
		case 2: return ET66Rarity::Yellow;
		case 3: return ET66Rarity::White;
		default: return ET66Rarity::Black;
		}
	}
}


FReply UT66GamblerOverlayWidget::OnBetClicked()
{
	if (GambleAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (World->GetGameInstance())
			{
				Loc2 = World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_GambleAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "GambleAmountMustBePositive", "Gamble amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || RunState->GetCurrentGold() < GambleAmount)
	{
		UT66LocalizationSubsystem* Loc2 = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return FReply::Handled();
	}

	// Lottery / Plinko / Box Opening: Bet starts the game (deduct and run)
	if (CasinoSwitcher.IsValid())
	{
		const int32 Idx = CasinoSwitcher->GetActiveWidgetIndex();
		if (Idx == 4)
		{
			if (LotterySelected.Num() != 5)
			{
				SetStatus(NSLOCTEXT("T66.Gambler", "Pick5Numbers", "Pick exactly 5 numbers."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
				return FReply::Handled();
			}
			if (!RunState->TrySpendGold(GambleAmount)) { RefreshTopBar(); return FReply::Handled(); }
			PendingBetAmount = GambleAmount;
			LockedBetAmount = 0;
			RefreshTopBar();
			StartLotteryDraw();
			return FReply::Handled();
		}
		if (Idx == 5)
		{
			if (!RunState->TrySpendGold(GambleAmount)) { RefreshTopBar(); return FReply::Handled(); }
			PendingBetAmount = GambleAmount;
			LockedBetAmount = 0;
			RefreshTopBar();
			StartPlinkoDrop();
			return FReply::Handled();
		}
		if (Idx == 6)
		{
			if (!RunState->TrySpendGold(GambleAmount)) { RefreshTopBar(); return FReply::Handled(); }
			PendingBetAmount = GambleAmount;
			LockedBetAmount = 0;
			RefreshTopBar();
			StartBoxOpeningSpin();
			return FReply::Handled();
		}
	}

	LockedBetAmount = GambleAmount;
	SetStatus(FText::GetEmpty());
	RefreshTopBar();
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnBorrowClicked()
{
	if (BorrowAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_BorrowAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "BorrowAmountMustBePositive", "Borrow amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
				if (!RunState->BorrowGold(BorrowAmount))
				{
					BorrowAmount = FMath::Clamp(BorrowAmount, 0, RunState->GetRemainingBorrowCapacity());
					const FText Fmt = Loc2 ? Loc2->GetText_BorrowExceedsNetWorthFormat() : NSLOCTEXT("T66.Gambler", "BorrowExceedsNetWorthFormat", "Borrow amount exceeds remaining Net Worth ({0}).");
					SetStatus(FText::Format(Fmt, FText::AsNumber(RunState->GetRemainingBorrowCapacity())), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
					RefreshTopBar();
					return FReply::Handled();
				}
				const FText Fmt = Loc2 ? Loc2->GetText_BorrowedAmountFormat() : NSLOCTEXT("T66.Gambler", "BorrowedAmountFormat", "Borrowed {0}.");
				SetStatus(FText::Format(Fmt, FText::AsNumber(BorrowAmount)), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
				RefreshTopBar();
			}
		}
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnPaybackClicked()
{
	if (PaybackAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_PaybackAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "PaybackAmountMustBePositive", "Payback amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				const int32 Paid = RunState->PayDebt(PaybackAmount);
				if (Paid <= 0)
				{
					UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
					SetStatus(Loc2 ? Loc2->GetText_NotEnoughGoldOrNoDebt() : NSLOCTEXT("T66.Gambler", "NotEnoughGoldOrNoDebt", "Not enough gold (or no debt)."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
				}
				else
				{
					UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
					const FText Fmt = Loc2 ? Loc2->GetText_PaidBackAmountFormat() : NSLOCTEXT("T66.Gambler", "PaidBackAmountFormat", "Paid back {0}.");
					SetStatus(FText::Format(Fmt, FText::AsNumber(Paid)), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
				}
				RefreshTopBar();
			}
		}
	}
	return FReply::Handled();
}


void UT66GamblerOverlayWidget::RefreshCasinoGameChrome()
{
	const int32 SharedWagerAmount = (LockedBetAmount > 0) ? LockedBetAmount : PendingBetAmount;
	if (GambleAmountSpin.IsValid())
	{
		GambleAmountSpin->SetValue(FMath::Max(0, GambleAmount));
	}
	if (BorrowAmountSpin.IsValid())
	{
		BorrowAmountSpin->SetValue(FMath::Max(0, BorrowAmount));
	}
	if (PaybackAmountSpin.IsValid())
	{
		PaybackAmountSpin->SetValue(FMath::Max(0, PaybackAmount));
	}
	if (CoinFlipWagerText.IsValid())
	{
		CoinFlipWagerText->SetText(BuildGamblerEconomyWagerText(SharedWagerAmount));
	}
	if (RpsWagerText.IsValid())
	{
		RpsWagerText->SetText(BuildGamblerEconomyWagerText(SharedWagerAmount));
	}

	const int32 BlackJackWagerAmount = (LockedBetAmount > 0)
		? LockedBetAmount
		: ((BJBetAmount > 0) ? BJBetAmount : PendingBetAmount);
	if (BlackJackWagerText.IsValid())
	{
		BlackJackWagerText->SetText(BuildGamblerEconomyWagerText(BlackJackWagerAmount));
	}

	if (BlackJackDealerValueText.IsValid())
	{
		if (BJDealerHand.Num() < 2)
		{
			BlackJackDealerValueText->SetText(FText::FromString(TEXT("-")));
		}
		else if (!BJDealerHoleRevealed)
		{
			const int32 HiddenCard = BJDealerHand[1];
			const int32 Rank = HiddenCard % 13;
			const int32 Value = (Rank == 0) ? 11 : (Rank >= 9) ? 10 : (Rank + 1);
			BlackJackDealerValueText->SetText(FText::AsNumber(Value));
		}
		else
		{
			BlackJackDealerValueText->SetText(FText::AsNumber(BJHandValue(BJDealerHand)));
		}
	}

	if (BlackJackPlayerValueText.IsValid())
	{
		if (BJPlayerHands.Num() == 0 || !BJPlayerHands.IsValidIndex(BJCurrentHandIndex))
		{
			BlackJackPlayerValueText->SetText(FText::FromString(TEXT("-")));
		}
		else
		{
			BlackJackPlayerValueText->SetText(FText::AsNumber(BJHandValue(BJPlayerHands[BJCurrentHandIndex])));
		}
	}

	const bool bBlackJackRoundActive = BJDeck.Num() > 0;
	if (BlackJackDealerHoleCardBackBox.IsValid())
	{
		BlackJackDealerHoleCardBackBox->SetVisibility((bBlackJackRoundActive && !BJDealerHoleRevealed) ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (BlackJackDealButtonBox.IsValid())
	{
		BlackJackDealButtonBox->SetVisibility(bBlackJackRoundActive ? EVisibility::Collapsed : EVisibility::Visible);
	}
	if (BlackJackHitButtonBox.IsValid())
	{
		BlackJackHitButtonBox->SetVisibility(bBlackJackRoundActive ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (BlackJackStandButtonBox.IsValid())
	{
		BlackJackStandButtonBox->SetVisibility(bBlackJackRoundActive ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (BlackJackDoubleButtonBox.IsValid())
	{
		BlackJackDoubleButtonBox->SetVisibility(bBlackJackRoundActive ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (BlackJackSplitButtonBox.IsValid())
	{
		BlackJackSplitButtonBox->SetVisibility(bBlackJackRoundActive ? EVisibility::Visible : EVisibility::Collapsed);
	}

	if (CasinoBetRowBox.IsValid())
	{
		int32 ActiveCasinoIndex = 0;
		if (CasinoSwitcher.IsValid())
		{
			ActiveCasinoIndex = CasinoSwitcher->GetActiveWidgetIndex();
		}

		const bool bShowBetRow = ActiveCasinoIndex != 0 && !(ActiveCasinoIndex == 3 && bBlackJackRoundActive);
		CasinoBetRowBox->SetVisibility(bShowBetRow ? EVisibility::Visible : EVisibility::Collapsed);
	}

	const bool bShowingBuyback = CasinoBuybackSwitcher.IsValid() && CasinoBuybackSwitcher->GetActiveWidgetIndex() == 1;
	if (CasinoModeToggleText.IsValid())
	{
		CasinoModeToggleText->SetText(
			bShowingBuyback
				? NSLOCTEXT("T66.Gambler", "Games", "GAMES")
				: NSLOCTEXT("T66.Vendor", "Buyback", "BUYBACK"));
	}
	if (CasinoRerollButtonWidget.IsValid())
	{
		UT66RunStateSubsystem* RunState = GetWorld() && GetWorld()->GetGameInstance()
			? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>()
			: nullptr;
		const bool bCanReroll = RunState
			&& bShowingBuyback
			&& RunState->GetBuybackPoolSize() > UT66RunStateSubsystem::BuybackDisplaySlotCount;
		CasinoRerollButtonWidget->SetEnabled(bCanReroll);
	}
	if (CloseButtonBox.IsValid())
	{
		CloseButtonBox->SetVisibility(bEmbeddedInCasinoShell ? EVisibility::Collapsed : EVisibility::Visible);
	}
}

void UT66GamblerOverlayWidget::RefreshTopBar()
{
	int32 NetWorth = 0;
	int32 Gold = 0;
	int32 Debt = 0;
	float Anger01 = 0.f;
	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				BorrowAmount = FMath::Clamp(BorrowAmount, 0, RunState->GetRemainingBorrowCapacity());
				Gold = RunState->GetCurrentGold();
				Debt = RunState->GetCurrentDebt();
				NetWorth = RunState->GetNetWorth();
				Anger01 = RunState->GetGamblerAnger01();
			}
		}
	}
	const FText NetWorthFmt = Loc2 ? Loc2->GetText_NetWorthFormat() : NSLOCTEXT("T66.GameplayHUD", "NetWorthFormat", "Net Worth: {0}");
	const FText GoldFmt = Loc2 ? Loc2->GetText_GoldFormat() : NSLOCTEXT("T66.GameplayHUD", "GoldFormat", "Gold: {0}");
	const FText OweFmt = Loc2 ? Loc2->GetText_OweFormat() : NSLOCTEXT("T66.GameplayHUD", "OweFormat", "Debt: {0}");
	if (NetWorthText.IsValid())
	{
		NetWorthText->SetText(FText::Format(NetWorthFmt, FText::AsNumber(NetWorth)));
		const FLinearColor NetWorthColor = NetWorth > 0
			? FT66Style::Tokens::Success
			: (NetWorth < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
		NetWorthText->SetColorAndOpacity(NetWorthColor);
	}
	if (GoldText.IsValid())
	{
		GoldText->SetText(FText::Format(GoldFmt, FText::AsNumber(Gold)));
	}
	if (DebtText.IsValid())
	{
		DebtText->SetText(FText::Format(OweFmt, FText::AsNumber(Debt)));
	}
	// Anger face sprites: Happy (0-33%), Neutral (34-66%), Angry (67-99%), boss at 100%
	if (AngerCircleImage.IsValid())
	{
		const int32 Pct = FMath::Clamp(FMath::RoundToInt(Anger01 * 100.f), 0, 100);
		if (Pct >= 67)
		{
			AngerCircleImage->SetImage(&AngerFace_Angry);
		}
		else if (Pct >= 34)
		{
			AngerCircleImage->SetImage(&AngerFace_Neutral);
		}
		else
		{
			AngerCircleImage->SetImage(&AngerFace_Happy);
		}
	}
	if (AngerText.IsValid())
	{
		const int32 Pct = FMath::RoundToInt(Anger01 * 100.f);
		AngerText->SetText(FText::Format(NSLOCTEXT("T66.Common", "PercentInt", "{0}%"), FText::AsNumber(Pct)));
	}

	RefreshCasinoGameChrome();
}

void UT66GamblerOverlayWidget::RefreshBuyback()
{
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = World && World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	const TArray<FT66InventorySlot>& Slots = RunState->GetBuybackDisplaySlots();
	const int32 SlotCount = BuybackNameTexts.Num();
	for (int32 i = 0; i < SlotCount; ++i)
	{
		const bool bHasSlot = Slots.IsValidIndex(i) && Slots[i].IsValid();
		FItemData D;
		const bool bHasData = bHasSlot && GI && GI->GetItemData(Slots[i].ItemTemplateID, D);
		const ET66ItemRarity SlotRarity = bHasSlot ? Slots[i].Rarity : ET66ItemRarity::Black;
		const int32 SellPrice = (bHasSlot && RunState) ? RunState->GetSellGoldForInventorySlot(Slots[i]) : 0;

		if (BuybackNameTexts.IsValidIndex(i) && BuybackNameTexts[i].IsValid())
		{
			BuybackNameTexts[i]->SetText(bHasSlot
				? (Loc ? Loc->GetText_ItemDisplayNameForRarity(Slots[i].ItemTemplateID, SlotRarity) : FText::FromName(Slots[i].ItemTemplateID))
				: NSLOCTEXT("T66.Common", "Empty", "EMPTY"));
		}
		if (BuybackDescTexts.IsValidIndex(i) && BuybackDescTexts[i].IsValid())
		{
			if (!bHasData)
			{
				BuybackDescTexts[i]->SetText(FText::GetEmpty());
			}
			else
			{
				const int32 MainValue = Slots[i].Line1RolledValue;
				const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
				BuybackDescTexts[i]->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SlotRarity, MainValue, ScaleMult, Slots[i].GetLine2Multiplier()));
			}
		}
		if (BuybackIconBorders.IsValidIndex(i) && BuybackIconBorders[i].IsValid())
		{
			BuybackIconBorders[i]->SetBorderBackgroundColor(bHasData ? FItemData::GetItemRarityColor(SlotRarity) : FT66Style::Tokens::Panel2);
		}
		if (BuybackIconBrushes.IsValidIndex(i) && BuybackIconBrushes[i].IsValid())
		{
			const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
			if (!SlotIconSoft.IsNull() && TexPool)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, BuybackIconBrushes[i], FName(TEXT("GamblerBuyback"), i + 1), /*bClearWhileLoading*/ true);
			}
			else
			{
				BuybackIconBrushes[i]->SetResourceObject(nullptr);
			}
		}
		if (BuybackIconImages.IsValidIndex(i) && BuybackIconImages[i].IsValid())
		{
			const bool bHasIcon = bHasData && !D.GetIconForRarity(SlotRarity).IsNull();
			BuybackIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (BuybackTileBorders.IsValidIndex(i) && BuybackTileBorders[i].IsValid())
		{
			BuybackTileBorders[i]->SetBorderBackgroundColor(FT66Style::Tokens::Panel2);
		}
		if (BuybackPriceTexts.IsValidIndex(i) && BuybackPriceTexts[i].IsValid())
		{
			BuybackPriceTexts[i]->SetText(bHasSlot
				? FText::Format(NSLOCTEXT("T66.Vendor", "BuyPriceFormat", "BUY ({0}g)"), FText::AsNumber(SellPrice > 0 ? SellPrice : 1))
				: (Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY")));
		}
		if (BuybackBuyButtons.IsValidIndex(i) && BuybackBuyButtons[i].IsValid())
		{
			BuybackBuyButtons[i]->SetEnabled(bHasSlot && RunState->GetCurrentGold() >= (SellPrice > 0 ? SellPrice : 1) && RunState->HasInventorySpace());
		}
	}
}


FReply UT66GamblerOverlayWidget::OnSelectInventorySlot(int32 InventoryIndex)
{
	SelectedInventoryIndex = InventoryIndex;
	RefreshInventory();
	RefreshSellPanel();
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnSellSelectedClicked()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return FReply::Handled();

	if (SelectedInventoryIndex < 0)
	{
		SetStatus(NSLOCTEXT("T66.Vendor", "SelectItemToSell", "Select an item to sell."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}

	const bool bSold = RunState->SellInventoryItemAt(SelectedInventoryIndex);
	if (bSold)
	{
		SetStatus(NSLOCTEXT("T66.Vendor", "SoldStatus", "Sold."), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
	}
	else
	{
		SetStatus(NSLOCTEXT("T66.Vendor", "CouldNotSell", "Could not sell."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
	}

	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
	RefreshBuyback();
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnBuybackSlot(int32 SlotIndex)
{
	UT66RunStateSubsystem* RunState = GetWorld() && GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return FReply::Handled();

	const bool bBought = RunState->TryBuybackSlot(SlotIndex);
	SetStatus(bBought
		? NSLOCTEXT("T66.Vendor", "Purchased", "Purchased.")
		: NSLOCTEXT("T66.Vendor", "CouldNotPurchase", "Could not purchase."),
		bBought ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();
	RefreshBuyback();
	return FReply::Handled();
}

void UT66GamblerOverlayWidget::RefreshInventory()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!RunState) return;

	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	const TArray<FName>& Inv = RunState->GetInventory();
	const TArray<FT66InventorySlot>& InvSlots = RunState->GetInventorySlots();
	TMap<FString, int32> StackCounts;
	for (const FT66InventorySlot& InventorySlotData : InvSlots)
	{
		if (InventorySlotData.IsValid())
		{
			StackCounts.FindOrAdd(MakeGamblerEconomyInventoryStackKey(InventorySlotData))++;
		}
	}

	// Auto-select the first valid item so the details panel is "big" immediately.
	if (SelectedInventoryIndex < 0)
	{
		for (int32 i = 0; i < Inv.Num(); ++i)
		{
			if (!Inv[i].IsNone())
			{
				SelectedInventoryIndex = i;
				break;
			}
		}
	}
	if (SelectedInventoryIndex >= Inv.Num())
	{
		SelectedInventoryIndex = (Inv.Num() > 0) ? (Inv.Num() - 1) : -1;
	}

	for (int32 i = 0; i < InventorySlotTexts.Num(); ++i)
	{
		const bool bHasItem = Inv.IsValidIndex(i) && !Inv[i].IsNone();
		if (InventorySlotTexts[i].IsValid())
		{
			// Keep the strip clean: icons for items, "-" for empty slots.
			InventorySlotTexts[i]->SetText(bHasItem ? FText::GetEmpty() : NSLOCTEXT("T66.Common", "Dash", "-"));
		}
		if (InventorySlotButtons.IsValidIndex(i) && InventorySlotButtons[i].IsValid())
		{
			InventorySlotButtons[i]->SetEnabled(bHasItem && !IsBossActive());
		}
		const int32 StackCount = (bHasItem && InvSlots.IsValidIndex(i) && InvSlots[i].IsValid())
			? StackCounts.FindRef(MakeGamblerEconomyInventoryStackKey(InvSlots[i]))
			: 0;
		if (InventorySlotCountTexts.IsValidIndex(i) && InventorySlotCountTexts[i].IsValid())
		{
			InventorySlotCountTexts[i]->SetText(
				StackCount > 1
					? FText::Format(NSLOCTEXT("T66.Inventory", "StackCountFormat", "{0}x"), FText::AsNumber(StackCount))
					: FText::GetEmpty());
			InventorySlotCountTexts[i]->SetVisibility(StackCount > 1 ? EVisibility::Visible : EVisibility::Hidden);
		}
		if (InventorySlotBorders.IsValidIndex(i) && InventorySlotBorders[i].IsValid())
		{
			FLinearColor Fill = FT66Style::Tokens::Panel2;
			FItemData D;
			const bool bHasData = bHasItem && T66GI && T66GI->GetItemData(Inv[i], D);

			if (i == SelectedInventoryIndex)
			{
				Fill = (Fill * 0.45f + FT66Style::Tokens::Accent * 0.55f);
			}
			InventorySlotBorders[i]->SetBorderBackgroundColor(Fill);

			if (InventorySlotIconBrushes.IsValidIndex(i) && InventorySlotIconBrushes[i].IsValid())
			{
				const ET66ItemRarity SlotRarity = (bHasData && InvSlots.IsValidIndex(i)) ? InvSlots[i].Rarity : ET66ItemRarity::Black;
				const TSoftObjectPtr<UTexture2D> SlotIconSoft = bHasData ? D.GetIconForRarity(SlotRarity) : TSoftObjectPtr<UTexture2D>();
				if (!SlotIconSoft.IsNull() && TexPool)
				{
					T66SlateTexture::BindSharedBrushAsync(TexPool, SlotIconSoft, this, InventorySlotIconBrushes[i], FName(TEXT("GamblerInv"), i + 1), /*bClearWhileLoading*/ true);
				}
				else
				{
					InventorySlotIconBrushes[i]->SetResourceObject(nullptr);
				}
			}
			if (InventorySlotIconImages.IsValidIndex(i) && InventorySlotIconImages[i].IsValid())
			{
				const ET66ItemRarity SlotRarity = (bHasData && InvSlots.IsValidIndex(i)) ? InvSlots[i].Rarity : ET66ItemRarity::Black;
				const bool bHasIcon = bHasData && !D.GetIconForRarity(SlotRarity).IsNull();
				InventorySlotIconImages[i]->SetVisibility(bHasIcon ? EVisibility::Visible : EVisibility::Hidden);
			}
		}
	}
	RefreshStatsPanel();
}

void UT66GamblerOverlayWidget::RefreshStatsPanel()
{
	if (!LiveStatsPanel.IsValid()) return;
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = nullptr;
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	LiveStatsPanel->Update(RunState, Loc);
}

void UT66GamblerOverlayWidget::RefreshSellPanel()
{
	UWorld* World = GetWorld();
	UGameInstance* GI0 = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI0 ? GI0->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const TArray<FName>& Inv = RunState->GetInventory();
	const bool bValidSelection = Inv.IsValidIndex(SelectedInventoryIndex) && !Inv[SelectedInventoryIndex].IsNone();

	if (SellPanelContainer.IsValid())
	{
		// Keep visible so the inventory layout doesn't "pop" when selecting an item.
		SellPanelContainer->SetVisibility(EVisibility::Visible);
	}
	if (!bValidSelection)
	{
		if (SellItemNameText.IsValid()) SellItemNameText->SetText(FText::GetEmpty());
		if (SellItemDescText.IsValid()) SellItemDescText->SetText(FText::GetEmpty());
		if (SellItemPriceText.IsValid()) SellItemPriceText->SetText(FText::GetEmpty());
		if (SellItemButton.IsValid()) SellItemButton->SetEnabled(false);
		return;
	}

	UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	FItemData D;
	const bool bHasData = T66GI && T66GI->GetItemData(Inv[SelectedInventoryIndex], D);
	ET66ItemRarity SelectedRarity = ET66ItemRarity::Black;
	int32 MainValue = 0;
	float Line2Multiplier = 0.f;
	if (RunState)
	{
		const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
		if (SelectedInventoryIndex >= 0 && SelectedInventoryIndex < Slots.Num())
		{
			MainValue = Slots[SelectedInventoryIndex].Line1RolledValue;
			SelectedRarity = Slots[SelectedInventoryIndex].Rarity;
			Line2Multiplier = Slots[SelectedInventoryIndex].GetLine2Multiplier();
		}
	}

	UT66LocalizationSubsystem* Loc = GI0 ? GI0->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	if (SellItemNameText.IsValid())
	{
		SellItemNameText->SetText(Loc ? Loc->GetText_ItemDisplayNameForRarity(Inv[SelectedInventoryIndex], SelectedRarity) : FText::FromName(Inv[SelectedInventoryIndex]));
	}

	if (SellItemDescText.IsValid())
	{
		if (!bHasData)
		{
			SellItemDescText->SetText(FText::GetEmpty());
		}
		else
		{
			const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
			SellItemDescText->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, D, SelectedRarity, MainValue, ScaleMult, Line2Multiplier));
		}
	}

	if (SellItemPriceText.IsValid())
	{
		int32 SellValue = 0;
		if (bHasData && RunState)
		{
			const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
			if (SelectedInventoryIndex >= 0 && SelectedInventoryIndex < Slots.Num())
			{
				SellValue = RunState->GetSellGoldForInventorySlot(Slots[SelectedInventoryIndex]);
			}
		}
		SellItemPriceText->SetText(FText::Format(
			NSLOCTEXT("T66.Vendor", "SellForFormat", "SELL FOR: {0}g"),
			FText::AsNumber(SellValue)));
	}

	if (SellItemButton.IsValid())
	{
		SellItemButton->SetEnabled(!IsBossActive());
	}
}


void UT66GamblerOverlayWidget::TriggerGamblerBossIfAngry()
{
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (PC->TriggerCasinoBossIfAngry())
		{
			return;
		}
	}

	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	if (RunState->GetGamblerAnger01() < 1.f)
	{
		return;
	}

	// Spawn GamblerBoss at gambler NPC location (and remove the NPC).
	FVector SpawnLoc = FVector::ZeroVector;
	AT66GamblerNPC* Gambler = GetRegisteredGamblerEconomyNpc<AT66GamblerNPC>(World);
	if (Gambler)
	{
		SpawnLoc = Gambler->GetActorLocation();
		Gambler->Destroy();
	}
	else
	{
		if (APawn* P = GetOwningPlayerPawn())
		{
			SpawnLoc = P->GetActorLocation() + FVector(600.f, 0.f, 0.f);
		}
	}

	// Avoid duplicates
	if (HasRegisteredGamblerEconomyBoss(World))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66GamblerBoss>(AT66GamblerBoss::StaticClass(), SpawnLoc + FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator, Params);
}

bool UT66GamblerOverlayWidget::IsBossActive() const
{
	return bCachedBossActive;
}

bool UT66GamblerOverlayWidget::TryPayToPlay()
{
	if (bInputLocked) return false;
	if (GambleAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_GambleAmountMustBePositive() : NSLOCTEXT("T66.Gambler", "GambleAmountMustBePositive", "Gamble amount must be > 0."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return false;
	}
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				if (!RunState->TrySpendGold(GambleAmount))
				{
					UT66LocalizationSubsystem* Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
					SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
					RefreshTopBar();
					return false;
				}
				RefreshTopBar();
				return true;
			}
		}
	}
	return false;
}

bool UT66GamblerOverlayWidget::TryPayWithLockedBet()
{
	if (bInputLocked) return false;
	if (LockedBetAmount <= 0)
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (World->GetGameInstance())
			{
				Loc2 = World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_BetFirst() : NSLOCTEXT("T66.Gambler", "BetFirst", "Bet First"), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return false;
	}
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || !RunState->TrySpendGold(LockedBetAmount))
	{
		UT66LocalizationSubsystem* Loc2 = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return false;
	}
	PendingBetAmount = LockedBetAmount;
	LockedBetAmount = 0;
	RefreshTopBar();
	return true;
}

void UT66GamblerOverlayWidget::AwardWin()
{
	AwardWin(PendingBetAmount);
}

void UT66GamblerOverlayWidget::AwardWin(int32 BetAmount)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				const int64 Payout = static_cast<int64>(BetAmount) * 2;
				RunState->AddGold(static_cast<int32>(FMath::Clamp<int64>(Payout, 0, INT32_MAX)));
				RefreshTopBar();
			}
			if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
			{
				Achieve->NotifyGamblerWin();
			}
		}
	}
}

void UT66GamblerOverlayWidget::AwardPayout(int32 BetAmount, float Multiplier)
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				const int64 Payout = FMath::RoundToInt64(static_cast<double>(BetAmount) * static_cast<double>(Multiplier));
				RunState->AddGold(static_cast<int32>(FMath::Clamp<int64>(Payout, 0, INT32_MAX)));
				RefreshTopBar();
			}
		}
	}
}

