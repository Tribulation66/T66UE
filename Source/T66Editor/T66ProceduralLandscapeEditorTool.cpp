// Copyright Tribulation 66. All Rights Reserved.

#include "T66ProceduralLandscapeEditorTool.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Gameplay/T66ProceduralLandscapeGenerator.h"
#include "T66Editor.h"

#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"
#include "LandscapeLayerInfoObject.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#if WITH_EDITOR
#include "Materials/Material.h"
#include "UObject/SavePackage.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#endif

#define LOCTEXT_NAMESPACE "T66ProceduralLandscape"

/** Actor tag for procedural foliage so we can remove it when regenerating. */
static const FName T66ProceduralFoliageTag(TEXT("T66ProceduralFoliage"));

namespace T66ProceduralLandscapeEditor
{
	static constexpr int32 SectionSizeQuads = 63;
	static constexpr int32 SectionsPerComponent = 1;
	static constexpr int32 QuadsPerComponent = SectionSizeQuads * SectionsPerComponent;

		void PlacePolytopeFoliage(UWorld* World, const TArray<float>& Heights, int32 SizeX, int32 SizeY,
		float OriginX, float OriginY, float QuadSizeUU, int32 Seed, FName FoliageTag);

#if WITH_EDITOR
		/** Set bUsedWithInstancedStaticMeshes on Polytope base materials so they render correctly with HISM outside the editor. */
		void FixPolytopeMaterialUsageFlags();
#endif

	bool GenerateProceduralHillsLandscape(UWorld* World, const FT66ProceduralLandscapeParams& Params)
	{
		if (!World)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: World is null"));
			return false;
		}

		int32 SizeX = 0, SizeY = 0;
		FT66ProceduralLandscapeGenerator::GetDimensionsForPreset(Params.SizePreset, SizeX, SizeY);

		// Origin so main map covers play area; Coliseum/Boost/Tutorial stay as islands to the side
		const FVector LandscapeOrigin(Params.LandscapeOriginX, Params.LandscapeOriginY, 0.f);
		FT66ProceduralLandscapeParams ParamsWithOrigin = Params;
		ParamsWithOrigin.LandscapeOriginX = LandscapeOrigin.X;
		ParamsWithOrigin.LandscapeOriginY = LandscapeOrigin.Y;

		TArray<float> Heights;
		if (!FT66ProceduralLandscapeGenerator::GenerateHeightfield(
			ParamsWithOrigin, SizeX, SizeY, FT66ProceduralLandscapeGenerator::DefaultQuadSizeUU, Heights))
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: GenerateHeightfield failed"));
			return false;
		}

		TArray<uint16> HeightData;
		const float ScaleZ = 100.f;
		FT66ProceduralLandscapeGenerator::FloatsToLandscapeHeightData(Heights, ScaleZ, HeightData);

		if (HeightData.Num() != SizeX * SizeY)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: height data size mismatch"));
			return false;
		}

		// Debug: log height range to verify variation
		uint16 MinH = 65535, MaxH = 0;
		for (uint16 H : HeightData)
		{
			MinH = FMath::Min(MinH, H);
			MaxH = FMath::Max(MaxH, H);
		}
		UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: height range %u - %u (flat=32768)"), MinH, MaxH);

		// Remove existing procedural foliage (tagged) so we don't duplicate when re-running
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->ActorHasTag(T66ProceduralFoliageTag))
			{
				It->Destroy();
			}
		}

		// Remove any existing Landscape(s) so we always create fresh
		TArray<ALandscape*> ExistingLandscapes;
		for (TActorIterator<ALandscape> It(World); It; ++It)
		{
			ExistingLandscapes.Add(*It);
		}
		for (ALandscape* L : ExistingLandscapes)
		{
			if (L)
			{
				UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: removing existing Landscape %s"), *L->GetName());
				L->Destroy();
			}
		}

		// Create new Landscape
		const int32 NumComponentsX = (SizeX - 1) / QuadsPerComponent;
		const int32 NumComponentsY = (SizeY - 1) / QuadsPerComponent;
		if (NumComponentsX < 1 || NumComponentsY < 1)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: invalid component count for size %dx%d"), SizeX, SizeY);
			return false;
		}

		TMap<FGuid, TArray<uint16>> HeightMapForImport;
		HeightMapForImport.Add(FGuid(), MoveTemp(HeightData));

		FActorSpawnParameters SpawnParams;
		ALandscape* Landscape = World->SpawnActor<ALandscape>(ALandscape::StaticClass(), LandscapeOrigin, FRotator::ZeroRotator, SpawnParams);
		if (!Landscape)
		{
			UE_LOG(LogT66Editor, Error, TEXT("[MAP] GenerateProceduralHillsLandscape: failed to SpawnActor ALandscape"));
			return false;
		}

		Landscape->SetActorScale3D(FVector(100.f, 100.f, 100.f));

		// Landscape: Cozy Nature (from T66MapAssets if set up, else CozyNature)
		UMaterialInterface* LandscapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/T66MapAssets/Landscape/MI_Landscape.MI_Landscape"));
		if (!LandscapeMat) LandscapeMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/CozyNature/MaterialsInstances/Terrain/MI_Landscape.MI_Landscape"));
		if (LandscapeMat)
		{
			Landscape->LandscapeMaterial = LandscapeMat;
		}
		else
		{
			if (UMaterialInterface* Fallback = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Ground/M_GroundAtlas_2x2_R0.M_GroundAtlas_2x2_R0")))
			{
				Landscape->LandscapeMaterial = Fallback;
			}
			UE_LOG(LogT66Editor, Warning, TEXT("[MAP] Landscape material not found (T66MapAssets or CozyNature); using fallback."));
		}

		// Grass layer: Cozy Nature layer info (T66MapAssets or CozyNature). Required for full grass coverage on hills.
		ULandscapeLayerInfoObject* GrassLayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, TEXT("/Game/T66MapAssets/Landscape/Grass_LayerInfo.Grass_LayerInfo"));
		if (!GrassLayerInfo) GrassLayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, TEXT("/Game/CozyNature/Maps/LayerInfo/Grass_LayerInfo.Grass_LayerInfo"));
		if (!GrassLayerInfo)
		{
			UE_LOG(LogT66Editor, Warning, TEXT("[MAP] Grass_LayerInfo not found. Run Window -> T66 Tools -> Setup T66 Map Assets, or grass will not apply to landscape (pale stripes on hills)."));
		}

		// Full grass coverage: one weight layer at max (255) for every vertex.
		const int32 VertsX = SizeX;
		const int32 VertsY = SizeY;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayersForImport;
		TArray<FLandscapeImportLayerInfo> GrassLayerInfos;
		if (GrassLayerInfo)
		{
			FLandscapeImportLayerInfo GrassImportInfo(GrassLayerInfo->GetLayerName());
			GrassImportInfo.LayerInfo = GrassLayerInfo;
			GrassImportInfo.LayerData.SetNumUninitialized(VertsX * VertsY);
			for (int32 i = 0; i < GrassImportInfo.LayerData.Num(); ++i)
			{
				GrassImportInfo.LayerData[i] = 255;
			}
			GrassLayerInfos.Add(MoveTemp(GrassImportInfo));
			UE_LOG(LogT66Editor, Log, TEXT("[MAP] Grass layer \"%s\" applied at full weight to %d vertices"), *GrassLayerInfo->GetLayerName().ToString(), VertsX * VertsY);
		}
		MaterialLayerDataPerLayersForImport.Add(FGuid(), MoveTemp(GrassLayerInfos));
		TArray<FLandscapeLayer> ImportLayers;
		Landscape->Import(
			FGuid::NewGuid(),
			0, 0,
			SizeX - 1, SizeY - 1,
			SectionsPerComponent,
			QuadsPerComponent,
			HeightMapForImport,
			nullptr,
			MaterialLayerDataPerLayersForImport,
			ELandscapeImportAlphamapType::Additive,
			TArrayView<const FLandscapeLayer>(ImportLayers));

		UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: Landscape->Import completed"));

		Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(
			FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1), (uint32)2);

		ULandscapeInfo* NewInfo = Landscape->GetLandscapeInfo();
		if (NewInfo)
		{
			NewInfo->UpdateLayerInfoMap(Landscape);
		}
		Landscape->RegisterAllComponents();

		// Re-apply landscape material to the whole map so every component uses it (fixes hills missing ground/grass material).
		if (LandscapeMat)
		{
			Landscape->LandscapeMaterial = LandscapeMat;
			UE_LOG(LogT66Editor, Log, TEXT("[MAP] Landscape material re-applied to full landscape (all components)."));
		}

		// Force visual and material update so all components pick up the material and layer data.
		Landscape->PostEditChange();
		Landscape->MarkComponentsRenderStateDirty();

		// Full bright: no shadows from landscape
		for (UActorComponent* Comp : Landscape->GetComponents())
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
			{
				Prim->SetCastShadow(false);
			}
		}

#if WITH_EDITOR
		FixPolytopeMaterialUsageFlags();
#endif
		// Place grass (Polytope), trees/rocks (Cozy Nature); from T66MapAssets if set up (random 10 trees, 6 rocks)
		PlacePolytopeFoliage(World, Heights, SizeX, SizeY, Params.LandscapeOriginX, Params.LandscapeOriginY,
			FT66ProceduralLandscapeGenerator::DefaultQuadSizeUU, Params.Seed, T66ProceduralFoliageTag);

		UE_LOG(LogT66Editor, Log, TEXT("[MAP] GenerateProceduralHillsLandscape: created new Landscape (seed=%d, %dx%d)"), Params.Seed, SizeX, SizeY);
		return true;
	}

	void PlacePolytopeFoliage(UWorld* World, const TArray<float>& Heights, int32 SizeX, int32 SizeY,
		float OriginX, float OriginY, float QuadSizeUU, int32 Seed, FName FoliageTag)
	{
		if (!World || Heights.Num() != SizeX * SizeY || SizeX < 2 || SizeY < 2)
		{
			return;
		}
		FRandomStream Rng(Seed);

		auto SpawnHISMWithMeshAndMat = [World, FoliageTag](UStaticMesh* Mesh, UMaterialInterface* Mat, bool bNoCollision) -> UHierarchicalInstancedStaticMeshComponent*
		{
			if (!Mesh) return nullptr;
			AActor* Owner = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			if (!Owner) return nullptr;
			Owner->Tags.Add(FoliageTag);
			Owner->SetActorLabel(TEXT("T66 Procedural Foliage"));
			UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(Owner, UHierarchicalInstancedStaticMeshComponent::StaticClass(), TEXT("FoliageHISM"), RF_Transactional);
			HISM->SetStaticMesh(Mesh);
			if (Mat) HISM->SetMaterial(0, Mat);
			HISM->SetCastShadow(false);
			HISM->SetReceivesDecals(false);
			if (bNoCollision) HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HISM->RegisterComponent();
			Owner->AddInstanceComponent(HISM);
			return HISM;
		};
		auto SpawnHISM = [World, FoliageTag, &SpawnHISMWithMeshAndMat](const TCHAR* MeshPath, const TCHAR* MatPath, UMaterialInterface*& OutMat, bool bNoCollision) -> UHierarchicalInstancedStaticMeshComponent*
		{
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
			if (!Mesh) return nullptr;
			OutMat = LoadObject<UMaterialInterface>(nullptr, MatPath);
			return SpawnHISMWithMeshAndMat(Mesh, OutMat, bNoCollision);
		};

		auto AddInstanceAt = [&Heights, SizeX, SizeY, OriginX, OriginY, QuadSizeUU, &Rng](UHierarchicalInstancedStaticMeshComponent* HISM, int32 Ix, int32 Iy, float ScaleMin, float ScaleMax)
		{
			if (!HISM) return;
			float Z = Heights[Iy * SizeX + Ix];
			FVector Loc(OriginX + Ix * QuadSizeUU, OriginY + Iy * QuadSizeUU, Z);
			float Yaw = Rng.FRandRange(0.f, 360.f);
			float Scale = Rng.FRandRange(ScaleMin, ScaleMax);
			FTransform T(FRotator(0.f, Yaw, 0.f), Loc, FVector(Scale));
			HISM->AddInstance(T, true);
		};

		// Grass: disabled (toggle off). Landscape and trees/rocks still spawn.
		static constexpr bool bSpawnGrassAssets = false;
		if (bSpawnGrassAssets)
		{
			UStaticMesh* GrassMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/T66MapAssets/Grass/SM_Grass_02.SM_Grass_02"));
			if (!GrassMesh) GrassMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Polytope_Studio/Nature_Free/Meshes/Plants/SM_Grass_02.SM_Grass_02"));
			UMaterialInterface* GrassMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/T66MapAssets/Grass/MI_Plants_Grass.MI_Plants_Grass"));
			if (!GrassMat) GrassMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Polytope_Studio/Nature_Free/Materials/MI_Plants_Grass.MI_Plants_Grass"));
			if (UHierarchicalInstancedStaticMeshComponent* GrassHISM = GrassMesh ? SpawnHISMWithMeshAndMat(GrassMesh, GrassMat, true) : nullptr)
			{
				for (int32 Iy = 1; Iy < SizeY - 1; ++Iy)
				{
					for (int32 Ix = 1; Ix < SizeX - 1; ++Ix)
					{
						AddInstanceAt(GrassHISM, Ix, Iy, 0.9f, 1.1f);
					}
				}
			}
		}

		// Trees: Cozy Nature (T66MapAssets or CozyNature), 10 total at random positions
		static const TCHAR* TreeMeshNames[] = { TEXT("SM_Tree1"), TEXT("SM_Tree2"), TEXT("SM_Tree3") };
		static const TCHAR* TreeMatNames[] = { TEXT("MI_Tree1"), TEXT("MI_Tree2"), TEXT("MI_Tree3") };
		UHierarchicalInstancedStaticMeshComponent* TreeHISMs[3] = { nullptr, nullptr, nullptr };
		for (int32 T = 0; T < 3; ++T)
		{
			UStaticMesh* TreeMesh = LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/T66MapAssets/Trees/%s.%s"), TreeMeshNames[T], TreeMeshNames[T]));
			if (!TreeMesh) TreeMesh = LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/CozyNature/Meshes/Trees/%s.%s"), TreeMeshNames[T], TreeMeshNames[T]));
			UMaterialInterface* TreeMat = LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/T66MapAssets/Trees/%s.%s"), TreeMatNames[T], TreeMatNames[T]));
			if (!TreeMat) TreeMat = LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/CozyNature/MaterialsInstances/Trees/%s.%s"), TreeMatNames[T], TreeMatNames[T]));
			TreeHISMs[T] = SpawnHISMWithMeshAndMat(TreeMesh, TreeMat, false);
		}
		const int32 InnerMin = 2;
		const int32 InnerMaxX = SizeX - 2;
		const int32 InnerMaxY = SizeY - 2;
		for (int32 n = 0; n < 10; ++n)
		{
			int32 Ix = Rng.RandRange(InnerMin, InnerMaxX);
			int32 Iy = Rng.RandRange(InnerMin, InnerMaxY);
			int32 Type = Rng.RandRange(0, 2);
			if (TreeHISMs[Type]) AddInstanceAt(TreeHISMs[Type], Ix, Iy, 0.9f, 1.1f);
		}

		// Rocks: Cozy Nature (T66MapAssets or CozyNature), 6 total at random positions
		static const TCHAR* RockMeshNames[] = { TEXT("SM_Rock1"), TEXT("SM_Rock2"), TEXT("SM_Rock3") };
		static const TCHAR* RockMatNames[] = { TEXT("MI_Rock1"), TEXT("MI_Rock2"), TEXT("MI_Rock3") };
		UHierarchicalInstancedStaticMeshComponent* RockHISMs[3] = { nullptr, nullptr, nullptr };
		for (int32 R = 0; R < 3; ++R)
		{
			UStaticMesh* RockMesh = LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/T66MapAssets/Rocks/%s.%s"), RockMeshNames[R], RockMeshNames[R]));
			if (!RockMesh) RockMesh = LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/CozyNature/Meshes/Rocks/%s.%s"), RockMeshNames[R], RockMeshNames[R]));
			UMaterialInterface* RockMat = LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/T66MapAssets/Rocks/%s.%s"), RockMatNames[R], RockMatNames[R]));
			if (!RockMat) RockMat = LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/CozyNature/MaterialsInstances/Rocks/%s.%s"), RockMatNames[R], RockMatNames[R]));
			RockHISMs[R] = SpawnHISMWithMeshAndMat(RockMesh, RockMat, false);
		}
		for (int32 n = 0; n < 6; ++n)
		{
			int32 Ix = Rng.RandRange(InnerMin, InnerMaxX);
			int32 Iy = Rng.RandRange(InnerMin, InnerMaxY);
			int32 Type = Rng.RandRange(0, 2);
			if (RockHISMs[Type]) AddInstanceAt(RockHISMs[Type], Ix, Iy, 0.85f, 1.15f);
		}
	}

#if WITH_EDITOR
	void FixPolytopeMaterialUsageFlags()
	{
		// Grass (Polytope M_Plants) + trees/rocks (Cozy M_Trees, M_Rocks) so all HISM materials render outside editor
		static const TCHAR* BaseMaterialPaths[] = {
			TEXT("/Game/Polytope_Studio/Nature_Free/Materials/MasterMaterials/M_Plants.M_Plants"),
			TEXT("/Game/CozyNature/Materials/Trees/M_Trees.M_Trees"),
			TEXT("/Game/CozyNature/Materials/Rocks/M_Rocks.M_Rocks")
		};
		for (const TCHAR* Path : BaseMaterialPaths)
		{
			UMaterial* M = LoadObject<UMaterial>(nullptr, Path);
			if (!M) continue;
			M->bUsedWithInstancedStaticMeshes = true;
			M->PreEditChange(nullptr);
			M->PostEditChange();
			M->MarkPackageDirty();
			UPackage* Package = M->GetOutermost();
			if (Package)
			{
				FSavePackageArgs SaveArgs;
				SaveArgs.TopLevelFlags = RF_Standalone;
				if (UPackage::SavePackage(Package, M, *FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension()), SaveArgs))
				{
					UE_LOG(LogT66Editor, Log, TEXT("[MAP] Set bUsedWithInstancedStaticMeshes and saved: %s"), Path);
				}
			}
		}
	}

	void SetupT66MapAssets()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		auto DuplicateInto = [&AssetTools](const TCHAR* SourcePath, const TCHAR* DestPackagePath) -> bool
		{
			UObject* Obj = LoadObject<UObject>(nullptr, SourcePath);
			if (!Obj)
			{
				UE_LOG(LogT66Editor, Warning, TEXT("[T66MapAssets] Source not found: %s"), SourcePath);
				return false;
			}
			UObject* Dup = AssetTools.DuplicateAsset(Obj->GetName(), DestPackagePath, Obj);
			if (Dup)
			{
				UE_LOG(LogT66Editor, Log, TEXT("[T66MapAssets] Copied to %s/%s"), DestPackagePath, *Obj->GetName());
				return true;
			}
			return false;
		};
		// Grass from Polytope
		DuplicateInto(TEXT("/Game/Polytope_Studio/Nature_Free/Meshes/Plants/SM_Grass_02.SM_Grass_02"), TEXT("/Game/T66MapAssets/Grass"));
		DuplicateInto(TEXT("/Game/Polytope_Studio/Nature_Free/Materials/MI_Plants_Grass.MI_Plants_Grass"), TEXT("/Game/T66MapAssets/Grass"));
		// Landscape from Cozy Nature
		DuplicateInto(TEXT("/Game/CozyNature/MaterialsInstances/Terrain/MI_Landscape.MI_Landscape"), TEXT("/Game/T66MapAssets/Landscape"));
		DuplicateInto(TEXT("/Game/CozyNature/Maps/LayerInfo/Grass_LayerInfo.Grass_LayerInfo"), TEXT("/Game/T66MapAssets/Landscape"));
		// Trees from Cozy Nature
		DuplicateInto(TEXT("/Game/CozyNature/Meshes/Trees/SM_Tree1.SM_Tree1"), TEXT("/Game/T66MapAssets/Trees"));
		DuplicateInto(TEXT("/Game/CozyNature/Meshes/Trees/SM_Tree2.SM_Tree2"), TEXT("/Game/T66MapAssets/Trees"));
		DuplicateInto(TEXT("/Game/CozyNature/Meshes/Trees/SM_Tree3.SM_Tree3"), TEXT("/Game/T66MapAssets/Trees"));
		DuplicateInto(TEXT("/Game/CozyNature/MaterialsInstances/Trees/MI_Tree1.MI_Tree1"), TEXT("/Game/T66MapAssets/Trees"));
		DuplicateInto(TEXT("/Game/CozyNature/MaterialsInstances/Trees/MI_Tree2.MI_Tree2"), TEXT("/Game/T66MapAssets/Trees"));
		DuplicateInto(TEXT("/Game/CozyNature/MaterialsInstances/Trees/MI_Tree3.MI_Tree3"), TEXT("/Game/T66MapAssets/Trees"));
		// Rocks from Cozy Nature
		DuplicateInto(TEXT("/Game/CozyNature/Meshes/Rocks/SM_Rock1.SM_Rock1"), TEXT("/Game/T66MapAssets/Rocks"));
		DuplicateInto(TEXT("/Game/CozyNature/Meshes/Rocks/SM_Rock2.SM_Rock2"), TEXT("/Game/T66MapAssets/Rocks"));
		DuplicateInto(TEXT("/Game/CozyNature/Meshes/Rocks/SM_Rock3.SM_Rock3"), TEXT("/Game/T66MapAssets/Rocks"));
		DuplicateInto(TEXT("/Game/CozyNature/MaterialsInstances/Rocks/MI_Rock1.MI_Rock1"), TEXT("/Game/T66MapAssets/Rocks"));
		DuplicateInto(TEXT("/Game/CozyNature/MaterialsInstances/Rocks/MI_Rock2.MI_Rock2"), TEXT("/Game/T66MapAssets/Rocks"));
		DuplicateInto(TEXT("/Game/CozyNature/MaterialsInstances/Rocks/MI_Rock3.MI_Rock3"), TEXT("/Game/T66MapAssets/Rocks"));
		UE_LOG(LogT66Editor, Log, TEXT("[T66MapAssets] Setup complete. Use Window -> T66 Tools -> Generate Procedural Hills Landscape (Dev) to apply."));
	}
#endif

}

#undef LOCTEXT_NAMESPACE
