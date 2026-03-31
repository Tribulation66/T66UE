// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66IdolVFXPreviewDummy.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;

UCLASS(NotBlueprintable)
class T66_API AT66IdolVFXPreviewDummy : public AActor
{
	GENERATED_BODY()

public:
	AT66IdolVFXPreviewDummy();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;
};
