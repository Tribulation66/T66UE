// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66NiagaraIsolationCaptureCommandlet.generated.h"

/**
 * Captures a Niagara system in an isolated editor preview scene for VFX review.
 *
 * This is diagnostic editor tooling only. It must not create gameplay damage,
 * actor-side VFX silhouettes, or production content.
 */
UCLASS()
class T66EDITOR_API UT66NiagaraIsolationCaptureCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66NiagaraIsolationCaptureCommandlet();

	virtual int32 Main(const FString& Params) override;
};
