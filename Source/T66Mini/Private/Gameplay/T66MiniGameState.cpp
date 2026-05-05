// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniGameState.h"

#include "Net/UnrealNetwork.h"
#include "Save/T66MiniRunSaveGame.h"

AT66MiniGameState::AT66MiniGameState()
{
	bReplicates = true;
}

void AT66MiniGameState::ApplyRunSave(const UT66MiniRunSaveGame* RunSave)
{
	if (!RunSave)
	{
		return;
	}

	HeroID = RunSave->HeroID;
	CompanionID = RunSave->CompanionID;
	DifficultyID = RunSave->DifficultyID;
	WaveIndex = FMath::Max(1, RunSave->WaveIndex);
	WaveSecondsRemaining = FMath::Clamp(RunSave->WaveSecondsRemaining > 0.f ? RunSave->WaveSecondsRemaining : 60.f, 0.f, 60.f);
}

void AT66MiniGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniGameState, HeroID);
	DOREPLIFETIME(AT66MiniGameState, CompanionID);
	DOREPLIFETIME(AT66MiniGameState, DifficultyID);
	DOREPLIFETIME(AT66MiniGameState, WaveIndex);
	DOREPLIFETIME(AT66MiniGameState, WaveSecondsRemaining);
}
