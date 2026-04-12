// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66SnakeGameModal.generated.h"

class SBorder;
class STextBlock;
struct FKeyEvent;

UCLASS(Blueprintable)
class T66_API UT66SnakeGameModal : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66SnakeGameModal(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	enum class ESnakeDirection : uint8
	{
		Up,
		Down,
		Left,
		Right
	};

	static constexpr int32 BoardWidth = 16;
	static constexpr int32 BoardHeight = 16;

	void ResetGame();
	void StepGame();
	void QueueDirection(ESnakeDirection NewDirection);
	void SpawnFood();
	void HandleCrash();
	void RefreshBoardVisuals();
	void RefreshHud();
	FLinearColor GetCellColor(const FIntPoint& Cell) const;
	bool IsCellOccupiedBySnake(const FIntPoint& Cell, bool bExcludeTail) const;
	int32 GetCellIndex(int32 X, int32 Y) const;

	FReply HandleCloseClicked();

	TArray<FIntPoint> SnakeSegments;
	FIntPoint FoodCell = FIntPoint(INDEX_NONE, INDEX_NONE);
	ESnakeDirection CurrentDirection = ESnakeDirection::Right;
	ESnakeDirection PendingDirection = ESnakeDirection::Right;
	bool bPendingDirectionQueued = false;
	bool bGameOver = false;
	bool bWonRound = false;
	float StepAccumulator = 0.f;
	float StepInterval = 0.17f;
	float RestartCountdown = 0.f;
	int32 Score = 0;
	TArray<TSharedPtr<SBorder>> CellBorders;
	TSharedPtr<STextBlock> ScoreTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
};
