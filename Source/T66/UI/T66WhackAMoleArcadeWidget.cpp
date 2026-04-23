// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WhackAMoleArcadeWidget.h"

#include "Core/T66RunStateSubsystem.h"
#include "UI/Style/T66Style.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	constexpr float GWhackOverlayWidth = 980.f;
	constexpr float GWhackCellWidth = 150.f;
	constexpr float GWhackCellHeight = 108.f;
	constexpr float GWhackAutoCloseDelay = 1.1f;
}

TSharedRef<SWidget> UT66WhackAMoleArcadeWidget::RebuildWidget()
{
	const FText RunningSubtitle = NSLOCTEXT("T66.Arcade", "WhackAMoleSubtitle", "The run keeps moving while this cabinet is open.");
	const FText TimerLabel = NSLOCTEXT("T66.Arcade", "WhackAMoleTimerLabel", "TIME");
	const FText HitsLabel = NSLOCTEXT("T66.Arcade", "WhackAMoleHitsLabel", "HITS");
	const FText MissesLabel = NSLOCTEXT("T66.Arcade", "WhackAMoleMissesLabel", "MISSES");

	CellBorders.Empty(CellCount);
	CellBorders.SetNum(CellCount);
	CellTextBlocks.Empty(CellCount);
	CellTextBlocks.SetNum(CellCount);

	TSharedRef<SUniformGridPanel> BoardGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(10.f));

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			const int32 CellIndex = (Y * GridWidth) + X;
			TSharedPtr<SBorder> CellBorder;
			TSharedPtr<STextBlock> CellText;

			BoardGrid->AddSlot(X, Y)
			[
				SNew(SBox)
				.WidthOverride(GWhackCellWidth)
				.HeightOverride(GWhackCellHeight)
				[
					SNew(SButton)
					.ButtonColorAndOpacity(FLinearColor::Transparent)
					.ContentPadding(0.f)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66WhackAMoleArcadeWidget::HandleCellClicked, CellIndex))
					[
						SAssignNew(CellBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FT66Style::Tokens::Panel2)
						.Padding(FMargin(6.f))
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.03f, 0.05f, 0.05f, 1.f))
							.Padding(0.f)
							[
								SNew(SOverlay)
								+ SOverlay::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SAssignNew(CellText, STextBlock)
									.Text(FText::GetEmpty())
									.Font(FT66Style::Tokens::FontBold(22))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
								]
							]
						]
					]
				]
			];

			CellBorders[CellIndex] = CellBorder;
			CellTextBlocks[CellIndex] = CellText;
		}
	}

	const TSharedRef<SWidget> InfoPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(ArcadeData.DisplayName.IsEmpty()
					? NSLOCTEXT("T66.Arcade", "WhackAMoleFallbackTitle", "Whack-a-Mole")
					: ArcadeData.DisplayName)
				.Font(FT66Style::Tokens::FontBold(38))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(RunningSubtitle)
				.Font(FT66Style::Tokens::FontRegular(18))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(0.f, 0.f, 10.f, 0.f)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(TimerLabel)
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SAssignNew(TimerTextBlock, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(30))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(0.f, 0.f, 10.f, 0.f)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(HitsLabel)
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SAssignNew(HitsTextBlock, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(30))
							.ColorAndOpacity(FT66Style::Tokens::Success)
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					FT66Style::MakePanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(STextBlock)
							.Text(MissesLabel)
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SAssignNew(MissesTextBlock, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(30))
							.ColorAndOpacity(FT66Style::Tokens::Danger)
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
				FT66Style::MakePanel(
					BoardGrid,
					FT66PanelParams(ET66PanelType::Panel2)
						.SetPadding(FMargin(18.f))
						.SetColor(FLinearColor(0.05f, 0.06f, 0.07f, 1.f)))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
				SAssignNew(StatusTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 10.f, 0.f, 0.f)
			[
				SAssignNew(RewardTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontRegular(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Arcade", "WhackAMoleAbort", "Abort"),
						FOnClicked::CreateUObject(this, &UT66WhackAMoleArcadeWidget::HandlePrimaryActionClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(220.f)
					.SetHeight(54.f)
					.SetContent(
						SAssignNew(PrimaryActionTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)))
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetPadding(FMargin(24.f))
				.SetColor(FLinearColor(0.022f, 0.028f, 0.041f, 0.985f)));

	RefreshBoardVisuals();
	RefreshHud();

	const TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(28.f))
		[
			SNew(SBox)
			.WidthOverride(GWhackOverlayWidth)
			[
				InfoPanel
			]
		];

	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66WhackAMoleArcadeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StartRound();
}

void UT66WhackAMoleArcadeWidget::NativeDestruct()
{
	ClearActiveTimers();
	Super::NativeDestruct();
}

void UT66WhackAMoleArcadeWidget::StartRound()
{
	ClearActiveTimers();

	RoundDurationSeconds = ResolveRoundDurationSeconds();
	SpawnIntervalSeconds = ResolveSpawnIntervalSeconds();
	TargetHits = ResolveTargetHits();
	SuccessGold = ResolveSuccessGold();
	FailureGoldPenalty = ResolveFailureGoldPenalty();
	RemainingSeconds = RoundDurationSeconds;
	RoundEndTimeSeconds = GetWorld() ? (GetWorld()->GetTimeSeconds() + RoundDurationSeconds) : RoundDurationSeconds;
	Hits = 0;
	Misses = 0;
	ActiveCellIndex = INDEX_NONE;
	bRoundEnded = false;
	bRoundSucceeded = false;

	SpawnNextMole(false);
	RestartSpawnTimer();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RoundTickHandle, this, &UT66WhackAMoleArcadeWidget::HandleRoundTick, 0.05f, true);
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::ClearActiveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundTickHandle);
		World->GetTimerManager().ClearTimer(SpawnTickHandle);
		World->GetTimerManager().ClearTimer(AutoCloseHandle);
	}
}

void UT66WhackAMoleArcadeWidget::RestartSpawnTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTickHandle);
		World->GetTimerManager().SetTimer(SpawnTickHandle, this, &UT66WhackAMoleArcadeWidget::HandleSpawnTick, SpawnIntervalSeconds, true);
	}
}

void UT66WhackAMoleArcadeWidget::HandleRoundTick()
{
	if (bRoundEnded)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		RemainingSeconds = FMath::Max(0.f, static_cast<float>(RoundEndTimeSeconds - World->GetTimeSeconds()));
	}

	if (RemainingSeconds <= KINDA_SMALL_NUMBER)
	{
		CompleteRound(Hits >= TargetHits);
		return;
	}

	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::HandleSpawnTick()
{
	if (bRoundEnded)
	{
		return;
	}

	SpawnNextMole(true);
	RefreshBoardVisuals();
	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::HandleAutoClose()
{
	StartCloseSequence(bRoundSucceeded);
}

void UT66WhackAMoleArcadeWidget::SpawnNextMole(const bool bCountExpiredAsMiss)
{
	if (bRoundEnded)
	{
		return;
	}

	if (bCountExpiredAsMiss && ActiveCellIndex != INDEX_NONE)
	{
		++Misses;
	}

	const int32 PreviousCellIndex = ActiveCellIndex;
	ActiveCellIndex = FMath::RandHelper(CellCount);
	if (CellCount > 1)
	{
		for (int32 Attempt = 0; Attempt < 4 && ActiveCellIndex == PreviousCellIndex; ++Attempt)
		{
			ActiveCellIndex = FMath::RandHelper(CellCount);
		}
	}
}

void UT66WhackAMoleArcadeWidget::CompleteRound(const bool bSucceeded)
{
	if (bRoundEnded)
	{
		return;
	}

	bRoundEnded = true;
	bRoundSucceeded = bSucceeded;
	ActiveCellIndex = INDEX_NONE;
	ClearActiveTimers();

	if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		if (bRoundSucceeded)
		{
			if (SuccessGold > 0)
			{
				RunState->AddGold(SuccessGold);
			}
		}
		else if (FailureGoldPenalty > 0)
		{
			RunState->TrySpendGold(FailureGoldPenalty);
		}
	}

	RefreshBoardVisuals();
	RefreshHud();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoCloseHandle, this, &UT66WhackAMoleArcadeWidget::HandleAutoClose, GWhackAutoCloseDelay, false);
	}
}

void UT66WhackAMoleArcadeWidget::RefreshBoardVisuals()
{
	for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
	{
		if (CellBorders.IsValidIndex(CellIndex) && CellBorders[CellIndex].IsValid())
		{
			CellBorders[CellIndex]->SetBorderBackgroundColor(GetCellColor(CellIndex));
		}

		if (CellTextBlocks.IsValidIndex(CellIndex) && CellTextBlocks[CellIndex].IsValid())
		{
			CellTextBlocks[CellIndex]->SetText(GetCellText(CellIndex));
		}
	}
}

void UT66WhackAMoleArcadeWidget::RefreshHud()
{
	if (TimerTextBlock.IsValid())
	{
		TimerTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds))));
	}

	if (HitsTextBlock.IsValid())
	{
		HitsTextBlock->SetText(FText::Format(
			NSLOCTEXT("T66.Arcade", "WhackAMoleHitsValue", "{0}/{1}"),
			FText::AsNumber(Hits),
			FText::AsNumber(TargetHits)));
	}

	if (MissesTextBlock.IsValid())
	{
		MissesTextBlock->SetText(FText::AsNumber(Misses));
	}

	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(BuildStatusText());
		StatusTextBlock->SetColorAndOpacity(
			bRoundEnded
				? (bRoundSucceeded ? FT66Style::Tokens::Success : FT66Style::Tokens::Danger)
				: FT66Style::Tokens::Text);
	}

	if (RewardTextBlock.IsValid())
	{
		RewardTextBlock->SetText(BuildRewardSummaryText());
	}

	if (PrimaryActionTextBlock.IsValid())
	{
		PrimaryActionTextBlock->SetText(BuildPrimaryActionText());
	}
}

float UT66WhackAMoleArcadeWidget::ResolveRoundDurationSeconds() const
{
	return FMath::Max(4.f, ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::PopupRoundSeconds, 12.f));
}

float UT66WhackAMoleArcadeWidget::ResolveSpawnIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::PopupSpawnInterval, 0.65f), 0.2f, 3.f);
}

int32 UT66WhackAMoleArcadeWidget::ResolveTargetHits() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::PopupTargetHits, 10));
}

int32 UT66WhackAMoleArcadeWidget::ResolveSuccessGold() const
{
	return FMath::Max(0, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::PopupSuccessGold, 150));
}

int32 UT66WhackAMoleArcadeWidget::ResolveFailureGoldPenalty() const
{
	return FMath::Max(0, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::PopupFailureGoldPenalty, 0));
}

FText UT66WhackAMoleArcadeWidget::BuildRewardSummaryText() const
{
	if (SuccessGold > 0 && FailureGoldPenalty > 0)
	{
		return FText::Format(
			NSLOCTEXT("T66.Arcade", "WhackAMoleRewardAndPenalty", "Win: +{0} gold. Fail: -{1} gold."),
			FText::AsNumber(SuccessGold),
			FText::AsNumber(FailureGoldPenalty));
	}

	if (SuccessGold > 0)
	{
		return FText::Format(
			NSLOCTEXT("T66.Arcade", "WhackAMoleRewardOnly", "Win: +{0} gold."),
			FText::AsNumber(SuccessGold));
	}

	if (FailureGoldPenalty > 0)
	{
		return FText::Format(
			NSLOCTEXT("T66.Arcade", "WhackAMolePenaltyOnly", "Fail: -{0} gold."),
			FText::AsNumber(FailureGoldPenalty));
	}

	return NSLOCTEXT("T66.Arcade", "WhackAMoleNoEconomy", "No payout modifier configured.");
}

FText UT66WhackAMoleArcadeWidget::BuildStatusText() const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "WhackAMoleStatusWon", "Round clear. Cashing out...")
			: NSLOCTEXT("T66.Arcade", "WhackAMoleStatusLost", "Time up. Cabinet shutting down.");
	}

	if (ActiveCellIndex != INDEX_NONE)
	{
		return NSLOCTEXT("T66.Arcade", "WhackAMoleStatusActive", "Bonk the lit slot before it hops.");
	}

	return NSLOCTEXT("T66.Arcade", "WhackAMoleStatusWarmup", "Machine booting...");
}

FText UT66WhackAMoleArcadeWidget::BuildPrimaryActionText() const
{
	return bRoundEnded
		? NSLOCTEXT("T66.Arcade", "WhackAMoleClose", "Continue")
		: NSLOCTEXT("T66.Arcade", "WhackAMoleAbortButton", "Abort");
}

FLinearColor UT66WhackAMoleArcadeWidget::GetCellColor(const int32 CellIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? FLinearColor(0.16f, 0.42f, 0.21f, 1.f)
			: FLinearColor(0.30f, 0.10f, 0.10f, 1.f);
	}

	return CellIndex == ActiveCellIndex
		? FLinearColor(0.93f, 0.63f, 0.14f, 1.f)
		: FT66Style::Tokens::Panel2;
}

FText UT66WhackAMoleArcadeWidget::GetCellText(const int32 CellIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "WhackAMoleDoneCell", "CLEAR")
			: NSLOCTEXT("T66.Arcade", "WhackAMoleDoneCellFail", "MISS");
	}

	return CellIndex == ActiveCellIndex
		? NSLOCTEXT("T66.Arcade", "WhackAMoleActiveCell", "BONK")
		: FText::GetEmpty();
}

FReply UT66WhackAMoleArcadeWidget::HandleCellClicked(const int32 CellIndex)
{
	if (bRoundEnded)
	{
		return FReply::Handled();
	}

	if (CellIndex != ActiveCellIndex)
	{
		return FReply::Handled();
	}

	++Hits;
	if (Hits >= TargetHits)
	{
		CompleteRound(true);
		return FReply::Handled();
	}

	SpawnNextMole(false);
	RestartSpawnTimer();
	RefreshBoardVisuals();
	RefreshHud();
	return FReply::Handled();
}

FReply UT66WhackAMoleArcadeWidget::HandlePrimaryActionClicked()
{
	StartCloseSequence(bRoundEnded ? bRoundSucceeded : false);
	return FReply::Handled();
}
