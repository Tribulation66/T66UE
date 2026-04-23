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


void UT66GamblerOverlayWidget::ShowCheatPrompt()
{
	bCheatPromptVisible = true;
	if (CheatPromptContainer.IsValid())
	{
		CheatPromptContainer->SetVisibility(EVisibility::Visible);
	}
}

void UT66GamblerOverlayWidget::HideCheatPrompt()
{
	bCheatPromptVisible = false;
	if (CheatPromptContainer.IsValid())
	{
		CheatPromptContainer->SetVisibility(EVisibility::Collapsed);
	}
}


FReply UT66GamblerOverlayWidget::OnCheatYes()
{
	if (!bInputLocked || PendingRevealType == ERevealType::None) return FReply::Handled();
	HideCheatPrompt();
	bPendingCheat = true;

	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = nullptr;
	UT66RngSubsystem* RngSub = nullptr;
	UT66GameInstance* T66GI = nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		T66GI = Cast<UT66GameInstance>(GI);
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		RngSub = GI->GetSubsystem<UT66RngSubsystem>();
		PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>();
	}

	bPendingWin = true;
	bPendingDraw = false;
	bPendingCheatSucceeded = false;
	PendingOutcomePreDrawSeed = 0;
	PendingOutcomeDrawIndex = INDEX_NONE;
	PendingOutcomeExpectedChance01 = -1.f;

	UT66LocalizationSubsystem* Loc2 = RunState ? RunState->GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	// Roll cheat success: success => force win with no anger increase. failure => anger increases and normal RNG plays out.
	bool bCheatSuccess = false;
	{
		const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
		const float Base = PlayerExperience
			? PlayerExperience->GetDifficultyGamblerCheatSuccessChanceBase(Difficulty)
			: 0.40f;
		if (RunState && RngSub && Base > 0.f)
		{
			RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
			const float Chance = RngSub->BiasChance01(Base);
			bCheatSuccess = RngSub->RollChance01(Chance);
			RunState->RecordLuckQuantityBool(
				FName(TEXT("GamblerCheatSuccess")),
				bCheatSuccess,
				Chance,
				RngSub->GetLastRunDrawIndex(),
				RngSub->GetLastRunPreDrawSeed());
		}
		else
		{
			bCheatSuccess = (FMath::FRand() < Base);
			if (RunState)
			{
				RunState->RecordLuckQuantityBool(FName(TEXT("GamblerCheatSuccess")), bCheatSuccess, Base);
			}
		}
	}
	bPendingCheatSucceeded = bCheatSuccess;

	if (!bCheatSuccess && RunState)
	{
		RunState->AddGamblerAngerFromBet(PendingBetAmount);
		RefreshTopBar();

		// If anger hit 100%, close overlay and spawn Gambler boss immediately.
		if (RunState->GetGamblerAnger01() >= 1.f)
		{
			CloseOverlay();
			TriggerGamblerBossIfAngry();
			return FReply::Handled();
		}
	}

	// Determine pending outcome.
	float RevealDelay = 0.35f;
	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
		{
			if (bCheatSuccess)
			{
				bPendingCoinFlipResultHeads = bPendingCoinFlipChoseHeads;
				bPendingWin = true;
			}
			else
			{
				if (RngSub)
				{
					PendingOutcomeExpectedChance01 = 0.5f;
					bPendingCoinFlipResultHeads = RngSub->RollChance01(PendingOutcomeExpectedChance01);
					PendingOutcomePreDrawSeed = RngSub->GetLastRunPreDrawSeed();
					PendingOutcomeDrawIndex = RngSub->GetLastRunDrawIndex();
				}
				else
				{
					bPendingCoinFlipResultHeads = FMath::RandBool();
				}
				bPendingWin = (bPendingCoinFlipChoseHeads == bPendingCoinFlipResultHeads);
				if (RunState)
				{
					RunState->RecordLuckQuantityBool(
						FName(TEXT("GamblerCoinFlipWin")),
						bPendingWin,
						0.5f);
				}
			}
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			// Start coin spin animation
			bCoinSpinResultHeads = bPendingCoinFlipResultHeads;
			bCoinSpinActive = true;
			CoinSpinElapsed = 0.f;
			if (World)
			{
				World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
				World->GetTimerManager().SetTimer(CoinSpinTimerHandle, this, &UT66GamblerOverlayWidget::TickCoinSpin, 0.033f, true);
			}
			RevealDelay = CoinSpinDuration + 0.15f; // reveal shortly after spin ends
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			if (bCheatSuccess)
			{
				// Opponent choice that loses to player
				PendingRpsOppChoice = (PendingRpsPlayerChoice + 2) % 3;
				bPendingWin = true;
			}
			else
			{
				bool bPlayerWins = false;
				if (RngSub)
				{
					PendingOutcomeExpectedChance01 = 0.5f;
					bPlayerWins = RngSub->RollChance01(PendingOutcomeExpectedChance01);
					PendingOutcomePreDrawSeed = RngSub->GetLastRunPreDrawSeed();
					PendingOutcomeDrawIndex = RngSub->GetLastRunDrawIndex();
				}
				else
				{
					bPlayerWins = FMath::RandBool();
				}
				bPendingWin = bPlayerWins;
				PendingRpsOppChoice = bPlayerWins
					? (PendingRpsPlayerChoice + 2) % 3
					: (PendingRpsPlayerChoice + 1) % 3;
				if (RunState)
				{
					RunState->RecordLuckQuantityBool(
						FName(TEXT("GamblerRpsWin")),
						bPendingWin,
						0.5f);
				}
			}
			if (RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		}
		default:
			break;
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, RevealDelay, false);
	}
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnCheatNo()
{
	if (!bInputLocked || PendingRevealType == ERevealType::None) return FReply::Handled();
	HideCheatPrompt();
	bPendingCheat = false;
	bPendingCheatSucceeded = false;
	PendingOutcomePreDrawSeed = 0;
	PendingOutcomeDrawIndex = INDEX_NONE;
	PendingOutcomeExpectedChance01 = -1.f;

	UWorld* World = GetWorld();
	UT66LocalizationSubsystem* Loc2 = nullptr;
	UT66RunStateSubsystem* RunState = nullptr;
	UT66RngSubsystem* RngSub = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		Loc2 = GI->GetSubsystem<UT66LocalizationSubsystem>();
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		RngSub = GI->GetSubsystem<UT66RngSubsystem>();
	}
	bPendingDraw = false;

	float RevealDelay = 0.35f;
	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
		{
			if (RngSub)
			{
				PendingOutcomeExpectedChance01 = 0.5f;
				bPendingCoinFlipResultHeads = RngSub->RollChance01(PendingOutcomeExpectedChance01);
				PendingOutcomePreDrawSeed = RngSub->GetLastRunPreDrawSeed();
				PendingOutcomeDrawIndex = RngSub->GetLastRunDrawIndex();
			}
			else
			{
				bPendingCoinFlipResultHeads = FMath::RandBool();
			}
			bPendingWin = (bPendingCoinFlipChoseHeads == bPendingCoinFlipResultHeads);
			if (RunState)
			{
				RunState->RecordLuckQuantityBool(
					FName(TEXT("GamblerCoinFlipWin")),
					bPendingWin,
					0.5f);
			}
			if (CoinFlipResultText.IsValid()) CoinFlipResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			// Start coin spin animation
			bCoinSpinResultHeads = bPendingCoinFlipResultHeads;
			bCoinSpinActive = true;
			CoinSpinElapsed = 0.f;
			if (World)
			{
				World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
				World->GetTimerManager().SetTimer(CoinSpinTimerHandle, this, &UT66GamblerOverlayWidget::TickCoinSpin, 0.033f, true);
			}
			RevealDelay = CoinSpinDuration + 0.15f;
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			bool bPlayerWins = false;
			if (RngSub)
			{
				PendingOutcomeExpectedChance01 = 0.5f;
				bPlayerWins = RngSub->RollChance01(PendingOutcomeExpectedChance01);
				PendingOutcomePreDrawSeed = RngSub->GetLastRunPreDrawSeed();
				PendingOutcomeDrawIndex = RngSub->GetLastRunDrawIndex();
			}
			else
			{
				bPlayerWins = FMath::RandBool();
			}
			bPendingWin = bPlayerWins;
			PendingRpsOppChoice = bPlayerWins
				? (PendingRpsPlayerChoice + 2) % 3
				: (PendingRpsPlayerChoice + 1) % 3;
			if (RunState)
			{
				RunState->RecordLuckQuantityBool(
					FName(TEXT("GamblerRpsWin")),
					bPendingWin,
					0.5f);
			}
			if (RpsResultText.IsValid()) RpsResultText->SetText(Loc2 ? Loc2->GetText_Rolling() : NSLOCTEXT("T66.Gambler", "Rolling", "Rolling..."));
			break;
		}
		default:
			bPendingWin = false;
			break;
	}

	if (World)
	{
		World->GetTimerManager().ClearTimer(RevealTimerHandle);
		World->GetTimerManager().SetTimer(RevealTimerHandle, this, &UT66GamblerOverlayWidget::RevealPendingOutcome, RevealDelay, false);
	}
	return FReply::Handled();
}

void UT66GamblerOverlayWidget::RevealPendingOutcome()
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

	switch (PendingRevealType)
	{
		case ERevealType::CoinFlip:
		{
			const int32 PayoutGold = bPendingWin ? FMath::Clamp(PendingBetAmount * 2, 0, INT32_MAX) : 0;
			const FText ResultStr = bPendingCoinFlipResultHeads
				? (Loc2 ? Loc2->GetText_Heads() : NSLOCTEXT("T66.Gambler", "Heads", "HEADS"))
				: (Loc2 ? Loc2->GetText_Tails() : NSLOCTEXT("T66.Gambler", "Tails", "TAILS"));
			if (bPendingWin) AwardWin();
			if (RunState)
			{
				RunState->RecordAntiCheatGamblerRound(
					ET66AntiCheatGamblerGameType::CoinFlip,
					PendingBetAmount,
					PayoutGold,
					bPendingCheat,
					bPendingCheatSucceeded,
					bPendingWin,
					false,
					bPendingCoinFlipChoseHeads ? 1 : 0,
					bPendingCoinFlipResultHeads ? 1 : 0,
					bPendingWin ? 1 : 0,
					0,
					0,
					0,
					0,
					0,
					INDEX_NONE,
					PendingOutcomePreDrawSeed,
					PendingOutcomeDrawIndex,
					PendingOutcomeExpectedChance01);
			}
			if (CoinFlipResultText.IsValid())
			{
				const FText Outcome = bPendingWin
					? (Loc2 ? Loc2->GetText_Win() : NSLOCTEXT("T66.Gambler", "Win", "WIN"))
					: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
				const FText Fmt = Loc2 ? Loc2->GetText_CoinFlipResultFormat() : NSLOCTEXT("T66.Gambler", "CoinFlipResultFormat", "Result: {0} — {1}");
				CoinFlipResultText->SetText(FText::Format(Fmt, ResultStr, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
			const FText Status = bPendingWin
				? FText::Format(WinFmt, FText::AsNumber(PendingBetAmount))
				: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::RockPaperScissors:
		{
			const int32 PayoutGold = bPendingWin ? FMath::Clamp(PendingBetAmount * 2, 0, INT32_MAX) : 0;
			if (bPendingWin) AwardWin();
			if (RunState)
			{
				RunState->RecordAntiCheatGamblerRound(
					ET66AntiCheatGamblerGameType::RockPaperScissors,
					PendingBetAmount,
					PayoutGold,
					bPendingCheat,
					bPendingCheatSucceeded,
					bPendingWin,
					false,
					PendingRpsPlayerChoice,
					PendingRpsOppChoice,
					bPendingWin ? 1 : 0,
					0,
					0,
					0,
					0,
					0,
					INDEX_NONE,
					PendingOutcomePreDrawSeed,
					PendingOutcomeDrawIndex,
					PendingOutcomeExpectedChance01);
			}
			// Show hand images
			if (RpsHandsContainer.IsValid()) RpsHandsContainer->SetVisibility(EVisibility::Visible);
			if (RpsHumanHandImage.IsValid())
			{
				FSlateBrush* HumanBrush = (PendingRpsPlayerChoice == 0) ? &RpsBrush_HumanRock
					: (PendingRpsPlayerChoice == 1) ? &RpsBrush_HumanPaper : &RpsBrush_HumanScissors;
				RpsHumanHandImage->SetImage(HumanBrush);
			}
			if (RpsDemonHandImage.IsValid())
			{
				FSlateBrush* DemonBrush = (PendingRpsOppChoice == 0) ? &RpsBrush_DemonRock
					: (PendingRpsOppChoice == 1) ? &RpsBrush_DemonPaper : &RpsBrush_DemonScissors;
				RpsDemonHandImage->SetImage(DemonBrush);
			}
			if (RpsResultText.IsValid())
			{
				const FText YouChoice = (PendingRpsPlayerChoice == 0) ? (Loc2 ? Loc2->GetText_Rock() : NSLOCTEXT("T66.Gambler", "Rock", "Rock"))
					: (PendingRpsPlayerChoice == 1) ? (Loc2 ? Loc2->GetText_Paper() : NSLOCTEXT("T66.Gambler", "Paper", "Paper"))
					: (Loc2 ? Loc2->GetText_Scissors() : NSLOCTEXT("T66.Gambler", "Scissors", "Scissors"));
				const FText OppChoice = (PendingRpsOppChoice == 0) ? (Loc2 ? Loc2->GetText_Rock() : NSLOCTEXT("T66.Gambler", "Rock", "Rock"))
					: (PendingRpsOppChoice == 1) ? (Loc2 ? Loc2->GetText_Paper() : NSLOCTEXT("T66.Gambler", "Paper", "Paper"))
					: (Loc2 ? Loc2->GetText_Scissors() : NSLOCTEXT("T66.Gambler", "Scissors", "Scissors"));
				const FText Outcome = bPendingWin
					? (Loc2 ? Loc2->GetText_Win() : NSLOCTEXT("T66.Gambler", "Win", "WIN"))
					: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
				const FText Fmt = Loc2 ? Loc2->GetText_RpsResultFormat() : NSLOCTEXT("T66.Gambler", "RpsResultFormat", "You: {0}  |  Gambler: {1}  —  {2}");
				RpsResultText->SetText(FText::Format(Fmt, YouChoice, OppChoice, Outcome));
			}
			const FText WinFmt = Loc2 ? Loc2->GetText_WinPlusAmountFormat() : NSLOCTEXT("T66.Gambler", "WinPlusAmountFormat", "WIN (+{0})");
			const FText Status = bPendingWin
				? FText::Format(WinFmt, FText::AsNumber(PendingBetAmount))
				: (Loc2 ? Loc2->GetText_Lose() : NSLOCTEXT("T66.Gambler", "Lose", "LOSE"));
			SetStatus(Status,
				bPendingWin ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
			break;
		}
		case ERevealType::BlackJack:
			RevealBlackJackOutcome();
			break;
		default:
			break;
	}

	PendingRevealType = ERevealType::None;
	bInputLocked = false;
	bPendingCheat = false;
	bPendingCheatSucceeded = false;
	PendingOutcomePreDrawSeed = 0;
	PendingOutcomeDrawIndex = INDEX_NONE;
	PendingOutcomeExpectedChance01 = -1.f;
}

// ── Coin flip spin animation ──────────────────────────────────────────────

