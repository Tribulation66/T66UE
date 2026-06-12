// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace T66CodeReferencedAssets
{
	// Startup guard for assets that C++ loads by string path (LoadObject/FindObject).
	// The cooker cannot see those references, so a missing DirectoriesToAlwaysCook entry
	// ships a staged build where the LoadObject silently returns null and gameplay falls
	// back to placeholder visuals. This check makes that gap a LOUD startup error naming
	// the path and the fix. Runs once; cheap (container existence lookups, no loads).
	void VerifyAllResolvable();
}
