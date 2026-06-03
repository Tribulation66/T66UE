// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66Hero1AxeDOTVFXCommandlet.generated.h"

/**
 * Builds the Hero 1 axe DOT lab/production VFX: a compact aura-ring mesh plus a
 * Niagara mesh-slash system that reuses the shared Hero 1 AOE slash materials.
 *
 * The DOT moving-shot silhouette lives in the Niagara renderer/material/mesh
 * assets, not in actor-side geometry. It is a forward-travelling aura ring that
 * the single hero->target DOT shot transports, deliberately distinct from the AOE
 * radial crescent, the Pierce forward vertical lane, and the Bounce horizontal
 * slash. The persistent target-attached sphere markers remain a separate runtime
 * placeholder and are unchanged by this carrier.
 */
UCLASS()
class T66_API UT66Hero1AxeDOTVFXCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66Hero1AxeDOTVFXCommandlet();

	virtual int32 Main(const FString& Params) override;
};
