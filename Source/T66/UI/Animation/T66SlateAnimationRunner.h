// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Animation/T66AnimationTimeline.h"
#include "Types/SlateEnums.h"

class SWidget;

class FT66SlateAnimationRunner
{
public:
	FT66SlateAnimationRunner() = default;
	~FT66SlateAnimationRunner();

	FT66SlateAnimationRunner(const FT66SlateAnimationRunner&) = delete;
	FT66SlateAnimationRunner& operator=(const FT66SlateAnimationRunner&) = delete;

	FT66SlateAnimationRunner(FT66SlateAnimationRunner&&) = delete;
	FT66SlateAnimationRunner& operator=(FT66SlateAnimationRunner&&) = delete;

	bool StartRunner(
		SWidget* OwningWidget,
		FT66AnimationTimeline&& Timeline,
		TFunction<void(const TArray<FT66AnimationMarkerEvent>&)> OnMarkerEvents);
	void StopRunner();
	bool IsRunning() const;

private:
	EActiveTimerReturnType HandleActiveTimer(double CurrentTime, float DeltaTime);
	void ClearTimerHandle();

	TWeakPtr<SWidget> OwningWidgetWeak;
	TSharedPtr<FActiveTimerHandle> ActiveTimerHandle;
	TUniquePtr<FT66AnimationTimeline> ActiveTimeline;
	TFunction<void(const TArray<FT66AnimationMarkerEvent>&)> MarkerEventsCallback;
};
