// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66StageCatchUpGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** Stage Catch Up exit gate: teleports run into the chosen difficulty start stage. */
UCLASS(Blueprintable)
class T66_API AT66StageCatchUpGate : public AActor
{
	GENERATED_BODY()

public:
	AT66StageCatchUpGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** Target stage to start at after Catch Up (e.g., 11/21/31...). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CatchUp")
	int32 TargetStage = 1;

	/** Called when player presses Interact (F) while overlapping. Returns true if transition triggered. */
	UFUNCTION(BlueprintCallable, Category = "CatchUp")
	bool EnterChosenStage();

protected:
	virtual void BeginPlay() override;
};
