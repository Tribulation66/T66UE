// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66StageBoostGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** Stage Boost exit gate: teleports run into the chosen difficulty start stage. */
UCLASS(Blueprintable)
class T66_API AT66StageBoostGate : public AActor
{
	GENERATED_BODY()

public:
	AT66StageBoostGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** Target stage to start at after Boost (e.g., 11/21/31...). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	int32 TargetStage = 1;

	/** Called when player presses Interact (F) while overlapping. Returns true if transition triggered. */
	UFUNCTION(BlueprintCallable, Category = "Boost")
	bool EnterChosenStage();

protected:
	virtual void BeginPlay() override;
};

