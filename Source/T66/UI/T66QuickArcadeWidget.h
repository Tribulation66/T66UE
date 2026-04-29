// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ArcadePopupWidget.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "T66QuickArcadeWidget.generated.h"

class SBorder;
class SCanvas;
class SBox;
class STextBlock;
class UTexture2D;

UCLASS(Blueprintable)
class T66_API UT66QuickArcadeWidget : public UT66ArcadePopupWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	enum class EQuickArcadeMode : uint8
	{
		HitTarget,
		AvoidHazard,
		CenterStack,
		MeterStop,
		Sequence,
	};

	struct FQuickArcadeSpec
	{
		EQuickArcadeMode Mode = EQuickArcadeMode::HitTarget;
		FText FallbackTitle;
		FText StatusActive;
		FText ActionText;
		FString ArtworkPath;
		FString PrimarySpritePath;
		FString HazardSpritePath;
		FString MarkerSpritePath;
		int32 CellCount = 9;
		int32 Columns = 3;
		int32 SequenceLength = 0;
	};

	struct FQuickArcadeSpriteBrush
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	void StartRound();
	void ClearActiveTimers();
	void RollPrompt();
	void HandleRoundTick();
	void HandleAutoClose();
	void CompleteRound();
	void RefreshBoardVisuals();
	void RefreshHud();
	FQuickArcadeSpec ResolveSpec() const;
	float ResolveCellSize() const;
	FVector2D ResolveBoardSize() const;
	FVector2D ResolveCellCenter(int32 CellIndex) const;
	FVector2D ResolveAnimatedCellCenter() const;
	float ResolveRoundDurationSeconds() const;
	float ResolveStartIntervalSeconds() const;
	float ResolveEndIntervalSeconds() const;
	float ResolveCurrentIntervalSeconds() const;
	int32 ResolveTargetScore() const;
	int32 ResolveScorePerHit() const;
	int32 ResolveMissPenalty() const;
	int32 ResolveSweepingCellIndex() const;
	bool IsCellGood(int32 CellIndex) const;
	bool IsCellHazard(int32 CellIndex) const;
	FText BuildStatusText() const;
	FText BuildPrimaryActionText() const;
	FText GetCellText(int32 CellIndex) const;
	FLinearColor GetCellColor(int32 CellIndex) const;
	FLinearColor GetInnerCellColor(int32 CellIndex) const;
	FReply HandleCellClicked(int32 CellIndex);
	FReply HandleActionClicked();
	FReply HandlePrimaryActionClicked();
	void LoadArtworkBrush();
	const FSlateBrush* FindOrLoadSpriteBrush(const FString& RelativePath);
	void AddSpriteToCanvas(const FString& RelativePath, const FVector2D& Center, float Size, const FLinearColor& Tint);
	void RefreshSpriteLayer();
	TSharedRef<SWidget> BuildArtworkLayer(float Opacity) const;

	TArray<TSharedPtr<SBorder>> CellBorders;
	TArray<TSharedPtr<SBorder>> InnerCellBorders;
	TArray<TSharedPtr<STextBlock>> CellTextBlocks;
	TSharedPtr<STextBlock> TimerTextBlock;
	TSharedPtr<STextBlock> ScoreTextBlock;
	TSharedPtr<STextBlock> HitsTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
	TSharedPtr<STextBlock> ActionTextBlock;
	TSharedPtr<STextBlock> PrimaryActionTextBlock;
	TSharedPtr<SBox> ActionButtonBox;
	TSharedPtr<SCanvas> SpriteCanvas;

	FTimerHandle RoundTickHandle;
	FTimerHandle AutoCloseHandle;
	TStrongObjectPtr<UTexture2D> ArtworkTexture;
	FSlateBrush ArtworkBrush;
	TMap<FString, FQuickArcadeSpriteBrush> SpriteBrushes;

	FQuickArcadeSpec Spec;
	TArray<int32> Sequence;
	float RemainingSeconds = 0.f;
	float RoundDurationSeconds = 10.f;
	float StartIntervalSeconds = 0.85f;
	float EndIntervalSeconds = 0.32f;
	double RoundStartTimeSeconds = 0.0;
	double RoundEndTimeSeconds = 0.0;
	double LastPromptTimeSeconds = 0.0;
	int32 TargetScore = 100;
	int32 ScorePerHit = 10;
	int32 MissPenalty = 3;
	int32 Score = 0;
	int32 Hits = 0;
	int32 Misses = 0;
	int32 ActiveCellIndex = INDEX_NONE;
	int32 PreviousCellIndex = INDEX_NONE;
	int32 HazardCellIndex = INDEX_NONE;
	int32 MeterTargetCellIndex = INDEX_NONE;
	int32 SequenceStep = 0;
	bool bRoundEnded = false;
	bool bRoundSucceeded = false;
};
