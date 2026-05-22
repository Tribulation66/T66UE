// Copyright Tribulation 66. All Rights Reserved.

// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class ET66AnimationMarkerType : uint8
{
	TimeBased,
	ProgressBased,
	PositionCrossing,
};

enum class ET66AnimationMarkerFirePolicy : uint8
{
	Once,
	Repeat,
	ContinuousWhileActive,
};

struct T66_API FT66AnimationMarker
{
	FName MarkerID = NAME_None;
	ET66AnimationMarkerType Type = ET66AnimationMarkerType::ProgressBased;
	float TimeOrProgress = 0.f;
	float PositionThreshold = 0.f;
	ET66AnimationMarkerFirePolicy FirePolicy = ET66AnimationMarkerFirePolicy::Once;
	FName Payload = NAME_None;
};

struct T66_API FT66AnimationMarkerEvent
{
	FName MarkerID = NAME_None;
	float PreviousProgress = 0.f;
	float CurrentProgress = 0.f;
	FName SourceTimelineName = NAME_None;
	FName Payload = NAME_None;
};
