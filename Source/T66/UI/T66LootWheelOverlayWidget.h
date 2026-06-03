// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/Animation/T66AnimationSequence.h"
#include "UI/Animation/T66AnimationMarkerDispatch.h"
#include "UI/T66LootWheelPresentationTypes.h"
#include "T66LootWheelOverlayWidget.generated.h"

class FActiveTimerHandle;
class SBorder;
class SBox;
class STextBlock;
class UT66GameplayHUDWidget;

/** Radial LootWheel reward presentation: target-owned UI spin, landing, commit, handoff. */
UCLASS(Blueprintable)
class T66_API UT66LootWheelOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UT66LootWheelOverlayWidget(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	void SetPresentationHost(UT66GameplayHUDWidget* InPresentationHost);
	void Configure(FT66LootWheelPresentationParams InParams);
	void RequestSkip();

private:
	enum class ELootWheelAnimationPhase : uint8
	{
		Idle,
		Anticipation,
		Spin,
		Deceleration,
		Reveal,
		Hold,
		Dismiss,
		Complete,
	};

	struct FLootWheelSegmentEntry
	{
		ET66LootWheelRewardVisualType Type = ET66LootWheelRewardVisualType::Gold;
		FText Label = FText::GetEmpty();
		FLinearColor Color = FLinearColor::White;
		TSharedPtr<SBorder> SegmentBorder;
		TSharedPtr<STextBlock> SegmentText;
	};

	void BuildSegments();
	int32 ResolveWinningSegmentIndex() const;
	void ResetVisualState();
	void BuildFullAnimationSequence();
	void BuildSkipToLandingSequence();
	void BuildDismissSequence();
	FT66AnimationTimeline MakeTimeline(FName TimelineName, float Duration, const FT66AnimationCurveSpec& Curve, TFunction<void(float)> ProgressCallback) const;
	void RegisterMarkerHandlers();
	void HandleMarkerEvents(const TArray<FT66AnimationMarkerEvent>& MarkerEvents);
	void StartAnimationActiveTimer(const TSharedRef<SWidget>& OwningWidget);
	void StopAnimationActiveTimer();
	EActiveTimerReturnType HandleAnimationActiveTimer(double CurrentTime, float DeltaTime);
	void TickAnimation(float DeltaSeconds);
	void ApplyWheelRotation(float NewRotationDegrees);
	void ApplyVisuals();
	void CommitRewardIfNeeded();
	void SignalAnimationComplete();
	bool IsAnimationTerminal() const;
	FText BuildResultTitleText() const;
	FText BuildResultDetailText() const;

	FT66LootWheelPresentationParams Params;
	TArray<FLootWheelSegmentEntry> Segments;
	FT66AnimationSequence AnimationSequence;
	FT66AnimationMarkerDispatcher MarkerDispatcher;
	int32 WinningSegmentIndex = INDEX_NONE;
	float SegmentAngleDegrees = 15.f;
	float WheelRotationDegrees = 0.f;
	float FinalRotationDegrees = 0.f;
	float RevealAlpha = 0.f;
	float RootOpacity = 1.f;
	float ActiveSequenceDuration = 0.f;
	bool bAnimationActive = false;
	bool bCompletionSignaled = false;
	bool bCommitAttempted = false;
	bool bSkipSequenceActive = false;
	ELootWheelAnimationPhase ActivePhase = ELootWheelAnimationPhase::Idle;

	TWeakObjectPtr<UT66GameplayHUDWidget> PresentationHost;
	TWeakPtr<SWidget> AnimationActiveTimerWidget;
	TSharedPtr<FActiveTimerHandle> AnimationActiveTimerHandle;

	TSharedPtr<SBox> RootAnimationBox;
	TSharedPtr<SBox> WheelRotationBox;
	TSharedPtr<SBorder> ResultPanelBorder;
	TSharedPtr<STextBlock> ResultTitleText;
	TSharedPtr<STextBlock> ResultDetailText;
	TSharedPtr<STextBlock> SkipText;

	static constexpr float WheelPanelSize = 620.f;
	static constexpr float WheelDialSize = 500.f;
	static constexpr int32 WheelSegmentCount = 24;
	static constexpr float SegmentLabelWidth = 58.f;
	static constexpr float SegmentLabelHeight = 16.f;
	static constexpr float SegmentLabelRadius = 163.f;
	static constexpr float AnticipationDuration = 0.18f;
	static constexpr float SpinDuration = 1.45f;
	static constexpr float DecelerationDuration = 1.65f;
	static constexpr float RevealDuration = 0.42f;
	static constexpr float HoldDuration = 0.86f;
	static constexpr float DismissDuration = 0.24f;
	static constexpr float SkipLandingDuration = 0.55f;
};
