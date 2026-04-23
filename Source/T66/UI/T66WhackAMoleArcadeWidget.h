// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ArcadePopupWidget.h"
#include "T66WhackAMoleArcadeWidget.generated.h"

class SBorder;
class STextBlock;

UCLASS(Blueprintable)
class T66_API UT66WhackAMoleArcadeWidget : public UT66ArcadePopupWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	static constexpr int32 GridWidth = 3;
	static constexpr int32 GridHeight = 3;
	static constexpr int32 CellCount = GridWidth * GridHeight;

	void StartRound();
	void ClearActiveTimers();
	void RestartSpawnTimer();
	void HandleRoundTick();
	void HandleSpawnTick();
	void HandleAutoClose();
	void SpawnNextMole(bool bCountExpiredAsMiss);
	void CompleteRound(bool bSucceeded);
	void RefreshBoardVisuals();
	void RefreshHud();
	float ResolveRoundDurationSeconds() const;
	float ResolveSpawnIntervalSeconds() const;
	int32 ResolveTargetHits() const;
	int32 ResolveSuccessGold() const;
	int32 ResolveFailureGoldPenalty() const;
	FText BuildRewardSummaryText() const;
	FText BuildStatusText() const;
	FText BuildPrimaryActionText() const;
	FLinearColor GetCellColor(int32 CellIndex) const;
	FText GetCellText(int32 CellIndex) const;
	FReply HandleCellClicked(int32 CellIndex);
	FReply HandlePrimaryActionClicked();

	TArray<TSharedPtr<SBorder>> CellBorders;
	TArray<TSharedPtr<STextBlock>> CellTextBlocks;
	TSharedPtr<STextBlock> TimerTextBlock;
	TSharedPtr<STextBlock> HitsTextBlock;
	TSharedPtr<STextBlock> MissesTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
	TSharedPtr<STextBlock> RewardTextBlock;
	TSharedPtr<STextBlock> PrimaryActionTextBlock;

	FTimerHandle RoundTickHandle;
	FTimerHandle SpawnTickHandle;
	FTimerHandle AutoCloseHandle;

	float RemainingSeconds = 0.f;
	float RoundDurationSeconds = 12.f;
	float SpawnIntervalSeconds = 0.65f;
	double RoundEndTimeSeconds = 0.0;
	int32 TargetHits = 10;
	int32 SuccessGold = 150;
	int32 FailureGoldPenalty = 0;
	int32 Hits = 0;
	int32 Misses = 0;
	int32 ActiveCellIndex = INDEX_NONE;
	bool bRoundEnded = false;
	bool bRoundSucceeded = false;
};
