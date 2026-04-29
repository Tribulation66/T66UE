// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GoldMinerArcadeWidget.h"

#include "Core/T66AudioSubsystem.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SCanvas.h"
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
	constexpr float GGoldMinerOverlayWidth = 980.f;
	constexpr float GGoldMinerBoardWidth = 908.f;
	constexpr float GGoldMinerBoardHeight = 342.f;
	constexpr float GGoldMinerAutoCloseDelay = 1.1f;
}

TSharedRef<SWidget> UT66GoldMinerArcadeWidget::RebuildWidget()
{
	LoadArtworkBrush();

	Lanes.SetNum(LaneCount);
	LaneBorders.Empty(LaneCount);
	LaneBorders.SetNum(LaneCount);
	TargetBorders.Empty(LaneCount);
	TargetBorders.SetNum(LaneCount);
	TargetTextBlocks.Empty(LaneCount);
	TargetTextBlocks.SetNum(LaneCount);
	HookStepBorders.Empty(LaneCount * HookStepCount);
	HookStepBorders.SetNum(LaneCount * HookStepCount);

	const FText TimerLabel = NSLOCTEXT("T66.Arcade", "GoldMinerTimerLabel", "TIME");
	const FText ScoreLabel = NSLOCTEXT("T66.Arcade", "GoldMinerScoreLabel", "SCORE");
	const FText CargoLabel = NSLOCTEXT("T66.Arcade", "GoldMinerCargoLabel", "CARGO");

	auto BuildStatPanel = [](const FText& Label, const TSharedRef<SWidget>& ValueWidget) -> TSharedRef<SWidget>
	{
		return FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				ValueWidget
			],
			FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(14.f)));
	};

	TSharedRef<SHorizontalBox> LaneRow = SNew(SHorizontalBox);
	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		TSharedPtr<SBorder> LaneBorder;
		TSharedPtr<SBorder> TargetBorder;
		TSharedPtr<STextBlock> TargetText;
		TSharedRef<SVerticalBox> LaneStack = SNew(SVerticalBox);

		LaneStack->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBox)
				.HeightOverride(22.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.10f, 0.08f, 0.05f, 0.92f))
				]
			];

		for (int32 StepIndex = 0; StepIndex < HookStepCount; ++StepIndex)
		{
			TSharedPtr<SBorder> HookStepBorder;
			LaneStack->AddSlot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 5.f)
				[
					SNew(SBox)
					.HeightOverride(28.f)
					[
						SAssignNew(HookStepBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor::Transparent)
					]
				];

			HookStepBorders[(LaneIndex * HookStepCount) + StepIndex] = HookStepBorder;
		}

		LaneStack->AddSlot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SBox)
				.HeightOverride(70.f)
				[
					SAssignNew(TargetBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.08f, 0.06f, 0.04f, 1.f))
					.Padding(FMargin(5.f))
					[
						SAssignNew(TargetText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(15))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
					]
				]
			];

		LaneRow->AddSlot()
			.FillWidth(1.f)
			.Padding(4.f, 0.f)
			[
				SAssignNew(LaneBorder, SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.025f, 0.030f, 0.040f, 0.80f))
				.Padding(FMargin(6.f))
				[
					LaneStack
				]
			];

		LaneBorders[LaneIndex] = LaneBorder;
		TargetBorders[LaneIndex] = TargetBorder;
		TargetTextBlocks[LaneIndex] = TargetText;
	}

	const TSharedRef<SWidget> BoardButton =
		FT66Style::MakeBareButton(
			FT66BareButtonParams(
				FOnClicked::CreateUObject(this, &UT66GoldMinerArcadeWidget::HandleDropClicked),
				SNew(SBox)
				.WidthOverride(GGoldMinerBoardWidth)
				.HeightOverride(GGoldMinerBoardHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						BuildArtworkLayer(0.42f)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Bottom)
					.Padding(18.f, 26.f, 18.f, 18.f)
					[
						LaneRow
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(SpriteCanvas, SCanvas)
					]
				])
			.SetColor(FLinearColor::Transparent)
			.SetPadding(FMargin(0.f))
			.SetToolTipText(NSLOCTEXT("T66.Arcade", "GoldMinerDropToolTip", "Drop claw")));

	const TSharedRef<SWidget> InfoPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(ArcadeData.DisplayName.IsEmpty()
					? NSLOCTEXT("T66.Arcade", "GoldMinerFallbackTitle", "Gold Miner")
					: ArcadeData.DisplayName)
				.Font(FT66Style::Tokens::FontBold(38))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
					BuildStatPanel(
						TimerLabel,
						SAssignNew(TimerTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(30))
						.ColorAndOpacity(FT66Style::Tokens::Text))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
					BuildStatPanel(
						ScoreLabel,
						SAssignNew(ScoreTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(30))
						.ColorAndOpacity(FT66Style::Tokens::Accent))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					BuildStatPanel(
						CargoLabel,
						SAssignNew(CargoTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(30))
						.ColorAndOpacity(FT66Style::Tokens::Success))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66Style::MakePanel(
					BoardButton,
					FT66PanelParams(ET66PanelType::Panel2)
						.SetPadding(FMargin(0.f))
						.SetColor(FLinearColor(0.04f, 0.035f, 0.030f, 1.f)))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0.f, 16.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Arcade", "GoldMinerDropButton", "Drop"),
						FOnClicked::CreateUObject(this, &UT66GoldMinerArcadeWidget::HandleDropClicked),
						ET66ButtonType::Primary)
					.SetMinWidth(220.f)
					.SetHeight(54.f)
					.SetContent(
						SAssignNew(DropActionTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				SAssignNew(StatusTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
			[
				SAssignNew(RewardTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontRegular(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Arcade", "GoldMinerAbort", "Abort"),
						FOnClicked::CreateUObject(this, &UT66GoldMinerArcadeWidget::HandlePrimaryActionClicked),
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
			.WidthOverride(GGoldMinerOverlayWidth)
			[
				InfoPanel
			]
		];

	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66GoldMinerArcadeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StartRound();
}

void UT66GoldMinerArcadeWidget::NativeDestruct()
{
	ClearActiveTimers();
	Super::NativeDestruct();
}

void UT66GoldMinerArcadeWidget::StartRound()
{
	ClearActiveTimers();

	RoundDurationSeconds = ResolveRoundDurationSeconds();
	SwingSpeed = ResolveSwingSpeed();
	ReelBaseSeconds = ResolveReelBaseSeconds();
	TargetScore = ResolveTargetScore();
	SmallGoldScore = ResolveSmallGoldScore();
	LargeGoldScore = ResolveLargeGoldScore();
	GemScore = ResolveGemScore();
	RockScore = ResolveRockScore();
	MissPenalty = ResolveMissPenalty();
	Score = 0;
	CargoCount = 0;
	ReelLaneIndex = INDEX_NONE;
	bReeling = false;
	bRoundEnded = false;
	bRoundSucceeded = false;

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	RoundStartTimeSeconds = NowSeconds;
	RoundEndTimeSeconds = NowSeconds + RoundDurationSeconds;
	LastDropTimeSeconds = NowSeconds;
	ReelEndTimeSeconds = NowSeconds;
	RemainingSeconds = RoundDurationSeconds;
	GenerateTargets();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RoundTickHandle, this, &UT66GoldMinerArcadeWidget::HandleRoundTick, 0.04f, true);
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66GoldMinerArcadeWidget::ClearActiveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundTickHandle);
		World->GetTimerManager().ClearTimer(AutoCloseHandle);
	}
}

void UT66GoldMinerArcadeWidget::GenerateTargets()
{
	Lanes.SetNum(LaneCount);
	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		const int32 Roll = FMath::RandRange(0, 99);
		FGoldMinerLane& Lane = Lanes[LaneIndex];
		Lane.Depth = FMath::RandRange(1, HookStepCount);
		if (Roll < 12)
		{
			Lane.TargetType = EGoldMinerTargetType::Empty;
			Lane.ScoreValue = 0;
		}
		else if (Roll < 27)
		{
			Lane.TargetType = EGoldMinerTargetType::Rock;
			Lane.ScoreValue = RockScore;
		}
		else if (Roll < 62)
		{
			Lane.TargetType = EGoldMinerTargetType::SmallGold;
			Lane.ScoreValue = SmallGoldScore;
		}
		else if (Roll < 88)
		{
			Lane.TargetType = EGoldMinerTargetType::LargeGold;
			Lane.ScoreValue = LargeGoldScore;
		}
		else
		{
			Lane.TargetType = EGoldMinerTargetType::Gem;
			Lane.ScoreValue = GemScore;
			Lane.Depth = HookStepCount;
		}
	}

	EnsureAtLeastOneValuableTarget();
}

void UT66GoldMinerArcadeWidget::EnsureAtLeastOneValuableTarget()
{
	if (CountRemainingValuableTargets() > 0 || !Lanes.IsValidIndex(LaneCount / 2))
	{
		return;
	}

	FGoldMinerLane& Lane = Lanes[LaneCount / 2];
	Lane.TargetType = EGoldMinerTargetType::SmallGold;
	Lane.ScoreValue = SmallGoldScore;
	Lane.Depth = FMath::Max(1, HookStepCount / 2);
}

void UT66GoldMinerArcadeWidget::HandleRoundTick()
{
	if (bRoundEnded)
	{
		return;
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	RemainingSeconds = FMath::Max(0.f, static_cast<float>(RoundEndTimeSeconds - NowSeconds));

	if (bReeling && NowSeconds >= ReelEndTimeSeconds)
	{
		CompleteReel();
	}

	if (RemainingSeconds <= KINDA_SMALL_NUMBER && !bReeling)
	{
		CompleteRound();
		return;
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66GoldMinerArcadeWidget::HandleAutoClose()
{
	StartCloseSequence(bRoundSucceeded, Score);
}

void UT66GoldMinerArcadeWidget::CompleteReel()
{
	if (!bReeling)
	{
		return;
	}

	const int32 CompletedLaneIndex = ReelLaneIndex;
	bReeling = false;
	ReelLaneIndex = INDEX_NONE;

	if (Lanes.IsValidIndex(CompletedLaneIndex))
	{
		FGoldMinerLane& Lane = Lanes[CompletedLaneIndex];
		if (Lane.TargetType == EGoldMinerTargetType::Empty)
		{
			Score = FMath::Max(0, Score - MissPenalty);
			UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Miss")));
		}
		else
		{
			Score += FMath::Max(0, Lane.ScoreValue);
			++CargoCount;
			Lane.TargetType = EGoldMinerTargetType::Empty;
			Lane.ScoreValue = 0;
			UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Hit")));
		}
	}

	if (CountRemainingValuableTargets() <= 0)
	{
		GenerateTargets();
	}

	RefreshBoardVisuals();
	RefreshHud();

	if (RemainingSeconds <= KINDA_SMALL_NUMBER)
	{
		CompleteRound();
	}
}

void UT66GoldMinerArcadeWidget::CompleteRound()
{
	if (bRoundEnded)
	{
		return;
	}

	bRoundEnded = true;
	bRoundSucceeded = Score > 0;
	bReeling = false;
	ReelLaneIndex = INDEX_NONE;
	ClearActiveTimers();
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Round.End")));

	RefreshBoardVisuals();
	RefreshHud();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoCloseHandle, this, &UT66GoldMinerArcadeWidget::HandleAutoClose, GGoldMinerAutoCloseDelay, false);
	}
}

void UT66GoldMinerArcadeWidget::RefreshBoardVisuals()
{
	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		if (LaneBorders.IsValidIndex(LaneIndex) && LaneBorders[LaneIndex].IsValid())
		{
			LaneBorders[LaneIndex]->SetBorderBackgroundColor(GetLaneColor(LaneIndex));
		}

		if (TargetBorders.IsValidIndex(LaneIndex) && TargetBorders[LaneIndex].IsValid())
		{
			TargetBorders[LaneIndex]->SetBorderBackgroundColor(GetTargetColor(LaneIndex));
		}

		if (TargetTextBlocks.IsValidIndex(LaneIndex) && TargetTextBlocks[LaneIndex].IsValid())
		{
			TargetTextBlocks[LaneIndex]->SetText(GetTargetText(LaneIndex));
		}

		for (int32 StepIndex = 0; StepIndex < HookStepCount; ++StepIndex)
		{
			const int32 HookIndex = (LaneIndex * HookStepCount) + StepIndex;
			if (HookStepBorders.IsValidIndex(HookIndex) && HookStepBorders[HookIndex].IsValid())
			{
				HookStepBorders[HookIndex]->SetBorderBackgroundColor(GetHookStepColor(LaneIndex, StepIndex));
			}
		}
	}

	RefreshSpriteLayer();
}

void UT66GoldMinerArcadeWidget::RefreshHud()
{
	if (TimerTextBlock.IsValid())
	{
		TimerTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds))));
	}

	if (ScoreTextBlock.IsValid())
	{
		ScoreTextBlock->SetText(FText::AsNumber(Score));
	}

	if (CargoTextBlock.IsValid())
	{
		CargoTextBlock->SetText(FText::Format(
			NSLOCTEXT("T66.Arcade", "GoldMinerCargoValue", "{0}/{1}"),
			FText::AsNumber(CargoCount),
			FText::AsNumber(FMath::Max(1, TargetScore / FMath::Max(1, SmallGoldScore)))));
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

	if (DropActionTextBlock.IsValid())
	{
		DropActionTextBlock->SetText(BuildDropActionText());
	}

	if (PrimaryActionTextBlock.IsValid())
	{
		PrimaryActionTextBlock->SetText(BuildPrimaryActionText());
	}
}

float UT66GoldMinerArcadeWidget::ResolveRoundDurationSeconds() const
{
	return FMath::Max(4.f, ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeRoundSeconds, 12.f));
}

float UT66GoldMinerArcadeWidget::ResolveSwingSpeed() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::GoldMinerSwingSpeed, 3.2f), 0.8f, 9.f);
}

float UT66GoldMinerArcadeWidget::ResolveReelBaseSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::GoldMinerReelBaseSeconds, 0.24f), 0.08f, 1.0f);
}

int32 UT66GoldMinerArcadeWidget::ResolveTargetScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeTargetScore, 140));
}

int32 UT66GoldMinerArcadeWidget::ResolveSmallGoldScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeScorePerHit, 12));
}

int32 UT66GoldMinerArcadeWidget::ResolveLargeGoldScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::GoldMinerLargeGoldScore, 28));
}

int32 UT66GoldMinerArcadeWidget::ResolveGemScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::GoldMinerGemScore, 45));
}

int32 UT66GoldMinerArcadeWidget::ResolveRockScore() const
{
	return FMath::Max(0, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::GoldMinerRockScore, 3));
}

int32 UT66GoldMinerArcadeWidget::ResolveMissPenalty() const
{
	return FMath::Max(0, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeMissPenalty, 5));
}

int32 UT66GoldMinerArcadeWidget::ResolveActiveLaneIndex() const
{
	if (bReeling)
	{
		return ReelLaneIndex;
	}

	if (bRoundEnded)
	{
		return INDEX_NONE;
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float SwingAlpha = 0.5f + 0.5f * FMath::Sin(static_cast<float>((NowSeconds - RoundStartTimeSeconds) * SwingSpeed));
	return FMath::Clamp(FMath::RoundToInt(SwingAlpha * static_cast<float>(LaneCount - 1)), 0, LaneCount - 1);
}

float UT66GoldMinerArcadeWidget::GetReelProgress01() const
{
	if (!bReeling)
	{
		return 0.f;
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const double DurationSeconds = FMath::Max(0.01, ReelEndTimeSeconds - LastDropTimeSeconds);
	return FMath::Clamp(static_cast<float>((NowSeconds - LastDropTimeSeconds) / DurationSeconds), 0.f, 1.f);
}

int32 UT66GoldMinerArcadeWidget::CountRemainingValuableTargets() const
{
	int32 Count = 0;
	for (const FGoldMinerLane& Lane : Lanes)
	{
		if (IsValuableTarget(Lane.TargetType))
		{
			++Count;
		}
	}
	return Count;
}

bool UT66GoldMinerArcadeWidget::IsValuableTarget(const EGoldMinerTargetType TargetType) const
{
	return TargetType == EGoldMinerTargetType::SmallGold
		|| TargetType == EGoldMinerTargetType::LargeGold
		|| TargetType == EGoldMinerTargetType::Gem;
}

FText UT66GoldMinerArcadeWidget::BuildStatusText() const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "GoldMinerStatusWon", "Claim locked. Cabinet paying out...")
			: NSLOCTEXT("T66.Arcade", "GoldMinerStatusLost", "No haul. Cabinet shutting down.");
	}

	if (bReeling)
	{
		return NSLOCTEXT("T66.Arcade", "GoldMinerStatusReeling", "Claw is reeling...");
	}

	return NSLOCTEXT("T66.Arcade", "GoldMinerStatusActive", "Drop on gold before the timer empties.");
}

FText UT66GoldMinerArcadeWidget::BuildRewardSummaryText() const
{
	return NSLOCTEXT("T66.Arcade", "GoldMinerRewardSummary", "Bigger hauls improve the cabinet payout when the timer hits zero.");
}

FText UT66GoldMinerArcadeWidget::BuildPrimaryActionText() const
{
	return bRoundEnded
		? NSLOCTEXT("T66.Arcade", "GoldMinerClose", "Continue")
		: NSLOCTEXT("T66.Arcade", "GoldMinerAbortButton", "Abort");
}

FText UT66GoldMinerArcadeWidget::BuildDropActionText() const
{
	if (bRoundEnded)
	{
		return NSLOCTEXT("T66.Arcade", "GoldMinerDropDone", "Done");
	}

	return bReeling
		? NSLOCTEXT("T66.Arcade", "GoldMinerDropReeling", "Reeling")
		: NSLOCTEXT("T66.Arcade", "GoldMinerDropReady", "Drop");
}

FText UT66GoldMinerArcadeWidget::GetTargetText(const int32 LaneIndex) const
{
	if (!Lanes.IsValidIndex(LaneIndex))
	{
		return FText::GetEmpty();
	}

	switch (Lanes[LaneIndex].TargetType)
	{
	case EGoldMinerTargetType::Rock:
		return NSLOCTEXT("T66.Arcade", "GoldMinerTargetRock", "ROCK");
	case EGoldMinerTargetType::SmallGold:
		return NSLOCTEXT("T66.Arcade", "GoldMinerTargetGold", "GOLD");
	case EGoldMinerTargetType::LargeGold:
		return NSLOCTEXT("T66.Arcade", "GoldMinerTargetBig", "BIG");
	case EGoldMinerTargetType::Gem:
		return NSLOCTEXT("T66.Arcade", "GoldMinerTargetGem", "GEM");
	case EGoldMinerTargetType::Empty:
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UT66GoldMinerArcadeWidget::GetLaneColor(const int32 LaneIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? FLinearColor(0.11f, 0.32f, 0.17f, 0.92f)
			: FLinearColor(0.30f, 0.10f, 0.09f, 0.92f);
	}

	if (bReeling && LaneIndex == ReelLaneIndex)
	{
		return FLinearColor(0.20f, 0.34f, 0.46f, 0.96f);
	}

	return LaneIndex == ResolveActiveLaneIndex()
		? FLinearColor(0.42f, 0.24f, 0.05f, 0.96f)
		: FLinearColor(0.025f, 0.030f, 0.040f, 0.80f);
}

FLinearColor UT66GoldMinerArcadeWidget::GetTargetColor(const int32 LaneIndex) const
{
	if (!Lanes.IsValidIndex(LaneIndex))
	{
		return FLinearColor(0.06f, 0.05f, 0.04f, 1.f);
	}

	switch (Lanes[LaneIndex].TargetType)
	{
	case EGoldMinerTargetType::Rock:
		return FLinearColor(0.30f, 0.30f, 0.31f, 1.f);
	case EGoldMinerTargetType::SmallGold:
		return FLinearColor(0.92f, 0.62f, 0.12f, 1.f);
	case EGoldMinerTargetType::LargeGold:
		return FLinearColor(1.00f, 0.78f, 0.16f, 1.f);
	case EGoldMinerTargetType::Gem:
		return FLinearColor(0.10f, 0.78f, 0.84f, 1.f);
	case EGoldMinerTargetType::Empty:
	default:
		return FLinearColor(0.07f, 0.055f, 0.040f, 1.f);
	}
}

FLinearColor UT66GoldMinerArcadeWidget::GetHookStepColor(const int32 LaneIndex, const int32 StepIndex) const
{
	if (bRoundEnded)
	{
		return FLinearColor::Transparent;
	}

	if (bReeling && LaneIndex == ReelLaneIndex)
	{
		const int32 LitSteps = FMath::Clamp(FMath::CeilToInt(GetReelProgress01() * HookStepCount), 1, HookStepCount);
		if (StepIndex < LitSteps)
		{
			return FLinearColor(0.82f, 0.72f, 0.42f, 0.96f);
		}
		return FLinearColor(0.06f, 0.08f, 0.09f, 0.45f);
	}

	if (LaneIndex == ResolveActiveLaneIndex() && StepIndex == 0)
	{
		const float Pulse = 0.5f + 0.5f * FMath::Sin(static_cast<float>(FPlatformTime::Seconds() * 15.0));
		return FLinearColor(
			FMath::Lerp(0.70f, 1.00f, Pulse),
			FMath::Lerp(0.38f, 0.74f, Pulse),
			0.08f,
			0.95f);
	}

	return FLinearColor::Transparent;
}

FReply UT66GoldMinerArcadeWidget::HandleDropClicked()
{
	if (bRoundEnded || bReeling)
	{
		return FReply::Handled();
	}

	const int32 LaneIndex = ResolveActiveLaneIndex();
	if (!Lanes.IsValidIndex(LaneIndex))
	{
		return FReply::Handled();
	}

	const FGoldMinerLane& Lane = Lanes[LaneIndex];
	const int32 ReelDepth = Lane.TargetType == EGoldMinerTargetType::Empty
		? HookStepCount
		: FMath::Clamp(Lane.Depth, 1, HookStepCount);
	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	ReelLaneIndex = LaneIndex;
	bReeling = true;
	LastDropTimeSeconds = NowSeconds;
	ReelEndTimeSeconds = NowSeconds + FMath::Max(0.10f, ReelBaseSeconds * static_cast<float>(ReelDepth + 1));

	RefreshBoardVisuals();
	RefreshHud();
	return FReply::Handled();
}

FReply UT66GoldMinerArcadeWidget::HandlePrimaryActionClicked()
{
	StartCloseSequence(bRoundEnded ? bRoundSucceeded : false, bRoundEnded ? Score : 0);
	return FReply::Handled();
}

void UT66GoldMinerArcadeWidget::LoadArtworkBrush()
{
	if (ArtworkTexture.IsValid())
	{
		return;
	}

	const TArray<FString> CandidatePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(TEXT("SourceAssets/Arcade/GoldMiner/gold_miner_panel.png"));
	for (const FString& CandidatePath : CandidatePaths)
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("GoldMinerArcadePanel")))
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

const FSlateBrush* UT66GoldMinerArcadeWidget::FindOrLoadSpriteBrush(const FString& RelativePath)
{
	if (RelativePath.IsEmpty())
	{
		return nullptr;
	}

	if (FGoldMinerSpriteBrush* Existing = SpriteBrushes.Find(RelativePath))
	{
		return Existing->Texture.IsValid() && Existing->Brush.IsValid() ? Existing->Brush.Get() : nullptr;
	}

	FGoldMinerSpriteBrush Entry;
	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Nearest, TEXT("GoldMinerSprite")))
		{
			Entry.Texture.Reset(Texture);
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->SetResourceObject(Texture);
			Entry.Brush->DrawAs = ESlateBrushDrawType::Image;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());

			FGoldMinerSpriteBrush& Stored = SpriteBrushes.Add(RelativePath, MoveTemp(Entry));
			return Stored.Brush.Get();
		}
	}

	SpriteBrushes.Add(RelativePath, MoveTemp(Entry));
	return nullptr;
}

const TCHAR* UT66GoldMinerArcadeWidget::GetTargetSpritePath(const EGoldMinerTargetType TargetType) const
{
	switch (TargetType)
	{
	case EGoldMinerTargetType::Rock:
		return TEXT("SourceAssets/Arcade/GoldMiner/rock_sprite.png");
	case EGoldMinerTargetType::SmallGold:
		return TEXT("SourceAssets/Arcade/GoldMiner/small_gold_sprite.png");
	case EGoldMinerTargetType::LargeGold:
		return TEXT("SourceAssets/Arcade/GoldMiner/large_gold_sprite.png");
	case EGoldMinerTargetType::Gem:
		return TEXT("SourceAssets/Arcade/GoldMiner/gem_sprite.png");
	case EGoldMinerTargetType::Empty:
	default:
		return TEXT("");
	}
}

float UT66GoldMinerArcadeWidget::ResolveLaneCenterX(const int32 LaneIndex) const
{
	const float LeftPadding = 42.f;
	const float RightPadding = 42.f;
	const float UsableWidth = GGoldMinerBoardWidth - LeftPadding - RightPadding;
	const float Step = UsableWidth / static_cast<float>(FMath::Max(1, LaneCount - 1));
	return LeftPadding + (Step * static_cast<float>(FMath::Clamp(LaneIndex, 0, LaneCount - 1)));
}

float UT66GoldMinerArcadeWidget::ResolveTargetY(const FGoldMinerLane& Lane) const
{
	const float DepthAlpha = static_cast<float>(FMath::Clamp(Lane.Depth, 1, HookStepCount) - 1)
		/ static_cast<float>(FMath::Max(1, HookStepCount - 1));
	return FMath::Lerp(194.f, 286.f, DepthAlpha);
}

FVector2D UT66GoldMinerArcadeWidget::ResolveVisualClawCenter() const
{
	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	float X = GGoldMinerBoardWidth * 0.5f;
	float Y = 58.f + (FMath::Sin(static_cast<float>(NowSeconds * 6.0)) * 3.f);

	if (bReeling && Lanes.IsValidIndex(ReelLaneIndex))
	{
		const float ReelProgress = GetReelProgress01();
		const float DropAlpha = ReelProgress < 0.58f
			? FMath::Clamp(ReelProgress / 0.58f, 0.f, 1.f)
			: FMath::Clamp(1.f - ((ReelProgress - 0.58f) / 0.42f), 0.f, 1.f);
		X = ResolveLaneCenterX(ReelLaneIndex);
		Y = FMath::Lerp(58.f, ResolveTargetY(Lanes[ReelLaneIndex]) - 30.f, DropAlpha);
	}
	else if (!bRoundEnded)
	{
		const float SwingAlpha = 0.5f + (0.5f * FMath::Sin(static_cast<float>((NowSeconds - RoundStartTimeSeconds) * SwingSpeed)));
		X = FMath::Lerp(ResolveLaneCenterX(0), ResolveLaneCenterX(LaneCount - 1), SwingAlpha);
	}

	return FVector2D(X, Y);
}

void UT66GoldMinerArcadeWidget::AddSpriteToCanvas(
	const FString& RelativePath,
	const FVector2D& Center,
	const float Size,
	const FLinearColor& Tint)
{
	if (!SpriteCanvas.IsValid())
	{
		return;
	}

	const FSlateBrush* SpriteBrush = FindOrLoadSpriteBrush(RelativePath);
	if (!SpriteBrush)
	{
		return;
	}

	const FVector2D SpriteSize(Size, Size);
	SpriteCanvas->AddSlot()
		.Position(Center - (SpriteSize * 0.5f))
		.Size(SpriteSize)
		[
			SNew(SImage)
			.Image(SpriteBrush)
			.ColorAndOpacity(Tint)
		];
}

void UT66GoldMinerArcadeWidget::AddRopeToCanvas(const FVector2D& ClawCenter) const
{
	if (!SpriteCanvas.IsValid())
	{
		return;
	}

	const float RopeTop = 20.f;
	const float RopeHeight = FMath::Max(12.f, ClawCenter.Y - RopeTop);
	SpriteCanvas->AddSlot()
		.Position(FVector2D(ClawCenter.X - 3.f, RopeTop))
		.Size(FVector2D(6.f, RopeHeight))
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.74f, 0.61f, 0.34f, 0.92f))
		];
}

void UT66GoldMinerArcadeWidget::RefreshSpriteLayer()
{
	if (!SpriteCanvas.IsValid())
	{
		return;
	}

	SpriteCanvas->ClearChildren();
	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();

	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		if (!Lanes.IsValidIndex(LaneIndex))
		{
			continue;
		}

		const FGoldMinerLane& Lane = Lanes[LaneIndex];
		const TCHAR* SpritePath = GetTargetSpritePath(Lane.TargetType);
		if (!SpritePath || !*SpritePath)
		{
			continue;
		}

		const float Bob = FMath::Sin(static_cast<float>((NowSeconds * 3.0) + LaneIndex)) * 4.f;
		const float Size = Lane.TargetType == EGoldMinerTargetType::LargeGold || Lane.TargetType == EGoldMinerTargetType::Gem ? 72.f : 58.f;
		AddSpriteToCanvas(
			SpritePath,
			FVector2D(ResolveLaneCenterX(LaneIndex), ResolveTargetY(Lane) + Bob),
			Size,
			FLinearColor(1.f, 1.f, 1.f, bRoundEnded ? 0.60f : 1.f));
	}

	const FVector2D ClawCenter = ResolveVisualClawCenter();
	AddRopeToCanvas(ClawCenter);

	if (bReeling && Lanes.IsValidIndex(ReelLaneIndex))
	{
		const float ReelProgress = GetReelProgress01();
		if (ReelProgress > 0.58f && Lanes[ReelLaneIndex].TargetType != EGoldMinerTargetType::Empty)
		{
			AddSpriteToCanvas(
				GetTargetSpritePath(Lanes[ReelLaneIndex].TargetType),
				ClawCenter + FVector2D(0.f, 38.f),
				52.f,
				FLinearColor(1.f, 1.f, 1.f, 0.94f));
		}
	}

	AddSpriteToCanvas(
		TEXT("SourceAssets/Arcade/GoldMiner/claw_sprite.png"),
		ClawCenter,
		92.f,
		FLinearColor(1.f, 1.f, 1.f, bRoundEnded ? 0.58f : 1.f));
}

TSharedRef<SWidget> UT66GoldMinerArcadeWidget::BuildArtworkLayer(const float Opacity) const
{
	if (!ArtworkTexture.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SImage)
		.Image(&ArtworkBrush)
		.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(Opacity, 0.f, 1.f)));
}
