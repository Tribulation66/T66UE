// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66IdleFrontendStateSubsystem.h"

void UT66IdleFrontendStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSessionState();
}

void UT66IdleFrontendStateSubsystem::ResetSessionState()
{
	SelectedHeroID = NAME_None;
	SelectedDifficultyID = NAME_None;
	bDailySession = false;
	DailyChallengeId.Reset();
	DailySeed = 0;
	ProfileSnapshot = FT66IdleProfileSnapshot();
}

void UT66IdleFrontendStateSubsystem::BeginNewSession()
{
	ResetSessionState();
}

void UT66IdleFrontendStateSubsystem::BeginDailySession(const FName DifficultyID, const FString& ChallengeId, const int32 Seed)
{
	ResetSessionState();
	SelectedDifficultyID = DifficultyID;
	bDailySession = true;
	DailyChallengeId = ChallengeId;
	DailySeed = Seed;
}

void UT66IdleFrontendStateSubsystem::SelectHero(const FName HeroID)
{
	SelectedHeroID = HeroID;
}

void UT66IdleFrontendStateSubsystem::SetProfileSnapshot(const FT66IdleProfileSnapshot& Snapshot)
{
	ProfileSnapshot = Snapshot;
}
