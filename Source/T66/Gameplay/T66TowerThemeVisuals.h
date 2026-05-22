// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66TowerMapTerrain.h"

class UObject;
class UMaterialInterface;
class UStaticMesh;

namespace T66TowerThemeVisuals
{
	enum class EWallFamily : uint8
	{
		SolidMaterial,
		MeshCluster,
		SplitCollisionVisual,
	};

	enum class EEnvironmentSurfaceType : uint8
	{
		WallXZ,
		WallYZ,
		Floor,
		Ceiling,
	};

	struct FResolvedTheme
	{
		FName ThemeName = NAME_None;
		UMaterialInterface* FloorMaterial = nullptr;
		UMaterialInterface* WallMaterial = nullptr;
		UMaterialInterface* RoofMaterial = nullptr;
		UMaterialInterface* WallXZMaterial = nullptr;
		UMaterialInterface* WallYZMaterial = nullptr;
		UMaterialInterface* CeilingMaterial = nullptr;
		UMaterialInterface* WallMeshMaterialOverride = nullptr;
		UMaterialInterface* DecorationMaterialOverride = nullptr;
		TArray<UStaticMesh*> WallMeshes;
		TArray<UStaticMesh*> FloorMeshes;
		TArray<UStaticMesh*> DecorationMeshes;
		EWallFamily WallFamily = EWallFamily::SolidMaterial;
		float CeilingOffset = 1600.0f;
		bool bBossFloor = false;
	};

	bool ResolveTheme(
		UObject* Outer,
		T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme,
		bool bBossFloor,
		FResolvedTheme& OutTheme);

	UMaterialInterface* ResolveEnvironmentSurfaceMaterial(
		UObject* Outer,
		T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme,
		EEnvironmentSurfaceType Surface);

	bool ResolveFloorTheme(
		UObject* Outer,
		const T66TowerMapTerrain::FFloor& Floor,
		FResolvedTheme& OutTheme);
}
