// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Animation/T66AnimationTimeline.h"

class FT66AnimationGroup;

class T66_API FT66AnimationSequence final : public IT66AnimationNode
{
public:
	FT66AnimationSequence() = default;
	FT66AnimationSequence(FT66AnimationSequence&&) = default;
	FT66AnimationSequence& operator=(FT66AnimationSequence&&) = default;
	FT66AnimationSequence(const FT66AnimationSequence&) = delete;
	FT66AnimationSequence& operator=(const FT66AnimationSequence&) = delete;
	virtual ~FT66AnimationSequence() override = default;

	void AddTimeline(FT66AnimationTimeline&& Timeline);
	void AddGroup(FT66AnimationGroup&& Group);

	virtual void Play() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void Cancel() override;
	virtual void Finish() override;
	virtual void RequestSkip() override;
	virtual void Tick(float DeltaSeconds, TArray<FT66AnimationMarkerEvent>& OutMarkerEvents) override;
	virtual float GetDuration() const override;
	virtual float GetProgress() const override { return GetTotalProgress(); }
	virtual ET66AnimationPlayState GetState() const override { return State; }

	int32 GetCurrentChildIndex() const { return CurrentChildIndex; }
	float GetTotalProgress() const;
	int32 GetChildCount() const { return Children.Num(); }

private:
	IT66AnimationNode* GetCurrentChild() const;
	void AdvanceCompletedChildren();

	TArray<TUniquePtr<IT66AnimationNode>> Children;
	int32 CurrentChildIndex = INDEX_NONE;
	ET66AnimationPlayState State = ET66AnimationPlayState::Stopped;
	bool bSkipRequested = false;
};
