#pragma once

#include "CoreMinimal.h"

class APostProcessVolume;
class UWorld;

class T66_API FT66WorldVisualSetup
{
public:
	static void EnsureNeutralVisualSetupForWorld(UWorld* World);
	static APostProcessVolume* FindOrCreateRuntimePostProcessVolume(UWorld* World);
};
