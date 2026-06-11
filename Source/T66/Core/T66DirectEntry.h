// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66RunTypes.h"
#include "Data/T66DataTypes.h"
#include "UI/T66UITypes.h"

class UT66GameInstance;
class UWorld;

enum class ET66DirectEntryKind : uint8
{
	None,
	FrontendScreen,
	GameplayRun
};

struct T66_API FT66DirectEntryRequest
{
	ET66DirectEntryKind Kind = ET66DirectEntryKind::None;
	ET66ScreenType Screen = ET66ScreenType::None;
	ET66ScreenType Modal = ET66ScreenType::None;
	ET66RunCategory RunCategory = ET66RunCategory::Tower;
	FName HeroID = NAME_None;
	ET66BodyType HeroBodyType = ET66BodyType::Chad;
	FName CompanionID = NAME_None;
	ET66Difficulty Difficulty = ET66Difficulty::Easy;
	bool bLeaderboardIneligible = true;
	FString Source;
};

namespace T66DirectEntry
{
	T66_API FString GetAcceptedFrontendScreenNamesForLog();
	T66_API FString GetAcceptedRunNamesForLog();

	T66_API bool TryResolveFrontendScreenName(const FString& ScreenName, ET66ScreenType& OutScreenType);
	T66_API bool TryResolveRunCategoryName(const FString& RunName, ET66RunCategory& OutRunCategory);
	T66_API bool TryResolveDifficultyName(const FString& DifficultyName, ET66Difficulty& OutDifficulty);

	T66_API bool TryParseEntryValue(const FString& EntryValue, FT66DirectEntryRequest& OutRequest, FString& OutError);
	T66_API bool TryParseCommandLine(FT66DirectEntryRequest& OutRequest, FString& OutError);

	T66_API void ApplyRequestToGameInstance(UT66GameInstance& GameInstance, const FT66DirectEntryRequest& Request);
	T66_API bool ExecuteRequest(UWorld* World, const FT66DirectEntryRequest& Request, FString& OutError);
}
