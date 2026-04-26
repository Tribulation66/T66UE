// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ArcadePopupWidget.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "T66WhackAMoleArcadeWidget.generated.h"

class SBorder;
class STextBlock;
class UTexture2D;

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
	void ScheduleNextSpawn();
	void HandleRoundTick();
	void HandleSpawnTick();
	void HandleAutoClose();
	void SpawnNextMole(bool bCountExpiredAsMiss);
	void CompleteRound();
	void RefreshBoardVisuals();
	void RefreshHud();
	float ResolveRoundDurationSeconds() const;
	float ResolveStartSpawnIntervalSeconds() const;
	float ResolveEndSpawnIntervalSeconds() const;
	float ResolveCurrentSpawnIntervalSeconds() const;
	int32 ResolveTargetScore() const;
	int32 ResolveScorePerHit() const;
	int32 ResolveMissPenalty() const;
	FText BuildRewardSummaryText() const;
	FText BuildStatusText() const;
	FText BuildPrimaryActionText() const;
	FLinearColor GetCellColor(int32 CellIndex) const;
	FLinearColor GetTargetBodyColor(int32 CellIndex) const;
	FLinearColor GetTargetRingColor(int32 CellIndex) const;
	FText GetCellText(int32 CellIndex) const;
	float GetActiveCellAgeAlpha() const;
	FReply HandleCellClicked(int32 CellIndex);
	FReply HandlePrimaryActionClicked();
	void LoadArtworkBrush();
	TSharedRef<SWidget> BuildArtworkLayer(float Opacity) const;

	TArray<TSharedPtr<SBorder>> CellBorders;
	TArray<TSharedPtr<SBorder>> TargetBodies;
	TArray<TSharedPtr<SBorder>> TargetRings;
	TArray<TSharedPtr<STextBlock>> CellTextBlocks;
	TSharedPtr<STextBlock> TimerTextBlock;
	TSharedPtr<STextBlock> HitsTextBlock;
	TSharedPtr<STextBlock> ScoreTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
	TSharedPtr<STextBlock> RewardTextBlock;
	TSharedPtr<STextBlock> PrimaryActionTextBlock;

	FTimerHandle RoundTickHandle;
	FTimerHandle SpawnTickHandle;
	FTimerHandle AutoCloseHandle;
	TStrongObjectPtr<UTexture2D> ArtworkTexture;
	FSlateBrush ArtworkBrush;

	float RemainingSeconds = 0.f;
	float RoundDurationSeconds = 10.f;
	float StartSpawnIntervalSeconds = 0.70f;
	float EndSpawnIntervalSeconds = 0.24f;
	double RoundEndTimeSeconds = 0.0;
	int32 TargetScore = 100;
	int32 ScorePerHit = 10;
	int32 MissPenalty = 2;
	int32 Score = 0;
	int32 Hits = 0;
	int32 Misses = 0;
	int32 ActiveCellIndex = INDEX_NONE;
	double ActiveCellSpawnTimeSeconds = 0.0;
	bool bRoundEnded = false;
	bool bRoundSucceeded = false;
};
