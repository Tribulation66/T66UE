// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66Hero1AxeAOEVFXCommandlet.generated.h"

/**
 * Binds generated Hero 1 axe AOE lab materials into the lab Niagara system.
 *
 * This keeps the slash silhouette inside Niagara renderer/material assets rather
 * than actor-side geometry.
 */
UCLASS()
class T66_API UT66Hero1AxeAOEVFXCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66Hero1AxeAOEVFXCommandlet();

	virtual int32 Main(const FString& Params) override;
};
