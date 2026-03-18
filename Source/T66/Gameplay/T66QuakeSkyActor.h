// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66QuakeSkyActor.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UProceduralMeshComponent;

/**
 * Camera-centered retro sky canopy that renders moving Quake-style cloud layers overhead only.
 * It is excluded from sky/reflection captures so it stays purely visual.
 */
UCLASS(NotBlueprintable)
class T66_API AT66QuakeSkyActor : public AActor
{
	GENERATED_BODY()

public:
	AT66QuakeSkyActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void BuildCanopyMesh();
	void UpdateSkyLocation();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> CloudCanopy;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> SkyMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkyMaterialInstance;
};
