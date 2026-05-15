// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniRuntimeSubsystem.h"

#include "Core/T66MiniRunStateSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Save/T66MiniRunSaveGame.h"

bool UT66MiniRuntimeSubsystem::LaunchMiniBattle(UObject* WorldContextObject, FString* OutFailureReason) const
{
	if (!WorldContextObject)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("Mini launch failed: invalid world context.");
		}
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("Mini launch failed: no world is available.");
		}
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("Mini launch failed: no game instance is available.");
		}
		return false;
	}

	const UT66MiniRunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>();
	if (!RunState || !RunState->GetActiveRun())
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("Mini launch failed: no active mini run is available.");
		}
		return false;
	}

	return true;
}

FName UT66MiniRuntimeSubsystem::GetMiniBattleLevelName() const
{
	return FName(TEXT("MiniBattleWidget"));
}

FString UT66MiniRuntimeSubsystem::GetMiniBattleTravelOptions() const
{
	return FString();
}
