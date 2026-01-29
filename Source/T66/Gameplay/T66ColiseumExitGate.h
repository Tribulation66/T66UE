// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66ColiseumExitGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** Gate that exits Coliseum back to GameplayLevel (does not increment stage). */
UCLASS(Blueprintable)
class T66_API AT66ColiseumExitGate : public AActor
{
	GENERATED_BODY()

public:
	AT66ColiseumExitGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	bool Interact(APlayerController* PC);
};

