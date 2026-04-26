// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WhackAMoleArcadeWidget.h"

#include "Core/T66AudioSubsystem.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Texture2D.h"
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
	LoadArtworkBrush();

	const FText RunningSubtitle = NSLOCTEXT("T66.Arcade", "WhackAMoleSubtitle", "The run keeps moving while this cabinet is open.");
	const FText TimerLabel = NSLOCTEXT("T66.Arcade", "WhackAMoleTimerLabel", "TIME");
	const FText HitsLabel = NSLOCTEXT("T66.Arcade", "WhackAMoleHitsLabel", "HITS");
	const FText ScoreLabel = NSLOCTEXT("T66.Arcade", "WhackAMoleScoreLabel", "SCORE");

	CellBorders.Empty(CellCount);
	CellBorders.SetNum(CellCount);
	TargetBodies.Empty(CellCount);
	TargetBodies.SetNum(CellCount);
	TargetRings.Empty(CellCount);
	TargetRings.SetNum(CellCount);
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
			TSharedPtr<SBorder> TargetBody;
			TSharedPtr<SBorder> TargetRing;
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
									SNew(SBox)
									.WidthOverride(104.f)
									.HeightOverride(76.f)
									[
										SAssignNew(TargetRing, SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor::Transparent)
										.Padding(FMargin(7.f))
										[
											SAssignNew(TargetBody, SBorder)
											.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
											.BorderBackgroundColor(FLinearColor::Transparent)
											.Padding(FMargin(4.f))
											[
												SNew(SBorder)
												.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
												.BorderBackgroundColor(FLinearColor(0.12f, 0.07f, 0.03f, 1.f))
												.Padding(FMargin(6.f))
												[
													SAssignNew(CellText, STextBlock)
													.Text(FText::GetEmpty())
													.Font(FT66Style::Tokens::FontBold(20))
													.ColorAndOpacity(FT66Style::Tokens::Text)
													.Justification(ETextJustify::Center)
												]
											]
										]
									]
								]
							]
						]
					]
				]
			];

			CellBorders[CellIndex] = CellBorder;
			TargetBodies[CellIndex] = TargetBody;
			TargetRings[CellIndex] = TargetRing;
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
							.Text(ScoreLabel)
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
						[
							SAssignNew(ScoreTextBlock, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(30))
							.ColorAndOpacity(FT66Style::Tokens::Accent)
						],
						FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
				FT66Style::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						BuildArtworkLayer(0.22f)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						BoardGrid
					],
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
	StartSpawnIntervalSeconds = ResolveStartSpawnIntervalSeconds();
	EndSpawnIntervalSeconds = ResolveEndSpawnIntervalSeconds();
	TargetScore = ResolveTargetScore();
	ScorePerHit = ResolveScorePerHit();
	MissPenalty = ResolveMissPenalty();
	RemainingSeconds = RoundDurationSeconds;
	RoundEndTimeSeconds = GetWorld() ? (GetWorld()->GetTimeSeconds() + RoundDurationSeconds) : RoundDurationSeconds;
	Score = 0;
	Hits = 0;
	Misses = 0;
	ActiveCellIndex = INDEX_NONE;
	bRoundEnded = false;
	bRoundSucceeded = false;

	SpawnNextMole(false);
	ScheduleNextSpawn();

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

void UT66WhackAMoleArcadeWidget::ScheduleNextSpawn()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTickHandle);
		World->GetTimerManager().SetTimer(SpawnTickHandle, this, &UT66WhackAMoleArcadeWidget::HandleSpawnTick, ResolveCurrentSpawnIntervalSeconds(), false);
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
		CompleteRound();
		return;
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::HandleSpawnTick()
{
	if (bRoundEnded)
	{
		return;
	}

	SpawnNextMole(true);
	ScheduleNextSpawn();
	RefreshBoardVisuals();
	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::HandleAutoClose()
{
	StartCloseSequence(bRoundSucceeded, Score);
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
		Score = FMath::Max(0, Score - MissPenalty);
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Miss")));
	}

	const int32 PreviousCellIndex = ActiveCellIndex;
	ActiveCellIndex = FMath::RandHelper(CellCount);
	ActiveCellSpawnTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	if (CellCount > 1)
	{
		for (int32 Attempt = 0; Attempt < 4 && ActiveCellIndex == PreviousCellIndex; ++Attempt)
		{
			ActiveCellIndex = FMath::RandHelper(CellCount);
		}
	}
	ActiveCellSpawnTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
}

void UT66WhackAMoleArcadeWidget::CompleteRound()
{
	if (bRoundEnded)
	{
		return;
	}

	bRoundEnded = true;
	bRoundSucceeded = Score > 0;
	ActiveCellIndex = INDEX_NONE;
	ClearActiveTimers();
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Round.End")));

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

		const bool bTargetVisible = (CellIndex == ActiveCellIndex && !bRoundEnded)
			|| (bRoundEnded && bRoundSucceeded);
		if (TargetBodies.IsValidIndex(CellIndex) && TargetBodies[CellIndex].IsValid())
		{
			TargetBodies[CellIndex]->SetVisibility(bTargetVisible ? EVisibility::Visible : EVisibility::Hidden);
			TargetBodies[CellIndex]->SetBorderBackgroundColor(GetTargetBodyColor(CellIndex));
		}

		if (TargetRings.IsValidIndex(CellIndex) && TargetRings[CellIndex].IsValid())
		{
			TargetRings[CellIndex]->SetVisibility(bTargetVisible ? EVisibility::Visible : EVisibility::Hidden);
			TargetRings[CellIndex]->SetBorderBackgroundColor(GetTargetRingColor(CellIndex));
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
		const int32 TargetHitCount = FMath::Max(1, FMath::CeilToInt(static_cast<float>(TargetScore) / static_cast<float>(FMath::Max(1, ScorePerHit))));
		HitsTextBlock->SetText(FText::Format(
			NSLOCTEXT("T66.Arcade", "WhackAMoleHitsValue", "{0}/{1}"),
			FText::AsNumber(Hits),
			FText::AsNumber(TargetHitCount)));
	}

	if (ScoreTextBlock.IsValid())
	{
		ScoreTextBlock->SetText(FText::AsNumber(Score));
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
	return FMath::Max(4.f, ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeRoundSeconds, 10.f));
}

float UT66WhackAMoleArcadeWidget::ResolveStartSpawnIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeStartInterval, 0.70f), 0.2f, 3.f);
}

float UT66WhackAMoleArcadeWidget::ResolveEndSpawnIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeEndInterval, 0.24f), 0.12f, 3.f);
}

float UT66WhackAMoleArcadeWidget::ResolveCurrentSpawnIntervalSeconds() const
{
	const float Progress01 = RoundDurationSeconds > KINDA_SMALL_NUMBER
		? FMath::Clamp(1.f - (RemainingSeconds / RoundDurationSeconds), 0.f, 1.f)
		: 1.f;
	return FMath::Lerp(StartSpawnIntervalSeconds, EndSpawnIntervalSeconds, Progress01);
}

int32 UT66WhackAMoleArcadeWidget::ResolveTargetScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeTargetScore, 100));
}

int32 UT66WhackAMoleArcadeWidget::ResolveScorePerHit() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeScorePerHit, 10));
}

int32 UT66WhackAMoleArcadeWidget::ResolveMissPenalty() const
{
	return FMath::Max(0, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeMissPenalty, 2));
}

FText UT66WhackAMoleArcadeWidget::BuildRewardSummaryText() const
{
	return NSLOCTEXT("T66.Arcade", "WhackAMoleRewardSummary", "Score powers the cabinet payout when the timer hits zero.");
}

FText UT66WhackAMoleArcadeWidget::BuildStatusText() const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "WhackAMoleStatusWon", "Score locked. Cabinet paying out...")
			: NSLOCTEXT("T66.Arcade", "WhackAMoleStatusLost", "No score. Cabinet shutting down.");
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
		? FLinearColor(0.22f, 0.14f, 0.04f, 1.f)
		: FLinearColor(0.025f, 0.032f, 0.040f, 1.f);
}

FLinearColor UT66WhackAMoleArcadeWidget::GetTargetBodyColor(const int32 CellIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? FLinearColor(0.20f, 0.74f, 0.34f, 1.f)
			: FLinearColor(0.34f, 0.10f, 0.10f, 1.f);
	}

	if (CellIndex != ActiveCellIndex)
	{
		return FLinearColor::Transparent;
	}

	const float AgeAlpha = GetActiveCellAgeAlpha();
	return FMath::Lerp(
		FLinearColor(0.24f, 0.78f, 0.42f, 1.f),
		FLinearColor(0.98f, 0.38f, 0.13f, 1.f),
		FMath::Clamp(AgeAlpha, 0.f, 1.f));
}

FLinearColor UT66WhackAMoleArcadeWidget::GetTargetRingColor(const int32 CellIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? FLinearColor(0.88f, 0.82f, 0.20f, 1.f)
			: FLinearColor(0.55f, 0.12f, 0.10f, 1.f);
	}

	if (CellIndex != ActiveCellIndex)
	{
		return FLinearColor::Transparent;
	}

	const float Pulse = 0.5f + 0.5f * FMath::Sin(static_cast<float>(FPlatformTime::Seconds() * 18.0));
	const float AgeAlpha = GetActiveCellAgeAlpha();
	return FLinearColor(
		FMath::Lerp(0.78f, 1.f, Pulse),
		FMath::Lerp(0.50f, 0.92f, 1.f - AgeAlpha),
		FMath::Lerp(0.10f, 0.04f, AgeAlpha),
		1.f);
}

FText UT66WhackAMoleArcadeWidget::GetCellText(const int32 CellIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "WhackAMoleDoneCell", "SCORE")
			: NSLOCTEXT("T66.Arcade", "WhackAMoleDoneCellFail", "MISS");
	}

	return CellIndex == ActiveCellIndex
		? NSLOCTEXT("T66.Arcade", "WhackAMoleActiveCell", "HIT")
		: FText::GetEmpty();
}

float UT66WhackAMoleArcadeWidget::GetActiveCellAgeAlpha() const
{
	if (ActiveCellIndex == INDEX_NONE || !GetWorld())
	{
		return 0.f;
	}

	const float AgeSeconds = FMath::Max(0.f, static_cast<float>(GetWorld()->GetTimeSeconds() - ActiveCellSpawnTimeSeconds));
	const float Interval = FMath::Max(0.1f, ResolveCurrentSpawnIntervalSeconds());
	return FMath::Clamp(AgeSeconds / Interval, 0.f, 1.f);
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
	Score += ScorePerHit;
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Hit")));

	SpawnNextMole(false);
	ScheduleNextSpawn();
	RefreshBoardVisuals();
	RefreshHud();
	return FReply::Handled();
}

FReply UT66WhackAMoleArcadeWidget::HandlePrimaryActionClicked()
{
	StartCloseSequence(bRoundEnded ? bRoundSucceeded : false, bRoundEnded ? Score : 0);
	return FReply::Handled();
}

void UT66WhackAMoleArcadeWidget::LoadArtworkBrush()
{
	if (ArtworkTexture.IsValid())
	{
		return;
	}

	const TArray<FString> CandidatePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(TEXT("SourceAssets/Arcade/WhackAMole/whack_a_mole_panel.png"));
	for (const FString& CandidatePath : CandidatePaths)
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("WhackAMoleArcadePanel")))
		{
			ArtworkTexture.Reset(Texture);
			ArtworkBrush = FSlateBrush();
			ArtworkBrush.SetResourceObject(Texture);
			ArtworkBrush.DrawAs = ESlateBrushDrawType::Image;
			ArtworkBrush.Tiling = ESlateBrushTileType::NoTile;
			ArtworkBrush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
			return;
		}
	}
}

TSharedRef<SWidget> UT66WhackAMoleArcadeWidget::BuildArtworkLayer(const float Opacity) const
{
	if (!ArtworkTexture.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SImage)
		.Image(&ArtworkBrush)
		.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(Opacity, 0.f, 1.f)));
}
