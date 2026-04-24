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

static FString MakeBlackJackInventoryStackKey(const FT66InventorySlot& Slot)
{
	return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
}

static FText BuildBlackJackWagerText(const int32 WagerAmount)
{
	return WagerAmount > 0
		? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
		: FText::GetEmpty();
}

template <typename TNpcType>
static TNpcType* GetRegisteredBlackJackNpc(UWorld* World)
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

static bool HasRegisteredBlackJackBoss(UWorld* World)
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
	static int32 T66BuildBlackJackNumberMask(const TSet<int32>& Numbers)
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

	static int32 T66BuildBlackJackNumberMask(const TArray<int32>& Numbers)
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

	static int32 T66GetBlackJackPlinkoPayoutTierFromSlot(const int32 SlotIndex)
	{
		static const int32 Tiers[9] = { 4, 3, 2, 1, 0, 1, 2, 3, 4 };
		return Tiers[FMath::Clamp(SlotIndex, 0, 8)];
	}

	static ET66Rarity T66BlackJackBoxOpeningIndexToRarity(const int32 ColorIndex)
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


FReply UT66GamblerOverlayWidget::OnOpenBlackJack()
{
	SetPage(EGamblerPage::BlackJack);
	return FReply::Handled();
}


FReply UT66GamblerOverlayWidget::OnBJDealClicked()
{
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
		return FReply::Handled();
	}
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || !RunState->TrySpendGold(LockedBetAmount))
	{
		UT66LocalizationSubsystem* Loc2 = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return FReply::Handled();
	}
	BJBetAmount = LockedBetAmount;
	LockedBetAmount = 0;
	RefreshTopBar();
	StartBlackJackRound();
	return FReply::Handled();
}


FReply UT66GamblerOverlayWidget::OnBJHit() { HandleBJHit(); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBJStand() { HandleBJStand(); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBJDouble() { HandleBJDouble(); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnBJSplit() { HandleBJSplit(); return FReply::Handled(); }


void UT66GamblerOverlayWidget::InitBJCardBrushes()
{
	BJCardBrushes.SetNum(53); // 0..51 face cards, 52 = back
	static constexpr float CardW = 80.f;
	static constexpr float CardH = 112.f;
	for (int32 i = 0; i < 53; ++i)
	{
		BJCardBrushes[i] = FSlateBrush();
		BJCardBrushes[i].ImageSize = FVector2D(CardW, CardH);
		BJCardBrushes[i].DrawAs = ESlateBrushDrawType::Image;
	}
}

FSlateBrush* UT66GamblerOverlayWidget::GetBJCardBrush(int32 CardIndex)
{
	if (CardIndex >= 0 && CardIndex < 52 && BJCardBrushes.Num() > CardIndex)
	{
		return &BJCardBrushes[CardIndex];
	}
	return GetBJCardBackBrush();
}

FSlateBrush* UT66GamblerOverlayWidget::GetBJCardBackBrush()
{
	if (BJCardBrushes.Num() > 52)
	{
		return &BJCardBrushes[52];
	}
	return &BlackJackCardBackBrush; // legacy fallback
}

void UT66GamblerOverlayWidget::StartBlackJackRound()
{
	BJDeck.SetNum(52);
	for (int32 i = 0; i < 52; ++i) BJDeck[i] = i;
	UT66RngSubsystem* RngSub = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			RngSub = GI->GetSubsystem<UT66RngSubsystem>();
		}
	}
	BJShufflePreDrawSeed = 0;
	BJShuffleStartDrawIndex = INDEX_NONE;
	BJActionSequence = TEXT("D");
	if (RngSub)
	{
		BJShufflePreDrawSeed = RngSub->GetRunStream().GetCurrentSeed();
		BJShuffleStartDrawIndex = RngSub->GetRunDrawCount();
	}
	// Fisher-Yates shuffle
	for (int32 i = 51; i >= 1; --i)
	{
		const int32 j = RngSub ? RngSub->RunRandRange(0, i) : FMath::RandRange(0, i);
		Swap(BJDeck[i], BJDeck[j]);
	}
	BJDealerHand.Reset();
	BJPlayerHands.Reset();
	BJPlayerHands.AddDefaulted(1);
	BJCurrentHandIndex = 0;
	// BJBetAmount set by OnOpenBlackJack before calling StartBlackJackRound
	BJDealerHoleRevealed = false;
	bBJPlayerBusted = false;
	BJPhase = EBlackJackPhase::WaitingForAction;

	// Deal 2 to player, 2 to dealer (one dealer card hidden)
	auto Draw = [this]() -> int32 { if (BJDeck.Num() == 0) return -1; const int32 c = BJDeck.Pop(); return c; };
	BJPlayerHands[0].Add(Draw());
	BJPlayerHands[0].Add(Draw());
	BJDealerHand.Add(Draw());
	BJDealerHand.Add(Draw());

	RefreshBlackJackCards();
}

void UT66GamblerOverlayWidget::RefreshBlackJackCards()
{
	const int32 MaxCards = FMath::Min(11, BlackJackDealerCardImages.Num());
	for (int32 i = 0; i < MaxCards; ++i)
	{
		if (BlackJackDealerCardImages[i].IsValid())
		{
			if (i == 0 && !BJDealerHoleRevealed)
			{
				// Hole card hidden — handled by the overlay visibility lambda
				BlackJackDealerCardImages[i]->SetVisibility(EVisibility::Collapsed);
			}
			else if (i < BJDealerHand.Num())
			{
				BlackJackDealerCardImages[i]->SetImage(GetBJCardBrush(BJDealerHand[i]));
				BlackJackDealerCardImages[i]->SetVisibility(EVisibility::Visible);
			}
			else
			{
				BlackJackDealerCardImages[i]->SetVisibility(EVisibility::Collapsed);
			}
		}
	}
	const TArray<int32>& PlayerHand = (BJPlayerHands.Num() > 0 && BJCurrentHandIndex < BJPlayerHands.Num())
		? BJPlayerHands[BJCurrentHandIndex] : (BJPlayerHands.Num() > 0 ? BJPlayerHands[0] : TArray<int32>());
	const int32 MaxPlayer = FMath::Min(11, BlackJackPlayerCardImages.Num());
	for (int32 i = 0; i < MaxPlayer; ++i)
	{
		if (BlackJackPlayerCardImages[i].IsValid())
		{
			if (i < PlayerHand.Num())
			{
				BlackJackPlayerCardImages[i]->SetImage(GetBJCardBrush(PlayerHand[i]));
				BlackJackPlayerCardImages[i]->SetVisibility(EVisibility::Visible);
			}
			else
			{
				BlackJackPlayerCardImages[i]->SetVisibility(EVisibility::Collapsed);
			}
		}
	}

	RefreshCasinoGameChrome();
}

int32 UT66GamblerOverlayWidget::BJHandValue(const TArray<int32>& Hand) const
{
	int32 value = 0;
	int32 aces = 0;
	for (int32 c : Hand)
	{
		if (c < 0) continue;
		const int32 r = c % 13;
		if (r == 0) { aces++; value += 11; }
		else if (r >= 9) value += 10; // 10, J, Q, K
		else value += r + 1;
	}
	while (value > 21 && aces > 0) { value -= 10; aces--; }
	return value;
}

FText UT66GamblerOverlayWidget::BJCardToText(int32 CardIndex) const
{
	if (CardIndex < 0 || CardIndex > 51) return FText::FromString(TEXT("?"));
	const int32 r = CardIndex % 13;
	const int32 s = CardIndex / 13;
	static const TCHAR* Ranks[] = { TEXT("A"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"), TEXT("10"), TEXT("J"), TEXT("Q"), TEXT("K") };
	static const TCHAR* Suits[] = { TEXT("\u2660"), TEXT("\u2665"), TEXT("\u2666"), TEXT("\u2663") }; // ♠♥♦♣
	FString str = FString::Printf(TEXT("%s%s"), Ranks[r], Suits[s]);
	return FText::FromString(str);
}

void UT66GamblerOverlayWidget::BJDealerPlay()
{
	BJDealerHoleRevealed = true;
	RefreshBlackJackCards();

	while (BJHandValue(BJDealerHand) < 17 && BJDeck.Num() > 0)
	{
		BJDealerHand.Add(BJDeck.Pop());
		RefreshBlackJackCards();
	}
	BJEndRound();
}

void UT66GamblerOverlayWidget::BJEndRound()
{
	BJPhase = EBlackJackPhase::RoundOver;
	const int32 playerVal = BJHandValue(BJPlayerHands[0]);
	const int32 dealerVal = BJHandValue(BJDealerHand);
	const bool playerBust = (playerVal > 21);
	bBJPlayerBusted = playerBust;
	const bool dealerBust = (dealerVal > 21);

	if (playerBust)
	{
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
	}
	else if (dealerBust)
	{
		bPendingWin = true;
		bPendingDraw = false;
		BJWinAmount = BJBetAmount;
	}
	else if (playerVal > dealerVal)
	{
		bPendingWin = true;
		bPendingDraw = false;
		BJWinAmount = BJBetAmount;
	}
	else if (playerVal < dealerVal)
	{
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
	}
	else
	{
		bPendingWin = false;
		bPendingDraw = true;
		BJWinAmount = 0;
	}

	PendingRevealType = ERevealType::BlackJack;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.5f, false);
	}
}

void UT66GamblerOverlayWidget::RevealBlackJackOutcome()
{
	UT66LocalizationSubsystem* Loc2 = nullptr;
	UT66RunStateSubsystem* RunState = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		}
	}
	const int32 PlayerValue = BJPlayerHands.Num() > 0 ? BJHandValue(BJPlayerHands[0]) : 0;
	const int32 DealerValue = BJHandValue(BJDealerHand);
	const int32 OutcomeTier = bPendingWin ? 2 : (bPendingDraw ? 1 : 0);
	const int32 PayoutGold = bPendingWin ? FMath::Clamp(BJBetAmount * 2, 0, INT32_MAX) : (bPendingDraw ? BJBetAmount : 0);
	if (RunState)
	{
		RunState->RecordLuckQuantityRoll(FName(TEXT("GamblerBlackJackOutcome")), OutcomeTier, 0, 2);
		RunState->RecordAntiCheatGamblerRound(
			ET66AntiCheatGamblerGameType::BlackJack,
			BJBetAmount,
			PayoutGold,
			false,
			false,
			bPendingWin,
			bPendingDraw,
			PlayerValue,
			DealerValue,
			OutcomeTier,
			bBJPlayerBusted ? 1 : 0,
			0,
			0,
			0,
			BJShufflePreDrawSeed,
			BJShuffleStartDrawIndex,
			0,
			INDEX_NONE,
			-1.f,
			BJActionSequence);
	}
	if (bPendingWin)
	{
		AwardWin(BJBetAmount);
		if (BlackJackResultText.IsValid())
			BlackJackResultText->SetText(Loc2 ? Loc2->GetText_Win() : NSLOCTEXT("T66.Gambler", "Win", "WIN"));
		const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
		SetStatus(FText::Format(WinFmt, FText::AsNumber(BJBetAmount)), FLinearColor(0.3f, 1.f, 0.4f, 1.f));
	}
	else if (bPendingDraw)
	{
		if (RunState) RunState->AddGold(BJBetAmount);
		RefreshTopBar();
		if (BlackJackResultText.IsValid())
			BlackJackResultText->SetText(Loc2 ? Loc2->GetText_Push() : NSLOCTEXT("T66.Gambler", "Push", "PUSH"));
		SetStatus(Loc2 ? Loc2->GetText_Push() : NSLOCTEXT("T66.Gambler", "Push", "PUSH"), FLinearColor(1.f, 1.f, 0.6f, 1.f));
	}
	else
	{
		const FText LossText = bBJPlayerBusted
			? (Loc2 ? Loc2->GetText_Bust() : NSLOCTEXT("T66.Gambler", "Bust", "BUST"))
			: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
		if (BlackJackResultText.IsValid()) BlackJackResultText->SetText(LossText);
		SetStatus(LossText, FLinearColor(1.f, 0.3f, 0.3f, 1.f));
	}

	// After outcome is shown, wait then animate cards being taken and reset for another round
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(BJCardsTakenTimerHandle);
		World->GetTimerManager().SetTimer(BJCardsTakenTimerHandle, this, &UT66GamblerOverlayWidget::StartBJCardsTakenAnimation, 2.0f, false);
	}
}

void UT66GamblerOverlayWidget::StartBJCardsTakenAnimation()
{
	UWorld* World = GetWorld();
	if (!World) return;
	World->GetTimerManager().ClearTimer(BJCardsTakenTimerHandle);
	BJCardsTakenStep = 0;
	World->GetTimerManager().SetTimer(BJCardsTakenTimerHandle, this, &UT66GamblerOverlayWidget::TickBJCardsTaken, 0.08f, true);
}

void UT66GamblerOverlayWidget::TickBJCardsTaken()
{
	constexpr int32 MaxCards = 11;
	// Steps 0..10: hide dealer cards; steps 11..21: hide player cards
	if (BJCardsTakenStep < MaxCards)
	{
		if (BlackJackDealerCardImages.IsValidIndex(BJCardsTakenStep) && BlackJackDealerCardImages[BJCardsTakenStep].IsValid())
		{
			BlackJackDealerCardImages[BJCardsTakenStep]->SetVisibility(EVisibility::Collapsed);
		}
		// Also hide dealer hole card back on step 0 if still visible
		if (BJCardsTakenStep == 0 && BlackJackDealerHoleCardBackImage.IsValid())
		{
			BlackJackDealerHoleCardBackImage->SetVisibility(EVisibility::Collapsed);
		}
	}
	else if (BJCardsTakenStep < MaxCards + MaxCards)
	{
		const int32 PlayerIdx = BJCardsTakenStep - MaxCards;
		if (BlackJackPlayerCardImages.IsValidIndex(PlayerIdx) && BlackJackPlayerCardImages[PlayerIdx].IsValid())
		{
			BlackJackPlayerCardImages[PlayerIdx]->SetVisibility(EVisibility::Collapsed);
		}
	}

	++BJCardsTakenStep;
	if (BJCardsTakenStep >= MaxCards + MaxCards)
	{
		UWorld* World = GetWorld();
		if (World) World->GetTimerManager().ClearTimer(BJCardsTakenTimerHandle);
		ResetBlackJackForNewRound();
	}
}

void UT66GamblerOverlayWidget::ResetBlackJackForNewRound()
{
	BJDeck.Reset();
	BJDealerHand.Reset();
	BJPlayerHands.Reset();
	BJCurrentHandIndex = 0;
	BJBetAmount = 0;
	BJDealerHoleRevealed = false;
	BJPhase = EBlackJackPhase::WaitingForAction;
	bBJPlayerBusted = false;
	BJWinAmount = 0;
	BJShufflePreDrawSeed = 0;
	BJShuffleStartDrawIndex = INDEX_NONE;
	BJActionSequence.Reset();

	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (World->GetGameInstance())
		{
			Loc2 = World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}
	if (BlackJackResultText.IsValid())
		BlackJackResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
	if (BlackJackStatusText.IsValid())
		BlackJackStatusText->SetText(FText::GetEmpty());

	RefreshBlackJackCards();
}

void UT66GamblerOverlayWidget::HandleBJHit()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction || BJPlayerHands.Num() == 0) return;
	if (BJDeck.Num() == 0) return;
	BJActionSequence += TEXT("H");
	BJPlayerHands[BJCurrentHandIndex].Add(BJDeck.Pop());
	RefreshBlackJackCards();
	if (BJHandValue(BJPlayerHands[BJCurrentHandIndex]) > 21)
	{
		BJPhase = EBlackJackPhase::PlayerBusted;
		bBJPlayerBusted = true;
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
		PendingRevealType = ERevealType::BlackJack;
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(RevealTimerHandle);
			World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.5f, false);
		}
	}
}

void UT66GamblerOverlayWidget::HandleBJStand()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction) return;
	BJActionSequence += TEXT("S");
	BJDealerPlay();
}

void UT66GamblerOverlayWidget::HandleBJDouble()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction || BJPlayerHands.Num() == 0) return;
	if (BJPlayerHands[BJCurrentHandIndex].Num() != 2) return; // only on first two cards
	UT66RunStateSubsystem* RunState = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	}
	if (!RunState || !RunState->TrySpendGold(BJBetAmount))
	{
		UWorld* World = GetWorld();
		UT66LocalizationSubsystem* Loc2 = (World && World->GetGameInstance()) ? World->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
		SetStatus(Loc2 ? Loc2->GetText_NotEnoughGold() : NSLOCTEXT("T66.Gambler", "NotEnoughGold", "Not enough gold."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		RefreshTopBar();
		return;
	}
	BJBetAmount += BJBetAmount;
	BJActionSequence += TEXT("D");
	RefreshTopBar();
	if (BJDeck.Num() > 0) BJPlayerHands[BJCurrentHandIndex].Add(BJDeck.Pop());
	RefreshBlackJackCards();
	if (BJHandValue(BJPlayerHands[BJCurrentHandIndex]) > 21)
	{
		BJPhase = EBlackJackPhase::PlayerBusted;
		bBJPlayerBusted = true;
		bPendingWin = false;
		bPendingDraw = false;
		BJWinAmount = 0;
		PendingRevealType = ERevealType::BlackJack;
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(RevealTimerHandle);
			World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, 0.5f, false);
		}
	}
	else
	{
		BJDealerPlay();
	}
}

void UT66GamblerOverlayWidget::HandleBJSplit()
{
	if (BJPhase != EBlackJackPhase::WaitingForAction || BJPlayerHands.Num() != 1) return;
	if (BJPlayerHands[0].Num() != 2) return;
	const int32 r0 = BJPlayerHands[0][0] % 13;
	const int32 r1 = BJPlayerHands[0][1] % 13;
	if (r0 != r1) return; // same rank only
	if (BJDeck.Num() < 2) return;
	BJActionSequence += TEXT("P");
	TArray<int32> hand1;
	hand1.Add(BJPlayerHands[0][0]);
	hand1.Add(BJDeck.Pop());
	BJPlayerHands[0].SetNum(1);
	BJPlayerHands[0].Add(BJDeck.Pop());
	BJPlayerHands.Add(hand1);
	BJCurrentHandIndex = 0;
	RefreshBlackJackCards();
}

