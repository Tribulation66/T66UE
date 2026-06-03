// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66Hero1AxePierceVFXCommandlet.generated.h"

/**
 * Builds the Hero 1 axe Pierce lab/production VFX: a vertical blade-plane lane mesh
 * plus a PathAnchored Niagara mesh-slash system that reuses the shared Hero 1 AOE
 * slash materials.
 *
 * The Pierce silhouette lives in the Niagara renderer/material/mesh assets, not in
 * actor-side geometry, and is deliberately a forward vertical blade rather than the
 * AOE radial crescent.
 */
UCLASS()
class T66_API UT66Hero1AxePierceVFXCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66Hero1AxePierceVFXCommandlet();

	virtual int32 Main(const FString& Params) override;
};
