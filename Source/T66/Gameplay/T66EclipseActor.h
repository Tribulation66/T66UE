// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66EclipseActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Eclipse corona visual — a billboard plane with a procedural ring-of-fire
 * material positioned far along the moon directional light's direction.
 * Spawned in Dark mode, destroyed/hidden in Light mode.
 */
UCLASS()
class T66_API AT66EclipseActor : public AActor
{
	GENERATED_BODY()

public:
	AT66EclipseActor();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eclipse")
	TObjectPtr<UStaticMeshComponent> PlaneMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoronaDMI;

	void SetCoronaIntensity(float Intensity);

protected:
	virtual void BeginPlay() override;

private:
	void OrientTowardCamera();
};
