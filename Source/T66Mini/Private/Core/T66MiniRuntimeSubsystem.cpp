// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniRuntimeSubsystem.h"

#include "Core/T66GameInstance.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SessionSubsystem.h"
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

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		const UT66PartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UT66PartySubsystem>();
		UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>();
		const bool bOnlineMiniParty = PartySubsystem && PartySubsystem->HasRemotePartyMembers();
		if (bOnlineMiniParty)
		{
			if (!SessionSubsystem || !SessionSubsystem->IsPartySessionActive())
			{
				if (OutFailureReason)
				{
					*OutFailureReason = TEXT("Mini co-op requires an active party session.");
				}
				return false;
			}

			if (!SessionSubsystem->IsLocalPlayerPartyHost() || World->GetNetMode() == NM_Client)
			{
				if (OutFailureReason)
				{
					*OutFailureReason = TEXT("Only the mini party host can start the run.");
				}
				return false;
			}

			SessionSubsystem->PreparePartySessionForWorldTravel(TEXT("mini_gameplay_travel"));

			const FString TravelURL = FString::Printf(TEXT("%s?listen?%s"), *BattleLevelName.ToString(), *GetMiniBattleTravelOptions());
			World->ServerTravel(TravelURL);
			return true;
		}
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
