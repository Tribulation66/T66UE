// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66QuickArcadeWidget.h"

#include "Core/T66AudioSubsystem.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
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
	constexpr float GQuickArcadeOverlayWidth = 940.f;
	constexpr float GQuickArcadeLargeCellSize = 96.f;
	constexpr float GQuickArcadeCompactCellSize = 64.f;
	constexpr float GQuickArcadeCellPadding = 7.f;
	constexpr float GQuickArcadeAutoCloseDelay = 1.1f;
}

TSharedRef<SWidget> UT66QuickArcadeWidget::RebuildWidget()
{
	Spec = ResolveSpec();
	LoadArtworkBrush();

	const int32 CellCount = FMath::Max(1, Spec.CellCount);
	const int32 Columns = FMath::Max(1, Spec.Columns);
	const float CellSize = ResolveCellSize();
	const FVector2D BoardSize = ResolveBoardSize();
	CellBorders.Empty(CellCount);
	CellBorders.SetNum(CellCount);
	InnerCellBorders.Empty(CellCount);
	InnerCellBorders.SetNum(CellCount);
	CellTextBlocks.Empty(CellCount);
	CellTextBlocks.SetNum(CellCount);

	TSharedRef<SUniformGridPanel> BoardGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(GQuickArcadeCellPadding));

	for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
	{
		TSharedPtr<SBorder> CellBorder;
		TSharedPtr<SBorder> InnerCellBorder;
		TSharedPtr<STextBlock> CellText;
		const int32 X = CellIndex % Columns;
		const int32 Y = CellIndex / Columns;

		BoardGrid->AddSlot(X, Y)
		[
			SNew(SBox)
			.WidthOverride(CellSize)
			.HeightOverride(CellSize)
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						FOnClicked::CreateUObject(this, &UT66QuickArcadeWidget::HandleCellClicked, CellIndex),
						SAssignNew(CellBorder, SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.025f, 0.030f, 0.040f, 0.90f))
						.Padding(FMargin(6.f))
						[
							SAssignNew(InnerCellBorder, SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.06f, 0.065f, 0.075f, 0.95f))
							.Padding(FMargin(4.f))
							[
								SAssignNew(CellText, STextBlock)
								.Text(FText::GetEmpty())
								.Font(FT66Style::Tokens::FontBold(16))
								.ColorAndOpacity(FT66Style::Tokens::Text)
								.Justification(ETextJustify::Center)
							]
						])
					.SetColor(FLinearColor::Transparent)
					.SetPadding(FMargin(0.f)))
			]
		];

		CellBorders[CellIndex] = CellBorder;
		InnerCellBorders[CellIndex] = InnerCellBorder;
		CellTextBlocks[CellIndex] = CellText;
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

	const TSharedRef<SWidget> InfoPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(ArcadeData.DisplayName.IsEmpty() ? Spec.FallbackTitle : ArcadeData.DisplayName)
				.Font(FT66Style::Tokens::FontBold(38))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
					BuildStatPanel(
						NSLOCTEXT("T66.Arcade", "QuickArcadeTimerLabel", "TIME"),
						SAssignNew(TimerTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(30))
						.ColorAndOpacity(FT66Style::Tokens::Text))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 10.f, 0.f)
				[
					BuildStatPanel(
						NSLOCTEXT("T66.Arcade", "QuickArcadeScoreLabel", "SCORE"),
						SAssignNew(ScoreTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(30))
						.ColorAndOpacity(FT66Style::Tokens::Accent))
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					BuildStatPanel(
						NSLOCTEXT("T66.Arcade", "QuickArcadeHitsLabel", "HITS"),
						SAssignNew(HitsTextBlock, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(30))
						.ColorAndOpacity(FT66Style::Tokens::Success))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66Style::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						BuildArtworkLayer(0.38f)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(FMargin(18.f))
					[
						SNew(SBox)
						.WidthOverride(BoardSize.X)
						.HeightOverride(BoardSize.Y)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								BoardGrid
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SAssignNew(SpriteCanvas, SCanvas)
							]
						]
					],
					FT66PanelParams(ET66PanelType::Panel2)
						.SetPadding(FMargin(18.f))
						.SetColor(FLinearColor(0.035f, 0.036f, 0.045f, 1.f)))
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0.f, 16.f, 0.f, 0.f)
			[
				SAssignNew(ActionButtonBox, SBox)
				[
					FT66Style::MakeButton(
						FT66ButtonParams(
							Spec.ActionText,
							FOnClicked::CreateUObject(this, &UT66QuickArcadeWidget::HandleActionClicked),
							ET66ButtonType::Primary)
						.SetMinWidth(220.f)
						.SetHeight(54.f)
						.SetContent(
							SAssignNew(ActionTextBlock, STextBlock)
							.Text(FText::GetEmpty())
							.Font(FT66Style::Tokens::FontBold(18))
							.ColorAndOpacity(FT66Style::Tokens::Text)))
				]
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
						NSLOCTEXT("T66.Arcade", "QuickArcadeAbort", "Abort"),
						FOnClicked::CreateUObject(this, &UT66QuickArcadeWidget::HandlePrimaryActionClicked),
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
			.WidthOverride(GQuickArcadeOverlayWidth)
			[
				InfoPanel
			]
		];

	return FT66Style::MakeResponsiveRoot(Root);
}

void UT66QuickArcadeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StartRound();
}

void UT66QuickArcadeWidget::NativeDestruct()
{
	ClearActiveTimers();
	Super::NativeDestruct();
}

void UT66QuickArcadeWidget::StartRound()
{
	ClearActiveTimers();

	Spec = ResolveSpec();
	RoundDurationSeconds = ResolveRoundDurationSeconds();
	StartIntervalSeconds = ResolveStartIntervalSeconds();
	EndIntervalSeconds = ResolveEndIntervalSeconds();
	TargetScore = ResolveTargetScore();
	ScorePerHit = ResolveScorePerHit();
	MissPenalty = ResolveMissPenalty();
	Score = 0;
	Hits = 0;
	Misses = 0;
	SequenceStep = 0;
	ActiveCellIndex = INDEX_NONE;
	PreviousCellIndex = INDEX_NONE;
	HazardCellIndex = INDEX_NONE;
	MeterTargetCellIndex = INDEX_NONE;
	bRoundEnded = false;
	bRoundSucceeded = false;

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	RoundStartTimeSeconds = NowSeconds;
	RoundEndTimeSeconds = NowSeconds + RoundDurationSeconds;
	LastPromptTimeSeconds = NowSeconds;
	RemainingSeconds = RoundDurationSeconds;
	RollPrompt();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RoundTickHandle, this, &UT66QuickArcadeWidget::HandleRoundTick, 0.05f, true);
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66QuickArcadeWidget::ClearActiveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundTickHandle);
		World->GetTimerManager().ClearTimer(AutoCloseHandle);
	}
}

void UT66QuickArcadeWidget::RollPrompt()
{
	const int32 CellCount = FMath::Max(1, Spec.CellCount);
	const int32 OldActiveCellIndex = ActiveCellIndex;
	HazardCellIndex = INDEX_NONE;
	MeterTargetCellIndex = INDEX_NONE;

	if (Spec.Mode == EQuickArcadeMode::Sequence)
	{
		Sequence.Reset();
		for (int32 Index = 0; Index < FMath::Max(1, Spec.SequenceLength); ++Index)
		{
			Sequence.Add(FMath::RandHelper(CellCount));
		}
		SequenceStep = 0;
		ActiveCellIndex = Sequence.IsValidIndex(0) ? Sequence[0] : FMath::RandHelper(CellCount);
	}
	else if (Spec.Mode == EQuickArcadeMode::MeterStop)
	{
		MeterTargetCellIndex = FMath::RandRange(FMath::Max(0, CellCount / 3), FMath::Max(0, (CellCount * 2) / 3));
		ActiveCellIndex = ResolveSweepingCellIndex();
	}
	else if (Spec.Mode == EQuickArcadeMode::CenterStack)
	{
		ActiveCellIndex = ResolveSweepingCellIndex();
		MeterTargetCellIndex = CellCount / 2;
	}
	else
	{
		ActiveCellIndex = FMath::RandHelper(CellCount);
		if (Spec.Mode == EQuickArcadeMode::AvoidHazard && CellCount > 1)
		{
			do
			{
				HazardCellIndex = FMath::RandHelper(CellCount);
			}
			while (HazardCellIndex == ActiveCellIndex);
		}
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	PreviousCellIndex = OldActiveCellIndex != INDEX_NONE ? OldActiveCellIndex : ActiveCellIndex;
	LastPromptTimeSeconds = NowSeconds;
}

void UT66QuickArcadeWidget::HandleRoundTick()
{
	if (bRoundEnded)
	{
		return;
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	RemainingSeconds = FMath::Max(0.f, static_cast<float>(RoundEndTimeSeconds - NowSeconds));

	if (Spec.Mode == EQuickArcadeMode::MeterStop || Spec.Mode == EQuickArcadeMode::CenterStack)
	{
		ActiveCellIndex = ResolveSweepingCellIndex();
	}
	else if (NowSeconds - LastPromptTimeSeconds >= ResolveCurrentIntervalSeconds())
	{
		RollPrompt();
	}

	if (RemainingSeconds <= KINDA_SMALL_NUMBER)
	{
		CompleteRound();
		return;
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66QuickArcadeWidget::HandleAutoClose()
{
	StartCloseSequence(bRoundSucceeded, Score);
}

void UT66QuickArcadeWidget::CompleteRound()
{
	if (bRoundEnded)
	{
		return;
	}

	bRoundEnded = true;
	bRoundSucceeded = Score > 0;
	ClearActiveTimers();
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Round.End")));

	RefreshBoardVisuals();
	RefreshHud();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoCloseHandle, this, &UT66QuickArcadeWidget::HandleAutoClose, GQuickArcadeAutoCloseDelay, false);
	}
}

void UT66QuickArcadeWidget::RefreshBoardVisuals()
{
	for (int32 CellIndex = 0; CellIndex < Spec.CellCount; ++CellIndex)
	{
		if (CellBorders.IsValidIndex(CellIndex) && CellBorders[CellIndex].IsValid())
		{
			CellBorders[CellIndex]->SetBorderBackgroundColor(GetCellColor(CellIndex));
		}
		if (InnerCellBorders.IsValidIndex(CellIndex) && InnerCellBorders[CellIndex].IsValid())
		{
			InnerCellBorders[CellIndex]->SetBorderBackgroundColor(GetInnerCellColor(CellIndex));
		}
		if (CellTextBlocks.IsValidIndex(CellIndex) && CellTextBlocks[CellIndex].IsValid())
		{
			CellTextBlocks[CellIndex]->SetText(GetCellText(CellIndex));
		}
	}

	RefreshSpriteLayer();
}

void UT66QuickArcadeWidget::RefreshHud()
{
	if (TimerTextBlock.IsValid())
	{
		TimerTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingSeconds))));
	}
	if (ScoreTextBlock.IsValid())
	{
		ScoreTextBlock->SetText(FText::AsNumber(Score));
	}
	if (HitsTextBlock.IsValid())
	{
		HitsTextBlock->SetText(FText::Format(
			NSLOCTEXT("T66.Arcade", "QuickArcadeHitsValue", "{0}/{1}"),
			FText::AsNumber(Hits),
			FText::AsNumber(FMath::Max(1, FMath::CeilToInt(static_cast<float>(TargetScore) / static_cast<float>(FMath::Max(1, ScorePerHit)))))));
	}
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(BuildStatusText());
		StatusTextBlock->SetColorAndOpacity(
			bRoundEnded
				? (bRoundSucceeded ? FT66Style::Tokens::Success : FT66Style::Tokens::Danger)
				: FT66Style::Tokens::Text);
	}
	if (ActionTextBlock.IsValid())
	{
		ActionTextBlock->SetText(Spec.ActionText);
	}
	if (ActionButtonBox.IsValid())
	{
		ActionButtonBox->SetVisibility(
			Spec.Mode == EQuickArcadeMode::MeterStop || Spec.Mode == EQuickArcadeMode::CenterStack
				? EVisibility::Visible
				: EVisibility::Collapsed);
	}
	if (PrimaryActionTextBlock.IsValid())
	{
		PrimaryActionTextBlock->SetText(BuildPrimaryActionText());
	}
}

UT66QuickArcadeWidget::FQuickArcadeSpec UT66QuickArcadeWidget::ResolveSpec() const
{
	FQuickArcadeSpec Out;
	switch (ArcadeData.ArcadeGameType)
	{
	case ET66ArcadeGameType::CartSwitcher:
		Out = { EQuickArcadeMode::HitTarget, NSLOCTEXT("T66.Arcade", "CartSwitcherFallbackTitle", "Cart Switcher"), NSLOCTEXT("T66.Arcade", "CartSwitcherStatus", "Switch the cart into the lit track."), NSLOCTEXT("T66.Arcade", "CartSwitcherAction", "Switch"), TEXT("SourceAssets/Arcade/CartSwitcher/cart_switcher_panel.png"), TEXT("SourceAssets/Arcade/CartSwitcher/cart_sprite.png"), FString(), TEXT("SourceAssets/Arcade/PotionPour/timing_marker_sprite.png"), 5, 5, 0 };
		break;
	case ET66ArcadeGameType::CrystalDash:
		Out = { EQuickArcadeMode::AvoidHazard, NSLOCTEXT("T66.Arcade", "CrystalDashFallbackTitle", "Crystal Dash"), NSLOCTEXT("T66.Arcade", "CrystalDashStatus", "Dash through crystals and away from spikes."), NSLOCTEXT("T66.Arcade", "CrystalDashAction", "Dash"), TEXT("SourceAssets/Arcade/CrystalDash/crystal_dash_panel.png"), TEXT("SourceAssets/Arcade/CrystalDash/runner_sprite.png"), TEXT("SourceAssets/Arcade/CrystalDash/spike_sprite.png"), FString(), 3, 3, 0 };
		break;
	case ET66ArcadeGameType::PotionPour:
		Out = { EQuickArcadeMode::MeterStop, NSLOCTEXT("T66.Arcade", "PotionPourFallbackTitle", "Potion Pour"), NSLOCTEXT("T66.Arcade", "PotionPourStatus", "Stop the pour on the glowing bottle mark."), NSLOCTEXT("T66.Arcade", "PotionPourAction", "Stop"), TEXT("SourceAssets/Arcade/PotionPour/potion_pour_panel.png"), TEXT("SourceAssets/Arcade/PotionPour/potion_sprite.png"), FString(), TEXT("SourceAssets/Arcade/PotionPour/timing_marker_sprite.png"), 9, 9, 0 };
		break;
	case ET66ArcadeGameType::RelicStack:
		Out = { EQuickArcadeMode::CenterStack, NSLOCTEXT("T66.Arcade", "RelicStackFallbackTitle", "Relic Stack"), NSLOCTEXT("T66.Arcade", "RelicStackStatus", "Drop the moving relic over the center stack."), NSLOCTEXT("T66.Arcade", "RelicStackAction", "Drop"), TEXT("SourceAssets/Arcade/RelicStack/relic_stack_panel.png"), TEXT("SourceAssets/Arcade/RelicStack/relic_sprite.png"), FString(), TEXT("SourceAssets/Arcade/PotionPour/timing_marker_sprite.png"), 5, 5, 0 };
		break;
	case ET66ArcadeGameType::ShieldParry:
		Out = { EQuickArcadeMode::HitTarget, NSLOCTEXT("T66.Arcade", "ShieldParryFallbackTitle", "Shield Parry"), NSLOCTEXT("T66.Arcade", "ShieldParryStatus", "Parry the lit projectile direction."), NSLOCTEXT("T66.Arcade", "ShieldParryAction", "Parry"), TEXT("SourceAssets/Arcade/ShieldParry/shield_parry_panel.png"), TEXT("SourceAssets/Arcade/ShieldParry/projectile_sprite.png"), FString(), TEXT("SourceAssets/Arcade/ShieldParry/shield_sprite.png"), 4, 4, 0 };
		break;
	case ET66ArcadeGameType::MimicMemory:
		Out = { EQuickArcadeMode::Sequence, NSLOCTEXT("T66.Arcade", "MimicMemoryFallbackTitle", "Mimic Memory"), NSLOCTEXT("T66.Arcade", "MimicMemoryStatus", "Repeat the chest sequence before it changes."), NSLOCTEXT("T66.Arcade", "MimicMemoryAction", "Repeat"), TEXT("SourceAssets/Arcade/MimicMemory/mimic_memory_panel.png"), TEXT("SourceAssets/Arcade/MimicMemory/mimic_sprite.png"), FString(), TEXT("SourceAssets/Arcade/PotionPour/timing_marker_sprite.png"), 6, 3, 3 };
		break;
	case ET66ArcadeGameType::BombSorter:
		Out = { EQuickArcadeMode::HitTarget, NSLOCTEXT("T66.Arcade", "BombSorterFallbackTitle", "Bomb Sorter"), NSLOCTEXT("T66.Arcade", "BombSorterStatus", "Sort the lit bomb into the matching chute."), NSLOCTEXT("T66.Arcade", "BombSorterAction", "Sort"), TEXT("SourceAssets/Arcade/BombSorter/bomb_sorter_panel.png"), TEXT("SourceAssets/Arcade/BombSorter/bomb_sprite.png"), FString(), TEXT("SourceAssets/Arcade/BombSorter/chute_sprite.png"), 5, 5, 0 };
		break;
	case ET66ArcadeGameType::LanternLeap:
		Out = { EQuickArcadeMode::HitTarget, NSLOCTEXT("T66.Arcade", "LanternLeapFallbackTitle", "Lantern Leap"), NSLOCTEXT("T66.Arcade", "LanternLeapStatus", "Leap onto the glowing lantern platform."), NSLOCTEXT("T66.Arcade", "LanternLeapAction", "Leap"), TEXT("SourceAssets/Arcade/LanternLeap/lantern_leap_panel.png"), TEXT("SourceAssets/Arcade/LanternLeap/lantern_sprite.png"), FString(), TEXT("SourceAssets/Arcade/PotionPour/timing_marker_sprite.png"), 6, 3, 0 };
		break;
	case ET66ArcadeGameType::BladeSweep:
		Out = { EQuickArcadeMode::AvoidHazard, NSLOCTEXT("T66.Arcade", "BladeSweepFallbackTitle", "Blade Sweep"), NSLOCTEXT("T66.Arcade", "BladeSweepStatus", "Sweep cursed fruit and avoid the bombs."), NSLOCTEXT("T66.Arcade", "BladeSweepAction", "Sweep"), TEXT("SourceAssets/Arcade/BladeSweep/blade_sweep_panel.png"), TEXT("SourceAssets/Arcade/BladeSweep/fruit_sprite.png"), TEXT("SourceAssets/Arcade/BombSorter/bomb_sprite.png"), TEXT("SourceAssets/Arcade/BladeSweep/blade_sprite.png"), 9, 3, 0 };
		break;
	case ET66ArcadeGameType::RuneSwipe:
	default:
		Out = { EQuickArcadeMode::HitTarget, NSLOCTEXT("T66.Arcade", "RuneSwipeFallbackTitle", "Rune Swipe"), NSLOCTEXT("T66.Arcade", "RuneSwipeStatus", "Tap the glowing rune chain before it fades."), NSLOCTEXT("T66.Arcade", "RuneSwipeAction", "Swipe"), TEXT("SourceAssets/Arcade/RuneSwipe/rune_swipe_panel.png"), TEXT("SourceAssets/Arcade/RuneSwipe/rune_sprite.png"), FString(), TEXT("SourceAssets/Arcade/PotionPour/timing_marker_sprite.png"), 9, 3, 0 };
		break;
	}
	return Out;
}

float UT66QuickArcadeWidget::ResolveCellSize() const
{
	return FMath::Max(1, Spec.Columns) > 6 ? GQuickArcadeCompactCellSize : GQuickArcadeLargeCellSize;
}

FVector2D UT66QuickArcadeWidget::ResolveBoardSize() const
{
	const int32 Columns = FMath::Max(1, Spec.Columns);
	const int32 Rows = FMath::Max(1, FMath::DivideAndRoundUp(FMath::Max(1, Spec.CellCount), Columns));
	const float CellSize = ResolveCellSize();
	const float PaddedCellSize = CellSize + (GQuickArcadeCellPadding * 2.f);
	return FVector2D(PaddedCellSize * static_cast<float>(Columns), PaddedCellSize * static_cast<float>(Rows));
}

FVector2D UT66QuickArcadeWidget::ResolveCellCenter(const int32 CellIndex) const
{
	const int32 CellCount = FMath::Max(1, Spec.CellCount);
	const int32 ClampedCellIndex = FMath::Clamp(CellIndex, 0, CellCount - 1);
	const int32 Columns = FMath::Max(1, Spec.Columns);
	const float CellSize = ResolveCellSize();
	const float PaddedCellSize = CellSize + (GQuickArcadeCellPadding * 2.f);
	const int32 X = ClampedCellIndex % Columns;
	const int32 Y = ClampedCellIndex / Columns;
	return FVector2D(
		GQuickArcadeCellPadding + (static_cast<float>(X) * PaddedCellSize) + (CellSize * 0.5f),
		GQuickArcadeCellPadding + (static_cast<float>(Y) * PaddedCellSize) + (CellSize * 0.5f));
}

FVector2D UT66QuickArcadeWidget::ResolveAnimatedCellCenter() const
{
	if (ActiveCellIndex == INDEX_NONE)
	{
		return ResolveBoardSize() * 0.5f;
	}

	const FVector2D To = ResolveCellCenter(ActiveCellIndex);
	if (PreviousCellIndex == INDEX_NONE || PreviousCellIndex == ActiveCellIndex)
	{
		return To;
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float TravelAlpha = FMath::Clamp(static_cast<float>((NowSeconds - LastPromptTimeSeconds) / 0.18), 0.f, 1.f);
	const float EasedAlpha = TravelAlpha * TravelAlpha * (3.f - (2.f * TravelAlpha));
	return FMath::Lerp(ResolveCellCenter(PreviousCellIndex), To, EasedAlpha);
}

float UT66QuickArcadeWidget::ResolveRoundDurationSeconds() const
{
	return FMath::Max(4.f, ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeRoundSeconds, 10.f));
}

float UT66QuickArcadeWidget::ResolveStartIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeStartInterval, 0.85f), 0.2f, 3.f);
}

float UT66QuickArcadeWidget::ResolveEndIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeEndInterval, 0.32f), 0.12f, 3.f);
}

float UT66QuickArcadeWidget::ResolveCurrentIntervalSeconds() const
{
	const float Progress01 = RoundDurationSeconds > KINDA_SMALL_NUMBER
		? FMath::Clamp(1.f - (RemainingSeconds / RoundDurationSeconds), 0.f, 1.f)
		: 1.f;
	return FMath::Lerp(StartIntervalSeconds, EndIntervalSeconds, Progress01);
}

int32 UT66QuickArcadeWidget::ResolveTargetScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeTargetScore, 100));
}

int32 UT66QuickArcadeWidget::ResolveScorePerHit() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeScorePerHit, 10));
}

int32 UT66QuickArcadeWidget::ResolveMissPenalty() const
{
	return FMath::Max(0, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeMissPenalty, 3));
}

int32 UT66QuickArcadeWidget::ResolveSweepingCellIndex() const
{
	const int32 CellCount = FMath::Max(1, Spec.CellCount);
	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float SweepAlpha = 0.5f + 0.5f * FMath::Sin(static_cast<float>((NowSeconds - RoundStartTimeSeconds) * 4.5));
	return FMath::Clamp(FMath::RoundToInt(SweepAlpha * static_cast<float>(CellCount - 1)), 0, CellCount - 1);
}

bool UT66QuickArcadeWidget::IsCellGood(const int32 CellIndex) const
{
	if (Spec.Mode == EQuickArcadeMode::CenterStack)
	{
		return CellIndex == MeterTargetCellIndex && ActiveCellIndex == MeterTargetCellIndex;
	}
	if (Spec.Mode == EQuickArcadeMode::MeterStop)
	{
		return CellIndex == MeterTargetCellIndex;
	}
	return CellIndex == ActiveCellIndex;
}

bool UT66QuickArcadeWidget::IsCellHazard(const int32 CellIndex) const
{
	return CellIndex == HazardCellIndex
		|| (Spec.Mode == EQuickArcadeMode::AvoidHazard && CellIndex != ActiveCellIndex && CellIndex % 2 == 0);
}

FText UT66QuickArcadeWidget::BuildStatusText() const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "QuickArcadeStatusWon", "Score locked. Cabinet paying out...")
			: NSLOCTEXT("T66.Arcade", "QuickArcadeStatusLost", "No score. Cabinet shutting down.");
	}

	if (Spec.Mode == EQuickArcadeMode::Sequence)
	{
		return FText::Format(
			NSLOCTEXT("T66.Arcade", "QuickArcadeSequenceStatus", "{0} Step {1}/{2}"),
			Spec.StatusActive,
			FText::AsNumber(SequenceStep + 1),
			FText::AsNumber(FMath::Max(1, Spec.SequenceLength)));
	}

	return Spec.StatusActive;
}

FText UT66QuickArcadeWidget::BuildPrimaryActionText() const
{
	return bRoundEnded
		? NSLOCTEXT("T66.Arcade", "QuickArcadeClose", "Continue")
		: NSLOCTEXT("T66.Arcade", "QuickArcadeAbortButton", "Abort");
}

FText UT66QuickArcadeWidget::GetCellText(const int32 CellIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "QuickArcadeCellDone", "OK")
			: FText::GetEmpty();
	}
	if (Spec.Mode == EQuickArcadeMode::MeterStop)
	{
		if (CellIndex == ActiveCellIndex)
		{
			return NSLOCTEXT("T66.Arcade", "QuickArcadeMeterFill", "FILL");
		}
		return CellIndex == MeterTargetCellIndex
			? NSLOCTEXT("T66.Arcade", "QuickArcadeMeterTarget", "MARK")
			: FText::GetEmpty();
	}
	if (Spec.Mode == EQuickArcadeMode::CenterStack)
	{
		if (CellIndex == ActiveCellIndex)
		{
			return NSLOCTEXT("T66.Arcade", "QuickArcadeMovingCell", "DROP");
		}
		return CellIndex == MeterTargetCellIndex
			? NSLOCTEXT("T66.Arcade", "QuickArcadeStackCell", "STACK")
			: FText::GetEmpty();
	}
	if (Spec.Mode == EQuickArcadeMode::Sequence && CellIndex == ActiveCellIndex)
	{
		return NSLOCTEXT("T66.Arcade", "QuickArcadeSequenceCell", "NEXT");
	}
	if (CellIndex == ActiveCellIndex)
	{
		return Spec.ActionText;
	}
	if (IsCellHazard(CellIndex))
	{
		return NSLOCTEXT("T66.Arcade", "QuickArcadeHazardCell", "BAD");
	}
	return FText::GetEmpty();
}

FLinearColor UT66QuickArcadeWidget::GetCellColor(const int32 CellIndex) const
{
	if (bRoundEnded)
	{
		return bRoundSucceeded
			? FLinearColor(0.12f, 0.34f, 0.18f, 0.92f)
			: FLinearColor(0.30f, 0.10f, 0.09f, 0.92f);
	}
	if (CellIndex == ActiveCellIndex)
	{
		return FLinearColor(0.40f, 0.25f, 0.06f, 0.98f);
	}
	if (CellIndex == MeterTargetCellIndex)
	{
		return FLinearColor(0.10f, 0.45f, 0.36f, 0.95f);
	}
	if (IsCellHazard(CellIndex))
	{
		return FLinearColor(0.30f, 0.06f, 0.05f, 0.86f);
	}
	return FLinearColor(0.025f, 0.030f, 0.040f, 0.80f);
}

FLinearColor UT66QuickArcadeWidget::GetInnerCellColor(const int32 CellIndex) const
{
	if (CellIndex == ActiveCellIndex)
	{
		return FLinearColor(0.96f, 0.64f, 0.12f, 1.f);
	}
	if (CellIndex == MeterTargetCellIndex)
	{
		return FLinearColor(0.16f, 0.80f, 0.62f, 1.f);
	}
	if (IsCellHazard(CellIndex))
	{
		return FLinearColor(0.60f, 0.10f, 0.08f, 1.f);
	}
	return FLinearColor(0.055f, 0.060f, 0.075f, 0.95f);
}

FReply UT66QuickArcadeWidget::HandleCellClicked(const int32 CellIndex)
{
	if (bRoundEnded)
	{
		return FReply::Handled();
	}

	bool bHit = false;
	if (Spec.Mode == EQuickArcadeMode::MeterStop)
	{
		bHit = ActiveCellIndex == MeterTargetCellIndex;
	}
	else if (Spec.Mode == EQuickArcadeMode::CenterStack)
	{
		bHit = ActiveCellIndex == MeterTargetCellIndex;
	}
	else if (Spec.Mode == EQuickArcadeMode::Sequence)
	{
		bHit = CellIndex == ActiveCellIndex;
	}
	else
	{
		bHit = CellIndex == ActiveCellIndex;
	}

	if (bHit)
	{
		++Hits;
		Score += ScorePerHit;
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Hit")));
		if (Spec.Mode == EQuickArcadeMode::Sequence)
		{
			++SequenceStep;
			if (Sequence.IsValidIndex(SequenceStep))
			{
				PreviousCellIndex = ActiveCellIndex;
				ActiveCellIndex = Sequence[SequenceStep];
				LastPromptTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
			}
			else
			{
				RollPrompt();
			}
		}
		else
		{
			RollPrompt();
		}
	}
	else
	{
		++Misses;
		Score = FMath::Max(0, Score - MissPenalty);
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Miss")));
	}

	RefreshBoardVisuals();
	RefreshHud();
	return FReply::Handled();
}

FReply UT66QuickArcadeWidget::HandleActionClicked()
{
	if ((Spec.Mode == EQuickArcadeMode::MeterStop || Spec.Mode == EQuickArcadeMode::CenterStack)
		&& ActiveCellIndex != INDEX_NONE)
	{
		return HandleCellClicked(ActiveCellIndex);
	}

	return FReply::Handled();
}

FReply UT66QuickArcadeWidget::HandlePrimaryActionClicked()
{
	StartCloseSequence(bRoundEnded ? bRoundSucceeded : false, bRoundEnded ? Score : 0);
	return FReply::Handled();
}

void UT66QuickArcadeWidget::LoadArtworkBrush()
{
	if (ArtworkTexture.IsValid())
	{
		return;
	}

	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(Spec.ArtworkPath))
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Trilinear, TEXT("QuickArcadePanel")))
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

const FSlateBrush* UT66QuickArcadeWidget::FindOrLoadSpriteBrush(const FString& RelativePath)
{
	if (RelativePath.IsEmpty())
	{
		return nullptr;
	}

	if (FQuickArcadeSpriteBrush* Existing = SpriteBrushes.Find(RelativePath))
	{
		return Existing->Texture.IsValid() && Existing->Brush.IsValid() ? Existing->Brush.Get() : nullptr;
	}

	FQuickArcadeSpriteBrush Entry;
	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(CandidatePath, TextureFilter::TF_Nearest, TEXT("QuickArcadeSprite")))
		{
			Entry.Texture.Reset(Texture);
			Entry.Brush = MakeShared<FSlateBrush>();
			Entry.Brush->SetResourceObject(Texture);
			Entry.Brush->DrawAs = ESlateBrushDrawType::Image;
			Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
			Entry.Brush->ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());

			FQuickArcadeSpriteBrush& Stored = SpriteBrushes.Add(RelativePath, MoveTemp(Entry));
			return Stored.Brush.Get();
		}
	}

	SpriteBrushes.Add(RelativePath, MoveTemp(Entry));
	return nullptr;
}

void UT66QuickArcadeWidget::AddSpriteToCanvas(
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

void UT66QuickArcadeWidget::RefreshSpriteLayer()
{
	if (!SpriteCanvas.IsValid())
	{
		return;
	}

	SpriteCanvas->ClearChildren();
	if (bRoundEnded)
	{
		const FVector2D Center = ResolveBoardSize() * 0.5f;
		AddSpriteToCanvas(
			Spec.MarkerSpritePath.IsEmpty() ? Spec.PrimarySpritePath : Spec.MarkerSpritePath,
			Center,
			ResolveCellSize() * 1.18f,
			bRoundSucceeded ? FLinearColor(1.f, 1.f, 1.f, 0.94f) : FLinearColor(1.f, 0.34f, 0.30f, 0.72f));
		return;
	}

	const double NowSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : FPlatformTime::Seconds();
	const float CellSize = ResolveCellSize();
	const float Pulse = 0.5f + (0.5f * FMath::Sin(static_cast<float>(NowSeconds * 9.0)));
	const float SpriteSize = CellSize * FMath::Lerp(0.78f, 0.95f, Pulse);
	const FVector2D ActiveCenter = ResolveAnimatedCellCenter();

	if (Spec.Mode == EQuickArcadeMode::MeterStop)
	{
		if (MeterTargetCellIndex != INDEX_NONE)
		{
			AddSpriteToCanvas(
				Spec.MarkerSpritePath,
				ResolveCellCenter(MeterTargetCellIndex),
				CellSize * 0.84f,
				FLinearColor(1.f, 0.86f, 0.24f, 0.92f));
		}
		AddSpriteToCanvas(Spec.PrimarySpritePath, ActiveCenter, SpriteSize, FLinearColor(1.f, 1.f, 1.f, 1.f));
		return;
	}

	if (Spec.Mode == EQuickArcadeMode::CenterStack)
	{
		if (MeterTargetCellIndex != INDEX_NONE)
		{
			AddSpriteToCanvas(
				Spec.MarkerSpritePath,
				ResolveCellCenter(MeterTargetCellIndex),
				CellSize * 0.72f,
				FLinearColor(0.42f, 1.f, 0.82f, 0.86f));
		}
		AddSpriteToCanvas(Spec.PrimarySpritePath, ActiveCenter, SpriteSize, FLinearColor(1.f, 1.f, 1.f, 1.f));
		return;
	}

	if (Spec.Mode == EQuickArcadeMode::AvoidHazard)
	{
		for (int32 CellIndex = 0; CellIndex < Spec.CellCount; ++CellIndex)
		{
			if (IsCellHazard(CellIndex))
			{
				const float HazardPulse = 0.5f + (0.5f * FMath::Sin(static_cast<float>((NowSeconds * 7.0) + CellIndex)));
				AddSpriteToCanvas(
					Spec.HazardSpritePath,
					ResolveCellCenter(CellIndex),
					CellSize * FMath::Lerp(0.58f, 0.75f, HazardPulse),
					FLinearColor(1.f, 1.f, 1.f, CellIndex == HazardCellIndex ? 0.94f : 0.66f));
			}
		}
	}

	if (!Spec.MarkerSpritePath.IsEmpty()
		&& (ArcadeData.ArcadeGameType == ET66ArcadeGameType::ShieldParry
			|| ArcadeData.ArcadeGameType == ET66ArcadeGameType::BombSorter
			|| ArcadeData.ArcadeGameType == ET66ArcadeGameType::BladeSweep))
	{
		const FVector2D Offset(
			FMath::Cos(static_cast<float>(NowSeconds * 6.0)) * CellSize * 0.16f,
			FMath::Sin(static_cast<float>(NowSeconds * 6.0)) * CellSize * 0.12f);
		AddSpriteToCanvas(
			Spec.MarkerSpritePath,
			ActiveCenter + Offset,
			CellSize * 0.62f,
			FLinearColor(1.f, 1.f, 1.f, 0.88f));
	}

	AddSpriteToCanvas(Spec.PrimarySpritePath, ActiveCenter, SpriteSize, FLinearColor(1.f, 1.f, 1.f, 1.f));
}

TSharedRef<SWidget> UT66QuickArcadeWidget::BuildArtworkLayer(const float Opacity) const
{
	if (!ArtworkTexture.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	return SNew(SImage)
		.Image(&ArtworkBrush)
		.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(Opacity, 0.f, 1.f)));
}
