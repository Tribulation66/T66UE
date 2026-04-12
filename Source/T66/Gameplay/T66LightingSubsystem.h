// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "T66LightingSubsystem.generated.h"

/**
 * World-scoped owner for lighting orchestration.
 * Keeps runtime lighting entry points out of GameMode while preserving
 * the shared implementation in FT66WorldLightingSetup.
 */
UCLASS()
class T66_API UT66LightingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UT66LightingSubsystem* Get(UWorld* World);

	static void EnsureSharedLightingForWorld(UWorld* World);
	static void ConfigureGameplayFogForWorld(UWorld* World);
	static void ApplyThemeToDirectionalLightsForWorld(UWorld* World);
	static void ApplyThemeToAtmosphereAndLightingForWorld(UWorld* World);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	void EnsureSharedLighting();
	void ConfigureGameplayFog();
	void ApplyThemeToDirectionalLights();
	void ApplyThemeToAtmosphereAndLighting();
};
