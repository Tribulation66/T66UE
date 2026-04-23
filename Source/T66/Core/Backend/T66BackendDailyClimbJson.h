// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66DailyClimbTypes.h"

class FJsonObject;

namespace T66BackendDailyClimbJson
{
	bool ParseChallengeData(const TSharedPtr<FJsonObject>& Json, FT66DailyClimbChallengeData& OutChallenge);
}
