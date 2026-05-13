// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66WhackAMoleArcadeWidget.h"

#include "Core/T66AudioSubsystem.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Styling/CoreStyle.h"
#include "TimerManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float GWhackReferenceWidth = 1280.f;
	constexpr float GWhackReferenceHeight = 720.f;
	constexpr float GWhackBoardLeft = 230.f;
	constexpr float GWhackBoardTop = 170.f;
	constexpr float GWhackBoardWidth = 820.f;
	constexpr float GWhackBoardHeight = 500.f;
	constexpr float GWhackSlotWidth = 180.f;
	constexpr float GWhackSlotHeight = 170.f;
	constexpr float GWhackTickRateSeconds = 1.f / 30.f;
	constexpr float GWhackRiseSeconds = 0.16f;
	constexpr float GWhackHitSeconds = 0.22f;
	constexpr float GWhackRetreatSeconds = 0.16f;
	constexpr float GWhackEffectSeconds = 0.28f;
	constexpr float GWhackPopupSeconds = 0.56f;
	constexpr float GWhackHammerSeconds = 0.14f;

	const FVector2D GWhackSlotPositions[9] =
	{
		FVector2D(300.f, 218.f),
		FVector2D(550.f, 218.f),
		FVector2D(800.f, 218.f),
		FVector2D(300.f, 342.f),
		FVector2D(550.f, 342.f),
		FVector2D(800.f, 342.f),
		FVector2D(300.f, 466.f),
		FVector2D(550.f, 466.f),
		FVector2D(800.f, 466.f)
	};

	FName MakeSpriteName(const TCHAR* Name)
	{
		return FName(Name);
	}
}

TSharedRef<SWidget> UT66WhackAMoleArcadeWidget::RebuildWidget()
{
	SpriteBrushes.Empty();
	SpriteTextures.Empty();

	MoleSlots.Empty(CellCount);
	MoleSlots.SetNum(CellCount);
	SlotButtons.Empty(CellCount);
	SlotButtons.SetNum(CellCount);
	HoleImages.Empty(CellCount);
	HoleImages.SetNum(CellCount);
	MoleImages.Empty(CellCount);
	MoleImages.SetNum(CellCount);
	HoverImages.Empty(CellCount);
	HoverImages.SetNum(CellCount);
	EffectImages.Empty(CellCount);
	EffectImages.SetNum(CellCount);
	ScorePopupImages.Empty(CellCount);
	ScorePopupImages.SetNum(CellCount);
	HammerImages.Empty(CellCount);
	HammerImages.SetNum(CellCount);
	ScorePopupTextBlocks.Empty(CellCount);
	ScorePopupTextBlocks.SetNum(CellCount);
	LifeImages.Empty();

	for (int32 SlotIndex = 0; SlotIndex < CellCount; ++SlotIndex)
	{
		MoleSlots[SlotIndex].SlotIndex = SlotIndex;
	}

	const TSharedRef<SConstraintCanvas> GameCanvas = SNew(SConstraintCanvas)
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(0.f, 0.f, GWhackReferenceWidth, GWhackReferenceHeight))
		[
			BuildSpriteImage(MakeSpriteName(TEXT("bg_whackamole_8bit")), FVector2D(GWhackReferenceWidth, GWhackReferenceHeight))
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(GWhackBoardLeft, GWhackBoardTop, GWhackBoardWidth, GWhackBoardHeight))
		[
			BuildSpriteImage(MakeSpriteName(TEXT("board_dirt_8bit")), FVector2D(GWhackBoardWidth, GWhackBoardHeight))
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(44.f, 24.f, 256.f, 72.f))
		[
			BuildHudPanel(
				MakeSpriteName(TEXT("hud_trophy_8bit")),
				NSLOCTEXT("T66.Arcade", "WhackScoreLabel", "SCORE"),
				SAssignNew(ScoreTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(28))
				.ColorAndOpacity(FT66Style::Tokens::Accent))
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(512.f, 24.f, 256.f, 72.f))
		[
			BuildHudPanel(
				MakeSpriteName(TEXT("hud_timer_8bit")),
				NSLOCTEXT("T66.Arcade", "WhackTimerLabel", "TIME"),
				SAssignNew(TimerTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(30))
				.ColorAndOpacity(FT66Style::Tokens::Text))
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(980.f, 24.f, 256.f, 72.f))
		[
			BuildHudPanel(
				MakeSpriteName(TEXT("hud_heart_full_8bit")),
				NSLOCTEXT("T66.Arcade", "WhackComboLabel", "COMBO"),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(ComboTextBlock, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(24))
					.ColorAndOpacity(FT66Style::Tokens::Success)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 2.f, 0.f, 0.f)
				[
					BuildLivesWidget()
				])
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(454.f, 104.f, 372.f, 40.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SAssignNew(MultiplierTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(20))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SAssignNew(BestComboTextBlock, STextBlock)
				.Text(FText::GetEmpty())
				.Font(FT66Style::Tokens::FontBold(20))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(340.f, 668.f, 600.f, 34.f))
		[
			SAssignNew(StatusTextBlock, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FT66Style::Tokens::FontBold(22))
			.ColorAndOpacity(FT66Style::Tokens::Text)
			.Justification(ETextJustify::Center)
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(38.f, 662.f, 360.f, 42.f))
		[
			SAssignNew(RewardTextBlock, STextBlock)
			.Text(FText::GetEmpty())
			.Font(FT66Style::Tokens::FontRegular(14))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.AutoWrapText(true)
		]
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(1064.f, 652.f, 176.f, 54.f))
		[
			FT66Style::MakeButton(
				FT66ButtonParams(
					NSLOCTEXT("T66.Arcade", "WhackAbort", "Abort"),
					FOnClicked::CreateUObject(this, &UT66WhackAMoleArcadeWidget::HandlePrimaryActionClicked),
					ET66ButtonType::Neutral)
				.SetMinWidth(176.f)
				.SetHeight(54.f)
				.SetContent(
					SAssignNew(PrimaryActionTextBlock, STextBlock)
					.Text(FText::GetEmpty())
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)))
		];

	for (int32 SlotIndex = 0; SlotIndex < CellCount; ++SlotIndex)
	{
		GameCanvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(GWhackSlotPositions[SlotIndex].X, GWhackSlotPositions[SlotIndex].Y, GWhackSlotWidth, GWhackSlotHeight))
			[
				BuildSlotWidget(SlotIndex)
			];
	}

	const TSharedRef<SWidget> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.78f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(GWhackReferenceWidth)
			.HeightOverride(GWhackReferenceHeight)
			[
				GameCanvas
			]
		];

	RefreshBoardVisuals();
	RefreshHud();

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
	VisibleStartSeconds = ResolveVisibleStartSeconds();
	VisibleEndSeconds = ResolveVisibleEndSeconds();
	TargetScore = ResolveTargetScore();
	ScorePerHit = ResolveScorePerHit();
	GoldenScore = ResolveGoldenScore();
	BombPenalty = ResolveBombPenalty();
	Lives = ResolveStartingLives();
	RemainingSeconds = RoundDurationSeconds;
	SpawnAccumulator = 0.f;
	HammerTimer = 0.f;
	HammerCellIndex = INDEX_NONE;
	HoveredCellIndex = INDEX_NONE;
	LastTickTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	Score = 0;
	Hits = 0;
	Misses = 0;
	Combo = 0;
	BestCombo = 0;
	bRoundSucceeded = false;
	GameState = EWhackAMoleGameState::Playing;

	for (int32 SlotIndex = 0; SlotIndex < MoleSlots.Num(); ++SlotIndex)
	{
		MoleSlots[SlotIndex] = FWhackAMoleSlotRuntime();
		MoleSlots[SlotIndex].SlotIndex = SlotIndex;
	}

	TrySpawnMole();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RoundTickHandle, this, &UT66WhackAMoleArcadeWidget::HandleRoundTick, GWhackTickRateSeconds, true);
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::ClearActiveTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RoundTickHandle);
	}
}

void UT66WhackAMoleArcadeWidget::HandleRoundTick()
{
	if (GameState != EWhackAMoleGameState::Playing)
	{
		return;
	}

	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : LastTickTimeSeconds + GWhackTickRateSeconds;
	const float DeltaSeconds = FMath::Clamp(
		LastTickTimeSeconds > 0.0 ? static_cast<float>(Now - LastTickTimeSeconds) : GWhackTickRateSeconds,
		0.f,
		0.12f);
	LastTickTimeSeconds = Now;

	RemainingSeconds = FMath::Max(0.f, RemainingSeconds - DeltaSeconds);
	HammerTimer = FMath::Max(0.f, HammerTimer - DeltaSeconds);
	if (HammerTimer <= KINDA_SMALL_NUMBER)
	{
		HammerCellIndex = INDEX_NONE;
	}

	TickSlots(DeltaSeconds);

	SpawnAccumulator -= DeltaSeconds;
	if (SpawnAccumulator <= 0.f)
	{
		TrySpawnMole();
		SpawnAccumulator = ResolveCurrentSpawnIntervalSeconds();
	}

	if (RemainingSeconds <= KINDA_SMALL_NUMBER || Lives <= 0)
	{
		CompleteRound();
		return;
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::CompleteRound()
{
	if (GameState == EWhackAMoleGameState::Finished)
	{
		return;
	}

	GameState = EWhackAMoleGameState::Finished;
	bRoundSucceeded = Score > 0;
	ClearActiveTimers();
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Round.End")));

	for (FWhackAMoleSlotRuntime& MoleSlot : MoleSlots)
	{
		MoleSlot.CurrentState = EWhackAMoleSlotState::Hidden;
		MoleSlot.bCanBeHit = false;
		MoleSlot.StateTimer = 0.f;
		MoleSlot.StateDuration = 0.f;
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66WhackAMoleArcadeWidget::TickSlots(const float DeltaSeconds)
{
	for (FWhackAMoleSlotRuntime& MoleSlot : MoleSlots)
	{
		MoleSlot.EffectTimer = FMath::Max(0.f, MoleSlot.EffectTimer - DeltaSeconds);
		MoleSlot.ScorePopupTimer = FMath::Max(0.f, MoleSlot.ScorePopupTimer - DeltaSeconds);

		if (MoleSlot.CurrentState == EWhackAMoleSlotState::Hidden)
		{
			continue;
		}

		MoleSlot.StateTimer += DeltaSeconds;
		if (MoleSlot.CurrentState == EWhackAMoleSlotState::Rising && MoleSlot.StateTimer >= MoleSlot.StateDuration)
		{
			MoleSlot.CurrentState = EWhackAMoleSlotState::Visible;
			MoleSlot.StateTimer = 0.f;
			MoleSlot.StateDuration = MoleSlot.VisibleDuration;
			MoleSlot.bCanBeHit = true;
			continue;
		}

		if (MoleSlot.CurrentState == EWhackAMoleSlotState::Visible && MoleSlot.StateTimer >= MoleSlot.StateDuration)
		{
			ExpireVisibleMole(MoleSlot);
			continue;
		}

		if ((MoleSlot.CurrentState == EWhackAMoleSlotState::Hit || MoleSlot.CurrentState == EWhackAMoleSlotState::Retreating)
			&& MoleSlot.StateTimer >= MoleSlot.StateDuration)
		{
			MoleSlot.CurrentState = EWhackAMoleSlotState::Hidden;
			MoleSlot.MoleType = EWhackAMoleType::Normal;
			MoleSlot.StateTimer = 0.f;
			MoleSlot.StateDuration = 0.f;
			MoleSlot.VisibleDuration = 0.f;
			MoleSlot.bCanBeHit = false;
		}
	}
}

void UT66WhackAMoleArcadeWidget::TrySpawnMole()
{
	if (GameState != EWhackAMoleGameState::Playing || CountActiveMoles() >= ResolveMaxActiveMoles())
	{
		return;
	}

	TArray<int32> HiddenSlots;
	for (int32 SlotIndex = 0; SlotIndex < MoleSlots.Num(); ++SlotIndex)
	{
		if (MoleSlots[SlotIndex].CurrentState == EWhackAMoleSlotState::Hidden)
		{
			HiddenSlots.Add(SlotIndex);
		}
	}

	if (HiddenSlots.Num() == 0)
	{
		return;
	}

	const int32 ChosenSlot = HiddenSlots[FMath::RandHelper(HiddenSlots.Num())];
	const float Roll = FMath::FRand();
	const float GoldenChance = ResolveGoldenChance();
	const float BombChance = ResolveBombChance();
	EWhackAMoleType MoleType = EWhackAMoleType::Normal;
	if (Roll < GoldenChance)
	{
		MoleType = EWhackAMoleType::Golden;
	}
	else if (Roll < GoldenChance + BombChance)
	{
		MoleType = EWhackAMoleType::Bomb;
	}

	SpawnMoleInSlot(ChosenSlot, MoleType);
}

void UT66WhackAMoleArcadeWidget::SpawnMoleInSlot(const int32 SlotIndex, const EWhackAMoleType MoleType)
{
	if (!MoleSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	FWhackAMoleSlotRuntime& MoleSlot = MoleSlots[SlotIndex];
	MoleSlot.CurrentState = EWhackAMoleSlotState::Rising;
	MoleSlot.MoleType = MoleType;
	MoleSlot.StateTimer = 0.f;
	MoleSlot.StateDuration = GWhackRiseSeconds;
	MoleSlot.VisibleDuration = ResolveVisibleDurationSeconds();
	MoleSlot.EffectTimer = GWhackEffectSeconds;
	MoleSlot.ScorePopupTimer = 0.f;
	MoleSlot.ScorePopupValue = 0;
	MoleSlot.EffectSpriteName = MakeSpriteName(TEXT("fx_dirt_puff_8bit"));
	MoleSlot.ScorePopupSpriteName = NAME_None;
	MoleSlot.bCanBeHit = false;
}

void UT66WhackAMoleArcadeWidget::ExpireVisibleMole(FWhackAMoleSlotRuntime& MoleSlot)
{
	if (MoleSlot.MoleType != EWhackAMoleType::Bomb)
	{
		Combo = 0;
		++Misses;
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Miss")));
	}

	MoleSlot.CurrentState = EWhackAMoleSlotState::Retreating;
	MoleSlot.StateTimer = 0.f;
	MoleSlot.StateDuration = GWhackRetreatSeconds;
	MoleSlot.EffectTimer = GWhackEffectSeconds;
	MoleSlot.EffectSpriteName = MakeSpriteName(TEXT("fx_dirt_puff_8bit"));
	MoleSlot.bCanBeHit = false;
}

void UT66WhackAMoleArcadeWidget::RefreshBoardVisuals()
{
	for (int32 SlotIndex = 0; SlotIndex < CellCount; ++SlotIndex)
	{
		if (!MoleSlots.IsValidIndex(SlotIndex))
		{
			continue;
		}

		const FWhackAMoleSlotRuntime& MoleSlot = MoleSlots[SlotIndex];
		if (HoleImages.IsValidIndex(SlotIndex) && HoleImages[SlotIndex].IsValid())
		{
			HoleImages[SlotIndex]->SetImage(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("hole_idle_8bit"))));
		}

		const bool bMoleVisible = MoleSlot.CurrentState != EWhackAMoleSlotState::Hidden;
		if (MoleImages.IsValidIndex(SlotIndex) && MoleImages[SlotIndex].IsValid())
		{
			MoleImages[SlotIndex]->SetImage(FindOrLoadSpriteBrush(ResolveMoleSpriteName(MoleSlot)));
			MoleImages[SlotIndex]->SetVisibility(bMoleVisible ? EVisibility::HitTestInvisible : EVisibility::Hidden);
		}

		if (HoverImages.IsValidIndex(SlotIndex) && HoverImages[SlotIndex].IsValid())
		{
			HoverImages[SlotIndex]->SetImage(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("hole_hover_highlight_8bit"))));
			HoverImages[SlotIndex]->SetVisibility(
				GameState == EWhackAMoleGameState::Playing && HoveredCellIndex == SlotIndex
					? EVisibility::HitTestInvisible
					: EVisibility::Hidden);
		}

		if (EffectImages.IsValidIndex(SlotIndex) && EffectImages[SlotIndex].IsValid())
		{
			EffectImages[SlotIndex]->SetImage(FindOrLoadSpriteBrush(ResolveEffectSpriteName(MoleSlot)));
			EffectImages[SlotIndex]->SetVisibility(MoleSlot.EffectTimer > KINDA_SMALL_NUMBER ? EVisibility::HitTestInvisible : EVisibility::Hidden);
		}

		const bool bScorePopupVisible = MoleSlot.ScorePopupTimer > KINDA_SMALL_NUMBER && !MoleSlot.ScorePopupSpriteName.IsNone();
		if (ScorePopupImages.IsValidIndex(SlotIndex) && ScorePopupImages[SlotIndex].IsValid())
		{
			ScorePopupImages[SlotIndex]->SetImage(FindOrLoadSpriteBrush(MoleSlot.ScorePopupSpriteName));
			ScorePopupImages[SlotIndex]->SetVisibility(bScorePopupVisible ? EVisibility::HitTestInvisible : EVisibility::Hidden);
		}

		if (ScorePopupTextBlocks.IsValidIndex(SlotIndex) && ScorePopupTextBlocks[SlotIndex].IsValid())
		{
			ScorePopupTextBlocks[SlotIndex]->SetText(BuildSlotScorePopupText(MoleSlot));
			ScorePopupTextBlocks[SlotIndex]->SetVisibility(bScorePopupVisible ? EVisibility::HitTestInvisible : EVisibility::Hidden);
			ScorePopupTextBlocks[SlotIndex]->SetColorAndOpacity(MoleSlot.ScorePopupValue < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Text);
		}

		const bool bShowHitHammer = HammerTimer > KINDA_SMALL_NUMBER && HammerCellIndex == SlotIndex;
		const bool bShowIdleHammer = !bShowHitHammer && GameState == EWhackAMoleGameState::Playing && HoveredCellIndex == SlotIndex;
		if (HammerImages.IsValidIndex(SlotIndex) && HammerImages[SlotIndex].IsValid())
		{
			HammerImages[SlotIndex]->SetImage(FindOrLoadSpriteBrush(bShowHitHammer
				? MakeSpriteName(TEXT("hammer_hit_8bit"))
				: MakeSpriteName(TEXT("hammer_idle_8bit"))));
			HammerImages[SlotIndex]->SetVisibility(bShowHitHammer || bShowIdleHammer ? EVisibility::HitTestInvisible : EVisibility::Hidden);
			HammerImages[SlotIndex]->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, bShowHitHammer ? 1.f : 0.88f)));
		}
	}
}

void UT66WhackAMoleArcadeWidget::RefreshHud()
{
	if (TimerTextBlock.IsValid())
	{
		TimerTextBlock->SetText(FText::AsNumber(FMath::CeilToInt(FMath::Max(0.f, RemainingSeconds))));
	}

	if (ScoreTextBlock.IsValid())
	{
		ScoreTextBlock->SetText(FText::AsNumber(Score));
		ScoreTextBlock->SetColorAndOpacity(Score < 0 ? FT66Style::Tokens::Danger : FT66Style::Tokens::Accent);
	}

	if (ComboTextBlock.IsValid())
	{
		ComboTextBlock->SetText(FText::Format(NSLOCTEXT("T66.Arcade", "WhackComboValue", "{0}  x{1}"), FText::AsNumber(Combo), FText::AsNumber(GetComboMultiplier())));
	}

	if (MultiplierTextBlock.IsValid())
	{
		MultiplierTextBlock->SetText(FText::Format(NSLOCTEXT("T66.Arcade", "WhackMultiplierValue", "MULTIPLIER x{0}"), FText::AsNumber(GetComboMultiplier())));
	}

	if (BestComboTextBlock.IsValid())
	{
		BestComboTextBlock->SetText(FText::Format(NSLOCTEXT("T66.Arcade", "WhackBestComboValue", "BEST {0}"), FText::AsNumber(BestCombo)));
	}

	for (int32 LifeIndex = 0; LifeIndex < LifeImages.Num(); ++LifeIndex)
	{
		if (LifeImages[LifeIndex].IsValid())
		{
			LifeImages[LifeIndex]->SetImage(FindOrLoadSpriteBrush(LifeIndex < Lives
				? MakeSpriteName(TEXT("hud_heart_full_8bit"))
				: MakeSpriteName(TEXT("hud_heart_empty_8bit"))));
		}
	}

	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(BuildStatusText());
		StatusTextBlock->SetColorAndOpacity(GameState == EWhackAMoleGameState::Finished
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
	return FMath::Max(4.f, ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeRoundSeconds, 30.f));
}

float UT66WhackAMoleArcadeWidget::ResolveStartSpawnIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeStartInterval, 1.0f), 0.2f, 3.f);
}

float UT66WhackAMoleArcadeWidget::ResolveEndSpawnIntervalSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::ArcadeEndInterval, 0.35f), 0.12f, 3.f);
}

float UT66WhackAMoleArcadeWidget::ResolveCurrentSpawnIntervalSeconds() const
{
	return FMath::Lerp(StartSpawnIntervalSeconds, EndSpawnIntervalSeconds, ResolveDifficultyAlpha());
}

float UT66WhackAMoleArcadeWidget::ResolveVisibleDurationSeconds() const
{
	return FMath::Lerp(VisibleStartSeconds, VisibleEndSeconds, ResolveDifficultyAlpha());
}

float UT66WhackAMoleArcadeWidget::ResolveVisibleStartSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::WhackVisibleStartSeconds, 1.2f), 0.2f, 3.f);
}

float UT66WhackAMoleArcadeWidget::ResolveVisibleEndSeconds() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::WhackVisibleEndSeconds, 0.45f), 0.12f, 3.f);
}

float UT66WhackAMoleArcadeWidget::ResolveDifficultyAlpha() const
{
	return RoundDurationSeconds > KINDA_SMALL_NUMBER
		? FMath::Clamp(1.f - (RemainingSeconds / RoundDurationSeconds), 0.f, 1.f)
		: 1.f;
}

float UT66WhackAMoleArcadeWidget::ResolveGoldenChance() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::WhackGoldenChance, 0.05f), 0.f, 0.35f);
}

float UT66WhackAMoleArcadeWidget::ResolveBombChance() const
{
	const float StartChance = FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::WhackBombChanceStart, 0.05f), 0.f, 0.35f);
	const float EndChance = FMath::Clamp(ArcadeData.Modifiers.GetFloat(T66ArcadeModifierKeys::WhackBombChanceEnd, 0.12f), 0.f, 0.35f);
	return FMath::Lerp(StartChance, EndChance, ResolveDifficultyAlpha());
}

int32 UT66WhackAMoleArcadeWidget::ResolveTargetScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeTargetScore, 2500));
}

int32 UT66WhackAMoleArcadeWidget::ResolveScorePerHit() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::ArcadeScorePerHit, 100));
}

int32 UT66WhackAMoleArcadeWidget::ResolveGoldenScore() const
{
	return FMath::Max(1, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::WhackGoldenScore, 500));
}

int32 UT66WhackAMoleArcadeWidget::ResolveBombPenalty() const
{
	return FMath::Max(0, ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::WhackBombPenalty, 250));
}

int32 UT66WhackAMoleArcadeWidget::ResolveStartingLives() const
{
	return FMath::Clamp(ArcadeData.Modifiers.GetInt(T66ArcadeModifierKeys::WhackStartingLives, 3), 1, 5);
}

int32 UT66WhackAMoleArcadeWidget::ResolveMaxActiveMoles() const
{
	const float DifficultyAlpha = ResolveDifficultyAlpha();
	if (DifficultyAlpha < 0.30f)
	{
		return 1;
	}

	return DifficultyAlpha < 0.70f ? 2 : 3;
}

int32 UT66WhackAMoleArcadeWidget::GetComboMultiplier() const
{
	return FMath::Max(1, 1 + (Combo / 5));
}

int32 UT66WhackAMoleArcadeWidget::CountActiveMoles() const
{
	int32 Count = 0;
	for (const FWhackAMoleSlotRuntime& MoleSlot : MoleSlots)
	{
		if (MoleSlot.CurrentState == EWhackAMoleSlotState::Rising || MoleSlot.CurrentState == EWhackAMoleSlotState::Visible)
		{
			++Count;
		}
	}
	return Count;
}

FText UT66WhackAMoleArcadeWidget::BuildRewardSummaryText() const
{
	if (GameState == EWhackAMoleGameState::Finished)
	{
		return FText::Format(
			NSLOCTEXT("T66.Arcade", "WhackRewardFinal", "Final score {0} / target {1}"),
			FText::AsNumber(Score),
			FText::AsNumber(TargetScore));
	}

	return FText::GetEmpty();
}

FText UT66WhackAMoleArcadeWidget::BuildStatusText() const
{
	if (GameState == EWhackAMoleGameState::Finished)
	{
		if (Lives <= 0)
		{
			return NSLOCTEXT("T66.Arcade", "WhackStatusOutOfLives", "OUT OF LIVES");
		}

		return bRoundSucceeded
			? NSLOCTEXT("T66.Arcade", "WhackStatusScoreLocked", "SCORE LOCKED")
			: NSLOCTEXT("T66.Arcade", "WhackStatusNoScore", "NO SCORE");
	}

	if (Combo >= 10)
	{
		return NSLOCTEXT("T66.Arcade", "WhackStatusHotStreak", "HOT STREAK");
	}

	if (CountActiveMoles() >= ResolveMaxActiveMoles())
	{
		return NSLOCTEXT("T66.Arcade", "WhackStatusActive", "WHACK");
	}

	return NSLOCTEXT("T66.Arcade", "WhackStatusReady", "READY");
}

FText UT66WhackAMoleArcadeWidget::BuildPrimaryActionText() const
{
	return GameState == EWhackAMoleGameState::Finished
		? NSLOCTEXT("T66.Arcade", "WhackContinueButton", "Continue")
		: NSLOCTEXT("T66.Arcade", "WhackAbortButton", "Abort");
}

FText UT66WhackAMoleArcadeWidget::BuildSlotScorePopupText(const FWhackAMoleSlotRuntime& MoleSlot) const
{
	if (MoleSlot.ScorePopupValue == 0)
	{
		return FText::GetEmpty();
	}

	return FText::FromString(FString::Printf(TEXT("%+d"), MoleSlot.ScorePopupValue));
}

FName UT66WhackAMoleArcadeWidget::ResolveMoleSpriteName(const FWhackAMoleSlotRuntime& MoleSlot) const
{
	if (MoleSlot.CurrentState == EWhackAMoleSlotState::Rising)
	{
		const bool bSecondFrame = MoleSlot.StateDuration > KINDA_SMALL_NUMBER && MoleSlot.StateTimer >= MoleSlot.StateDuration * 0.5f;
		return MakeSpriteName(bSecondFrame ? TEXT("mole_rise_02_8bit") : TEXT("mole_rise_01_8bit"));
	}

	if (MoleSlot.CurrentState == EWhackAMoleSlotState::Retreating)
	{
		return MakeSpriteName(TEXT("mole_retreat_8bit"));
	}

	if (MoleSlot.CurrentState == EWhackAMoleSlotState::Hit)
	{
		if (MoleSlot.MoleType == EWhackAMoleType::Golden)
		{
			return MakeSpriteName(TEXT("golden_mole_hit_8bit"));
		}
		if (MoleSlot.MoleType == EWhackAMoleType::Bomb)
		{
			return MakeSpriteName(TEXT("bomb_mole_hit_8bit"));
		}
		return MakeSpriteName(TEXT("mole_hit_8bit"));
	}

	if (MoleSlot.MoleType == EWhackAMoleType::Golden)
	{
		return MakeSpriteName(TEXT("golden_mole_up_8bit"));
	}
	if (MoleSlot.MoleType == EWhackAMoleType::Bomb)
	{
		return MakeSpriteName(TEXT("bomb_mole_up_8bit"));
	}
	return MakeSpriteName(TEXT("mole_up_8bit"));
}

FName UT66WhackAMoleArcadeWidget::ResolveEffectSpriteName(const FWhackAMoleSlotRuntime& MoleSlot) const
{
	return MoleSlot.EffectSpriteName.IsNone()
		? MakeSpriteName(TEXT("fx_hit_starburst_8bit"))
		: MoleSlot.EffectSpriteName;
}

const FSlateBrush* UT66WhackAMoleArcadeWidget::FindOrLoadSpriteBrush(const FName SpriteName)
{
	if (FSlateBrush* ExistingBrush = SpriteBrushes.Find(SpriteName))
	{
		return ExistingBrush;
	}

	const FString SpriteFileName = FString::Printf(TEXT("%s.png"), *SpriteName.ToString());
	const FString RelativePath = FString::Printf(TEXT("SourceAssets/Arcade/WhackAMole/%s"), *SpriteFileName);
	const FString DebugLabel = FString::Printf(TEXT("WhackAMole.%s"), *SpriteName.ToString());
	const TArray<FString> CandidatePaths = T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath);
	for (const FString& CandidatePath : CandidatePaths)
	{
		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(CandidatePath, TextureFilter::TF_Nearest, true, *DebugLabel))
		{
			SpriteTextures.Add(SpriteName, Texture);
			FSlateBrush& NewBrush = SpriteBrushes.Add(SpriteName);
			NewBrush.SetResourceObject(Texture);
			NewBrush.DrawAs = ESlateBrushDrawType::Image;
			NewBrush.Tiling = ESlateBrushTileType::NoTile;
			NewBrush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
			return &NewBrush;
		}
	}

	return FCoreStyle::Get().GetBrush("NoBrush");
}

TSharedRef<SWidget> UT66WhackAMoleArcadeWidget::BuildSpriteImage(const FName SpriteName, const FVector2D& ImageSize, const float Opacity)
{
	return SNew(SBox)
		.WidthOverride(ImageSize.X)
		.HeightOverride(ImageSize.Y)
		[
			SNew(SImage)
			.Image(FindOrLoadSpriteBrush(SpriteName))
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, FMath::Clamp(Opacity, 0.f, 1.f)))
		];
}

TSharedRef<SWidget> UT66WhackAMoleArcadeWidget::BuildHudPanel(const FName IconSprite, const FText& Label, const TSharedRef<SWidget>& ValueWidget)
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			BuildSpriteImage(MakeSpriteName(TEXT("hud_panel_8bit")), FVector2D(256.f, 72.f))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(18.f, 10.f, 16.f, 10.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 12.f, 0.f)
			[
				BuildSpriteImage(IconSprite, FVector2D(42.f, 42.f))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontBold(13))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					ValueWidget
				]
			]
		];
}

TSharedRef<SWidget> UT66WhackAMoleArcadeWidget::BuildLivesWidget()
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	for (int32 LifeIndex = 0; LifeIndex < 3; ++LifeIndex)
	{
		TSharedPtr<SImage> LifeImage;
		Row->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, 3.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(25.f)
				.HeightOverride(23.f)
				[
					SAssignNew(LifeImage, SImage)
					.Image(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("hud_heart_full_8bit"))))
				]
			];
		LifeImages.Add(LifeImage);
	}

	return Row;
}

TSharedRef<SWidget> UT66WhackAMoleArcadeWidget::BuildSlotWidget(const int32 SlotIndex)
{
	TSharedPtr<SImage> HoleImage;
	TSharedPtr<SImage> MoleImage;
	TSharedPtr<SImage> HoverImage;
	TSharedPtr<SImage> EffectImage;
	TSharedPtr<SImage> ScorePopupImage;
	TSharedPtr<SImage> HammerImage;
	TSharedPtr<STextBlock> ScorePopupText;
	TSharedPtr<SButton> SlotButton;

	TSharedRef<SWidget> SlotContent =
		SNew(SBox)
		.WidthOverride(GWhackSlotWidth)
		.HeightOverride(GWhackSlotHeight)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBox)
				.WidthOverride(160.f)
				.HeightOverride(128.f)
				[
					SAssignNew(HoleImage, SImage)
					.Image(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("hole_idle_8bit"))))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBox)
				.WidthOverride(160.f)
				.HeightOverride(128.f)
				[
					SAssignNew(HoverImage, SImage)
					.Image(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("hole_hover_highlight_8bit"))))
					.Visibility(EVisibility::Hidden)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Bottom)
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBox)
				.WidthOverride(160.f)
				.HeightOverride(160.f)
				[
					SAssignNew(MoleImage, SImage)
					.Image(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("mole_up_8bit"))))
					.Visibility(EVisibility::Hidden)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(160.f)
				.HeightOverride(160.f)
				[
					SAssignNew(EffectImage, SImage)
					.Image(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("fx_hit_starburst_8bit"))))
					.Visibility(EVisibility::Hidden)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.Padding(0.f, -26.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(160.f)
				.HeightOverride(90.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(ScorePopupImage, SImage)
						.Image(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("fx_score_popup_100_8bit"))))
						.Visibility(EVisibility::Hidden)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(ScorePopupText, STextBlock)
						.Text(FText::GetEmpty())
						.Font(FT66Style::Tokens::FontBold(18))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.Visibility(EVisibility::Hidden)
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.Padding(0.f, -4.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(130.f)
				.HeightOverride(130.f)
				[
					SAssignNew(HammerImage, SImage)
					.Image(FindOrLoadSpriteBrush(MakeSpriteName(TEXT("hammer_idle_8bit"))))
					.Visibility(EVisibility::Hidden)
				]
			]
		];

	TSharedRef<SWidget> Button = FT66Style::MakeBareButton(
		FT66BareButtonParams(
			FOnClicked::CreateUObject(this, &UT66WhackAMoleArcadeWidget::HandleCellClicked, SlotIndex),
			SlotContent)
		.SetColor(FLinearColor::Transparent)
		.SetPadding(FMargin(0.f))
		.SetOnHovered(FSimpleDelegate::CreateUObject(this, &UT66WhackAMoleArcadeWidget::HandleCellHovered, SlotIndex))
		.SetOnUnhovered(FSimpleDelegate::CreateUObject(this, &UT66WhackAMoleArcadeWidget::HandleCellUnhovered, SlotIndex))
		.SetDebounceClick(false),
		&SlotButton);

	if (SlotButtons.IsValidIndex(SlotIndex))
	{
		SlotButtons[SlotIndex] = SlotButton;
		HoleImages[SlotIndex] = HoleImage;
		MoleImages[SlotIndex] = MoleImage;
		HoverImages[SlotIndex] = HoverImage;
		EffectImages[SlotIndex] = EffectImage;
		ScorePopupImages[SlotIndex] = ScorePopupImage;
		HammerImages[SlotIndex] = HammerImage;
		ScorePopupTextBlocks[SlotIndex] = ScorePopupText;
	}

	return Button;
}

void UT66WhackAMoleArcadeWidget::ShowHammerAtSlot(const int32 SlotIndex)
{
	HammerCellIndex = SlotIndex;
	HammerTimer = GWhackHammerSeconds;
}

void UT66WhackAMoleArcadeWidget::ApplyEmptyClick()
{
	if (Combo > 0)
	{
		Combo = 0;
	}
	++Misses;
	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Miss")));
}

FReply UT66WhackAMoleArcadeWidget::HandleCellClicked(const int32 CellIndex)
{
	ShowHammerAtSlot(CellIndex);

	if (GameState != EWhackAMoleGameState::Playing || !MoleSlots.IsValidIndex(CellIndex))
	{
		RefreshBoardVisuals();
		return FReply::Handled();
	}

	FWhackAMoleSlotRuntime& MoleSlot = MoleSlots[CellIndex];
	if (MoleSlot.CurrentState != EWhackAMoleSlotState::Visible || !MoleSlot.bCanBeHit)
	{
		ApplyEmptyClick();
		RefreshBoardVisuals();
		RefreshHud();
		return FReply::Handled();
	}

	MoleSlot.bCanBeHit = false;
	MoleSlot.CurrentState = EWhackAMoleSlotState::Hit;
	MoleSlot.StateTimer = 0.f;
	MoleSlot.StateDuration = GWhackHitSeconds;
	MoleSlot.EffectTimer = GWhackEffectSeconds;
	MoleSlot.ScorePopupTimer = GWhackPopupSeconds;
	MoleSlot.EffectSpriteName = MakeSpriteName(TEXT("fx_hit_starburst_8bit"));

	int32 ScoreDelta = 0;
	if (MoleSlot.MoleType == EWhackAMoleType::Bomb)
	{
		ScoreDelta = -BombPenalty;
		Score += ScoreDelta;
		Combo = 0;
		--Lives;
		MoleSlot.ScorePopupSpriteName = MakeSpriteName(TEXT("fx_dirt_puff_8bit"));
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Miss")));
	}
	else
	{
		const int32 Multiplier = GetComboMultiplier();
		const int32 BaseScore = MoleSlot.MoleType == EWhackAMoleType::Golden ? GoldenScore : ScorePerHit;
		ScoreDelta = BaseScore * Multiplier;
		Score += ScoreDelta;
		++Hits;
		++Combo;
		BestCombo = FMath::Max(BestCombo, Combo);
		MoleSlot.ScorePopupSpriteName = MoleSlot.MoleType == EWhackAMoleType::Golden
			? MakeSpriteName(TEXT("fx_perfect_hit_8bit"))
			: (Multiplier > 1 ? MakeSpriteName(TEXT("fx_combo_popup_8bit")) : MakeSpriteName(TEXT("fx_score_popup_100_8bit")));
		UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("Arcade.Whack.Hit")));
	}

	MoleSlot.ScorePopupValue = ScoreDelta;
	if (Lives <= 0)
	{
		CompleteRound();
	}

	RefreshBoardVisuals();
	RefreshHud();
	return FReply::Handled();
}

void UT66WhackAMoleArcadeWidget::HandleCellHovered(const int32 CellIndex)
{
	HoveredCellIndex = CellIndex;
	RefreshBoardVisuals();
}

void UT66WhackAMoleArcadeWidget::HandleCellUnhovered(const int32 CellIndex)
{
	if (HoveredCellIndex == CellIndex)
	{
		HoveredCellIndex = INDEX_NONE;
	}
	RefreshBoardVisuals();
}

FReply UT66WhackAMoleArcadeWidget::HandlePrimaryActionClicked()
{
	if (GameState != EWhackAMoleGameState::Finished)
	{
		GameState = EWhackAMoleGameState::Finished;
		bRoundSucceeded = false;
		ClearActiveTimers();
		StartCloseSequence(false, 0);
		return FReply::Handled();
	}

	StartCloseSequence(bRoundSucceeded, Score);
	return FReply::Handled();
}
