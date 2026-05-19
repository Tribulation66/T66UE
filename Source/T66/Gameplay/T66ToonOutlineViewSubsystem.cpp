// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ToonOutlineViewSubsystem.h"

#include "Gameplay/T66WorldVisualSetup.h"
#include "Stats/Stats.h"

void UT66ToonOutlineViewSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FT66WorldVisualSetup::UpdateToonOutlineViewParameters(GetWorld());
}

TStatId UT66ToonOutlineViewSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UT66ToonOutlineViewSubsystem, STATGROUP_Tickables);
}
