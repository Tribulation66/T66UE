#pragma once

#include "CoreMinimal.h"

class APostProcessVolume;
class UMaterialInstanceDynamic;
class UMeshComponent;
class UWorld;
struct FT66ThemeAtmosphereSpec;

namespace T66TowerMapTerrain
{
	enum class ET66TowerGameplayLevelTheme : uint8;
}


enum class ET66ToonMaterialKind : uint8
{
	Character,
	Outline,
	Environment
};
class T66_API FT66WorldVisualSetup
{
public:
	static void EnsureNeutralVisualSetupForWorld(UWorld* World);
	static void EnsureAtmosphereSkyLightForWorld(UWorld* World);
	static void EnsureAtmosphereForWorld(UWorld* World, T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme);
	static void ApplyAtmosphereToHeroCarryLights(UWorld* World, const FT66ThemeAtmosphereSpec& Spec);
	static APostProcessVolume* FindOrCreateRuntimePostProcessVolume(UWorld* World);
	static UMaterialInstanceDynamic* RegisterToonMaterial(UMeshComponent* Component, ET66ToonMaterialKind Kind, int32 MaterialIndex = 0);
	static void UnregisterToonMaterial(UMeshComponent* Component);
	static int32 ApplyToonCelAtmosphereToRegisteredMaterials(T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme);
	static int32 UpdateToonOutlineViewParameters(UWorld* World);
};
