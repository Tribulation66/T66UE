// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class AActor;
class USceneComponent;
class UStaticMeshComponent;

enum class ET66PreviewStageMode : uint8
{
	Selection,
	RunSummary
};

namespace T66PreviewStageEnvironment
{
	void EnsurePreviewEnvironmentBuilt(
		AActor* Owner,
		USceneComponent* RootComponent);

	void ApplyPreviewEnvironment(
		AActor* Owner,
		UStaticMeshComponent* GroundComponent,
		ET66Difficulty Difficulty,
		ET66PreviewStageMode PreviewStageMode,
		bool bVisible);
}
