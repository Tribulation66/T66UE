// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Animation/T66AnimationGroup.h"

#include "Core/Animation/T66AnimationSequence.h"

void FT66AnimationGroup::AddTimeline(FT66AnimationTimeline&& Timeline)
{
	Children.Add(MakeUnique<FT66AnimationTimeline>(MoveTemp(Timeline)));
}

void FT66AnimationGroup::AddSequence(FT66AnimationSequence&& Sequence)
{
	Children.Add(MakeUnique<FT66AnimationSequence>(MoveTemp(Sequence)));
}

void FT66AnimationGroup::Play()
{
	State = Children.Num() > 0 ? ET66AnimationPlayState::Playing : ET66AnimationPlayState::Completed;
	bSkipRequested = false;
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid())
		{
			Child->Play();
		}
	}
}

void FT66AnimationGroup::Pause()
{
	if (State == ET66AnimationPlayState::Playing || State == ET66AnimationPlayState::Skipping)
	{
		State = ET66AnimationPlayState::Paused;
		for (const TUniquePtr<IT66AnimationNode>& Child : Children)
		{
			if (Child.IsValid())
			{
				Child->Pause();
			}
		}
	}
}

void FT66AnimationGroup::Resume()
{
	if (State == ET66AnimationPlayState::Paused)
	{
		State = bSkipRequested ? ET66AnimationPlayState::Skipping : ET66AnimationPlayState::Playing;
		for (const TUniquePtr<IT66AnimationNode>& Child : Children)
		{
			if (Child.IsValid())
			{
				Child->Resume();
			}
		}
	}
}

void FT66AnimationGroup::Cancel()
{
	State = ET66AnimationPlayState::Cancelled;
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid() && !T66Animation::IsTerminalState(Child->GetState()))
		{
			Child->Cancel();
		}
	}
}

void FT66AnimationGroup::Finish()
{
	State = ET66AnimationPlayState::Completed;
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid() && !T66Animation::IsTerminalState(Child->GetState()))
		{
			Child->Finish();
		}
	}
}

void FT66AnimationGroup::RequestSkip()
{
	if (T66Animation::IsTerminalState(State))
	{
		return;
	}

	if (State == ET66AnimationPlayState::Stopped)
	{
		Play();
	}

	bSkipRequested = true;
	State = ET66AnimationPlayState::Skipping;
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid() && !T66Animation::IsTerminalState(Child->GetState()))
		{
			Child->RequestSkip();
		}
	}
}

void FT66AnimationGroup::Tick(const float DeltaSeconds, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents)
{
	if (State != ET66AnimationPlayState::Playing && State != ET66AnimationPlayState::Skipping)
	{
		return;
	}

	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid() && !T66Animation::IsTerminalState(Child->GetState()))
		{
			Child->Tick(DeltaSeconds, OutMarkerEvents);
		}
	}

	RefreshCompletionState();
}

float FT66AnimationGroup::GetDuration() const
{
	float MaxDuration = 0.f;
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid())
		{
			MaxDuration = FMath::Max(MaxDuration, Child->GetDuration());
		}
	}
	return MaxDuration;
}

float FT66AnimationGroup::GetProgress() const
{
	if (Children.Num() == 0)
	{
		return T66Animation::IsTerminalState(State) ? 1.f : 0.f;
	}

	float ProgressSum = 0.f;
	int32 ValidChildren = 0;
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid())
		{
			ProgressSum += Child->GetProgress();
			++ValidChildren;
		}
	}

	return ValidChildren > 0 ? FMath::Clamp(ProgressSum / static_cast<float>(ValidChildren), 0.f, 1.f) : 0.f;
}

void FT66AnimationGroup::RefreshCompletionState()
{
	if (Children.Num() == 0)
	{
		State = ET66AnimationPlayState::Completed;
		return;
	}

	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid() && !T66Animation::IsTerminalState(Child->GetState()))
		{
			return;
		}
	}

	State = bSkipRequested ? ET66AnimationPlayState::Skipped : ET66AnimationPlayState::Completed;
}
