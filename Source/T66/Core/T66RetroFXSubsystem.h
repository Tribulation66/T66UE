// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/T66RetroFXSettings.h"
#include "T66RetroFXSubsystem.generated.h"

class AActor;
class APostProcessVolume;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMaterialParameterCollection;
class UMeshComponent;

UENUM()
enum class ET66RetroGeometryGroup : uint8
{
	World,
	Character
};

USTRUCT()
struct FT66RetroManagedMaterialSlot
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UMeshComponent> MeshComponent;

	UPROPERTY()
	int32 MaterialIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> OriginalMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> RetroMaterial;

	UPROPERTY()
	ET66RetroGeometryGroup Group = ET66RetroGeometryGroup::World;
};

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
	void EnsurePs1PostProcessDMI(const FT66RetroFXSettings& Settings);
	void ApplyBlendableWeights(const FT66RetroFXSettings& Settings);
	void ApplyPs1Parameters(const FT66RetroFXSettings& Settings);
	void ApplyChromaticAberrationParameters(const FT66RetroFXSettings& Settings);
	void ApplyN64Parameters(const FT66RetroFXSettings& Settings);
	void ApplyResolutionCollection(const FT66RetroFXSettings& Settings, UWorld* World);
	void ApplyResolutionRuntime(const FT66RetroFXSettings& Settings, UWorld* World);
	void ApplyGeometryCollection(const FT66RetroFXSettings& Settings, UWorld* World);
	void ApplyGeometryMaterials(const FT66RetroFXSettings& Settings, UWorld* World);
	void RefreshWorldGeometryMaterials(UWorld* World, bool bEnableWorldGeometry, bool bEnableCharacterGeometry);
	void RefreshActorGeometryMaterials(AActor* Actor, bool bEnableWorldGeometry, bool bEnableCharacterGeometry);
	void RefreshMeshComponentGeometryMaterials(UMeshComponent* MeshComponent, bool bEnableWorldGeometry, bool bEnableCharacterGeometry);
	void RestoreManagedMaterials(bool bRestoreWorldGeometry, bool bRestoreCharacterGeometry);
	void RestoreManagedSlot(int32 SlotIndex);
	void CleanupManagedSlots();
	int32 FindManagedSlotIndex(const UMeshComponent* MeshComponent, int32 MaterialIndex) const;
	void UpdateGeometrySpawnBinding(UWorld* World, bool bShouldListen);
	void HandleActorSpawned(AActor* Actor);

	UMaterialInterface* LoadPs1PostProcessMaterial();
	UMaterialInterface* LoadPs1PostProcessMaterialVariant(const FT66RetroFXSettings& Settings);
	UMaterialInterface* LoadChromaticAberrationMaterial();
	UMaterialInterface* LoadN64BlurMaterial(bool bReplaceTonemapper);
	UMaterialParameterCollection* LoadResolutionCollection();
	UMaterialParameterCollection* LoadGeometryCollection();
	UMaterialInterface* LoadCharacterRetroGeometryMaterial();
	UMaterialInterface* LoadEnvironmentRetroGeometryMaterial();
	UMaterialInterface* LoadFbxRetroGeometryMaterial();
	UMaterialInterface* LoadGlbRetroGeometryMaterial();
	UMaterialInterface* ResolveRetroGeometryMaterial(UMaterialInterface* SourceMaterial, ET66RetroGeometryGroup& OutGroup);

	UMaterialInstanceDynamic* GetOrCreateDMI(UMaterialInterface* BaseMaterial, TObjectPtr<UMaterialInstanceDynamic>& CachedDMI);
	void EnsureBlendableEntry(UMaterialInstanceDynamic* DMI);
	void RemoveBlendableEntry(UObject* BlendableObject);
	void SetBlendableWeight(UMaterialInstanceDynamic* DMI, float Weight);
	void SetScalarParameter(UMaterialInstanceDynamic* DMI, FName ParamName, float Value);
	void CaptureResolutionRuntimeDefaults();
	void RestoreResolutionRuntimeDefaults();
	float GetViewportHeight(UWorld* World) const;
	void ExecuteConsoleCommand(UWorld* World, const FString& Command) const;

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
	TObjectPtr<UMaterialInstanceDynamic> ChromaticAberrationDMI;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> ResolutionCollection;

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> GeometryCollection;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> CharacterRetroGeometryMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> EnvironmentRetroGeometryMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> FbxRetroGeometryMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> GlbRetroGeometryMaterial;

	UPROPERTY()
	TObjectPtr<UWorld> ManagedGeometryWorld;

	UPROPERTY()
	TArray<FT66RetroManagedMaterialSlot> ManagedGeometrySlots;

	FDelegateHandle GeometrySpawnHandle;
	bool bWorldGeometryActive = false;
	bool bCharacterGeometryActive = false;
	bool bResolutionRuntimeDefaultsCaptured = false;
	bool bResolutionRuntimeActive = false;
	FString ActivePs1MaterialPath;
	float OriginalScreenPercentage = 100.0f;
	float OriginalSecondaryScreenPercentage = 100.0f;
	int32 OriginalUpscaleQuality = 0;
};
