// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ArcadePopupWidget.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "T66GoldMinerArcadeWidget.generated.h"

class SBorder;
class SCanvas;
class STextBlock;
class UTexture2D;

UCLASS(Blueprintable)
class T66_API UT66GoldMinerArcadeWidget : public UT66ArcadePopupWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	enum class EGoldMinerTargetType : uint8
	{
		Empty,
		Rock,
		SmallGold,
		LargeGold,
		Gem,
	};

	struct FGoldMinerLane
	{
		EGoldMinerTargetType TargetType = EGoldMinerTargetType::Empty;
		int32 ScoreValue = 0;
		int32 Depth = 1;
	};

	struct FGoldMinerSpriteBrush
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	static constexpr int32 LaneCount = 7;
	static constexpr int32 HookStepCount = 5;

	void StartRound();
	void ClearActiveTimers();
	void GenerateTargets();
	void EnsureAtLeastOneValuableTarget();
	void HandleRoundTick();
	void HandleAutoClose();
	void CompleteReel();
	void CompleteRound();
	void RefreshBoardVisuals();
	void RefreshHud();
	float ResolveRoundDurationSeconds() const;
	float ResolveSwingSpeed() const;
	float ResolveReelBaseSeconds() const;
	int32 ResolveTargetScore() const;
	int32 ResolveSmallGoldScore() const;
	int32 ResolveLargeGoldScore() const;
	int32 ResolveGemScore() const;
	int32 ResolveRockScore() const;
	int32 ResolveMissPenalty() const;
	int32 ResolveActiveLaneIndex() const;
	float GetReelProgress01() const;
	int32 CountRemainingValuableTargets() const;
	bool IsValuableTarget(EGoldMinerTargetType TargetType) const;
	FText BuildStatusText() const;
	FText BuildRewardSummaryText() const;
	FText BuildPrimaryActionText() const;
	FText BuildDropActionText() const;
	FText GetTargetText(int32 LaneIndex) const;
	FLinearColor GetLaneColor(int32 LaneIndex) const;
	FLinearColor GetTargetColor(int32 LaneIndex) const;
	FLinearColor GetHookStepColor(int32 LaneIndex, int32 StepIndex) const;
	FReply HandleDropClicked();
	FReply HandlePrimaryActionClicked();
	void LoadArtworkBrush();
	const FSlateBrush* FindOrLoadSpriteBrush(const FString& RelativePath);
	const TCHAR* GetTargetSpritePath(EGoldMinerTargetType TargetType) const;
	float ResolveLaneCenterX(int32 LaneIndex) const;
	float ResolveTargetY(const FGoldMinerLane& Lane) const;
	FVector2D ResolveVisualClawCenter() const;
	void AddSpriteToCanvas(const FString& RelativePath, const FVector2D& Center, float Size, const FLinearColor& Tint);
	void AddRopeToCanvas(const FVector2D& ClawCenter) const;
	void RefreshSpriteLayer();
	TSharedRef<SWidget> BuildArtworkLayer(float Opacity) const;

	TArray<FGoldMinerLane> Lanes;
	TArray<TSharedPtr<SBorder>> LaneBorders;
	TArray<TSharedPtr<SBorder>> TargetBorders;
	TArray<TSharedPtr<STextBlock>> TargetTextBlocks;
	TArray<TSharedPtr<SBorder>> HookStepBorders;
	TSharedPtr<STextBlock> TimerTextBlock;
	TSharedPtr<STextBlock> ScoreTextBlock;
	TSharedPtr<STextBlock> CargoTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
	TSharedPtr<STextBlock> RewardTextBlock;
	TSharedPtr<STextBlock> DropActionTextBlock;
	TSharedPtr<STextBlock> PrimaryActionTextBlock;
	TSharedPtr<SCanvas> SpriteCanvas;

	FTimerHandle RoundTickHandle;
	FTimerHandle AutoCloseHandle;
	TStrongObjectPtr<UTexture2D> ArtworkTexture;
	FSlateBrush ArtworkBrush;
	TMap<FString, FGoldMinerSpriteBrush> SpriteBrushes;

	float RemainingSeconds = 0.f;
	float RoundDurationSeconds = 12.f;
	float SwingSpeed = 3.2f;
	float ReelBaseSeconds = 0.24f;
	double RoundStartTimeSeconds = 0.0;
	double RoundEndTimeSeconds = 0.0;
	double LastDropTimeSeconds = 0.0;
	double ReelEndTimeSeconds = 0.0;
	int32 TargetScore = 140;
	int32 SmallGoldScore = 12;
	int32 LargeGoldScore = 28;
	int32 GemScore = 45;
	int32 RockScore = 3;
	int32 MissPenalty = 5;
	int32 Score = 0;
	int32 CargoCount = 0;
	int32 ReelLaneIndex = INDEX_NONE;
	bool bReeling = false;
	bool bRoundEnded = false;
	bool bRoundSucceeded = false;
};
