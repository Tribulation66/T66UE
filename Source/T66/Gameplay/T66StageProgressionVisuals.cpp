// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageProgressionVisuals.h"

#include "Core/T66StageProgressionSubsystem.h"
#include "Gameplay/T66WorldVisualSetup.h"
#include "Engine/PostProcessVolume.h"

namespace
{
	static bool T66_StageProgressionVisualWritesEnabled()
	{
		return false;
	}
}

void FT66StageProgressionVisuals::ApplyToWorld(UWorld* World, const FT66StageProgressionSnapshot& Snapshot)
{
	if (!T66_StageProgressionVisualWritesEnabled())
	{
		(void)World;
		(void)Snapshot;
		return;
	}

	APostProcessVolume* Volume = FT66WorldVisualSetup::FindOrCreateRuntimePostProcessVolume(World);
	if (!Volume)
	{
		return;
	}

	Volume->bUnbound = true;
	FPostProcessSettings& PPS = Volume->Settings;
	PPS.bOverride_ColorSaturation = true;
	PPS.ColorSaturation = Snapshot.ColorSaturation;
}
