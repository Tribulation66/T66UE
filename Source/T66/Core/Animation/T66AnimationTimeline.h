// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Animation/T66AnimationCurves.h"
#include "Core/Animation/T66AnimationMarker.h"

enum class ET66AnimationPlayState : uint8
{
	Stopped,
	Playing,
	Paused,
	Skipping,
	Completed,
	Cancelled,
	Skipped,
	Interrupted,
};

namespace T66Animation
{
	T66_API bool IsTerminalState(ET66AnimationPlayState State);
}

class T66_API IT66AnimationNode
{
public:
	virtual ~IT66AnimationNode() = default;

	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Cancel() = 0;
	virtual void Finish() = 0;
	virtual void RequestSkip() = 0;
	virtual void Tick(float DeltaSeconds, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents) = 0;
	virtual float GetDuration() const = 0;
	virtual float GetProgress() const = 0;
	virtual ET66AnimationPlayState GetState() const = 0;
};

class T66_API FT66AnimationTimeline final : public IT66AnimationNode
{
public:
	FT66AnimationTimeline() = default;
	explicit FT66AnimationTimeline(FName InTimelineName);
	FT66AnimationTimeline(FT66AnimationTimeline&&) = default;
	FT66AnimationTimeline& operator=(FT66AnimationTimeline&&) = default;
	FT66AnimationTimeline(const FT66AnimationTimeline&) = delete;
	FT66AnimationTimeline& operator=(const FT66AnimationTimeline&) = delete;
	virtual ~FT66AnimationTimeline() override = default;

	void SetTimelineName(FName InTimelineName) { TimelineName = InTimelineName; }
	FName GetTimelineName() const { return TimelineName; }

	void SetDuration(float InDuration);
	virtual float GetDuration() const override { return Duration; }

	void SetCurve(const FT66AnimationCurveSpec& InCurve);
	void AddMarker(const FT66AnimationMarker& Marker);
	void ClearMarkers();

	void SetProgressCallback(TFunction<void(float CurveValue)> InProgressCallback);
	void SetSkipDuration(float InSkipDuration);
	void SetMaxMarkerDispatchesPerTick(int32 InMaxDispatches);

	// Optional tracked value for PositionCrossing markers. If unset, position markers use the curve value.
	void SetPositionValue(float InPositionValue);
	void ClearPositionValue();

	virtual void Play() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void Cancel() override;
	virtual void Finish() override;
	virtual void RequestSkip() override;
	virtual void Tick(float DeltaSeconds, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents) override;

	virtual float GetProgress() const override;
	float GetCurveValue() const;
	virtual ET66AnimationPlayState GetState() const override { return State; }
	float GetElapsedTime() const { return ElapsedTime; }

private:
	struct FPendingMarkerEvent
	{
		FT66AnimationMarkerEvent Event;
		bool bTerminal = false;
	};

	bool ShouldFireMarker(int32 MarkerIndex, const FT66AnimationMarker& Marker, float PreviousProgress, float CurrentProgress, float PreviousCurveValue, float CurrentCurveValue) const;
	void MarkFired(int32 MarkerIndex);
	bool HasMarkerFired(int32 MarkerIndex) const;
	void AppendMarkerEvent(const FT66AnimationMarker& Marker, float PreviousProgress, float CurrentProgress, TArray<FPendingMarkerEvent>& PendingEvents) const;
	void FlushMarkerEvents(const TArray<FPendingMarkerEvent>& PendingEvents, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents) const;
	bool IsTerminalMarker(const FT66AnimationMarker& Marker) const;
	void ResetRuntimeState();
	void CompleteFromTick(bool bFromSkip);

	FName TimelineName = NAME_None;
	float Duration = 1.f;
	float ElapsedTime = 0.f;
	FT66AnimationCurveSpec PrimaryCurve;
	TArray<FT66AnimationMarker> Markers;
	ET66AnimationPlayState State = ET66AnimationPlayState::Stopped;
	ET66AnimationPlayState StateBeforePause = ET66AnimationPlayState::Stopped;
	TSet<int32> FiredMarkerIndices;
	TFunction<void(float CurveValue)> ProgressCallback;

	float SkipDuration = 0.1f;
	float SkipElapsed = 0.f;
	float SkipStartElapsedTime = 0.f;
	int32 MaxMarkerDispatchesPerTick = 16;

	bool bHasPositionValue = false;
	float PreviousPositionValue = 0.f;
	float CurrentPositionValue = 0.f;
};
