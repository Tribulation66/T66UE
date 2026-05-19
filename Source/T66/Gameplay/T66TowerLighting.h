// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "T66TowerLighting.generated.h"

class UPointLightComponent;
class USceneComponent;

UCLASS()
class T66_API AT66TowerLightingActor : public AActor
{
	GENERATED_BODY()

public:
	AT66TowerLightingActor();

	void SetFloorNumber(int32 InFloorNumber);
	int32 GetFloorNumber() const { return FloorNumber; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	int32 FloorNumber = INDEX_NONE;
};

namespace T66TowerLighting
{
	T66_API AT66TowerLightingActor* SpawnFloorTorchLights(
		UWorld* World,
		const T66TowerMapTerrain::FFloor& Floor,
		const T66TowerMapTerrain::FLayout& Layout,
		T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme,
		AActor* AttachOwner);

	T66_API void DestroyFloorTorchLights(UWorld* World, const T66TowerMapTerrain::FFloor& Floor);
}
