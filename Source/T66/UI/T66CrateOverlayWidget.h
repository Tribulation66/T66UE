// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/Animation/T66AnimationSequence.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngTuningConfig.h"
#include "Styling/SlateBrush.h"
#include "Types/SlateEnums.h"
#include "UI/Animation/T66AnimationMarkerDispatch.h"
#include "T66CrateOverlayWidget.generated.h"

class UT66GameplayHUDWidget;
class FActiveTimerHandle;
class SBorder;
class SBox;
class SImage;
class STextBlock;

/** CS:GO-style crate opening overlay: phased timeline-driven item strip reveal. */
UCLASS(Blueprintable)
class T66_API UT66CrateOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UT66CrateOverlayWidget(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	void SetPresentationHost(UT66GameplayHUDWidget* InPresentationHost);
	void SetSourceCrateRarity(ET66Rarity InSourceCrateRarity);
	void RequestSkip();

private:
	enum class ECrateAnimationPhase : uint8
	{
		Idle,
		Anticipation,
		Spin,
		Deceleration,
		Overshoot,
		Settle,
		Reveal,
		Hold,
		Dismiss,
		Complete,
	};

	struct FT66CrateRewardResult
	{
		FName ItemID;
		ET66Rarity Rarity = ET66Rarity::Black;
		int32 RarityDrawIndex = INDEX_NONE;
		int32 RarityPreDrawSeed = 0;
		bool bRarityHasReplayWeights = false;
		FT66RarityWeights RarityReplayWeights;
		bool bLocked = false;
		bool bCommitAttempted = false;
		bool bCommitted = false;
		bool bAddedToInventory = false;
	};

	struct FCrateItemEntry
	{
		FName ItemID;
		ET66Rarity Rarity;
		FLinearColor Color;
		TSharedPtr<FSlateBrush> IconBrush;
		float LayoutCenterX = 0.f;
		TSharedPtr<SBox> TileBox;
		TSharedPtr<SImage> IconImage;
		TSharedPtr<SBorder> GlowBorder;
		TSharedPtr<SBorder> FrameBorder;
	};

	struct FT66CrateStripPresentationState
	{
		float StripTotalWidth = 0.f;
		float SelectorPositionX = 0.f;
		float CurrentStripOffset = 0.f;
		float FinalStripOffset = 0.f;
		float SpinEndOffset = 0.f;
		float OvershootOffset = 0.f;
		float MaxVisibleOffset = 0.f;
		float RightContentAfterWinner = 0.f;
		int32 SlowdownStartIndex = INDEX_NONE;
		bool bHasRightContentForFinalView = false;
	};

	TArray<FCrateItemEntry> StripItems;
	FT66CrateRewardResult RewardResult;
	FT66CrateStripPresentationState StripState;
	int32 WinnerIndex = 0;
	ET66Rarity SourceCrateRarity = ET66Rarity::Black;
	TWeakObjectPtr<UT66GameplayHUDWidget> PresentationHost;

	TSharedPtr<SBorder> StripContainer;
	TSharedPtr<SBorder> BackdropBorder;
	TSharedPtr<SBorder> SelectorFlareBorder;
	TSharedPtr<SBox> SelectorFlareBox;
	TSharedPtr<SBox> RootAnimationBox;
	TSharedPtr<STextBlock> SkipText;

	FT66AnimationSequence AnimationSequence;
	FT66AnimationMarkerDispatcher MarkerDispatcher;
	TWeakPtr<SWidget> AnimationActiveTimerWidget;
	TSharedPtr<FActiveTimerHandle> AnimationActiveTimerHandle;

	ECrateAnimationPhase ActivePhase = ECrateAnimationPhase::Idle;
	bool bAnimationActive = false;
	bool bCompletionSignaled = false;
	bool bSkipSequenceActive = false;
	float ActiveSequenceDuration = 0.f;
	float SelectorGlowAlpha = 0.f;
	float SelectorTickPulseAlpha = 0.f;
	float WinnerGlowAlpha = 0.f;
	float WinnerLiftOffset = 0.f;
	float NonWinnerOpacity = 1.f;
	float RootOpacity = 1.f;

	static constexpr float AnticipationDuration = 0.22f;
	static constexpr float SpinDuration = 1.65f;
	static constexpr float DecelerationDuration = 2.05f;
	static constexpr float OvershootDuration = 0.22f;
	static constexpr float SettleDuration = 0.34f;
	static constexpr float RevealDuration = 0.55f;
	static constexpr float HoldDuration = 0.55f;
	static constexpr float DismissDuration = 0.24f;
	static constexpr float SkipTravelDuration = 0.62f;
	static constexpr float SkipSettleDuration = 0.20f;

	static constexpr float CratePanelWidth = 820.f;
	static constexpr float CratePanelHeight = 190.f;
	static constexpr float CrateStripViewportWidth = 714.f;

	static constexpr float ItemTileWidth = 96.f;
	static constexpr float ItemTileHeight = 96.f;
	static constexpr float ItemTileGap = 6.f;
	static constexpr float ItemTileStride = ItemTileWidth + ItemTileGap;
	static constexpr float ItemPreviewSize = 74.f;
	static constexpr int32 VisibleTileCount = 7;
	static constexpr int32 StripItemCount = 50;
	static constexpr int32 WinnerPosition = 42;
	static constexpr float FastTravelTileCount = 12.f;
	static constexpr float OvershootTileFraction = 0.18f;
	static constexpr float SlowdownTickTileCount = 6.f;
	static constexpr float SkipTravelTileCount = 3.f;

	void GenerateStrip();
	void BuildFullAnimationSequence();
	void BuildSkipToSettleSequence();
	void BuildSettleRevealSequence(float InSettleDuration);
	void BuildDismissSequence();
	FT66AnimationTimeline MakeTimeline(FName TimelineName, float Duration, const FT66AnimationCurveSpec& Curve, TFunction<void(float)> ProgressCallback) const;
	void AddStripTickMarkers(FT66AnimationTimeline& Timeline, float StartOffset, float EndOffset, bool bWeightedDecel) const;
	void RegisterMarkerHandlers();
	void HandleMarkerEvents(const TArray<FT66AnimationMarkerEvent>& MarkerEvents);
	void StartAnimationActiveTimer(const TSharedRef<SWidget>& OwningWidget);
	void StopAnimationActiveTimer();
	EActiveTimerReturnType HandleAnimationActiveTimer(double CurrentTime, float DeltaTime);
	void TickAnimation(float DeltaSeconds);
	void ApplyStripOffset(float NewOffset);
	void ApplySelectorVisuals();
	void ApplyRevealVisuals();
	void ApplyRootOpacity();
	void TriggerSelectorTickPulse();
	void CommitRewardIfNeeded();
	void SignalAnimationComplete();
	void ResetVisualState();
	void UpdateSkipText();
	bool IsAnimationTerminal() const;
};
