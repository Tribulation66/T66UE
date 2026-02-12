// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66PixelationSubsystem.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
struct FPostProcessSettings;

/**
 * Applies retro pixelation via a post-process material blendable.
 * Only the 3D scene is pixelated; UI stays crisp at native resolution.
 * Console: Pixel0 (off), Pixel1..Pixel10 (level).
 */
UCLASS()
class T66_API UT66PixelationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Set pixelation level: 0 = off, 1 = least, 10 = most (10 = former slight). Affects current world's post-process. */
	UFUNCTION(BlueprintCallable, Category = "T66|Pixelation")
	void SetPixelationLevel(int32 Level);

	/** Current level (0 = off). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Pixelation")
	int32 GetPixelationLevel() const { return CurrentLevel; }

	/** Get or create the pixelation post-process material (loads asset or creates in editor). */
	static UMaterialInterface* GetOrCreatePixelationMaterial();

private:
	void EnsureBlendableInWorld(UWorld* World);
	void ApplyLevelToBlendable();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PixelationDMI;

	UPROPERTY()
	TObjectPtr<class APostProcessVolume> PixelationVolume;

	int32 CurrentLevel = 0;

	/** Level 1 = high grid (subtle), 10 = low grid (strong). */
	static int32 LevelToPixelGridSize(int32 Level);
};
