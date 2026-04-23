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

static FString MakeInventoryStackKey(const FT66InventorySlot& Slot)
{
	return FString::Printf(TEXT("%s|%d"), *Slot.ItemTemplateID.ToString(), static_cast<int32>(Slot.Rarity));
}

static FText BuildGamblerWagerText(const int32 WagerAmount)
{
	return WagerAmount > 0
		? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
		: FText::GetEmpty();
}

template <typename TNpcType>
static TNpcType* GetRegisteredGamblerOverlayNpc(UWorld* World)
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

static bool HasRegisteredGamblerOverlayBoss(UWorld* World)
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
	static int32 T66BuildNumberMask(const TSet<int32>& Numbers)
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

	static int32 T66BuildNumberMask(const TArray<int32>& Numbers)
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

	static int32 T66GetPlinkoPayoutTierFromSlot(const int32 SlotIndex)
	{
		static const int32 Tiers[9] = { 4, 3, 2, 1, 0, 1, 2, 3, 4 };
		return Tiers[FMath::Clamp(SlotIndex, 0, 8)];
	}

	static ET66Rarity T66BoxOpeningIndexToRarity(const int32 ColorIndex)
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


FReply UT66GamblerOverlayWidget::OnOpenCoinFlip()
{
	SetPage(EGamblerPage::CoinFlip);
	// Reset coin to Heads face
	if (CoinFlipImage.IsValid()) CoinFlipImage->SetImage(&CoinBrush_Heads);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenRps()
{
	SetPage(EGamblerPage::RockPaperScissors);
	// Hide hand images until a round is revealed
	if (RpsHandsContainer.IsValid()) RpsHandsContainer->SetVisibility(EVisibility::Collapsed);
	return FReply::Handled();
}


FReply UT66GamblerOverlayWidget::OnCoinFlipHeads() { ResolveCoinFlip(true); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnCoinFlipTails() { ResolveCoinFlip(false); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsRock() { ResolveRps(0); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsPaper() { ResolveRps(1); return FReply::Handled(); }
FReply UT66GamblerOverlayWidget::OnRpsScissors() { ResolveRps(2); return FReply::Handled(); }

void UT66GamblerOverlayWidget::ResolveCoinFlip(bool bChoseHeads)
{
	if (!TryPayWithLockedBet()) return;
	bInputLocked = true;
	PendingRevealType = ERevealType::CoinFlip;
	bPendingCoinFlipChoseHeads = bChoseHeads;
	bPendingCheatSucceeded = false;
	PendingOutcomePreDrawSeed = 0;
	PendingOutcomeDrawIndex = INDEX_NONE;
	PendingOutcomeExpectedChance01 = -1.f;
	ShowCheatPrompt();
}

void UT66GamblerOverlayWidget::ResolveRps(int32 PlayerChoice)
{
	if (!TryPayWithLockedBet()) return;
	bInputLocked = true;
	PendingRevealType = ERevealType::RockPaperScissors;
	PendingRpsPlayerChoice = FMath::Clamp(PlayerChoice, 0, 2);
	bPendingCheatSucceeded = false;
	PendingOutcomePreDrawSeed = 0;
	PendingOutcomeDrawIndex = INDEX_NONE;
	PendingOutcomeExpectedChance01 = -1.f;
	ShowCheatPrompt();
}


void UT66GamblerOverlayWidget::TickCoinSpin()
{
	if (!bCoinSpinActive || !CoinFlipImage.IsValid()) return;
	UWorld* World = GetWorld();
	if (!World) return;

	CoinSpinElapsed += 0.033f; // ~30 Hz
	const float Alpha = FMath::Clamp(CoinSpinElapsed / CoinSpinDuration, 0.f, 1.f);

	// Cycle: Heads → Side → Tails → Side → repeat
	// Speed slows down via ease-out (fast at start, slow at end)
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);
	// Total half-flips: start with many, decelerate. We do ~12 half-flips total.
	const float TotalHalfFlips = 12.f;
	const float CurrentHalfFlip = TotalHalfFlips * Ease;
	const int32 Phase = FMath::FloorToInt(CurrentHalfFlip * 2.f); // 0..N where each unit = quarter flip

	if (Alpha >= 1.f)
	{
		// Land on final result
		FinishCoinSpin();
		return;
	}

	// Cycling pattern within each half-flip: Heads/Tails → Side → Tails/Heads → Side...
	const int32 Step = Phase % 4;
	switch (Step)
	{
	case 0: CoinFlipImage->SetImage(&CoinBrush_Heads); break;
	case 1: CoinFlipImage->SetImage(&CoinBrush_Side); break;
	case 2: CoinFlipImage->SetImage(&CoinBrush_Tails); break;
	case 3: CoinFlipImage->SetImage(&CoinBrush_Side); break;
	}
}

void UT66GamblerOverlayWidget::FinishCoinSpin()
{
	bCoinSpinActive = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
	}
	// Show final face
	if (CoinFlipImage.IsValid())
	{
		CoinFlipImage->SetImage(bCoinSpinResultHeads ? &CoinBrush_Heads : &CoinBrush_Tails);
	}
}

// ── BlackJack card helpers ────────────────────────────────────────────────

