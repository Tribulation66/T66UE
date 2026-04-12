// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

namespace T66LeaderboardPacing
{
	extern const TCHAR* const StageMarkerPrefix;

	FString MakeStageMarker(int32 Stage, int32 Score, float ElapsedSeconds);
	bool ParseStageMarker(const FString& Entry, FT66StagePacingPoint& OutPoint);
	void ExtractStageMarkers(const TArray<FString>& EventLog, TArray<FT66StagePacingPoint>& OutPoints);
	bool IsStageMarker(const FString& Entry);
}
