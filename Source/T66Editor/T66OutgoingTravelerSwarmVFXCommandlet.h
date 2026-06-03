// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66OutgoingTravelerSwarmVFXCommandlet.generated.h"

/**
 * Builds the isolated Phase 0 Niagara proof for array-fed outgoing travelers.
 *
 * This intentionally creates only a render proof asset. It does not touch the
 * projectile manager, combat damage, targeting, or gameplay spawn paths.
 */
UCLASS()
class T66EDITOR_API UT66OutgoingTravelerSwarmVFXCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66OutgoingTravelerSwarmVFXCommandlet();

	virtual int32 Main(const FString& Params) override;
};
