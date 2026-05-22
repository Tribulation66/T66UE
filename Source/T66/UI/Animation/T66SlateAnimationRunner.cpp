// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Animation/T66SlateAnimationRunner.h"

#include "Types/WidgetActiveTimerDelegate.h"
#include "Widgets/SWidget.h"

FT66SlateAnimationRunner::~FT66SlateAnimationRunner()
{
	StopRunner();
}

bool FT66SlateAnimationRunner::StartRunner(
	SWidget* OwningWidget,
	FT66AnimationTimeline&& Timeline,
	TFunction<void(const TArray<FT66AnimationMarkerEvent>&)> OnMarkerEvents)
{
	if (!OwningWidget)
	{
		return false;
	}

	StopRunner();

	TSharedRef<SWidget> OwningWidgetRef = OwningWidget->AsShared();
	OwningWidgetWeak = OwningWidgetRef;
	ActiveTimeline = MakeUnique<FT66AnimationTimeline>(MoveTemp(Timeline));
	MarkerEventsCallback = MoveTemp(OnMarkerEvents);

	ActiveTimeline->Play();
	ActiveTimerHandle = OwningWidgetRef->RegisterActiveTimer(
		0.f,
		FWidgetActiveTimerDelegate::CreateRaw(this, &FT66SlateAnimationRunner::HandleActiveTimer));

	return true;
}

void FT66SlateAnimationRunner::StopRunner()
{
	ClearTimerHandle();
	ActiveTimeline.Reset();
	MarkerEventsCallback = nullptr;
	OwningWidgetWeak.Reset();
}

bool FT66SlateAnimationRunner::IsRunning() const
{
	return ActiveTimeline.IsValid() && !T66Animation::IsTerminalState(ActiveTimeline->GetState());
}

EActiveTimerReturnType FT66SlateAnimationRunner::HandleActiveTimer(double CurrentTime, float DeltaTime)
{
	(void)CurrentTime;

	TSharedPtr<SWidget> OwningWidget = OwningWidgetWeak.Pin();
	if (!OwningWidget.IsValid() || !ActiveTimeline.IsValid())
	{
		StopRunner();
		return EActiveTimerReturnType::Stop;
	}

	TArray<FT66AnimationMarkerEvent> MarkerEvents;
	ActiveTimeline->Tick(FMath::Max(0.f, DeltaTime), MarkerEvents);

	if (MarkerEvents.Num() > 0 && MarkerEventsCallback)
	{
		MarkerEventsCallback(MarkerEvents);
	}

	OwningWidget->Invalidate(EInvalidateWidgetReason::Paint);

	if (T66Animation::IsTerminalState(ActiveTimeline->GetState()))
	{
		StopRunner();
		return EActiveTimerReturnType::Stop;
	}

	return EActiveTimerReturnType::Continue;
}

void FT66SlateAnimationRunner::ClearTimerHandle()
{
	TSharedPtr<SWidget> OwningWidget = OwningWidgetWeak.Pin();
	if (OwningWidget.IsValid() && ActiveTimerHandle.IsValid())
	{
		OwningWidget->UnRegisterActiveTimer(ActiveTimerHandle.ToSharedRef());
	}

	ActiveTimerHandle.Reset();
}
