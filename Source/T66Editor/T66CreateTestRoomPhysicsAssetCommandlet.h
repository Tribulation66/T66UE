// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "T66CreateTestRoomPhysicsAssetCommandlet.generated.h"

/**
 * Generates the temporary Hero 1 Chad PhysicsAsset used by the TestRoom ragdoll spike.
 */
UCLASS()
class T66EDITOR_API UT66CreateTestRoomPhysicsAssetCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UT66CreateTestRoomPhysicsAssetCommandlet();

	virtual int32 Main(const FString& Params) override;
};
