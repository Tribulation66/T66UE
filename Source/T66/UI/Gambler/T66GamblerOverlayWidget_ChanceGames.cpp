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


FReply UT66GamblerOverlayWidget::OnMoreGamesClicked()
{
	if (GameSelectionSwitcher.IsValid()) GameSelectionSwitcher->SetActiveWidgetIndex(1);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnBackToMainGames()
{
	if (GameSelectionSwitcher.IsValid()) GameSelectionSwitcher->SetActiveWidgetIndex(0);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenLottery()
{
	SetPage(EGamblerPage::Lottery);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenPlinko()
{
	SetPage(EGamblerPage::Plinko);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnOpenBoxOpening()
{
	SetPage(EGamblerPage::BoxOpening);
	return FReply::Handled();
}

FReply UT66GamblerOverlayWidget::OnLotteryNumberClicked(int32 Num)
{
	if (Num < 1 || Num > 10) return FReply::Handled();
	if (LotterySelected.Contains(Num))
		LotterySelected.Remove(Num);
	else if (LotterySelected.Num() < 5)
		LotterySelected.Add(Num);
	for (int32 i = 0; i < 10 && i < LotteryNumberBorders.Num(); ++i)
	{
		if (LotteryNumberBorders[i].IsValid())
			LotteryNumberBorders[i]->SetBorderBackgroundColor(LotterySelected.Contains(i + 1) ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2);
	}
	if (LotteryPicksText.IsValid())
	{
		TArray<int32> Sorted(LotterySelected.Array());
		Sorted.Sort();
		FString Str;
		for (int32 i = 0; i < Sorted.Num(); ++i) { if (i) Str += TEXT(", "); Str += FString::FromInt(Sorted[i]); }
		LotteryPicksText->SetText(FText::FromString(FString::Printf(TEXT("Your picks: %s"), *Str)));
	}
	return FReply::Handled();
}

void UT66GamblerOverlayWidget::StartLotteryDraw()
{
	LotteryDrawn.Reset();
	LotteryShufflePreDrawSeed = 0;
	LotteryShuffleStartDrawIndex = INDEX_NONE;
	UT66RunStateSubsystem* RunState = nullptr;
	UT66RngSubsystem* RngSub = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			RngSub = GI->GetSubsystem<UT66RngSubsystem>();
		}
	}
	TArray<int32> Pool;
	for (int32 i = 1; i <= 10; ++i) Pool.Add(i);
	if (RngSub)
	{
		LotteryShufflePreDrawSeed = RngSub->GetRunStream().GetCurrentSeed();
		LotteryShuffleStartDrawIndex = RngSub->GetRunDrawCount();
		for (int32 i = Pool.Num() - 1; i > 0; --i)
			Pool.Swap(i, RngSub->RunRandRange(0, i));
	}
	else
	{
		for (int32 i = Pool.Num() - 1; i > 0; --i)
			Pool.Swap(i, FMath::RandRange(0, i));
	}
	for (int32 i = 0; i < 5; ++i) LotteryDrawn.Add(Pool[i]);
	LotteryRevealedCount = 0;
	if (LotteryDrawnText.IsValid()) LotteryDrawnText->SetText(NSLOCTEXT("T66.Gambler", "Drawn", "Drawn:"));
	if (LotteryResultText.IsValid()) LotteryResultText->SetText(FText::GetEmpty());
	UWorld* World = GetWorld();
	if (World) World->GetTimerManager().SetTimer(LotteryDrawTimerHandle, this, &UT66GamblerOverlayWidget::TickLotteryRevealNext, 0.6f, true, 0.1f);
}

void UT66GamblerOverlayWidget::TickLotteryRevealNext()
{
	UWorld* World = GetWorld();
	if (!World || LotteryRevealedCount >= 5)
	{
		if (World) World->GetTimerManager().ClearTimer(LotteryDrawTimerHandle);
		if (LotteryRevealedCount >= 5)
		{
			int32 Matches = 0;
			for (int32 D : LotteryDrawn)
				if (LotterySelected.Contains(D)) ++Matches;
			static const float PayoutMultiplier[] = { 0.f, 0.25f, 0.5f, 1.f, 5.f, 30.f };
			float Mult = (Matches >= 0 && Matches <= 5) ? PayoutMultiplier[Matches] : 0.f;
			const int32 PayoutGold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(PendingBetAmount) * Mult));
			AwardPayout(PendingBetAmount, Mult);
			if (UT66RunStateSubsystem* RunState = World->GetGameInstance()
				? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>()
				: nullptr)
			{
				RunState->RecordLuckQuantityRoll(FName(TEXT("GamblerLotteryMatches")), Matches, 0, 5);
				RunState->RecordAntiCheatGamblerRound(
					ET66AntiCheatGamblerGameType::Lottery,
					PendingBetAmount,
					PayoutGold,
					false,
					false,
					PayoutGold > PendingBetAmount,
					PayoutGold == PendingBetAmount && PendingBetAmount > 0,
					INDEX_NONE,
					INDEX_NONE,
					Matches,
					0,
					T66BuildNumberMask(LotterySelected),
					T66BuildNumberMask(LotteryDrawn),
					0,
					LotteryShufflePreDrawSeed,
					LotteryShuffleStartDrawIndex);
			}
			if (LotteryResultText.IsValid())
				LotteryResultText->SetText(FText::Format(
					NSLOCTEXT("T66.Gambler", "LotteryResultFmt", "Matches: {0} — Payout: {1}x"),
					FText::AsNumber(Matches), FText::AsNumber(FMath::RoundToInt(Mult * 100) / 100.0)));
		}
		return;
	}
	LotteryRevealedCount++;
	FString DrawnStr = TEXT("Drawn: ");
	for (int32 i = 0; i < LotteryRevealedCount; ++i)
	{
		if (i) DrawnStr += TEXT(", ");
		DrawnStr += FString::FromInt(LotteryDrawn[i]);
	}
	if (LotteryDrawnText.IsValid()) LotteryDrawnText->SetText(FText::FromString(DrawnStr));
}

FLinearColor UT66GamblerOverlayWidget::GetBoxOpeningColor(int32 ColorIndex) const
{
	switch (ColorIndex % BoxColorCount)
	{
		case 0: return FLinearColor(0.15f, 0.15f, 0.15f, 1.f);   // Black
		case 1: return FLinearColor(0.9f, 0.2f, 0.2f, 1.f);       // Red
		case 2: return FLinearColor(0.95f, 0.85f, 0.2f, 1.f);     // Yellow
		case 3: return FLinearColor(0.95f, 0.95f, 0.95f, 1.f);    // White
		default: return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	}
}

void UT66GamblerOverlayWidget::RefreshBoxOpeningStrip()
{
	for (int32 Index = 0; Index < BoxOpeningColorBorders.Num(); ++Index)
	{
		if (BoxOpeningColorBorders[Index].IsValid())
		{
			BoxOpeningColorBorders[Index]->SetBorderBackgroundColor(
				FSlateColor(GetBoxOpeningColor((BoxOpeningStripOffset + Index) % BoxColorCount)));
		}
	}
}

void UT66GamblerOverlayWidget::StartPlinkoDrop()
{
	// 9 slots: classic plinko payouts (middle = 0.25x). Slots 0..8 left to right.
	PlinkoBallSlot = 4;
	PlinkoRow = 0;
	PlinkoPathBits = 0;
	PlinkoPreDrawSeed = 0;
	PlinkoStartDrawIndex = INDEX_NONE;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
			{
				PlinkoPreDrawSeed = RngSub->GetRunStream().GetCurrentSeed();
				PlinkoStartDrawIndex = RngSub->GetRunDrawCount();
			}
		}
	}
	if (PlinkoResultText.IsValid()) PlinkoResultText->SetText(FText::GetEmpty());
	UpdatePlinkoBallVisual();
	UWorld* World = GetWorld();
	if (World) World->GetTimerManager().SetTimer(PlinkoTimerHandle, this, &UT66GamblerOverlayWidget::TickPlinkoDrop, 0.12f, true, 0.05f);
}

void UT66GamblerOverlayWidget::TickPlinkoDrop()
{
	UWorld* World = GetWorld();
	// 8 rows of pins; each step 50% left (-1 slot) or right (+1 slot), clamped to 0..8
	static const float SlotPayouts[PlinkoSlotCount] = { 30.f, 5.f, 1.f, 0.5f, 0.25f, 0.5f, 1.f, 5.f, 30.f };
	if (!World) return;
	if (PlinkoRow >= 8)
	{
		World->GetTimerManager().ClearTimer(PlinkoTimerHandle);
		float Mult = SlotPayouts[PlinkoBallSlot];
		const int32 PayoutGold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(PendingBetAmount) * Mult));
		AwardPayout(PendingBetAmount, Mult);
		if (UT66RunStateSubsystem* RunState = World->GetGameInstance()
			? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>()
			: nullptr)
		{
			RunState->RecordLuckQuantityRoll(FName(TEXT("GamblerPlinkoPayoutTier")), T66GetPlinkoPayoutTierFromSlot(PlinkoBallSlot), 0, 4);
			RunState->RecordAntiCheatGamblerRound(
				ET66AntiCheatGamblerGameType::Plinko,
				PendingBetAmount,
				PayoutGold,
				false,
				false,
				PayoutGold > PendingBetAmount,
				PayoutGold == PendingBetAmount && PendingBetAmount > 0,
				INDEX_NONE,
				INDEX_NONE,
				PlinkoBallSlot,
				T66GetPlinkoPayoutTierFromSlot(PlinkoBallSlot),
				0,
				0,
				PlinkoPathBits,
				0,
				INDEX_NONE,
				PlinkoPreDrawSeed,
				PlinkoStartDrawIndex,
				0.5f);
		}
		if (PlinkoResultText.IsValid())
			PlinkoResultText->SetText(FText::Format(
				NSLOCTEXT("T66.Gambler", "PlinkoWinFmt", "Landed in slot {0} — {1}x — You won {2}!"),
				FText::AsNumber(PlinkoBallSlot + 1), FText::AsNumber(Mult),
				FText::AsNumber(FMath::RoundToInt(static_cast<float>(PendingBetAmount) * Mult))));
		return;
	}
	UT66RngSubsystem* RngSub = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RngSubsystem>() : nullptr;
	bool bRight = RngSub ? (RngSub->GetRunFraction() >= 0.5f) : (FMath::FRand() >= 0.5f);
	if (bRight)
	{
		PlinkoPathBits |= (1 << FMath::Clamp(PlinkoRow, 0, 30));
	}
	if (bRight && PlinkoBallSlot < 8) PlinkoBallSlot++;
	else if (!bRight && PlinkoBallSlot > 0) PlinkoBallSlot--;
	PlinkoRow++;
	UpdatePlinkoBallVisual();
}

void UT66GamblerOverlayWidget::StartBoxOpeningSpin()
{
	BoxOpeningCurrentIndex = 0;
	BoxOpeningSpinSteps = 0;
	BoxOpeningStripOffset = 0;
	BoxOpeningPreDrawSeed = 0;
	BoxOpeningDrawIndex = INDEX_NONE;
	// Weighted result: Black (0) most likely, then Red (1), Yellow (2), White (3) rarest. 0.25x, 1x, 5x, 30x
	UT66RngSubsystem* RngSub = nullptr;
	UWorld* World = GetWorld();
	if (World && World->GetGameInstance())
		RngSub = World->GetGameInstance()->GetSubsystem<UT66RngSubsystem>();
	if (RngSub)
	{
		BoxOpeningPreDrawSeed = RngSub->GetRunStream().GetCurrentSeed();
		BoxOpeningDrawIndex = RngSub->GetRunDrawCount();
	}
	float R = RngSub ? RngSub->GetRunFraction() : FMath::FRand();
	if (R < 0.55f) BoxOpeningResultIndex = 0;
	else if (R < 0.85f) BoxOpeningResultIndex = 1;
	else if (R < 0.95f) BoxOpeningResultIndex = 2;
	else BoxOpeningResultIndex = 3;
	if (BoxOpeningResultText.IsValid()) BoxOpeningResultText->SetText(FText::GetEmpty());
	RefreshBoxOpeningStrip();
	if (World) World->GetTimerManager().SetTimer(BoxOpeningTimerHandle, this, &UT66GamblerOverlayWidget::TickBoxOpeningSpin, 0.08f, true, 0.05f);
}

void UT66GamblerOverlayWidget::TickBoxOpeningSpin()
{
	UWorld* World = GetWorld();
	static const TCHAR* ColorNames[] = { TEXT("Black"), TEXT("Red"), TEXT("Yellow"), TEXT("White") };
	static const float Payouts[] = { 0.25f, 1.f, 5.f, 30.f };
	if (!World) return;
	BoxOpeningSpinSteps++;
	// Fast spin then slow; middle of 5 boxes shows (StripOffset+2)%4. When done, set StripOffset so middle = result.
	const int32 FastSteps = 25;
	const int32 SlowSteps = 8;
	if (BoxOpeningSpinSteps <= FastSteps)
		BoxOpeningStripOffset++;
	else if (BoxOpeningSpinSteps <= FastSteps + SlowSteps)
		BoxOpeningStripOffset++;
	else
		BoxOpeningStripOffset = (BoxOpeningResultIndex + 2) % BoxColorCount;  // so (StripOffset+2)%4 == ResultIndex

	RefreshBoxOpeningStrip();

	if (BoxOpeningSpinSteps == FastSteps + SlowSteps + 1)
	{
		World->GetTimerManager().ClearTimer(BoxOpeningTimerHandle);
		float Mult = Payouts[BoxOpeningResultIndex];
		const int32 PayoutGold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(PendingBetAmount) * Mult));
		AwardPayout(PendingBetAmount, Mult);
		if (UT66RunStateSubsystem* RunState = World->GetGameInstance()
			? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>()
			: nullptr)
		{
			FT66RarityWeights ReplayWeights;
			ReplayWeights.Black = 55.f;
			ReplayWeights.Red = 30.f;
			ReplayWeights.Yellow = 10.f;
			ReplayWeights.White = 5.f;
			RunState->RecordLuckQualityRarity(
				FName(TEXT("GamblerBoxOpeningColor")),
				T66BoxOpeningIndexToRarity(BoxOpeningResultIndex),
				BoxOpeningDrawIndex,
				BoxOpeningPreDrawSeed,
				&ReplayWeights);
			RunState->RecordAntiCheatGamblerRound(
				ET66AntiCheatGamblerGameType::BoxOpening,
				PendingBetAmount,
				PayoutGold,
				false,
				false,
				PayoutGold > PendingBetAmount,
				PayoutGold == PendingBetAmount && PendingBetAmount > 0,
				INDEX_NONE,
				INDEX_NONE,
				BoxOpeningResultIndex,
				BoxOpeningResultIndex,
				0,
				0,
				0,
				0,
				INDEX_NONE,
				BoxOpeningPreDrawSeed,
				BoxOpeningDrawIndex);
		}
		if (BoxOpeningResultText.IsValid())
			BoxOpeningResultText->SetText(FText::Format(
				NSLOCTEXT("T66.Gambler", "BoxResultFmt", "{0} — {1}x — Won {2}!"),
				FText::FromString(ColorNames[BoxOpeningResultIndex]),
				FText::AsNumber(Mult),
				FText::AsNumber(FMath::RoundToInt(static_cast<float>(PendingBetAmount) * Mult))));
	}
}


void UT66GamblerOverlayWidget::UpdatePlinkoBallVisual()
{
	if (!PlinkoBallWidget.IsValid())
	{
		return;
	}

	static constexpr float PlinkoBoardW = 280.f;
	static constexpr float PlinkoRowH = 20.f;
	static constexpr float PlinkoBallSize = 14.f;

	const float ColW = PlinkoBoardW / static_cast<float>(PlinkoSlotCount);
	const FVector2D BallOffset(
		ColW * static_cast<float>(PlinkoBallSlot) + (ColW - PlinkoBallSize) * 0.5f,
		static_cast<float>(PlinkoRow) * PlinkoRowH + (PlinkoRowH - PlinkoBallSize) * 0.5f);

	PlinkoBallWidget->SetRenderTransform(FSlateRenderTransform(BallOffset));
}

