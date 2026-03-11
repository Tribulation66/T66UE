// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/T66RetroFXSettings.h"
#include "T66RetroFXSubsystem.generated.h"

class APostProcessVolume;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMaterialParameterCollection;

/**
 * Runtime owner for experimental retro blendables and geometry collections.
 * The subsystem only touches effects that are safe to drive from C++
 * and gracefully no-ops when the UE5RFX assets are not installed.
 */
UCLASS()
class T66_API UT66RetroFXSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Apply the currently persisted player Retro FX settings to the target world. */
	void ApplyCurrentSettings(UWorld* World = nullptr);

	/** Apply a specific settings snapshot to the target world. */
	void ApplySettings(const FT66RetroFXSettings& Settings, UWorld* World = nullptr);

private:
	void EnsureBlendablesInWorld(UWorld* World);
	void ApplyBlendableWeights(const FT66RetroFXSettings& Settings);
	void ApplyPs1Parameters(const FT66RetroFXSettings& Settings);
	void ApplyN64Parameters(const FT66RetroFXSettings& Settings);
	void ApplyResolutionCollection(const FT66RetroFXSettings& Settings, UWorld* World);
	void ApplyGeometryCollection(const FT66RetroFXSettings& Settings, UWorld* World);

	UMaterialInterface* LoadPs1PostProcessMaterial();
	UMaterialInterface* LoadN64BlurMaterial(bool bReplaceTonemapper);
	UMaterialInterface* LoadCRTMaterial();
	UMaterialParameterCollection* LoadResolutionCollection();
	UMaterialParameterCollection* LoadGeometryCollection();

	UMaterialInstanceDynamic* GetOrCreateDMI(UMaterialInterface* BaseMaterial, TObjectPtr<UMaterialInstanceDynamic>& CachedDMI);
	void EnsureBlendableEntry(UMaterialInstanceDynamic* DMI);
	void SetBlendableWeight(UMaterialInstanceDynamic* DMI, float Weight);
	void SetScalarParameter(UMaterialInstanceDynamic* DMI, FName ParamName, float Value);

	UWorld* ResolveWorld(UWorld* PreferredWorld) const;

private:
	UPROPERTY()
	TObjectPtr<APostProcessVolume> ActiveVolume;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Ps1PostProcessDMI;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> N64BlurDMI;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> N64BlurReplaceTonemapperDMI;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CRTDMI;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> ResolutionCollection;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> GeometryCollection;
};
