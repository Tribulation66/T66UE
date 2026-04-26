// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66TopwarArcadeWidget.h"

#include "Core/T66AudioSubsystem.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
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
	constexpr float GTopwarOverlayWidth = 980.f;
	constexpr float GTopwarAutoCloseDelay = 1.1f;
	constexpr int32 GTopwarMarkerCount = 12;
	constexpr int32 GTopwarEnemyMarkerCount = 10;

	static const TArray<ET66HeroStatType>& T66GetTopwarStatPool()
	{
		static const TArray<ET66HeroStatType> StatPool = {
			ET66HeroStatType::Damage,
			ET66HeroStatType::AttackSpeed,
			ET66HeroStatType::AttackScale,
			ET66HeroStatType::Armor,
			ET66HeroStatType::Evasion,
			ET66HeroStatType::Luck,
			ET66HeroStatType::Speed,
			ET66HeroStatType::Accuracy,
		};
		return StatPool;
	}
}

TSharedRef<SWidget> UT66TopwarArcadeWidget::RebuildWidget()
{
	LoadArtworkBrush();
	SquadMarkers.Reset();
	EnemyMarkers.Reset();

	const FText TimerLabel = NSLOCTEXT("T66.Arcade", "TopwarTimerLabel", "TIME");
	const FText ScoreLabel = NSLOCTEXT("T66.Arcade", "TopwarScoreLabel", "SCORE");
	const FText SquadLabel = NSLOCTEXT("T66.Arcade", "TopwarSquadLabel", "SQUAD");

	TSharedRef<SHorizontalBox> SquadRow = SNew(SHorizontalBox);
	for (int32 Index = 0; Index < GTopwarMarkerCount; ++Index)
	{
		TSharedPtr<SBorder> Marker;
		SquadRow->AddSlot()
			.AutoWidth()
			.Padding(3.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(22.f)
				.HeightOverride(28.f)
				[
					SAssignNew(Marker, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.05f, 0.12f, 0.20f, 1.f))
				]
			];
		SquadMarkers.Add(Marker);
	}

	TSharedRef<SHorizontalBox> EnemyRow = SNew(SHorizontalBox);
	for (int32 Index = 0; Index < GTopwarEnemyMarkerCount; ++Index)
	{
		TSharedPtr<SBorder> Marker;
		EnemyRow->AddSlot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(26.f)
				.HeightOverride(32.f)
				[
					SAssignNew(Marker, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.16f, 0.04f, 0.04f, 1.f))
				]
			];
		EnemyMarkers.Add(Marker);
	}

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

	const TSharedRef<SWidget> LeftGateButton =
		SNew(SButton)
		.ButtonColorAndOpacity(FLinearColor::Transparent)
		.ContentPadding(0.f)
		.OnClicked(FOnClicked::CreateUObject(this, &UT66TopwarArcadeWidget::HandleGateClicked, true))
		[
			SAssignNew(LeftGateBorder, SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.10f, 0.28f, 0.82f, 1.f))
			.Padding(FMargin(20.f))
			[
				SAssignNew(LeftGateTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(30))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
		];

	const TSharedRef<SWidget> RightGateButton =
		SNew(SButton)
		.ButtonColorAndOpacity(FLinearColor::Transparent)
		.ContentPadding(0.f)
		.OnClicked(FOnClicked::CreateUObject(this, &UT66TopwarArcadeWidget::HandleGateClicked, false))
		[
			SAssignNew(RightGateBorder, SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.78f, 0.18f, 0.12f, 1.f))
			.Padding(FMargin(20.f))
			[
				SAssignNew(RightGateTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(30))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
		];

	const TSharedRef<SWidget> InfoPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(ArcadeData.DisplayName.IsEmpty()
					? NSLOCTEXT("T66.Arcade", "TopwarFallbackTitle", "Topwar")
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
						SquadLabel,
						SAssignNew(SquadTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(30))
						.ColorAndOpacity(FT66Style::Tokens::Success))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
					[
						SNew(SBox)
						.HeightOverride(168.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								BuildArtworkLayer(0.35f)
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Top)
							.Padding(0.f, 14.f, 0.f, 0.f)
							[
								EnemyRow
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.05f, 0.08f, 0.12f, 0.80f))
								.Padding(FMargin(18.f, 8.f))
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.Arcade", "TopwarRunwayLabel", "CHOOSE A GATE"))
									.Font(FT66Style::Tokens::FontBold(18))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
								]
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Bottom)
							.Padding(0.f, 0.f, 0.f, 14.f)
							[
								SquadRow
							]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.40f, 0.08f, 0.08f, 1.f))
						.Padding(FMargin(14.f))
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Arcade", "TopwarEnemyLine", "ENEMY LINE"))
							.Font(FT66Style::Tokens::FontBold(20))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						]
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
						[
							SNew(SBox).HeightOverride(140.f)[LeftGateButton]
						]
						+ SHorizontalBox::Slot().FillWidth(1.f).Padding(10.f, 0.f, 0.f, 0.f)
						[
							SNew(SBox).HeightOverride(140.f)[RightGateButton]
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.08f, 0.10f, 0.14f, 1.f))
						.Padding(FMargin(0.f))
						[
							SNew(SBox)
							.HeightOverride(8.f)
						]
					],
					FT66PanelParams(ET66PanelType::Panel2)
						.SetPadding(FMargin(18.f))
						.SetColor(FLinearColor(0.04f, 0.045f, 0.055f, 1.f)))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				SAssignNew(StatusTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.Arcade", "TopwarAbort", "Abort"),
						FOnClicked::CreateUObject(this, &UT66TopwarArcadeWidget::HandlePrimaryActionClicked),
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

	RefreshGateVisuals();
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
			.WidthOverride(GTopwarOverlayWidth)
			[
				InfoPanel
			]
		];

	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66TopwarArcadeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StartRound();
}

void UT66TopwarArcadeWidget::NativeDestruct()
{
	ClearActiveTimers();
	Super::NativeDestruct();
}

void UT66TopwarArcadeWidget::StartRound()
{
	ClearActiveTimers();

	RoundDurationSeconds = ResolveRoundDurationSeconds();
	StartGateIntervalSeconds = ResolveStartGateIntervalSeconds();
	EndGateIntervalSeconds = ResolveEndGateIntervalSeconds();
	SquadPower = ResolveStartingSquad();
	ChoiceScore = ResolveChoiceScore();
	SquadGain = ResolveSquadGain();
	Score = 0;
	RemainingSeconds = RoundDurationSeconds;
	RoundEndTimeSeconds = GetWorld() ? (GetWorld()->GetTimeSeconds() + RoundDurationSeconds) : RoundDurationSeconds;
	bRoundEnded = false;
	bRoundSucceeded = false;

	RollGateChoices();
	ScheduleNextGate();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RoundTickHandle, this, &UT66TopwarArcadeWidget::HandleRoundTick, 0.05f, true);
	}

	RefreshGateVisuals();
	RefreshHud();
}

void UT66TopwarArcadeWidget::ClearActiveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundTickHandle);
		World->GetTimerManager().ClearTimer(GateTickHandle);
		World->GetTimerManager().ClearTimer(AutoCloseHandle);
	}
}

void UT66TopwarArcadeWidget::ScheduleNextGate()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GateTickHandle);
		World->GetTimerManager().SetTimer(GateTickHandle, this, &UT66TopwarArcadeWidget::HandleGateTick, ResolveCurrentGateIntervalSeconds(), false);
	}
}

void UT66TopwarArcadeWidget::HandleRoundTick()
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

	RefreshGateVisuals();
	RefreshHud();
}

void UT66TopwarArcadeWidget::HandleGateTick()
{
	if (bRoundEnded)
	{
		return;
	}

	RollGateChoices();
	ScheduleNextGate();
	RefreshGateVisuals();
	RefreshHud();
}

void UT66TopwarArcadeWidget::HandleAutoClose()
{
	StartCloseSequence(bRoundSucceeded, Score);
}

void UT66TopwarArcadeWidget::CompleteRound()
{
	if (bRoundEnded)
	{
		return;
	}

	bRoundEnded = true;
	bRoundSucceeded = Score > 0;
	ClearActiveTimers();
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Round.End")));
	RefreshGateVisuals();
	RefreshHud();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoCloseHandle, this, &UT66TopwarArcadeWidget::HandleAutoClose, GTopwarAutoCloseDelay, false);
	}
}

void UT66TopwarArcadeWidget::RollGateChoices()
{
	const TArray<ET66HeroStatType>& StatPool = T66GetTopwarStatPool();
	if (StatPool.Num() <= 0)
	{
		LeftGateStat = ET66HeroStatType::Damage;
		RightGateStat = ET66HeroStatType::Speed;
		return;
	}

	LeftGateStat = StatPool[FMath::RandHelper(StatPool.Num())];
	RightGateStat = StatPool[FMath::RandHelper(StatPool.Num())];
	for (int32 Attempt = 0; Attempt < 6 && RightGateStat == LeftGateStat; ++Attempt)
	{
		RightGateStat = StatPool[FMath::RandHelper(StatPool.Num())];
	}
	LastGateRollTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
}

void UT66TopwarArcadeWidget::RefreshHud()
{
	if (TimerTextBlock.IsValid())
	{
		TimerTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds))));
	}

	if (ScoreTextBlock.IsValid())
	{
		ScoreTextBlock->SetText(FText::AsNumber(Score));
	}

	if (SquadTextBlock.IsValid())
	{
		SquadTextBlock->SetText(FText::AsNumber(SquadPower));
	}

	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(bRoundEnded
			? (bRoundSucceeded
				? NSLOCTEXT("T66.Arcade", "TopwarStatusDone", "Squad score locked. Cabinet paying out...")
				: NSLOCTEXT("T66.Arcade", "TopwarStatusEmpty", "No upgrades chosen. Cabinet shutting down."))
			: NSLOCTEXT("T66.Arcade", "TopwarStatusActive", "Choose either gate before the route changes."));
		StatusTextBlock->SetColorAndOpacity(
			bRoundEnded
				? (bRoundSucceeded ? FT66Style::Tokens::Success : FT66Style::Tokens::Danger)
				: FT66Style::Tokens::Text);
	}

	if (PrimaryActionTextBlock.IsValid())
	{
		PrimaryActionTextBlock->SetText(bRoundEnded
			? NSLOCTEXT("T66.Arcade", "TopwarClose", "Continue")
			: NSLOCTEXT("T66.Arcade", "TopwarAbortButton", "Abort"));
	}
}

void UT66TopwarArcadeWidget::RefreshGateVisuals()
{
	if (LeftGateBorder.IsValid())
	{
		const float FeedbackAlpha = GetChoiceFeedbackAlpha(true);
		const float UrgencyAlpha = GetGateUrgencyAlpha();
		LeftGateBorder->SetBorderBackgroundColor(bRoundEnded
			? FT66Style::Tokens::Panel2
			: FMath::Lerp(GetStatColor(LeftGateStat), FLinearColor(0.94f, 0.88f, 0.28f, 1.f), FMath::Max(FeedbackAlpha, UrgencyAlpha * 0.18f)));
	}
	if (RightGateBorder.IsValid())
	{
		const float FeedbackAlpha = GetChoiceFeedbackAlpha(false);
		const float UrgencyAlpha = GetGateUrgencyAlpha();
		RightGateBorder->SetBorderBackgroundColor(bRoundEnded
			? FT66Style::Tokens::Panel2
			: FMath::Lerp(GetStatColor(RightGateStat), FLinearColor(0.94f, 0.88f, 0.28f, 1.f), FMath::Max(FeedbackAlpha, UrgencyAlpha * 0.18f)));
	}
	if (LeftGateTextBlock.IsValid())
	{
		LeftGateTextBlock->SetText(bRoundEnded ? FText::AsNumber(Score) : GetStatLabel(LeftGateStat));
	}
	if (RightGateTextBlock.IsValid())
	{
		RightGateTextBlock->SetText(bRoundEnded ? FText::AsNumber(SquadPower) : GetStatLabel(RightGateStat));
	}

	const int32 LitMarkers = FMath::Clamp(SquadPower, 0, SquadMarkers.Num());
	for (int32 Index = 0; Index < SquadMarkers.Num(); ++Index)
	{
		if (SquadMarkers[Index].IsValid())
		{
			SquadMarkers[Index]->SetVisibility(Index < LitMarkers ? EVisibility::Visible : EVisibility::Hidden);
			SquadMarkers[Index]->SetBorderBackgroundColor(GetSquadMarkerColor(Index));
		}
	}

	for (int32 Index = 0; Index < EnemyMarkers.Num(); ++Index)
	{
		if (EnemyMarkers[Index].IsValid())
		{
			EnemyMarkers[Index]->SetBorderBackgroundColor(GetEnemyMarkerColor(Index));
		}
	}
}

float UT66TopwarArcadeWidget::ResolveRoundDurationSeconds() const
{
	return FMath::Max(4.f, ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeRoundSeconds, 10.f));
}

float UT66TopwarArcadeWidget::ResolveStartGateIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeStartInterval, 0.90f), 0.2f, 3.f);
}

float UT66TopwarArcadeWidget::ResolveEndGateIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeEndInterval, 0.28f), 0.12f, 3.f);
}

float UT66TopwarArcadeWidget::ResolveCurrentGateIntervalSeconds() const
{
	const float Progress01 = RoundDurationSeconds > KINDA_SMALL_NUMBER
		? FMath::Clamp(1.f - (RemainingSeconds / RoundDurationSeconds), 0.f, 1.f)
		: 1.f;
	return FMath::Lerp(StartGateIntervalSeconds, EndGateIntervalSeconds, Progress01);
}

int32 UT66TopwarArcadeWidget::ResolveStartingSquad() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::TopwarStartingSquad, 4));
}

int32 UT66TopwarArcadeWidget::ResolveChoiceScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::TopwarChoiceScore, 9));
}

int32 UT66TopwarArcadeWidget::ResolveSquadGain() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::TopwarSquadGain, 2));
}

FReply UT66TopwarArcadeWidget::HandleGateClicked(const bool bLeftGate)
{
	if (bRoundEnded)
	{
		return FReply::Handled();
	}

	const ET66HeroStatType PickedStat = bLeftGate ? LeftGateStat : RightGateStat;
	Score += ChoiceScore + SquadPower;
	SquadPower += SquadGain;
	LastChoiceTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	bLastChoiceWasLeftGate = bLeftGate;
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Topwar.Choice")));

	RollGateChoices();
	ScheduleNextGate();
	RefreshGateVisuals();
	RefreshHud();
	static_cast<void>(PickedStat);
	return FReply::Handled();
}

FReply UT66TopwarArcadeWidget::HandlePrimaryActionClicked()
{
	StartCloseSequence(bRoundEnded ? bRoundSucceeded : false, bRoundEnded ? Score : 0);
	return FReply::Handled();
}

FText UT66TopwarArcadeWidget::GetStatLabel(const ET66HeroStatType StatType) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:
		return NSLOCTEXT("T66.Arcade", "TopwarStatDamage", "+DAMAGE");
	case ET66HeroStatType::AttackSpeed:
		return NSLOCTEXT("T66.Arcade", "TopwarStatAttackSpeed", "+FIRE RATE");
	case ET66HeroStatType::AttackScale:
		return NSLOCTEXT("T66.Arcade", "TopwarStatAttackScale", "+SCALE");
	case ET66HeroStatType::Armor:
		return NSLOCTEXT("T66.Arcade", "TopwarStatArmor", "+ARMOR");
	case ET66HeroStatType::Evasion:
		return NSLOCTEXT("T66.Arcade", "TopwarStatEvasion", "+DODGE");
	case ET66HeroStatType::Luck:
		return NSLOCTEXT("T66.Arcade", "TopwarStatLuck", "+LUCK");
	case ET66HeroStatType::Speed:
		return NSLOCTEXT("T66.Arcade", "TopwarStatSpeed", "+SPEED");
	case ET66HeroStatType::Accuracy:
		return NSLOCTEXT("T66.Arcade", "TopwarStatAccuracy", "+ACCURACY");
	default:
		return NSLOCTEXT("T66.Arcade", "TopwarStatFallback", "+POWER");
	}
}

FLinearColor UT66TopwarArcadeWidget::GetStatColor(const ET66HeroStatType StatType) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:
		return FLinearColor(0.70f, 0.12f, 0.10f, 1.f);
	case ET66HeroStatType::AttackSpeed:
		return FLinearColor(0.82f, 0.44f, 0.08f, 1.f);
	case ET66HeroStatType::AttackScale:
		return FLinearColor(0.52f, 0.20f, 0.82f, 1.f);
	case ET66HeroStatType::Armor:
		return FLinearColor(0.10f, 0.36f, 0.78f, 1.f);
	case ET66HeroStatType::Evasion:
		return FLinearColor(0.10f, 0.62f, 0.48f, 1.f);
	case ET66HeroStatType::Luck:
		return FLinearColor(0.16f, 0.58f, 0.20f, 1.f);
	case ET66HeroStatType::Speed:
		return FLinearColor(0.10f, 0.62f, 0.84f, 1.f);
	case ET66HeroStatType::Accuracy:
		return FLinearColor(0.72f, 0.70f, 0.12f, 1.f);
	default:
		return FLinearColor(0.18f, 0.22f, 0.30f, 1.f);
	}
}

FLinearColor UT66TopwarArcadeWidget::GetSquadMarkerColor(const int32 MarkerIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? FLinearColor(0.26f, 0.80f, 0.34f, 1.f)
			: FLinearColor(0.14f, 0.16f, 0.20f, 1.f);
	}

	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float March = 0.5f + 0.5f * FMath::Sin((TimeSeconds * 14.f) + static_cast<float>(MarkerIndex) * 0.85f);
	const float ChoicePulse = FMath::Max(GetChoiceFeedbackAlpha(true), GetChoiceFeedbackAlpha(false));
	return FLinearColor(
		FMath::Lerp(0.12f, 0.42f, ChoicePulse),
		FMath::Lerp(0.36f, 0.72f, March),
		FMath::Lerp(0.82f, 1.0f, March),
		1.f);
}

FLinearColor UT66TopwarArcadeWidget::GetEnemyMarkerColor(const int32 MarkerIndex) const
{
	const float Progress01 = RoundDurationSeconds > KINDA_SMALL_NUMBER
		? FMath::Clamp(1.f - (RemainingSeconds / RoundDurationSeconds), 0.f, 1.f)
		: 1.f;
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float Flicker = 0.5f + 0.5f * FMath::Sin((TimeSeconds * 10.f) + static_cast<float>(MarkerIndex) * 0.55f);
	return FLinearColor(
		FMath::Lerp(0.34f, 0.94f, FMath::Max(Progress01, Flicker * 0.35f)),
		FMath::Lerp(0.05f, 0.18f, Flicker),
		FMath::Lerp(0.04f, 0.08f, Flicker),
		1.f);
}

float UT66TopwarArcadeWidget::GetGateUrgencyAlpha() const
{
	if (bRoundEnded)
	{
		return 0.f;
	}

	const float Interval = FMath::Max(0.1f, ResolveCurrentGateIntervalSeconds());
	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float AgeSeconds = FMath::Max(0.f, static_cast<float>(TimeSeconds - LastGateRollTimeSeconds));
	return FMath::Clamp(AgeSeconds / Interval, 0.f, 1.f);
}

float UT66TopwarArcadeWidget::GetChoiceFeedbackAlpha(const bool bLeftGate) const
{
	if (bLastChoiceWasLeftGate != bLeftGate)
	{
		return 0.f;
	}

	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float AgeSeconds = FMath::Max(0.f, static_cast<float>(TimeSeconds - LastChoiceTimeSeconds));
	return FMath::Clamp(1.f - (AgeSeconds / 0.22f), 0.f, 1.f);
}

void UT66TopwarArcadeWidget::LoadArtworkBrush()
{
	if (ArtworkTexture.IsValid())
	{
		return;
	}

	const TArray<FString> CandidatePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(TEXT("SourceAssets/Arcade/Topwar/topwar_panel.png"));
	for (const FString& CandidatePath : CandidatePaths)
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("TopwarArcadePanel")))
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

TSharedRef<SWidget> UT66TopwarArcadeWidget::BuildArtworkLayer(const float Opacity) const
{
	if (!ArtworkTexture.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SImage)
		.Image(&ArtworkBrush)
		.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(Opacity, 0.f, 1.f)));
}
