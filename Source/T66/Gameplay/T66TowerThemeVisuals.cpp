// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerThemeVisuals.h"

#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	static const TCHAR* T66EnvironmentLitMaterialPath = TEXT("/Game/Materials/M_Environment_Lit.M_Environment_Lit");
	static const TCHAR* T66CoherentThemeKitRoot = TEXT("/Game/World/Terrain/TowerDungeon/GeneratedKit/CoherentThemeKit01");

	struct FT66TowerThemeSurfacePaths
	{
		const TCHAR* FloorMaterialPath = nullptr;
		const TCHAR* WallMaterialPath = nullptr;
		const TCHAR* RoofMaterialPath = nullptr;
		const TCHAR* BlockTexturePath = nullptr;
		const TCHAR* RoofTexturePath = nullptr;
	};

	static FT66TowerThemeSurfacePaths T66GetThemeSurfacePaths(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme ThemeId)
	{
		switch (ThemeId)
		{
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
			return {
				TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestGround.MI_TowerForestGround"),
				nullptr,
				TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestRoof.MI_TowerForestRoof"),
				nullptr,
				nullptr
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
			return {
				TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestGround.MI_TowerForestGround"),
				nullptr,
				TEXT("/Game/World/Terrain/TowerForest/MI_TowerForestRoof.MI_TowerForestRoof"),
				nullptr,
				nullptr
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
			return {
				nullptr,
				nullptr,
				TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof"),
				nullptr,
				nullptr
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
			return {
				nullptr,
				nullptr,
				TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof"),
				nullptr,
				nullptr
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
		default:
			return {
				nullptr,
				nullptr,
				TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof"),
				nullptr,
				nullptr
			};
		}
	}

	template <typename TObjectType>
	static TObjectType* T66TowerFindOrLoadObject(const TCHAR* ObjectPath)
	{
		if (!ObjectPath || !*ObjectPath)
		{
			return nullptr;
		}

		if (TObjectType* Existing = FindObject<TObjectType>(nullptr, ObjectPath))
		{
			return Existing;
		}

		return LoadObject<TObjectType>(nullptr, ObjectPath);
	}

	static UMaterialInterface* T66LoadThemeMaterial(UObject* Outer, const TCHAR* MaterialPath)
	{
		return T66TowerFindOrLoadObject<UMaterialInterface>(MaterialPath);
	}

	static const TCHAR* T66ThemeNameForPath(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme ThemeId)
	{
		switch (ThemeId)
		{
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
			return TEXT("Forest");
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
			return TEXT("Ocean");
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
			return TEXT("Martian");
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
			return TEXT("Hell");
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
		default:
			return TEXT("Dungeon");
		}
	}

	static const TCHAR* T66SurfaceNameForPath(const T66TowerThemeVisuals::EEnvironmentSurfaceType Surface)
	{
		switch (Surface)
		{
		case T66TowerThemeVisuals::EEnvironmentSurfaceType::WallYZ:
			return TEXT("Wall_YZ");
		case T66TowerThemeVisuals::EEnvironmentSurfaceType::Floor:
			return TEXT("Floor");
		case T66TowerThemeVisuals::EEnvironmentSurfaceType::Ceiling:
			return TEXT("Ceiling");
		case T66TowerThemeVisuals::EEnvironmentSurfaceType::WallXZ:
		default:
			return TEXT("Wall_XZ");
		}
	}

	static FName T66ThemeDisplayName(const T66TowerMapTerrain::ET66TowerGameplayLevelTheme ThemeId)
	{
		return FName(T66ThemeNameForPath(ThemeId));
	}

	static UMaterialInterface* T66BuildThemeMaterialFromTexture(
		UObject* Outer,
		const TCHAR* TexturePath,
		const TCHAR* FallbackMaterialPath)
	{
		static TMap<FString, TWeakObjectPtr<UMaterialInterface>> CachedMaterials;

		if (!TexturePath || !*TexturePath)
		{
			return T66TowerFindOrLoadObject<UMaterialInterface>(FallbackMaterialPath);
		}

		const FString CacheKey(TexturePath);
		if (const TWeakObjectPtr<UMaterialInterface>* Existing = CachedMaterials.Find(CacheKey))
		{
			if (Existing->IsValid())
			{
				return Existing->Get();
			}
		}

		UMaterialInterface* BaseMaterial = T66TowerFindOrLoadObject<UMaterialInterface>(T66EnvironmentLitMaterialPath);
		UTexture* ThemeTexture = T66TowerFindOrLoadObject<UTexture>(TexturePath);
		if (BaseMaterial && ThemeTexture)
		{
			if (UMaterialInstanceDynamic* ThemeMID = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : GetTransientPackage()))
			{
				ThemeMID->SetTextureParameterValue(TEXT("DiffuseColorMap"), ThemeTexture);
				ThemeMID->SetTextureParameterValue(TEXT("BaseColorTexture"), ThemeTexture);
				ThemeMID->SetScalarParameterValue(TEXT("Brightness"), 1.0f);
				ThemeMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
				ThemeMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
				CachedMaterials.Add(CacheKey, ThemeMID);
				return ThemeMID;
			}
		}

		UMaterialInterface* FallbackMaterial = T66TowerFindOrLoadObject<UMaterialInterface>(FallbackMaterialPath);
		CachedMaterials.Add(CacheKey, FallbackMaterial);
		return FallbackMaterial;
	}

	static void T66AddLoadedMesh(TArray<UStaticMesh*>& OutMeshes, const TCHAR* MeshPath)
	{
		if (!MeshPath || !*MeshPath)
		{
			return;
		}

		if (UStaticMesh* Mesh = T66TowerFindOrLoadObject<UStaticMesh>(MeshPath))
		{
			OutMeshes.Add(Mesh);
		}
	}

	static void T66AddGeneratedThemeKitMesh(TArray<UStaticMesh*>& OutMeshes, const TCHAR* ModuleId)
	{
		const FString MeshPath = FString::Printf(
			TEXT("%s/%s_UnrealReady.%s_UnrealReady"),
			T66CoherentThemeKitRoot,
			ModuleId,
			ModuleId);
		T66AddLoadedMesh(OutMeshes, *MeshPath);
	}

	static void T66AddGeneratedThemeKitMeshes(
		TArray<UStaticMesh*>& OutMeshes,
		const TCHAR* const* ModuleIds,
		const int32 ModuleCount)
	{
		for (int32 Index = 0; Index < ModuleCount; ++Index)
		{
			T66AddGeneratedThemeKitMesh(OutMeshes, ModuleIds[Index]);
		}
	}

	static void T66ConfigureGeneratedThemeKit(
		T66TowerThemeVisuals::FResolvedTheme& OutTheme,
		const TCHAR* const* WallModules,
		const int32 WallCount,
		const TCHAR* const* FloorModules,
		const int32 FloorCount)
	{
		OutTheme.WallFamily = T66TowerThemeVisuals::EWallFamily::SplitCollisionVisual;
		OutTheme.WallMeshMaterialOverride = nullptr;
		T66AddGeneratedThemeKitMeshes(OutTheme.WallMeshes, WallModules, WallCount);
		T66AddGeneratedThemeKitMeshes(OutTheme.FloorMeshes, FloorModules, FloorCount);
	}
}

bool T66TowerThemeVisuals::ResolveTheme(
	UObject* Outer,
	const T66TowerMapTerrain::ET66TowerGameplayLevelTheme ThemeId,
	const bool bBossFloor,
	FResolvedTheme& OutTheme)
{
	OutTheme = FResolvedTheme{};
	OutTheme.bBossFloor = bBossFloor;
	OutTheme.ThemeName = T66ThemeDisplayName(ThemeId);

	OutTheme.WallFamily = EWallFamily::SplitCollisionVisual;
	OutTheme.WallXZMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::WallXZ);
	OutTheme.WallYZMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::WallYZ);
	OutTheme.FloorMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::Floor);
	OutTheme.CeilingMaterial = ResolveEnvironmentSurfaceMaterial(Outer, ThemeId, EEnvironmentSurfaceType::Ceiling);
	OutTheme.WallMaterial = OutTheme.WallXZMaterial ? OutTheme.WallXZMaterial : OutTheme.WallYZMaterial;
	OutTheme.RoofMaterial = OutTheme.CeilingMaterial ? OutTheme.CeilingMaterial : OutTheme.FloorMaterial;
	OutTheme.DecorationMaterialOverride = OutTheme.WallMaterial;

	if (!OutTheme.FloorMaterial)
	{
		OutTheme.FloorMaterial = T66TowerFindOrLoadObject<UMaterialInterface>(T66EnvironmentLitMaterialPath);
	}
	if (!OutTheme.WallMaterial)
	{
		OutTheme.WallMaterial = T66TowerFindOrLoadObject<UMaterialInterface>(T66EnvironmentLitMaterialPath);
	}
	if (!OutTheme.RoofMaterial)
	{
		OutTheme.RoofMaterial = OutTheme.WallMaterial ? OutTheme.WallMaterial : OutTheme.FloorMaterial;
	}

	return OutTheme.FloorMaterial || OutTheme.WallMaterial || OutTheme.RoofMaterial;
}

UMaterialInterface* T66TowerThemeVisuals::ResolveEnvironmentSurfaceMaterial(
	UObject* Outer,
	const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme,
	const EEnvironmentSurfaceType Surface)
{
	const FString MaterialPath = FString::Printf(
		TEXT("/Game/ToonStyle/Environment/%s/Materials/MI_%s_%s.MI_%s_%s"),
		T66ThemeNameForPath(Theme),
		T66ThemeNameForPath(Theme),
		T66SurfaceNameForPath(Surface),
		T66ThemeNameForPath(Theme),
		T66SurfaceNameForPath(Surface));
	return T66LoadThemeMaterial(Outer, *MaterialPath);
}

bool T66TowerThemeVisuals::ResolveFloorTheme(
	UObject* Outer,
	const T66TowerMapTerrain::FFloor& Floor,
	FResolvedTheme& OutTheme)
{
	return ResolveTheme(
		Outer,
		Floor.Theme,
		Floor.FloorRole == T66TowerMapTerrain::ET66TowerFloorRole::Boss,
		OutTheme);
}
