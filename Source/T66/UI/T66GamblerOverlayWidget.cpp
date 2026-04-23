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


void UT66GamblerOverlayWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->BuybackChanged.RemoveDynamic(this, &UT66GamblerOverlayWidget::HandleBuybackChanged);
				RunState->BossChanged.RemoveDynamic(this, &UT66GamblerOverlayWidget::HandleBossChanged);
			}
		}
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
		World->GetTimerManager().ClearTimer(LotteryDrawTimerHandle);
		World->GetTimerManager().ClearTimer(PlinkoTimerHandle);
		World->GetTimerManager().ClearTimer(BoxOpeningTimerHandle);
	}
	Super::NativeDestruct();
}

void UT66GamblerOverlayWidget::SetWinGoldAmount(int32 InAmount)
{
	WinGoldAmount = FMath::Max(0, InAmount);
	// Use DT value as a default suggested bet amount (player can change it in the UI).
	GambleAmount = FMath::Max(0, WinGoldAmount);
	if (GambleAmountSpin.IsValid())
	{
		GambleAmountSpin->SetValue(GambleAmount);
	}
	RefreshTopBar();
}


FReply UT66GamblerOverlayWidget::OnBack()
{
	// Back should always exit the interaction (close the overlay).
	CloseOverlay();
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnDialogueGamble()
{
	SetPage(EGamblerPage::Casino);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnDialogueTeleport()
{
	if (IsBossActive())
	{
		UT66LocalizationSubsystem* Loc2 = nullptr;
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}
		SetStatus(Loc2 ? Loc2->GetText_TeleportDisabledBossActive() : NSLOCTEXT("T66.Gambler", "TeleportDisabledBossActive", "Teleport disabled (boss encounter started)."), FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		return FReply::Handled();
	}
	if (bEmbeddedInCasinoShell)
	{
		TeleportToVendor();
		return FReply::Handled();
	}
	TeleportToVendor();
	CloseOverlay();
	return FReply::Handled();
}


FReply UT66GamblerOverlayWidget::OnGameBackToSelection()
{
	SetPage(EGamblerPage::Casino);
	if (GameSelectionSwitcher.IsValid()) GameSelectionSwitcher->SetActiveWidgetIndex(0);
	return FReply::Handled();
}


void UT66GamblerOverlayWidget::SetPage(EGamblerPage Page)
{
	if (!PageSwitcher.IsValid()) return;
	LockedBetAmount = 0; // changing game requires locking a new bet
	// PageSwitcher now has 2 pages: Dialogue (0) and Casino (1).
	// CasinoSwitcher contains the sub-views: Cards (0), CoinFlip (1), RPS (2), FindBall (3).
	int32 PageIndex = 0;
	int32 CasinoIndex = 0;
	switch (Page)
	{
		case EGamblerPage::Dialogue:
			PageIndex = 0;
			CasinoIndex = 0;
			break;
		case EGamblerPage::Casino:
			PageIndex = 1;
			CasinoIndex = 0;
			break;
		case EGamblerPage::CoinFlip:
			PageIndex = 1;
			CasinoIndex = 1;
			break;
		case EGamblerPage::RockPaperScissors:
			PageIndex = 1;
			CasinoIndex = 2;
			break;
		case EGamblerPage::BlackJack:
			PageIndex = 1;
			CasinoIndex = 3;
			// Reset BJ table so Deal button shows and table is empty
			BJDeck.Reset();
			BJDealerHand.Reset();
			BJPlayerHands.Reset();
			BJCurrentHandIndex = 0;
			BJBetAmount = 0;
			BJDealerHoleRevealed = false;
			BJPhase = EBlackJackPhase::WaitingForAction;
			bBJPlayerBusted = false;
			BJWinAmount = 0;
			RefreshBlackJackCards();
			break;
		case EGamblerPage::Lottery:
			PageIndex = 1;
			CasinoIndex = 4;
			if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(LotteryDrawTimerHandle);
			LotterySelected.Reset();
			LotteryDrawn.Reset();
			LotteryRevealedCount = 0;
			break;
		case EGamblerPage::Plinko:
			PageIndex = 1;
			CasinoIndex = 5;
			if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(PlinkoTimerHandle);
			PlinkoBallSlot = 4;
			PlinkoRow = 0;
			break;
		case EGamblerPage::BoxOpening:
			PageIndex = 1;
			CasinoIndex = 6;
			if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(BoxOpeningTimerHandle);
			BoxOpeningCurrentIndex = 0;
			BoxOpeningResultIndex = 0;
			BoxOpeningSpinSteps = 0;
			break;
		default:
			PageIndex = 0;
			CasinoIndex = 0;
			break;
	}

	PageSwitcher->SetActiveWidgetIndex(PageIndex);
	if (CasinoSwitcher.IsValid())
	{
		CasinoSwitcher->SetActiveWidgetIndex(CasinoIndex);
	}
	bInputLocked = false;
	SetStatus(FText::GetEmpty());
	if (CoinFlipStatusText.IsValid()) CoinFlipStatusText->SetText(FText::GetEmpty());
	if (RpsStatusText.IsValid()) RpsStatusText->SetText(FText::GetEmpty());
	if (BlackJackStatusText.IsValid()) BlackJackStatusText->SetText(FText::GetEmpty());
	RefreshTopBar();
	RefreshInventory();
	RefreshSellPanel();

	// Clear per-game result text when opening pages
	UT66LocalizationSubsystem* Loc2 = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
		}
	}
	if (Page == EGamblerPage::CoinFlip && CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
	if (Page == EGamblerPage::RockPaperScissors && RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_PickOne() : NSLOCTEXT("T66.Gambler", "PickOne", "Pick one."));
	if (Page == EGamblerPage::BlackJack && BlackJackResultText.IsValid()) BlackJackResultText->SetText(Loc2 ? Loc2->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
	if (Page == EGamblerPage::Lottery)
	{
		for (int32 i = 0; i < 10 && i < LotteryNumberBorders.Num(); ++i)
			if (LotteryNumberBorders[i].IsValid()) LotteryNumberBorders[i]->SetBorderBackgroundColor(FT66Style::Tokens::Panel2);
		if (LotteryPicksText.IsValid()) LotteryPicksText->SetText(NSLOCTEXT("T66.Gambler", "YourPicks", "Your picks:"));
		if (LotteryDrawnText.IsValid()) LotteryDrawnText->SetText(NSLOCTEXT("T66.Gambler", "Drawn", "Drawn:"));
		if (LotteryResultText.IsValid()) LotteryResultText->SetText(FText::GetEmpty());
	}
	if (Page == EGamblerPage::Plinko)
	{
		if (PlinkoResultText.IsValid()) PlinkoResultText->SetText(FText::GetEmpty());
		UpdatePlinkoBallVisual();
	}
	if (Page == EGamblerPage::BoxOpening)
	{
		if (BoxOpeningResultText.IsValid()) BoxOpeningResultText->SetText(FText::GetEmpty());
		RefreshBoxOpeningStrip();
	}
}

void UT66GamblerOverlayWidget::OpenCasinoPage()
{
	SetPage(EGamblerPage::Casino);
	RefreshTopBar();
	RefreshBuyback();
}


void UT66GamblerOverlayWidget::TeleportToVendor()
{
	if (bEmbeddedInCasinoShell)
	{
		if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
		{
			PC->SwitchCircusOverlayToVendor();
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return;

	AT66VendorNPC* Vendor = GetRegisteredGamblerOverlayNpc<AT66VendorNPC>(World);
	if (!Vendor) return;

	const FVector VendorLoc = Vendor->GetActorLocation();
	Pawn->SetActorLocation(VendorLoc + FVector(250.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
}

void UT66GamblerOverlayWidget::CloseOverlay()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
	}
	PendingRevealType = ERevealType::None;
	bInputLocked = false;
	HideCheatPrompt();

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		if (bEmbeddedInCasinoShell)
		{
			PC->CloseCircusOverlay();
			return;
		}
	}

	RemoveFromParent();
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->RestoreGameplayInputMode();
	}
}

void UT66GamblerOverlayWidget::SetStatus(const FText& Msg, const FLinearColor& Color)
{
	// When a game is active, show status in that game panel instead of the top bar
	if (CasinoSwitcher.IsValid())
	{
		const int32 Idx = CasinoSwitcher->GetActiveWidgetIndex();
		if (Idx == 1 && CoinFlipStatusText.IsValid())
		{
			CoinFlipStatusText->SetText(Msg);
			CoinFlipStatusText->SetColorAndOpacity(FSlateColor(Color));
			if (StatusText.IsValid()) { StatusText->SetText(FText::GetEmpty()); }
			return;
		}
		if (Idx == 2 && RpsStatusText.IsValid())
		{
			RpsStatusText->SetText(Msg);
			RpsStatusText->SetColorAndOpacity(FSlateColor(Color));
			if (StatusText.IsValid()) { StatusText->SetText(FText::GetEmpty()); }
			return;
		}
		if (Idx == 3 && BlackJackStatusText.IsValid())
		{
			BlackJackStatusText->SetText(Msg);
			BlackJackStatusText->SetColorAndOpacity(FSlateColor(Color));
			if (StatusText.IsValid()) { StatusText->SetText(FText::GetEmpty()); }
			return;
		}
	}
	if (StatusText.IsValid())
	{
		StatusText->SetText(Msg);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UT66GamblerOverlayWidget::HandleBuybackChanged()
{
	RefreshTopBar();
	RefreshBuyback();
}

void UT66GamblerOverlayWidget::HandleBossChanged()
{
	bCachedBossActive = false;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				bCachedBossActive = RunState->GetBossActive();
			}
		}
	}

	RefreshInventory();
	RefreshSellPanel();
}

