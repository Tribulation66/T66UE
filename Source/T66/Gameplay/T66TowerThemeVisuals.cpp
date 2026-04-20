// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerThemeVisuals.h"

#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
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
				nullptr,
				nullptr,
				nullptr,
				TEXT("/Game/World/Terrain/MegabonkThemes/MediumOcean/T_MegabonkBlock_MediumOcean.T_MegabonkBlock_MediumOcean"),
				TEXT("/Game/World/Terrain/MegabonkThemes/MediumOcean/T_MegabonkSlope_MediumOcean.T_MegabonkSlope_MediumOcean")
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
			return {
				nullptr,
				nullptr,
				nullptr,
				TEXT("/Game/World/Terrain/MegabonkThemes/PerditionMars/T_MegabonkBlock_PerditionMars.T_MegabonkBlock_PerditionMars"),
				TEXT("/Game/World/Terrain/MegabonkThemes/PerditionMars/T_MegabonkSlope_PerditionMars.T_MegabonkSlope_PerditionMars")
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
			return {
				nullptr,
				nullptr,
				nullptr,
				TEXT("/Game/World/Terrain/MegabonkThemes/FinalHell/T_MegabonkBlock_FinalHell.T_MegabonkBlock_FinalHell"),
				TEXT("/Game/World/Terrain/MegabonkThemes/FinalHell/T_MegabonkSlope_FinalHell.T_MegabonkSlope_FinalHell")
			};
		case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
		default:
			return {
				TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonGround.MI_TowerDungeonGround"),
				TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonWall.MI_TowerDungeonWall"),
				TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof"),
				nullptr,
				nullptr
			};
		}
	}

	template <typename TObjectType>
	static TObjectType* T66FindOrLoadObject(const TCHAR* ObjectPath)
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
		return T66FindOrLoadObject<UMaterialInterface>(MaterialPath);
	}

	static UMaterialInterface* T66BuildThemeMaterialFromTexture(
		UObject* Outer,
		const TCHAR* TexturePath,
		const TCHAR* FallbackMaterialPath)
	{
		static TMap<FString, TWeakObjectPtr<UMaterialInterface>> CachedMaterials;

		if (!TexturePath || !*TexturePath)
		{
			return T66FindOrLoadObject<UMaterialInterface>(FallbackMaterialPath);
		}

		const FString CacheKey(TexturePath);
		if (const TWeakObjectPtr<UMaterialInterface>* Existing = CachedMaterials.Find(CacheKey))
		{
			if (Existing->IsValid())
			{
				return Existing->Get();
			}
		}

		UMaterialInterface* BaseMaterial = T66FindOrLoadObject<UMaterialInterface>(TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
		UTexture* ThemeTexture = T66FindOrLoadObject<UTexture>(TexturePath);
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

		UMaterialInterface* FallbackMaterial = T66FindOrLoadObject<UMaterialInterface>(FallbackMaterialPath);
		CachedMaterials.Add(CacheKey, FallbackMaterial);
		return FallbackMaterial;
	}

	static void T66AddLoadedMesh(TArray<UStaticMesh*>& OutMeshes, const TCHAR* MeshPath)
	{
		if (!MeshPath || !*MeshPath)
		{
			return;
		}

		if (UStaticMesh* Mesh = T66FindOrLoadObject<UStaticMesh>(MeshPath))
		{
			OutMeshes.Add(Mesh);
		}
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

	const FT66TowerThemeSurfacePaths SurfacePaths = T66GetThemeSurfacePaths(ThemeId);

	OutTheme.FloorMaterial = T66LoadThemeMaterial(Outer, SurfacePaths.FloorMaterialPath);
	OutTheme.WallMaterial = T66LoadThemeMaterial(Outer, SurfacePaths.WallMaterialPath);
	OutTheme.RoofMaterial = T66LoadThemeMaterial(Outer, SurfacePaths.RoofMaterialPath);

	if (!OutTheme.FloorMaterial)
	{
		OutTheme.FloorMaterial = T66BuildThemeMaterialFromTexture(
			Outer,
			SurfacePaths.BlockTexturePath,
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"));
	}
	if (!OutTheme.WallMaterial)
	{
		OutTheme.WallMaterial = T66BuildThemeMaterialFromTexture(
			Outer,
			SurfacePaths.BlockTexturePath,
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"));
	}
	if (!OutTheme.RoofMaterial)
	{
		OutTheme.RoofMaterial = T66BuildThemeMaterialFromTexture(
			Outer,
			SurfacePaths.RoofTexturePath ? SurfacePaths.RoofTexturePath : SurfacePaths.BlockTexturePath,
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope"));
	}

	switch (ThemeId)
	{
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
		OutTheme.ThemeName = TEXT("Forest");
		OutTheme.WallFamily = EWallFamily::MeshCluster;
		OutTheme.WallMeshMaterialOverride = nullptr;
		OutTheme.DecorationMaterialOverride = nullptr;
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Tree.Tree"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Tree2.Tree2"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Tree3.Tree3"));
		OutTheme.DecorationMeshes = OutTheme.WallMeshes;
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
		OutTheme.ThemeName = TEXT("Ocean");
		OutTheme.WallFamily = EWallFamily::MeshCluster;
		OutTheme.WallMeshMaterialOverride = OutTheme.WallMaterial;
		OutTheme.DecorationMaterialOverride = OutTheme.WallMaterial;
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Tree2.Tree2"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Tree3.Tree3"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Branch.Branch"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Tree2.Tree2"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Tree3.Tree3"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Branch.Branch"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
		OutTheme.ThemeName = TEXT("Martian");
		OutTheme.WallFamily = EWallFamily::MeshCluster;
		OutTheme.WallMeshMaterialOverride = OutTheme.WallMaterial;
		OutTheme.DecorationMaterialOverride = OutTheme.WallMaterial;
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Rock.Rock"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Boulder.Boulder"));
		OutTheme.DecorationMeshes = OutTheme.WallMeshes;
		break;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
		OutTheme.ThemeName = TEXT("Hell");
		OutTheme.WallFamily = EWallFamily::MeshCluster;
		OutTheme.WallMeshMaterialOverride = OutTheme.WallMaterial;
		OutTheme.DecorationMaterialOverride = OutTheme.WallMaterial;
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Branch.Branch"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Stump.Stump"));
		T66AddLoadedMesh(OutTheme.WallMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Branch.Branch"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Stump.Stump"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
	default:
		OutTheme.ThemeName = TEXT("Dungeon");
		OutTheme.WallFamily = EWallFamily::SolidMaterial;
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	}

	if (!OutTheme.FloorMaterial)
	{
		OutTheme.FloorMaterial = T66FindOrLoadObject<UMaterialInterface>(TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"));
	}
	if (!OutTheme.WallMaterial)
	{
		OutTheme.WallMaterial = T66FindOrLoadObject<UMaterialInterface>(TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"));
	}
	if (!OutTheme.RoofMaterial)
	{
		OutTheme.RoofMaterial = OutTheme.WallMaterial ? OutTheme.WallMaterial : OutTheme.FloorMaterial;
	}

	return OutTheme.FloorMaterial || OutTheme.WallMaterial || OutTheme.RoofMaterial;
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
