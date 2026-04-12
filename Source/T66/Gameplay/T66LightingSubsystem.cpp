// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LightingSubsystem.h"

#include "Engine/World.h"
#include "Gameplay/T66WorldLightingSetup.h"

UT66LightingSubsystem* UT66LightingSubsystem::Get(UWorld* World)
{
	return World ? World->GetSubsystem<UT66LightingSubsystem>() : nullptr;
}

void UT66LightingSubsystem::EnsureSharedLightingForWorld(UWorld* World)
{
	if (UT66LightingSubsystem* LightingSubsystem = Get(World))
	{
		LightingSubsystem->EnsureSharedLighting();
	}
}

void UT66LightingSubsystem::ConfigureGameplayFogForWorld(UWorld* World)
{
	if (UT66LightingSubsystem* LightingSubsystem = Get(World))
	{
		LightingSubsystem->ConfigureGameplayFog();
	}
}

void UT66LightingSubsystem::ApplyThemeToDirectionalLightsForWorld(UWorld* World)
{
	if (UT66LightingSubsystem* LightingSubsystem = Get(World))
	{
		LightingSubsystem->ApplyThemeToDirectionalLights();
	}
}

void UT66LightingSubsystem::ApplyThemeToAtmosphereAndLightingForWorld(UWorld* World)
{
	if (UT66LightingSubsystem* LightingSubsystem = Get(World))
	{
		LightingSubsystem->ApplyThemeToAtmosphereAndLighting();
	}
}

bool UT66LightingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UT66LightingSubsystem::EnsureSharedLighting()
{
	FT66WorldLightingSetup::EnsureSharedLightingForWorld(GetWorld());
}

void UT66LightingSubsystem::ConfigureGameplayFog()
{
	FT66WorldLightingSetup::ConfigureGameplayFogForWorld(GetWorld());
}

void UT66LightingSubsystem::ApplyThemeToDirectionalLights()
{
	FT66WorldLightingSetup::ApplyThemeToDirectionalLightsForWorld(GetWorld());
}

void UT66LightingSubsystem::ApplyThemeToAtmosphereAndLighting()
{
	FT66WorldLightingSetup::ApplyThemeToAtmosphereAndLightingForWorld(GetWorld());
}
