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

#if WITH_EDITOR
	/** Copy grass (Polytope), landscape/trees/rocks (Cozy Nature) into Content/T66MapAssets. Run once to consolidate. */
	void SetupT66MapAssets();
#endif
}
