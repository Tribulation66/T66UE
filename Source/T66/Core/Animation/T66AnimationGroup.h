// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Animation/T66AnimationTimeline.h"

class FT66AnimationSequence;

class T66_API FT66AnimationGroup final : public IT66AnimationNode
{
public:
	FT66AnimationGroup() = default;
	FT66AnimationGroup(FT66AnimationGroup&&) = default;
	FT66AnimationGroup& operator=(FT66AnimationGroup&&) = default;
	FT66AnimationGroup(const FT66AnimationGroup&) = delete;
	FT66AnimationGroup& operator=(const FT66AnimationGroup&) = delete;
	virtual ~FT66AnimationGroup() override = default;

	void AddTimeline(FT66AnimationTimeline&& Timeline);
	void AddSequence(FT66AnimationSequence&& Sequence);

	virtual void Play() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void Cancel() override;
	virtual void Finish() override;
	virtual void RequestSkip() override;
	virtual void Tick(float DeltaSeconds, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents) override;
	virtual float GetDuration() const override;
	virtual float GetProgress() const override;
	virtual ET66AnimationPlayState GetState() const override { return State; }

	int32 GetChildCount() const { return Children.Num(); }

private:
	void RefreshCompletionState();

	TArray<TUniquePtr<IT66AnimationNode>> Children;
	ET66AnimationPlayState State = ET66AnimationPlayState::Stopped;
	bool bSkipRequested = false;
};
