// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniRuntimeSubsystem.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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

	const FName BattleLevelName = GetMiniBattleLevelName();
	if (BattleLevelName.IsNone())
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("Mini launch failed: no battle level is configured.");
		}
		return false;
	}

	UGameplayStatics::OpenLevel(WorldContextObject, BattleLevelName, true, GetMiniBattleTravelOptions());
	return true;
}

FName UT66MiniRuntimeSubsystem::GetMiniBattleLevelName() const
{
	return FName(TEXT("/Game/Mini/Maps/T66MiniBattleMap"));
}

FString UT66MiniRuntimeSubsystem::GetMiniBattleTravelOptions() const
{
	return TEXT("game=/Script/T66Mini.T66MiniGameMode");
}
