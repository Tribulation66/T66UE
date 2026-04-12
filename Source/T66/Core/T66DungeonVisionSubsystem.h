// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "T66DungeonVisionSubsystem.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;

/**
 * World-scoped post-process visibility mask for dungeon lighting.
 * Keeps gameplay materials Unlit while limiting scene visibility to a player-centered radius.
 */
UCLASS()
class T66_API UT66DungeonVisionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UT66DungeonVisionSubsystem* Get(UWorld* World);
	static void SetEnabledForWorld(UWorld* World, bool bEnabled);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	void SetEnabled(bool bInEnabled);
	bool IsEnabled() const { return bEnabled; }

	static UMaterialInterface* GetOrLoadDungeonVisionMaterial();

private:
	void EnsureBlendableInWorld();
	void ApplyParametersToBlendable();
	void UpdateTrackedOrigin();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DungeonVisionDMI;

	UPROPERTY()
	TObjectPtr<class APostProcessVolume> DungeonVisionVolume;

	bool bEnabled = false;
	FVector TrackedOrigin = FVector::ZeroVector;
	float VisionRadius = 2200.0f;
	float FalloffDistance = 650.0f;
	float AmbientVisibility = 0.0f;
};
