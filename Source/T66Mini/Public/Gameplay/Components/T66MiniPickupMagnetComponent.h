// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66MiniPickupMagnetComponent.generated.h"

UCLASS(ClassGroup=(Mini), meta=(BlueprintSpawnableComponent))
class T66MINI_API UT66MiniPickupMagnetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MiniPickupMagnetComponent();

	float GetMagnetRadius() const { return MagnetRadius; }
	float GetPullSpeed() const { return PullSpeed; }
	void SetMagnetProfile(float InMagnetRadius, float InPullSpeed);

private:
	float MagnetRadius = 420.f;
	float PullSpeed = 620.f;
};
