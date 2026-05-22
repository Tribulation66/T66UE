// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Animation/T66AnimationTimeline.h"

namespace
{
	bool CrossedThreshold(const float PreviousValue, const float CurrentValue, const float Threshold)
	{
		return (PreviousValue < Threshold && CurrentValue >= Threshold)
			|| (PreviousValue > Threshold && CurrentValue <= Threshold)
			|| (FMath::IsNearlyEqual(CurrentValue, Threshold) && !FMath::IsNearlyEqual(PreviousValue, Threshold));
	}

	bool NameContainsAny(const FName Name, const TArray<FString>& Needles)
	{
		if (Name.IsNone())
		{
			return false;
		}

		const FString Value = Name.ToString();
		for (const FString& Needle : Needles)
		{
			if (Value.Contains(Needle, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}
}

bool T66Animation::IsTerminalState(const ET66AnimationPlayState State)
{
	return State == ET66AnimationPlayState::Completed
		|| State == ET66AnimationPlayState::Cancelled
		|| State == ET66AnimationPlayState::Skipped
		|| State == ET66AnimationPlayState::Interrupted;
}

FT66AnimationTimeline::FT66AnimationTimeline(const FName InTimelineName)
	: TimelineName(InTimelineName)
{
}

void FT66AnimationTimeline::SetDuration(const float InDuration)
{
	Duration = FMath::Max(InDuration, KINDA_SMALL_NUMBER);
	ElapsedTime = FMath::Clamp(ElapsedTime, 0.f, Duration);
}

void FT66AnimationTimeline::SetCurve(const FT66AnimationCurveSpec& InCurve)
{
	PrimaryCurve = InCurve;
}

void FT66AnimationTimeline::AddMarker(const FT66AnimationMarker& Marker)
{
	Markers.Add(Marker);
}

void FT66AnimationTimeline::ClearMarkers()
{
	Markers.Reset();
	FiredMarkerIndices.Reset();
}

void FT66AnimationTimeline::SetProgressCallback(TFunction<void(float CurveValue)> InProgressCallback)
{
	// Callers must capture UObjects and Slate widgets weakly; the core timeline does not own external lifetimes.
	ProgressCallback = MoveTemp(InProgressCallback);
}

void FT66AnimationTimeline::SetSkipDuration(const float InSkipDuration)
{
	SkipDuration = FMath::Max(InSkipDuration, KINDA_SMALL_NUMBER);
}

void FT66AnimationTimeline::SetMaxMarkerDispatchesPerTick(const int32 InMaxDispatches)
{
	MaxMarkerDispatchesPerTick = FMath::Max(0, InMaxDispatches);
}

void FT66AnimationTimeline::SetPositionValue(const float InPositionValue)
{
	if (!bHasPositionValue)
	{
		PreviousPositionValue = InPositionValue;
		CurrentPositionValue = InPositionValue;
		bHasPositionValue = true;
		return;
	}

	PreviousPositionValue = CurrentPositionValue;
	CurrentPositionValue = InPositionValue;
}

void FT66AnimationTimeline::ClearPositionValue()
{
	bHasPositionValue = false;
	PreviousPositionValue = 0.f;
	CurrentPositionValue = 0.f;
}

void FT66AnimationTimeline::Play()
{
	ResetRuntimeState();
	State = ET66AnimationPlayState::Playing;
}

void FT66AnimationTimeline::Pause()
{
	if (State == ET66AnimationPlayState::Playing || State == ET66AnimationPlayState::Skipping)
	{
		StateBeforePause = State;
		State = ET66AnimationPlayState::Paused;
	}
}

void FT66AnimationTimeline::Resume()
{
	if (State == ET66AnimationPlayState::Paused)
	{
		State = (StateBeforePause == ET66AnimationPlayState::Skipping)
			? ET66AnimationPlayState::Skipping
			: ET66AnimationPlayState::Playing;
		StateBeforePause = ET66AnimationPlayState::Stopped;
	}
}

void FT66AnimationTimeline::Cancel()
{
	State = ET66AnimationPlayState::Cancelled;
}

void FT66AnimationTimeline::Finish()
{
	ElapsedTime = Duration;
	State = ET66AnimationPlayState::Completed;
	if (ProgressCallback)
	{
		ProgressCallback(GetCurveValue());
	}
}

void FT66AnimationTimeline::RequestSkip()
{
	if (T66Animation::IsTerminalState(State))
	{
		return;
	}

	if (State == ET66AnimationPlayState::Stopped)
	{
		Play();
	}

	SkipStartElapsedTime = ElapsedTime;
	SkipElapsed = 0.f;
	State = ET66AnimationPlayState::Skipping;
}

void FT66AnimationTimeline::Tick(const float DeltaSeconds, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents)
{
	if (State != ET66AnimationPlayState::Playing && State != ET66AnimationPlayState::Skipping)
	{
		return;
	}

	const float SafeDelta = FMath::Max(0.f, DeltaSeconds);
	const float PreviousProgress = GetProgress();
	const float PreviousCurveValue = GetCurveValue();

	if (State == ET66AnimationPlayState::Skipping)
	{
		SkipElapsed += SafeDelta;
		const float SkipAlpha = FMath::Clamp(SkipElapsed / FMath::Max(KINDA_SMALL_NUMBER, SkipDuration), 0.f, 1.f);
		ElapsedTime = FMath::Lerp(SkipStartElapsedTime, Duration, SkipAlpha);
	}
	else
	{
		ElapsedTime = FMath::Min(Duration, ElapsedTime + SafeDelta);
	}

	const float CurrentProgress = GetProgress();
	const float CurrentCurveValue = GetCurveValue();

	TArray<FPendingMarkerEvent> PendingEvents;
	for (int32 MarkerIndex = 0; MarkerIndex < Markers.Num(); ++MarkerIndex)
	{
		const FT66AnimationMarker& Marker = Markers[MarkerIndex];
		if (ShouldFireMarker(MarkerIndex, Marker, PreviousProgress, CurrentProgress, PreviousCurveValue, CurrentCurveValue))
		{
			AppendMarkerEvent(Marker, PreviousProgress, CurrentProgress, PendingEvents);
			MarkFired(MarkerIndex);
		}
	}

	FlushMarkerEvents(PendingEvents, OutMarkerEvents);

	if (ProgressCallback)
	{
		ProgressCallback(CurrentCurveValue);
	}

	if (ElapsedTime >= Duration - KINDA_SMALL_NUMBER)
	{
		CompleteFromTick(State == ET66AnimationPlayState::Skipping);
	}
}

float FT66AnimationTimeline::GetProgress() const
{
	return FMath::Clamp(ElapsedTime / FMath::Max(KINDA_SMALL_NUMBER, Duration), 0.f, 1.f);
}

float FT66AnimationTimeline::GetCurveValue() const
{
	return T66AnimationCurves::EvaluateCurve(PrimaryCurve, GetProgress());
}

bool FT66AnimationTimeline::ShouldFireMarker(const int32 MarkerIndex, const FT66AnimationMarker& Marker, const float PreviousProgress, const float CurrentProgress, const float PreviousCurveValue, const float CurrentCurveValue) const
{
	if (Marker.MarkerID.IsNone())
	{
		return false;
	}

	if (Marker.FirePolicy == ET66AnimationMarkerFirePolicy::Once && HasMarkerFired(MarkerIndex))
	{
		return false;
	}

	bool bConditionMet = false;
	switch (Marker.Type)
	{
	case ET66AnimationMarkerType::TimeBased:
	{
		const float MarkerProgress = FMath::Clamp(Marker.TimeOrProgress / FMath::Max(KINDA_SMALL_NUMBER, Duration), 0.f, 1.f);
		bConditionMet = Marker.FirePolicy == ET66AnimationMarkerFirePolicy::ContinuousWhileActive
			? CurrentProgress >= MarkerProgress
			: PreviousProgress < MarkerProgress && CurrentProgress >= MarkerProgress;
		break;
	}

	case ET66AnimationMarkerType::ProgressBased:
	{
		const float MarkerProgress = FMath::Clamp(Marker.TimeOrProgress, 0.f, 1.f);
		bConditionMet = Marker.FirePolicy == ET66AnimationMarkerFirePolicy::ContinuousWhileActive
			? CurrentProgress >= MarkerProgress
			: PreviousProgress < MarkerProgress && CurrentProgress >= MarkerProgress;
		break;
	}

	case ET66AnimationMarkerType::PositionCrossing:
	{
		const float PreviousPosition = bHasPositionValue ? PreviousPositionValue : PreviousCurveValue;
		const float CurrentPosition = bHasPositionValue ? CurrentPositionValue : CurrentCurveValue;
		bConditionMet = Marker.FirePolicy == ET66AnimationMarkerFirePolicy::ContinuousWhileActive
			? CurrentPosition >= Marker.PositionThreshold
			: CrossedThreshold(PreviousPosition, CurrentPosition, Marker.PositionThreshold);
		break;
	}

	default:
		break;
	}

	return bConditionMet;
}

void FT66AnimationTimeline::MarkFired(const int32 MarkerIndex)
{
	if (Markers.IsValidIndex(MarkerIndex) && Markers[MarkerIndex].FirePolicy == ET66AnimationMarkerFirePolicy::Once)
	{
		FiredMarkerIndices.Add(MarkerIndex);
	}
}

bool FT66AnimationTimeline::HasMarkerFired(const int32 MarkerIndex) const
{
	return FiredMarkerIndices.Contains(MarkerIndex);
}

void FT66AnimationTimeline::AppendMarkerEvent(const FT66AnimationMarker& Marker, const float PreviousProgress, const float CurrentProgress, TArray<FPendingMarkerEvent>& PendingEvents) const
{
	FPendingMarkerEvent Pending;
	Pending.Event.MarkerID = Marker.MarkerID;
	Pending.Event.PreviousProgress = PreviousProgress;
	Pending.Event.CurrentProgress = CurrentProgress;
	Pending.Event.SourceTimelineName = TimelineName;
	Pending.Event.Payload = Marker.Payload;
	Pending.bTerminal = IsTerminalMarker(Marker);
	PendingEvents.Add(Pending);
}

void FT66AnimationTimeline::FlushMarkerEvents(const TArray<FPendingMarkerEvent>& PendingEvents, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents) const
{
	int32 NonTerminalCount = 0;
	for (const FPendingMarkerEvent& Pending : PendingEvents)
	{
		if (Pending.bTerminal)
		{
			OutMarkerEvents.Add(Pending.Event);
			continue;
		}

		if (MaxMarkerDispatchesPerTick <= 0 || NonTerminalCount < MaxMarkerDispatchesPerTick)
		{
			OutMarkerEvents.Add(Pending.Event);
			++NonTerminalCount;
		}
	}
}

bool FT66AnimationTimeline::IsTerminalMarker(const FT66AnimationMarker& Marker) const
{
	static const TArray<FString> TerminalNeedles =
	{
		TEXT("Landing"),
		TEXT("Reveal"),
		TEXT("Confirm"),
		TEXT("Cancel"),
		TEXT("Complete"),
		TEXT("Commit"),
		TEXT("Terminal")
	};

	return NameContainsAny(Marker.MarkerID, TerminalNeedles)
		|| NameContainsAny(Marker.Payload, TerminalNeedles);
}

void FT66AnimationTimeline::ResetRuntimeState()
{
	ElapsedTime = 0.f;
	SkipElapsed = 0.f;
	SkipStartElapsedTime = 0.f;
	FiredMarkerIndices.Reset();
	StateBeforePause = ET66AnimationPlayState::Stopped;
}

void FT66AnimationTimeline::CompleteFromTick(const bool bFromSkip)
{
	ElapsedTime = Duration;
	State = bFromSkip ? ET66AnimationPlayState::Skipped : ET66AnimationPlayState::Completed;
}
