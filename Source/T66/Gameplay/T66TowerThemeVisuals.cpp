// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerThemeVisuals.h"

#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	static const TCHAR* T66EnvironmentUnlitMaterialPath = TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit");
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

		UMaterialInterface* BaseMaterial = T66TowerFindOrLoadObject<UMaterialInterface>(TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
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

	const FT66TowerThemeSurfacePaths SurfacePaths = T66GetThemeSurfacePaths(ThemeId);

	OutTheme.FloorMaterial = T66LoadThemeMaterial(Outer, SurfacePaths.FloorMaterialPath);
	OutTheme.WallMaterial = T66LoadThemeMaterial(Outer, SurfacePaths.WallMaterialPath);
	OutTheme.RoofMaterial = T66LoadThemeMaterial(Outer, SurfacePaths.RoofMaterialPath);

	if (!OutTheme.FloorMaterial)
	{
		OutTheme.FloorMaterial = T66BuildThemeMaterialFromTexture(
			Outer,
			SurfacePaths.BlockTexturePath,
			T66EnvironmentUnlitMaterialPath);
	}
	if (!OutTheme.WallMaterial)
	{
		OutTheme.WallMaterial = T66BuildThemeMaterialFromTexture(
			Outer,
			SurfacePaths.BlockTexturePath,
			T66EnvironmentUnlitMaterialPath);
	}
	if (!OutTheme.RoofMaterial)
	{
		OutTheme.RoofMaterial = T66BuildThemeMaterialFromTexture(
			Outer,
			SurfacePaths.RoofTexturePath ? SurfacePaths.RoofTexturePath : SurfacePaths.BlockTexturePath,
			TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonRoof.MI_TowerDungeonRoof"));
	}

	switch (ThemeId)
	{
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Forest:
	{
		OutTheme.ThemeName = TEXT("Forest");
		static const TCHAR* WallModules[] = {
			TEXT("ForestWall_VineTotem_A"),
			TEXT("ForestWall_TrunkWeave_A"),
			TEXT("ForestWall_RootBraid_A"),
			TEXT("ForestWall_MushroomBark_A"),
		};
		static const TCHAR* FloorModules[] = {
			TEXT("ForestFloor_RootMat_A"),
			TEXT("ForestFloor_MossStone_A"),
			TEXT("ForestFloor_LeafCrack_A"),
			TEXT("ForestFloor_BrambleEdge_A"),
		};
		T66ConfigureGeneratedThemeKit(OutTheme, WallModules, UE_ARRAY_COUNT(WallModules), FloorModules, UE_ARRAY_COUNT(FloorModules));
		OutTheme.DecorationMaterialOverride = nullptr;
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Tree.Tree"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Tree2.Tree2"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Tree3.Tree3"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	}
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Ocean:
	{
		OutTheme.ThemeName = TEXT("Ocean");
		static const TCHAR* WallModules[] = {
			TEXT("OceanWall_CoralReef_A"),
			TEXT("OceanWall_ShellLimestone_A"),
			TEXT("OceanWall_KelpCoral_A"),
			TEXT("OceanWall_ReefRuin_A"),
		};
		static const TCHAR* FloorModules[] = {
			TEXT("OceanFloor_ReefStone_A"),
			TEXT("OceanFloor_ShellSand_A"),
			TEXT("OceanFloor_CoralCrack_A"),
			TEXT("OceanFloor_TidePool_A"),
		};
		T66ConfigureGeneratedThemeKit(OutTheme, WallModules, UE_ARRAY_COUNT(WallModules), FloorModules, UE_ARRAY_COUNT(FloorModules));
		OutTheme.DecorationMaterialOverride = OutTheme.WallMaterial;
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Tree2.Tree2"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Tree3.Tree3"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Branch.Branch"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	}
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Martian:
	{
		OutTheme.ThemeName = TEXT("Martian");
		OutTheme.DecorationMaterialOverride = OutTheme.WallMaterial;
		static const TCHAR* WallModules[] = {
			TEXT("MartianWall_RuinPanel_A"),
			TEXT("MartianWall_RedRock_A"),
			TEXT("MartianWall_MeteorScar_A"),
			TEXT("MartianWall_CrystalVein_A"),
		};
		static const TCHAR* FloorModules[] = {
			TEXT("MartianFloor_RuinTile_A"),
			TEXT("MartianFloor_RegolithPlates_A"),
			TEXT("MartianFloor_CrystalDust_A"),
			TEXT("MartianFloor_CraterCracks_A"),
		};
		T66ConfigureGeneratedThemeKit(OutTheme, WallModules, UE_ARRAY_COUNT(WallModules), FloorModules, UE_ARRAY_COUNT(FloorModules));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rock.Rock"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Boulder.Boulder"));
		break;
	}
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell:
	{
		OutTheme.ThemeName = TEXT("Hell");
		OutTheme.DecorationMaterialOverride = OutTheme.WallMaterial;
		static const TCHAR* WallModules[] = {
			TEXT("HellWall_SpikeBasalt_A"),
			TEXT("HellWall_LavaCrack_A"),
			TEXT("HellWall_ChainsSkulls_A"),
			TEXT("HellWall_Brimstone_A"),
		};
		static const TCHAR* FloorModules[] = {
			TEXT("HellFloor_RunePlate_A"),
			TEXT("HellFloor_Obsidian_A"),
			TEXT("HellFloor_EmberFissure_A"),
			TEXT("HellFloor_BoneAsh_A"),
		};
		T66ConfigureGeneratedThemeKit(OutTheme, WallModules, UE_ARRAY_COUNT(WallModules), FloorModules, UE_ARRAY_COUNT(FloorModules));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Branch.Branch"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Stump.Stump"));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	}
	case T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon:
	default:
	{
		OutTheme.ThemeName = TEXT("Dungeon");
		static const TCHAR* WallModules[] = {
			TEXT("DungeonWall_TorchSconce_A"),
			TEXT("DungeonWall_StoneBlocks_A"),
			TEXT("DungeonWall_Chains_A"),
			TEXT("DungeonWall_BonesNiche_A"),
		};
		static const TCHAR* FloorModules[] = {
			TEXT("DungeonFloor_StoneSlabs_A"),
			TEXT("DungeonFloor_Drain_A"),
			TEXT("DungeonFloor_Cracked_A"),
			TEXT("DungeonFloor_Bones_A"),
		};
		T66ConfigureGeneratedThemeKit(OutTheme, WallModules, UE_ARRAY_COUNT(WallModules), FloorModules, UE_ARRAY_COUNT(FloorModules));
		T66AddLoadedMesh(OutTheme.DecorationMeshes, TEXT("/Game/World/Props/Rocks.Rocks"));
		break;
	}
	}

	if (!OutTheme.FloorMaterial)
	{
		OutTheme.FloorMaterial = T66TowerFindOrLoadObject<UMaterialInterface>(T66EnvironmentUnlitMaterialPath);
	}
	if (!OutTheme.WallMaterial)
	{
		OutTheme.WallMaterial = T66TowerFindOrLoadObject<UMaterialInterface>(T66EnvironmentUnlitMaterialPath);
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
