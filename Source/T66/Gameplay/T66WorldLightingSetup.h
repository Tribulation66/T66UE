#pragma once

#include "CoreMinimal.h"

class UWorld;

class T66_API FT66WorldLightingSetup
{
public:
	static void EnsureSharedLightingForWorld(UWorld* World);
	static void ConfigureGameplayFogForWorld(UWorld* World);
	static void ApplyThemeToDirectionalLightsForWorld(UWorld* World);
	static void ApplyThemeToAtmosphereAndLightingForWorld(UWorld* World);
};
