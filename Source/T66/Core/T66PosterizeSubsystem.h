// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66PosterizeSubsystem.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
struct FPostProcessSettings;

/**
 * Full-screen posterization via a post-process material blendable.
 * Reduces the color palette to discrete steps for a retro banded look.
 * Activated only in Dark theme (blood-red moon mode).
 */
UCLASS()
class T66_API UT66PosterizeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "T66|Posterize")
	void SetEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Posterize")
	bool IsEnabled() const { return bCurrentlyEnabled; }

	UFUNCTION(BlueprintCallable, Category = "T66|Posterize")
	void SetSteps(float InSteps);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Posterize")
	float GetSteps() const { return Steps; }

	UFUNCTION(BlueprintCallable, Category = "T66|Posterize")
	void SetIntensity(float InIntensity);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|Posterize")
	float GetIntensity() const { return Intensity; }

	static UMaterialInterface* GetOrCreatePosterizeMaterial();

private:
	void EnsureBlendableInWorld(UWorld* World);
	void ApplyParametersToBlendable();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PosterizeDMI;

	UPROPERTY()
	TObjectPtr<class APostProcessVolume> PosterizeVolume;

	bool bCurrentlyEnabled = false;
	float Steps = 10.f;
	float Intensity = 0.85f;
};
