// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Animation/T66AnimationMarker.h"

struct FT66AnimationMarkerHandler
{
	FName MarkerID = NAME_None;
	TFunction<void(const FT66AnimationMarkerEvent&)> Handler;
};

class FT66AnimationMarkerDispatcher
{
public:
	void RegisterHandler(FName MarkerID, TFunction<void(const FT66AnimationMarkerEvent&)> Handler);
	void RegisterAudioMarker(FName MarkerID, FName AudioEventID, UObject* WorldContext);
	void Dispatch(const TArray<FT66AnimationMarkerEvent>& MarkerEvents) const;
	void ClearHandlers();

private:
	TArray<FT66AnimationMarkerHandler> Handlers;
};
