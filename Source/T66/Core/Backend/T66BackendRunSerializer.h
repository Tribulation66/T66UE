// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class FJsonObject;
class UT66LeaderboardRunSummarySaveGame;

namespace T66BackendRunSerializer
{
	TSharedPtr<FJsonObject> BuildRunJsonObject(
		const FString& HeroId,
		const FString& CompanionId,
		ET66Difficulty Difficulty,
		ET66PartySize PartySize,
		int32 StageReached,
		int32 Score,
		int32 TimeMs,
		UT66LeaderboardRunSummarySaveGame* Snapshot);

	bool SerializeJsonObjectToString(const TSharedPtr<FJsonObject>& Json, FString& OutJsonString);
	bool DeserializeJsonObjectString(const FString& JsonString, TSharedPtr<FJsonObject>& OutJson);
}
