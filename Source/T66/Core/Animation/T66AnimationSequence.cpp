// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Animation/T66AnimationSequence.h"

#include "Core/Animation/T66AnimationGroup.h"

void FT66AnimationSequence::AddTimeline(FT66AnimationTimeline&& Timeline)
{
	Children.Add(MakeUnique<FT66AnimationTimeline>(MoveTemp(Timeline)));
}

void FT66AnimationSequence::AddGroup(FT66AnimationGroup&& Group)
{
	Children.Add(MakeUnique<FT66AnimationGroup>(MoveTemp(Group)));
}

void FT66AnimationSequence::Play()
{
	CurrentChildIndex = Children.Num() > 0 ? 0 : INDEX_NONE;
	State = Children.Num() > 0 ? ET66AnimationPlayState::Playing : ET66AnimationPlayState::Completed;
	bSkipRequested = false;
	if (IT66AnimationNode* Child = GetCurrentChild())
	{
		Child->Play();
	}
}

void FT66AnimationSequence::Pause()
{
	if (State == ET66AnimationPlayState::Playing || State == ET66AnimationPlayState::Skipping)
	{
		State = ET66AnimationPlayState::Paused;
		if (IT66AnimationNode* Child = GetCurrentChild())
		{
			Child->Pause();
		}
	}
}

void FT66AnimationSequence::Resume()
{
	if (State == ET66AnimationPlayState::Paused)
	{
		State = bSkipRequested ? ET66AnimationPlayState::Skipping : ET66AnimationPlayState::Playing;
		if (IT66AnimationNode* Child = GetCurrentChild())
		{
			Child->Resume();
		}
	}
}

void FT66AnimationSequence::Cancel()
{
	State = ET66AnimationPlayState::Cancelled;
	if (IT66AnimationNode* Child = GetCurrentChild())
	{
		Child->Cancel();
	}
}

void FT66AnimationSequence::Finish()
{
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid() && !T66Animation::IsTerminalState(Child->GetState()))
		{
			Child->Finish();
		}
	}
	CurrentChildIndex = Children.Num() > 0 ? Children.Num() - 1 : INDEX_NONE;
	State = ET66AnimationPlayState::Completed;
}

void FT66AnimationSequence::RequestSkip()
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
	if (IT66AnimationNode* Child = GetCurrentChild())
	{
		Child->RequestSkip();
	}
}

void FT66AnimationSequence::Tick(const float DeltaSeconds, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents)
{
	if (State != ET66AnimationPlayState::Playing && State != ET66AnimationPlayState::Skipping)
	{
		return;
	}

	if (IT66AnimationNode* Child = GetCurrentChild())
	{
		Child->Tick(DeltaSeconds, OutMarkerEvents);
	}

	AdvanceCompletedChildren();
}

float FT66AnimationSequence::GetDuration() const
{
	float TotalDuration = 0.f;
	for (const TUniquePtr<IT66AnimationNode>& Child : Children)
	{
		if (Child.IsValid())
		{
			TotalDuration += Child->GetDuration();
		}
	}
	return TotalDuration;
}

float FT66AnimationSequence::GetTotalProgress() const
{
	const float TotalDuration = GetDuration();
	if (TotalDuration <= KINDA_SMALL_NUMBER)
	{
		return T66Animation::IsTerminalState(State) ? 1.f : 0.f;
	}

	float CompletedDuration = 0.f;
	for (int32 Index = 0; Index < Children.Num(); ++Index)
	{
		const TUniquePtr<IT66AnimationNode>& Child = Children[Index];
		if (!Child.IsValid())
		{
			continue;
		}

		const float ChildDuration = Child->GetDuration();
		if (Index < CurrentChildIndex)
		{
			CompletedDuration += ChildDuration;
		}
		else if (Index == CurrentChildIndex)
		{
			CompletedDuration += ChildDuration * Child->GetProgress();
			break;
		}
	}

	return FMath::Clamp(CompletedDuration / TotalDuration, 0.f, 1.f);
}

IT66AnimationNode* FT66AnimationSequence::GetCurrentChild() const
{
	return Children.IsValidIndex(CurrentChildIndex) ? Children[CurrentChildIndex].Get() : nullptr;
}

void FT66AnimationSequence::AdvanceCompletedChildren()
{
	while (IT66AnimationNode* Child = GetCurrentChild())
	{
		if (!T66Animation::IsTerminalState(Child->GetState()))
		{
			break;
		}

		++CurrentChildIndex;
		if (!Children.IsValidIndex(CurrentChildIndex))
		{
			State = bSkipRequested ? ET66AnimationPlayState::Skipped : ET66AnimationPlayState::Completed;
			CurrentChildIndex = Children.Num() > 0 ? Children.Num() - 1 : INDEX_NONE;
			return;
		}

		if (IT66AnimationNode* NextChild = GetCurrentChild())
		{
			NextChild->Play();
			if (bSkipRequested)
			{
				NextChild->RequestSkip();
			}
		}
	}
}
