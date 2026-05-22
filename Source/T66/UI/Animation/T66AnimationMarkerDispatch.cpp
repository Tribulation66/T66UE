// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Animation/T66AnimationMarkerDispatch.h"

#include "Core/T66AudioSubsystem.h"

void FT66AnimationMarkerDispatcher::RegisterHandler(
	const FName MarkerID,
	TFunction<void(const FT66AnimationMarkerEvent&)> Handler)
{
	if (MarkerID.IsNone() || !Handler)
	{
		return;
	}

	Handlers.Add(FT66AnimationMarkerHandler{ MarkerID, MoveTemp(Handler) });
}

void FT66AnimationMarkerDispatcher::RegisterAudioMarker(
	const FName MarkerID,
	const FName AudioEventID,
	UObject* WorldContext)
{
	TWeakObjectPtr<UObject> WorldContextWeak(WorldContext);
	RegisterHandler(
		MarkerID,
		[WorldContextWeak, AudioEventID](const FT66AnimationMarkerEvent& MarkerEvent)
		{
			const FName EventID = AudioEventID.IsNone() ? MarkerEvent.Payload : AudioEventID;
			if (EventID.IsNone())
			{
				return;
			}

			if (UObject* WorldContextPtr = WorldContextWeak.Get())
			{
				UT66AudioSubsystem::PlayEventFromWorldContext(WorldContextPtr, EventID, FVector::ZeroVector, nullptr);
			}
			else
			{
				UT66AudioSubsystem::PlayUIEventFromAnyWorld(EventID);
			}
		});
}

void FT66AnimationMarkerDispatcher::Dispatch(const TArray<FT66AnimationMarkerEvent>& MarkerEvents) const
{
	for (const FT66AnimationMarkerEvent& MarkerEvent : MarkerEvents)
	{
		for (const FT66AnimationMarkerHandler& Handler : Handlers)
		{
			if (Handler.MarkerID == MarkerEvent.MarkerID && Handler.Handler)
			{
				Handler.Handler(MarkerEvent);
			}
		}
	}
}

void FT66AnimationMarkerDispatcher::ClearHandlers()
{
	Handlers.Reset();
}
