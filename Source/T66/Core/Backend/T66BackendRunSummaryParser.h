// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FJsonObject;
class UObject;
class UT66LeaderboardRunSummarySaveGame;

namespace T66BackendRunSummaryParser
{
	UT66LeaderboardRunSummarySaveGame* Parse(const TSharedPtr<FJsonObject>& Json, UObject* Outer);
}
