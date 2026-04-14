// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniGameState.h"

#include "Save/T66MiniRunSaveGame.h"

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
	WaveSecondsRemaining = RunSave->WaveSecondsRemaining > 0.f ? RunSave->WaveSecondsRemaining : 180.f;
}
