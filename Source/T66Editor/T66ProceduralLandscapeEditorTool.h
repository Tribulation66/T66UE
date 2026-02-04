// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;
struct FT66ProceduralLandscapeParams;

namespace T66ProceduralLandscapeEditor
{
	/**
	 * Generate procedural hills and create or update the Landscape in the given world.
	 * Logs with [MAP] prefix on errors.
	 * Returns true on success.
	 */
	bool GenerateProceduralHillsLandscape(UWorld* World, const FT66ProceduralLandscapeParams& Params);
}
