// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FT66StageProgressionSnapshot;

class UWorld;

class T66_API FT66StageProgressionVisuals
{
public:
	static void ApplyToWorld(UWorld* World, const FT66StageProgressionSnapshot& Snapshot);
};
