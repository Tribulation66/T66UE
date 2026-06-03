// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66Hero1AxeBounceVFXCommandlet.generated.h"

/**
 * Builds the Hero 1 axe Bounce lab/production VFX: a small horizontal slash mesh
 * plus an ImpactAnchored Niagara mesh-slash system that reuses the shared Hero 1
 * AOE slash materials.
 *
 * The Bounce silhouette lives in the Niagara renderer/material/mesh assets, not in
 * actor-side geometry. It is a compact, centered horizontal slash placed at each
 * Bounce chain impact point, deliberately distinct from the AOE radial crescent and
 * the Pierce forward vertical lane.
 */
UCLASS()
class T66_API UT66Hero1AxeBounceVFXCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66Hero1AxeBounceVFXCommandlet();

	virtual int32 Main(const FString& Params) override;
};
