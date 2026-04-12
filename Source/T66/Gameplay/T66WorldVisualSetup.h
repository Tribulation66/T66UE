#pragma once

#include "CoreMinimal.h"

class UWorld;

class T66_API FT66WorldVisualSetup
{
public:
	static void EnsureNeutralVisualSetupForWorld(UWorld* World);
};
