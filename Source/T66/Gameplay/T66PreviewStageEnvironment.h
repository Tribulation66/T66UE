// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class AActor;
class USceneComponent;
class UStaticMeshComponent;

namespace T66PreviewStageEnvironment
{
	void ApplyPreviewGroundMaterial(UStaticMeshComponent* GroundComponent);

	void CreateEasyFarmPreviewProps(
		AActor* Owner,
		USceneComponent* RootComponent,
		TArray<TObjectPtr<UStaticMeshComponent>>& OutComponents);

	void SetPreviewPropsVisibility(
		const TArray<TObjectPtr<UStaticMeshComponent>>& Props,
		bool bVisible);

	bool ShouldShowEasyProps(ET66Difficulty Difficulty);
}
