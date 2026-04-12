// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SnakeGameModal.h"

#include "UI/Style/T66Style.h"

#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float GSnakeCellSize = 24.f;
	constexpr float GSnakeRestartDelay = 1.15f;
	constexpr float GSnakeMinStepInterval = 0.08f;
	constexpr float GSnakeStepSpeedupPerFood = 0.006f;

	FLinearColor T66SnakeScrimFill()
	{
		return FLinearColor(0.f, 0.f, 0.f, 0.72f);
	}

	FLinearColor T66SnakeShellFill()
	{
		return FLinearColor(0.028f, 0.032f, 0.046f, 0.985f);
	}

	FLinearColor T66SnakeBoardFill()
	{
		return FLinearColor(0.044f, 0.050f, 0.066f, 1.0f);
	}

	FLinearColor T66SnakeInfoFill()
	{
		return FLinearColor(0.060f, 0.067f, 0.086f, 1.0f);
	}

	FLinearColor T66SnakeEmptyCellFill()
	{
		return FLinearColor(0.080f, 0.090f, 0.112f, 1.0f);
	}

	FLinearColor T66SnakeBodyCellFill()
	{
		return FLinearColor(0.231f, 0.566f, 0.298f, 1.0f);
	}

	FLinearColor T66SnakeHeadCellFill()
	{
		return FLinearColor(0.639f, 0.886f, 0.357f, 1.0f);
	}

	FLinearColor T66SnakeCrashCellFill()
	{
		return FLinearColor(0.898f, 0.286f, 0.247f, 1.0f);
	}

	FLinearColor T66SnakeFoodCellFill()
	{
		return FLinearColor(0.949f, 0.541f, 0.204f, 1.0f);
	}
}

UT66SnakeGameModal::UT66SnakeGameModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::SnakeGame;
	bIsModal = true;
	SetIsFocusable(true);
}

void UT66SnakeGameModal::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	ResetGame();
	SetKeyboardFocus();
}

TSharedRef<SWidget> UT66SnakeGameModal::BuildSlateUI()
{
	const FText TitleText = NSLOCTEXT("T66.MiniGames", "SnakeModalTitle", "SNAKE GAME");
	const FText SubtitleText = NSLOCTEXT("T66.MiniGames", "SnakeModalSubtitle", "Arcade side mode");
	const FText ScoreLabelText = NSLOCTEXT("T66.MiniGames", "SnakeScoreLabel", "SCORE");
	const FText ControlsLabelText = NSLOCTEXT("T66.MiniGames", "SnakeControlsLabel", "CONTROLS");
	const FText ControlsBodyText = NSLOCTEXT("T66.MiniGames", "SnakeControlsBody", "Arrow keys or WASD to steer.\nEsc closes the modal.");
	const FText RestartHintText = NSLOCTEXT("T66.MiniGames", "SnakeRestartHint", "Crash or clear the board and the round restarts automatically.");
	const FText CloseText = NSLOCTEXT("T66.Common", "Close", "CLOSE");

	CellBorders.Empty(BoardWidth * BoardHeight);
	CellBorders.SetNum(BoardWidth * BoardHeight);

	TSharedRef<SUniformGridPanel> BoardGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(2.f));

	for (int32 Y = 0; Y < BoardHeight; ++Y)
	{
		for (int32 X = 0; X < BoardWidth; ++X)
		{
			TSharedPtr<SBorder> CellBorder;
			BoardGrid->AddSlot(X, Y)
			[
				SNew(SBox)
				.WidthOverride(GSnakeCellSize)
				.HeightOverride(GSnakeCellSize)
				[
					SAssignNew(CellBorder, SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(T66SnakeEmptyCellFill())
					.Padding(0.f)
				]
			];

			CellBorders[GetCellIndex(X, Y)] = CellBorder;
		}
	}

	const TSharedRef<SWidget> BoardPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(SubtitleText)
				.Font(FT66Style::Tokens::FontRegular(18))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				BoardGrid
			],
			FT66PanelParams(ET66PanelType::Panel2)
				.SetColor(T66SnakeBoardFill())
				.SetPadding(FMargin(16.f)));

	const TSharedRef<SWidget> InfoPanel =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(ScoreLabelText)
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 6.f, 0.f, 0.f)
			[
				SAssignNew(ScoreTextBlock, STextBlock)
				.Text(FText::AsNumber(0))
				.Font(FT66Style::Tokens::FontBold(40))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 22.f, 0.f, 0.f)
			[
				SAssignNew(StatusTextBlock, STextBlock)
				.Text(NSLOCTEXT("T66.MiniGames", "SnakeStatusRunning", "RUNNING"))
				.Font(FT66Style::Tokens::FontBold(22))
				.ColorAndOpacity(FT66Style::Tokens::Success)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 18.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(ControlsLabelText)
				.Font(FT66Style::Tokens::FontBold(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(ControlsBodyText)
				.Font(FT66Style::Tokens::FontRegular(18))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(RestartHintText)
				.Font(FT66Style::Tokens::FontRegular(16))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.VAlign(VAlign_Bottom)
			.Padding(0.f, 22.f, 0.f, 0.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(
						CloseText,
						FOnClicked::CreateUObject(this, &UT66SnakeGameModal::HandleCloseClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(200.f)
					.SetHeight(54.f)
					.SetFontSize(18))
			],
			FT66PanelParams(ET66PanelType::Panel2)
				.SetColor(T66SnakeInfoFill())
				.SetPadding(FMargin(20.f)));

	const TSharedRef<SWidget> ModalSurface =
		FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 20.f)
			[
				SNew(STextBlock)
				.Text(TitleText)
				.Font(FT66Style::Tokens::FontBold(42))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.f, 0.f, 18.f, 0.f)
				[
					BoardPanel
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					InfoPanel
				]
			],
			FT66PanelParams(ET66PanelType::Panel)
				.SetColor(T66SnakeShellFill())
				.SetPadding(FMargin(24.f)));

	RefreshBoardVisuals();
	RefreshHud();

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(T66SnakeScrimFill())
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(24.f))
		[
			SNew(SBox)
			.WidthOverride(860.f)
			[
				ModalSurface
			]
		];
}

FReply UT66SnakeGameModal::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();

	if (PressedKey == EKeys::Escape
		|| PressedKey == EKeys::Gamepad_FaceButton_Right
		|| PressedKey == EKeys::Gamepad_Special_Left)
	{
		HandleCloseClicked();
		return FReply::Handled();
	}

	if (PressedKey == EKeys::Up || PressedKey == EKeys::W)
	{
		QueueDirection(ESnakeDirection::Up);
		return FReply::Handled();
	}
	if (PressedKey == EKeys::Down || PressedKey == EKeys::S)
	{
		QueueDirection(ESnakeDirection::Down);
		return FReply::Handled();
	}
	if (PressedKey == EKeys::Left || PressedKey == EKeys::A)
	{
		QueueDirection(ESnakeDirection::Left);
		return FReply::Handled();
	}
	if (PressedKey == EKeys::Right || PressedKey == EKeys::D)
	{
		QueueDirection(ESnakeDirection::Right);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UT66SnakeGameModal::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bGameOver)
	{
		RestartCountdown -= InDeltaTime;
		if (RestartCountdown <= 0.f)
		{
			ResetGame();
		}
		return;
	}

	StepAccumulator += InDeltaTime;
	while (StepAccumulator >= StepInterval)
	{
		StepAccumulator -= StepInterval;
		StepGame();
		if (bGameOver)
		{
			break;
		}
	}
}

void UT66SnakeGameModal::ResetGame()
{
	SnakeSegments.Reset();
	SnakeSegments.Add(FIntPoint(7, 8));
	SnakeSegments.Add(FIntPoint(6, 8));
	SnakeSegments.Add(FIntPoint(5, 8));
	SnakeSegments.Add(FIntPoint(4, 8));

	CurrentDirection = ESnakeDirection::Right;
	PendingDirection = CurrentDirection;
	bPendingDirectionQueued = false;
	bGameOver = false;
	bWonRound = false;
	StepAccumulator = 0.f;
	StepInterval = 0.17f;
	RestartCountdown = 0.f;
	Score = 0;

	SpawnFood();
	RefreshBoardVisuals();
	RefreshHud();
}

void UT66SnakeGameModal::StepGame()
{
	if (bGameOver || SnakeSegments.Num() == 0)
	{
		return;
	}

	if (bPendingDirectionQueued)
	{
		CurrentDirection = PendingDirection;
		bPendingDirectionQueued = false;
	}

	FIntPoint MoveDelta(1, 0);
	switch (CurrentDirection)
	{
	case ESnakeDirection::Up:
		MoveDelta = FIntPoint(0, -1);
		break;
	case ESnakeDirection::Down:
		MoveDelta = FIntPoint(0, 1);
		break;
	case ESnakeDirection::Left:
		MoveDelta = FIntPoint(-1, 0);
		break;
	case ESnakeDirection::Right:
	default:
		MoveDelta = FIntPoint(1, 0);
		break;
	}

	const FIntPoint NewHead = SnakeSegments[0] + MoveDelta;
	const bool bWillGrow = (NewHead == FoodCell);
	const bool bOutOfBounds = NewHead.X < 0 || NewHead.X >= BoardWidth || NewHead.Y < 0 || NewHead.Y >= BoardHeight;
	if (bOutOfBounds || IsCellOccupiedBySnake(NewHead, !bWillGrow))
	{
		HandleCrash();
		return;
	}

	SnakeSegments.Insert(NewHead, 0);

	if (bWillGrow)
	{
		++Score;
		StepInterval = FMath::Max(GSnakeMinStepInterval, 0.17f - (static_cast<float>(Score) * GSnakeStepSpeedupPerFood));
		SpawnFood();
		if (bGameOver)
		{
			RefreshBoardVisuals();
			RefreshHud();
			return;
		}
	}
	else
	{
		SnakeSegments.Pop();
	}

	RefreshBoardVisuals();
	RefreshHud();
}

void UT66SnakeGameModal::QueueDirection(ESnakeDirection NewDirection)
{
	if (bGameOver)
	{
		return;
	}

	const bool bOppositeDirection =
		(CurrentDirection == ESnakeDirection::Up && NewDirection == ESnakeDirection::Down)
		|| (CurrentDirection == ESnakeDirection::Down && NewDirection == ESnakeDirection::Up)
		|| (CurrentDirection == ESnakeDirection::Left && NewDirection == ESnakeDirection::Right)
		|| (CurrentDirection == ESnakeDirection::Right && NewDirection == ESnakeDirection::Left);

	if (bOppositeDirection && SnakeSegments.Num() > 1)
	{
		return;
	}

	PendingDirection = NewDirection;
	bPendingDirectionQueued = true;
}

void UT66SnakeGameModal::SpawnFood()
{
	TArray<FIntPoint> EmptyCells;
	EmptyCells.Reserve(BoardWidth * BoardHeight);

	for (int32 Y = 0; Y < BoardHeight; ++Y)
	{
		for (int32 X = 0; X < BoardWidth; ++X)
		{
			const FIntPoint Candidate(X, Y);
			if (!IsCellOccupiedBySnake(Candidate, false))
			{
				EmptyCells.Add(Candidate);
			}
		}
	}

	if (EmptyCells.Num() == 0)
	{
		FoodCell = FIntPoint(INDEX_NONE, INDEX_NONE);
		bGameOver = true;
		bWonRound = true;
		RestartCountdown = GSnakeRestartDelay;
		return;
	}

	FoodCell = EmptyCells[FMath::RandRange(0, EmptyCells.Num() - 1)];
}

void UT66SnakeGameModal::HandleCrash()
{
	bGameOver = true;
	bWonRound = false;
	RestartCountdown = GSnakeRestartDelay;
	RefreshBoardVisuals();
	RefreshHud();
}

void UT66SnakeGameModal::RefreshBoardVisuals()
{
	for (int32 Y = 0; Y < BoardHeight; ++Y)
	{
		for (int32 X = 0; X < BoardWidth; ++X)
		{
			const int32 CellIndex = GetCellIndex(X, Y);
			if (CellBorders.IsValidIndex(CellIndex) && CellBorders[CellIndex].IsValid())
			{
				CellBorders[CellIndex]->SetBorderBackgroundColor(GetCellColor(FIntPoint(X, Y)));
			}
		}
	}
}

void UT66SnakeGameModal::RefreshHud()
{
	if (ScoreTextBlock.IsValid())
	{
		ScoreTextBlock->SetText(FText::AsNumber(Score));
	}

	if (StatusTextBlock.IsValid())
	{
		const FText StatusText = bGameOver
			? (bWonRound
				? NSLOCTEXT("T66.MiniGames", "SnakeStatusWin", "BOARD CLEARED. RESTARTING...")
				: NSLOCTEXT("T66.MiniGames", "SnakeStatusCrash", "CRASHED. RESTARTING..."))
			: NSLOCTEXT("T66.MiniGames", "SnakeStatusRunning", "RUNNING");
		const FLinearColor StatusColor = bGameOver
			? (bWonRound ? FT66Style::Tokens::Text : FT66Style::Tokens::Danger)
			: FT66Style::Tokens::Success;

		StatusTextBlock->SetText(StatusText);
		StatusTextBlock->SetColorAndOpacity(StatusColor);
	}
}

FLinearColor UT66SnakeGameModal::GetCellColor(const FIntPoint& Cell) const
{
	if (SnakeSegments.Num() > 0 && Cell == SnakeSegments[0])
	{
		return bGameOver && !bWonRound ? T66SnakeCrashCellFill() : T66SnakeHeadCellFill();
	}

	for (int32 Index = 1; Index < SnakeSegments.Num(); ++Index)
	{
		if (SnakeSegments[Index] == Cell)
		{
			return T66SnakeBodyCellFill();
		}
	}

	if (Cell == FoodCell)
	{
		return T66SnakeFoodCellFill();
	}

	return T66SnakeEmptyCellFill();
}

bool UT66SnakeGameModal::IsCellOccupiedBySnake(const FIntPoint& Cell, bool bExcludeTail) const
{
	const int32 Limit = SnakeSegments.Num() - ((bExcludeTail && SnakeSegments.Num() > 0) ? 1 : 0);
	for (int32 Index = 0; Index < Limit; ++Index)
	{
		if (SnakeSegments[Index] == Cell)
		{
			return true;
		}
	}

	return false;
}

int32 UT66SnakeGameModal::GetCellIndex(int32 X, int32 Y) const
{
	return (Y * BoardWidth) + X;
}

FReply UT66SnakeGameModal::HandleCloseClicked()
{
	CloseModal();
	return FReply::Handled();
}
