// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TDFrontendStateSubsystem.h"

void UT66TDFrontendStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetRunSetup();
}

void UT66TDFrontendStateSubsystem::ResetRunSetup()
{
	SelectedDifficultyID = NAME_None;
	SelectedMapID = NAME_None;
}

void UT66TDFrontendStateSubsystem::BeginNewRun()
{
	ResetRunSetup();
}

void UT66TDFrontendStateSubsystem::SelectDifficulty(const FName DifficultyID)
{
	if (SelectedDifficultyID != DifficultyID)
	{
		SelectedDifficultyID = DifficultyID;
		SelectedMapID = NAME_None;
	}
}

void UT66TDFrontendStateSubsystem::SelectMap(const FName MapID)
{
	SelectedMapID = MapID;
}
