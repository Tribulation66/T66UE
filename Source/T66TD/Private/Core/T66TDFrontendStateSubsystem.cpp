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
	SelectedStageID = NAME_None;
	bBattleRewardGranted = false;
	bDailyRun = false;
	DailyChallengeId.Reset();
	DailySeed = 0;
}

void UT66TDFrontendStateSubsystem::BeginNewRun()
{
	ResetRunSetup();
}

void UT66TDFrontendStateSubsystem::BeginDailyRun(const FName DifficultyID, const FString& ChallengeId, const int32 Seed)
{
	ResetRunSetup();
	SelectedDifficultyID = DifficultyID;
	bDailyRun = true;
	DailyChallengeId = ChallengeId;
	DailySeed = Seed;
}

void UT66TDFrontendStateSubsystem::SelectDifficulty(const FName DifficultyID)
{
	if (SelectedDifficultyID != DifficultyID)
	{
		SelectedDifficultyID = DifficultyID;
		SelectedMapID = NAME_None;
		SelectedStageID = NAME_None;
		bBattleRewardGranted = false;
	}
}

void UT66TDFrontendStateSubsystem::SelectMap(const FName MapID)
{
	SelectedMapID = MapID;
	SelectedStageID = NAME_None;
	bBattleRewardGranted = false;
}

void UT66TDFrontendStateSubsystem::SelectStage(const FName StageID)
{
	SelectedStageID = StageID;
	bBattleRewardGranted = false;
}

void UT66TDFrontendStateSubsystem::ResetBattleRewardGrant()
{
	bBattleRewardGranted = false;
}

bool UT66TDFrontendStateSubsystem::TryMarkBattleRewardGranted()
{
	if (bBattleRewardGranted)
	{
		return false;
	}

	bBattleRewardGranted = true;
	return true;
}
