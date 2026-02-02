// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66CompanionUnlockSaveGame.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"

#include "Kismet/GameplayStatics.h"

const FString UT66CompanionUnlockSubsystem::SlotName(TEXT("T66_CompanionUnlocks"));

void UT66CompanionUnlockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreate();
}

void UT66CompanionUnlockSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UT66CompanionUnlockSubsystem::LoadOrCreate()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	SaveObj = Cast<UT66CompanionUnlockSaveGame>(Loaded);
	if (!SaveObj)
	{
		SaveObj = Cast<UT66CompanionUnlockSaveGame>(UGameplayStatics::CreateSaveGameObject(UT66CompanionUnlockSaveGame::StaticClass()));
		Save();
	}
}

void UT66CompanionUnlockSubsystem::Save()
{
	if (!SaveObj) return;
	UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, 0);
}

bool UT66CompanionUnlockSubsystem::IsCompanionUnlocked(FName CompanionID) const
{
	// "No companion" is always allowed.
	if (CompanionID.IsNone())
	{
		return true;
	}

	if (SaveObj && SaveObj->UnlockedCompanionIDs.Contains(CompanionID))
	{
		return true;
	}

	// Optional: companions can be configured as "unlocked by default" via DT_Companions.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			FCompanionData Data;
			if (T66GI->GetCompanionData(CompanionID, Data))
			{
				return Data.bUnlockedByDefault;
			}
		}
	}

	return false;
}

bool UT66CompanionUnlockSubsystem::UnlockCompanion(FName CompanionID)
{
	if (CompanionID.IsNone())
	{
		return false;
	}

	if (!SaveObj)
	{
		LoadOrCreate();
		if (!SaveObj) return false;
	}

	const int32 OldNum = SaveObj->UnlockedCompanionIDs.Num();
	SaveObj->UnlockedCompanionIDs.Add(CompanionID);
	const bool bNewlyUnlocked = (SaveObj->UnlockedCompanionIDs.Num() != OldNum);
	if (bNewlyUnlocked)
	{
		Save();
	}
	return bNewlyUnlocked;
}

void UT66CompanionUnlockSubsystem::ResetAllUnlocks()
{
	if (!SaveObj)
	{
		LoadOrCreate();
	}
	if (!SaveObj) return;

	SaveObj->UnlockedCompanionIDs.Reset();
	Save();
}

