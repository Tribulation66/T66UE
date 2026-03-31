// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once
#include "ZibraVDBCommon.h"
#include "Components/PrimitiveComponent.h"

#include "ZibraVDBReflectionsManagerComponent.generated.h"



UCLASS(Blueprintable, hideCategories = (Activation, Collision, Cooking, Tags, AssetUserData, Navigation))
class ZIBRAVDBRUNTIME_API UZibraVdbReflectionsManagerComponent final : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) final;
	void OnRegister() final;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	void AddActorToReflectionsList(AActor* Actor);

	/** List of meshes that will get reflections of the volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Reflective Meshes", Category = "Reflections", meta = (NoResetToDefault))
	TArray<AActor*> ActorsToApplyReflections;

	/** Reflections quality */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflections")
	ResolutionDownscaleFactors ReflectionsResolution = ResolutionDownscaleFactors::Quarter;

	/** Maximum steps for Global Distance Field raymarching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflections", meta = (ClampMin = "8", UIMin = "1", ClampMax = "512", UIMax = "4096"))
	int32 MaxGDFRaymarchSteps = 64;

	/** Maximum steps for volume raymarching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflections", meta = (ClampMin = "8", UIMin = "1", ClampMax = "512", UIMax = "4096"))
	int32 MaxVolumeRaymarchSteps = 64;

	/** Minimum step size for volume raymarching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reflections", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "0.1"))
	float MinVolumeRaymarchDistance = 0.01;


private:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void CheckOwnerVisibilityChanged();
	void ApplyReflectionsMaterial(AActor* Target);
	void RemoveReflectionsMaterial(AActor* Target);
	void UpdateRayMarchParams();

	bool bInitialized;
	bool bGDFEnabled;
	TObjectPtr<UMaterialInterface> BaseMaterial;
	TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;
	TArray<AActor*> PreviousActorsToApplyReflections;
};
