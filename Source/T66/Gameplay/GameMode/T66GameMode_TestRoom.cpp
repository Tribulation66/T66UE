// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameMode_TestRoom.h"

#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Scene.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Gameplay/T66ThemeAtmosphereData.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66PerActorLightDirection.h"
#include "Gameplay/T66WorldVisualSetup.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInterface.h"

static TAutoConsoleVariable<int32> CVarT66TestRoomSpawnLuBuMatrix(
	TEXT("t66.TestRoom.SpawnLuBuMatrix"),
	0,
	TEXT("Spawns the ToonStyle Lu Bu Pixal3D comparison matrix in the TestRoom."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarT66TestRoomSpawnFullLineup(
	TEXT("t66.TestRoom.SpawnFullLineup"),
	1,
	TEXT("Spawns the ToonStyle full asset lineup in the TestRoom."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarT66TestRoomUseManualExposure(
	TEXT("t66.TestRoom.UseManualExposure"),
	1,
	TEXT("Use the Phase 1C manual exposure path in TestRoom. Set 0 to use the Phase 1B fixed exposure clamp rollback."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarT66TestRoomTestPerActorLightOverride(
	TEXT("t66.TestRoom.TestPerActorLightOverride"),
	0,
	TEXT("Temporarily attaches UT66PerActorLightDirection to Lu Bu in TestRoom for ToonStyle smoke verification."),
	ECVF_Default);

namespace T66TestRoom
{
	FName RoomActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom"));
		return Tag;
	}

	FName RoomSurfaceTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_Surface"));
		return Tag;
	}

	FName LightingActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_Lighting"));
		return Tag;
	}

	FName LuBuMatrixActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_LuBuMatrix"));
		return Tag;
	}

	FName LineupActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_Lineup"));
		return Tag;
	}

	FName AtmosphereSparedTag()
	{
		static const FName Tag(TEXT("T66_AtmosphereSpared"));
		return Tag;
	}

	FVector PlayerStartLocation()
	{
		return FVector(0.f, 0.f, 220.f);
	}

	namespace
	{
		FName LineupOutlineActorTag()
		{
			static const FName Tag(TEXT("T66_TestRoom_Lineup_Outline"));
			return Tag;
		}

		UStaticMesh* LoadCubeMesh()
		{
			return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		}

		UMaterialInterface* LoadWallMaterial()
		{
			return LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Wall.MI_TestRoom_Wall"));
		}

		UMaterialInterface* LoadFloorMaterial()
		{
			return LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Floor.MI_TestRoom_Floor"));
		}

		UMaterialInterface* LoadCeilingMaterial()
		{
			return LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/ToonStyle/TestAssets/Environment/Materials/MI_TestRoom_Ceiling.MI_TestRoom_Ceiling"));
		}

		FRotator LineupDisplayRotation()
		{
			return FRotator(0.f, -90.f, 0.f);
		}

		void DestroyActorsWithTag(UWorld* World, const FName Tag)
		{
			if (!World || Tag.IsNone())
			{
				return;
			}

			TArray<AActor*> ToDestroy;
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (It->Tags.Contains(Tag))
				{
					ToDestroy.Add(*It);
				}
			}

			for (AActor* Actor : ToDestroy)
			{
				if (Actor)
				{
					Actor->Destroy();
				}
			}
		}

		void TagTestRoomActor(AActor* Actor, const bool bSurface, const bool bLighting)
		{
			if (!Actor)
			{
				return;
			}

			Actor->Tags.AddUnique(RoomActorTag());
			Actor->Tags.AddUnique(AtmosphereSparedTag());
			if (bSurface)
			{
				Actor->Tags.AddUnique(RoomSurfaceTag());
			}
			if (bLighting)
			{
				Actor->Tags.AddUnique(LightingActorTag());
			}
		}

		void SpawnCubeSurface(UWorld* World, UStaticMesh* CubeMesh, UMaterialInterface* Material, const TCHAR* Label, const FVector& Location, const FVector& Scale)
		{
			if (!World || !CubeMesh)
			{
				return;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AStaticMeshActor* Surface = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
			if (!Surface)
			{
				return;
			}

			TagTestRoomActor(Surface, true, false);
#if WITH_EDITOR
			Surface->SetActorLabel(Label);
#endif
			if (UStaticMeshComponent* MeshComponent = Surface->GetStaticMeshComponent())
			{
				MeshComponent->SetMobility(EComponentMobility::Movable);
				MeshComponent->SetStaticMesh(CubeMesh);
				if (Material)
				{
					MeshComponent->SetMaterial(0, Material);
				}
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
				Surface->SetActorScale3D(Scale);
				FT66WorldVisualSetup::RegisterToonMaterial(MeshComponent, ET66ToonMaterialKind::Environment);
				MeshComponent->SetMobility(EComponentMobility::Static);
			}
		}

		struct FLineupEntry
		{
			const TCHAR* MeshPath;
			const TCHAR* OutlineMeshPath;
			const TCHAR* MaterialPath;
			const TCHAR* OutlineMaterialPath;
			const TCHAR* Label;
			FVector Location;
		};

		struct FLuBuMatrixEntry
		{
			const TCHAR* MeshPath;
			const TCHAR* Label;
			float Y;
		};

		void SpawnLuBuMatrix(UWorld* World)
		{
			if (!World || CVarT66TestRoomSpawnLuBuMatrix.GetValueOnGameThread() == 0)
			{
				return;
			}

			static const FLuBuMatrixEntry Entries[] =
			{
				{ TEXT("/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T2048_Default.SM_LuBu_R1024_T2048_Default"), TEXT("R1024 T2048 Default"), -1000.f },
				{ TEXT("/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T2048_Default.SM_LuBu_R1536_T2048_Default"), TEXT("R1536 T2048 Default"), -600.f },
				{ TEXT("/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T4096_Default.SM_LuBu_R1024_T4096_Default"), TEXT("R1024 T4096 Default"), -200.f },
				{ TEXT("/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T4096_Default.SM_LuBu_R1536_T4096_Default"), TEXT("R1536 T4096 Default"), 200.f },
				{ TEXT("/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1024_T2048_High.SM_LuBu_R1024_T2048_High"), TEXT("R1024 T2048 High"), 600.f },
				{ TEXT("/Game/ToonStyle/TestAssets/LuBu_Matrix/SM_LuBu_R1536_T4096_High.SM_LuBu_R1536_T4096_High"), TEXT("R1536 T4096 High"), 1000.f },
			};

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			for (const FLuBuMatrixEntry& Entry : Entries)
			{
				UStaticMesh* VariantMesh = LoadObject<UStaticMesh>(nullptr, Entry.MeshPath);
				if (!VariantMesh)
				{
					UE_LOG(LogTemp, Warning, TEXT("ToonStyle TestRoom could not load Lu Bu matrix mesh %s"), Entry.MeshPath);
					continue;
				}

				const FVector MeshLocation(1500.f, Entry.Y, 0.f);
				AStaticMeshActor* VariantActor = World->SpawnActor<AStaticMeshActor>(
					AStaticMeshActor::StaticClass(),
					MeshLocation,
					LineupDisplayRotation(),
					SpawnParams);
				if (VariantActor)
				{
					TagTestRoomActor(VariantActor, false, false);
					VariantActor->Tags.AddUnique(LuBuMatrixActorTag());
					VariantActor->SetActorScale3D(FVector(0.01f));
#if WITH_EDITOR
					VariantActor->SetActorLabel(FString::Printf(TEXT("DEV_TestRoom_LuBu_%s"), Entry.Label));
#endif
					if (UStaticMeshComponent* MeshComponent = VariantActor->GetStaticMeshComponent())
					{
						MeshComponent->SetMobility(EComponentMobility::Movable);
						MeshComponent->SetStaticMesh(VariantMesh);
						MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
						MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
						MeshComponent->SetMobility(EComponentMobility::Static);
					}
				}

				const FVector TextLocation(1500.f, Entry.Y, 400.f);
				const FVector TextTarget(0.f, 0.f, 400.f);
				const FRotator TextRotation = (TextTarget - TextLocation).Rotation();
				ATextRenderActor* LabelActor = World->SpawnActor<ATextRenderActor>(
					ATextRenderActor::StaticClass(),
					TextLocation,
					TextRotation,
					SpawnParams);
				if (LabelActor)
				{
					TagTestRoomActor(LabelActor, false, false);
					LabelActor->Tags.AddUnique(LuBuMatrixActorTag());
#if WITH_EDITOR
					LabelActor->SetActorLabel(FString::Printf(TEXT("DEV_TestRoom_Label_%s"), Entry.Label));
#endif
					if (UTextRenderComponent* TextComponent = LabelActor->GetTextRender())
					{
						TextComponent->SetText(FText::FromString(Entry.Label));
						TextComponent->SetWorldSize(65.f);
						TextComponent->SetTextRenderColor(FColor(255, 235, 120));
						TextComponent->SetHorizontalAlignment(EHTA_Center);
						TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
					}
				}
			}
		}

		void SpawnTextLabel(UWorld* World, const TCHAR* ActorLabelPrefix, const TCHAR* Text, const FVector& TextLocation, const FName ExtraTag)
		{
			if (!World || !Text)
			{
				return;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			const FVector TextTarget(0.f, 0.f, 400.f);
			const FRotator TextRotation = (TextTarget - TextLocation).Rotation();
			ATextRenderActor* LabelActor = World->SpawnActor<ATextRenderActor>(
				ATextRenderActor::StaticClass(),
				TextLocation,
				TextRotation,
				SpawnParams);
			if (!LabelActor)
			{
				return;
			}

			TagTestRoomActor(LabelActor, false, false);
			if (!ExtraTag.IsNone())
			{
				LabelActor->Tags.AddUnique(ExtraTag);
			}
#if WITH_EDITOR
			LabelActor->SetActorLabel(FString::Printf(TEXT("%s_%s"), ActorLabelPrefix ? ActorLabelPrefix : TEXT("DEV_TestRoom_Label"), Text));
#endif
			if (UTextRenderComponent* TextComponent = LabelActor->GetTextRender())
			{
				TextComponent->SetText(FText::FromString(Text));
				TextComponent->SetWorldSize(65.f);
				TextComponent->SetTextRenderColor(FColor(255, 235, 120));
				TextComponent->SetHorizontalAlignment(EHTA_Center);
				TextComponent->SetVerticalAlignment(EVRTA_TextCenter);
			}
		}

		AStaticMeshActor* SpawnLineupStaticMeshActor(UWorld* World, const FLineupEntry& Entry, const bool bOutline)
		{
			if (!World)
			{
				return nullptr;
			}

			const TCHAR* MeshPath = bOutline ? Entry.OutlineMeshPath : Entry.MeshPath;
			const TCHAR* MaterialPath = bOutline ? Entry.OutlineMaterialPath : Entry.MaterialPath;
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
			if (!Mesh)
			{
				UE_LOG(LogTemp, Warning, TEXT("ToonStyle TestRoom could not load lineup %s mesh %s"), bOutline ? TEXT("outline") : TEXT("shading"), MeshPath);
				return nullptr;
			}

			UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, MaterialPath);
			if (!Material)
			{
				UE_LOG(LogTemp, Warning, TEXT("ToonStyle TestRoom could not load lineup %s material %s"), bOutline ? TEXT("outline") : TEXT("shading"), MaterialPath);
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(),
				Entry.Location,
				LineupDisplayRotation(),
				SpawnParams);
			if (!Actor)
			{
				return nullptr;
			}

			TagTestRoomActor(Actor, false, false);
			Actor->Tags.AddUnique(LineupActorTag());
			if (bOutline)
			{
				Actor->Tags.AddUnique(LineupOutlineActorTag());
			}
#if WITH_EDITOR
			Actor->SetActorLabel(FString::Printf(TEXT("DEV_TestRoom_%s_%s"), bOutline ? TEXT("LineupOutline") : TEXT("Lineup"), Entry.Label));
#endif
			if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
			{
				MeshComponent->SetMobility(EComponentMobility::Movable);
				MeshComponent->SetStaticMesh(Mesh);
				if (Material)
				{
					MeshComponent->SetMaterial(0, Material);
				}
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
				const bool bTestPerActorOverride = CVarT66TestRoomTestPerActorLightOverride.GetValueOnGameThread() != 0
					|| FParse::Param(FCommandLine::Get(), TEXT("T66TestRoomTestPerActorLightOverride"));
				if (!bOutline
					&& FCString::Stricmp(Entry.Label, TEXT("Lu Bu")) == 0
					&& bTestPerActorOverride)
				{
					if (UT66PerActorLightDirection* Override = NewObject<UT66PerActorLightDirection>(Actor, TEXT("R1_ToonLightDirectionOverride")))
					{
						Override->RegisterComponent();
						Override->SetLightDirectionOverride(FVector(0.65f, -0.25f, -0.72f));
						Actor->AddInstanceComponent(Override);
						UE_LOG(LogTemp, Display, TEXT("ToonStyle TestRoom attached temporary per-actor light override to Lu Bu for R1 smoke verification."));
					}
				}
				FT66WorldVisualSetup::RegisterToonMaterial(MeshComponent, bOutline ? ET66ToonMaterialKind::Outline : ET66ToonMaterialKind::Character);
				MeshComponent->SetMobility(EComponentMobility::Static);
			}

			return Actor;
		}

		void SpawnFullLineup(UWorld* World)
		{
			if (!World || CVarT66TestRoomSpawnFullLineup.GetValueOnGameThread() == 0)
			{
				return;
			}

			static const FLineupEntry Entries[] =
			{
				{ TEXT("/Game/ToonStyle/TestAssets/Validation/SM_lubu_validation.SM_lubu_validation"), TEXT("/Game/ToonStyle/TestAssets/Validation/SM_lubu_validation_Outline.SM_lubu_validation_Outline"), TEXT("/Game/ToonStyle/TestAssets/Validation/Materials/MI_lubu_validation.MI_lubu_validation"), TEXT("/Game/ToonStyle/TestAssets/Validation/Materials/MI_lubu_validation_Outline.MI_lubu_validation_Outline"), TEXT("Lu Bu"), FVector(1500.f, -500.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_aria.SM_aria"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_aria_Outline.SM_aria_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_aria.MI_aria"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_aria_Outline.MI_aria_Outline"), TEXT("ARIA"), FVector(1500.f, 0.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_gambler.SM_gambler"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_gambler_Outline.SM_gambler_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_gambler.MI_gambler"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_gambler_Outline.MI_gambler_Outline"), TEXT("Gambler"), FVector(1500.f, 500.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_slime.SM_slime"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_slime_Outline.SM_slime_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_slime.MI_slime"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_slime_Outline.MI_slime_Outline"), TEXT("Slime"), FVector(2500.f, -750.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_tombspider.SM_tombspider"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_tombspider_Outline.SM_tombspider_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_tombspider.MI_tombspider"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_tombspider_Outline.MI_tombspider_Outline"), TEXT("TombSpider"), FVector(2500.f, -250.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_cavebat.SM_cavebat"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_cavebat_Outline.SM_cavebat_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_cavebat.MI_cavebat"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_cavebat_Outline.MI_cavebat_Outline"), TEXT("CaveBat"), FVector(2500.f, 250.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_idolaltar.SM_idolaltar"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_idolaltar_Outline.SM_idolaltar_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_idolaltar.MI_idolaltar"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_idolaltar_Outline.MI_idolaltar_Outline"), TEXT("Idol Altar"), FVector(2500.f, 750.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_arcademachine.SM_arcademachine"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_arcademachine_Outline.SM_arcademachine_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_arcademachine.MI_arcademachine"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_arcademachine_Outline.MI_arcademachine_Outline"), TEXT("Arcade Machine"), FVector(3500.f, -750.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_lootchest.SM_lootchest"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_lootchest_Outline.SM_lootchest_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootchest.MI_lootchest"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootchest_Outline.MI_lootchest_Outline"), TEXT("Loot Chest"), FVector(3500.f, -250.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_lootbag_yellow.SM_lootbag_yellow"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_lootbag_yellow_Outline.SM_lootbag_yellow_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootbag_yellow.MI_lootbag_yellow"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootbag_yellow_Outline.MI_lootbag_yellow_Outline"), TEXT("Loot Bag Yellow"), FVector(3500.f, 250.f, 0.f) },
				{ TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_lootcrate.SM_lootcrate"), TEXT("/Game/ToonStyle/TestAssets/Lineup/SM_lootcrate_Outline.SM_lootcrate_Outline"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootcrate.MI_lootcrate"), TEXT("/Game/ToonStyle/TestAssets/Lineup/Materials/MI_lootcrate_Outline.MI_lootcrate_Outline"), TEXT("Loot Crate"), FVector(3500.f, 750.f, 0.f) },
			};

			for (const FLineupEntry& Entry : Entries)
			{
				SpawnLineupStaticMeshActor(World, Entry, false);
				SpawnLineupStaticMeshActor(World, Entry, true);
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupLabel"), Entry.Label, Entry.Location + FVector(0.f, 0.f, 400.f), LineupActorTag());
			}
		}

		void SpawnPostProcessVolume(UWorld* World)
		{
			if (!World)
			{
				return;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>(
				APostProcessVolume::StaticClass(),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams);
			if (!Volume)
			{
				return;
			}

			TagTestRoomActor(Volume, false, true);
			Volume->bUnbound = true;
			Volume->Priority = 2000.f;
#if WITH_EDITOR
			Volume->SetActorLabel(TEXT("DEV_TestRoom_PostProcess"));
#endif
			FPostProcessSettings& PPS = Volume->Settings;
			PPS.bOverride_AutoExposureMethod = true;
			PPS.AutoExposureMethod = AEM_Manual;
			PPS.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
			PPS.AutoExposureApplyPhysicalCameraExposure = false;
			PPS.bOverride_AutoExposureBias = true;
			PPS.bOverride_AmbientOcclusionIntensity = true;
			PPS.AmbientOcclusionIntensity = 0.0f;

			if (CVarT66TestRoomUseManualExposure.GetValueOnGameThread() != 0)
			{
				PPS.AutoExposureBias = 0.7f;
				PPS.bOverride_AutoExposureMinBrightness = false;
				PPS.bOverride_AutoExposureMaxBrightness = false;
				UE_LOG(LogTemp, Display, TEXT("ToonStyle TestRoom using Phase 1C manual exposure path: AEM_Manual, Bias=+0.7."));
			}
			else
			{
				PPS.AutoExposureBias = 0.0f;
				PPS.bOverride_AutoExposureMinBrightness = true;
				PPS.AutoExposureMinBrightness = 1.0f;
				PPS.bOverride_AutoExposureMaxBrightness = true;
				PPS.AutoExposureMaxBrightness = 1.0f;
				UE_LOG(LogTemp, Display, TEXT("ToonStyle TestRoom using Phase 1B fixed exposure clamp rollback."));
			}
		}
	}

	void SpawnRoom(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		DestroyActorsWithTag(World, RoomActorTag());

		UStaticMesh* CubeMesh = LoadCubeMesh();
		if (!CubeMesh)
		{
			return;
		}

		constexpr float InteriorHalfWidth = 5000.f;
		constexpr float InteriorHalfDepth = 5000.f;
		constexpr float InteriorHeight = 600.f;
		constexpr float WallThickness = 40.f;
		constexpr float CubeSize = 100.f;

		const float TotalWidth = (InteriorHalfWidth * 2.f) + (WallThickness * 2.f);
		const float TotalDepth = (InteriorHalfDepth * 2.f) + (WallThickness * 2.f);
		const float WallCenterZ = InteriorHeight * 0.5f;
		const FVector FloorCeilingScale(TotalWidth / CubeSize, TotalDepth / CubeSize, WallThickness / CubeSize);
		const FVector NorthSouthWallScale(TotalWidth / CubeSize, WallThickness / CubeSize, InteriorHeight / CubeSize);
		const FVector EastWestWallScale(WallThickness / CubeSize, (InteriorHalfDepth * 2.f) / CubeSize, InteriorHeight / CubeSize);

		UMaterialInterface* FloorMaterial = LoadFloorMaterial();
		UMaterialInterface* WallMaterial = LoadWallMaterial();
		UMaterialInterface* CeilingMaterial = LoadCeilingMaterial();
		if (!CeilingMaterial)
		{
			CeilingMaterial = WallMaterial;
		}

		SpawnCubeSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_Floor"), FVector(0.f, 0.f, -WallThickness * 0.5f), FloorCeilingScale);
		SpawnCubeSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_Ceiling"), FVector(0.f, 0.f, InteriorHeight + (WallThickness * 0.5f)), FloorCeilingScale);
		SpawnCubeSurface(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_NorthWall"), FVector(0.f, InteriorHalfDepth + (WallThickness * 0.5f), WallCenterZ), NorthSouthWallScale);
		SpawnCubeSurface(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_SouthWall"), FVector(0.f, -InteriorHalfDepth - (WallThickness * 0.5f), WallCenterZ), NorthSouthWallScale);
		SpawnCubeSurface(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_EastWall"), FVector(InteriorHalfWidth + (WallThickness * 0.5f), 0.f, WallCenterZ), EastWestWallScale);
		SpawnCubeSurface(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_WestWall"), FVector(-InteriorHalfWidth - (WallThickness * 0.5f), 0.f, WallCenterZ), EastWestWallScale);
		SpawnLuBuMatrix(World);
		SpawnFullLineup(World);

		const int32 DungeonInitialCount = FT66WorldVisualSetup::ApplyToonCelAtmosphereToRegisteredMaterials(T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon);
		const int32 HellProbeCount = FT66WorldVisualSetup::ApplyToonCelAtmosphereToRegisteredMaterials(T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Hell);
		const int32 DungeonFinalCount = FT66WorldVisualSetup::ApplyToonCelAtmosphereToRegisteredMaterials(T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon);
		UE_LOG(LogTemp, Display, TEXT("ToonStyle TestRoom G6 parameter probe applied Dungeon=%d Hell=%d RestoredDungeon=%d."), DungeonInitialCount, HellProbeCount, DungeonFinalCount);
	}

	void SpawnLighting(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		DestroyActorsWithTag(World, LightingActorTag());

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ADirectionalLight* DirectionalLight = World->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0.f, 0.f, 500.f),
			FRotator(-60.f, -35.f, 0.f),
			SpawnParams);
		if (DirectionalLight)
		{
			TagTestRoomActor(DirectionalLight, false, true);
#if WITH_EDITOR
			DirectionalLight->SetActorLabel(TEXT("DEV_TestRoom_DirectionalLight"));
#endif
			if (ULightComponent* LightComponent = DirectionalLight->GetLightComponent())
			{
				LightComponent->SetIntensity(2.0f);
				LightComponent->SetLightColor(FLinearColor::White);
			}
		}

		ASkyLight* SkyLight = World->SpawnActor<ASkyLight>(
			ASkyLight::StaticClass(),
			FVector(0.f, 0.f, 450.f),
			FRotator::ZeroRotator,
			SpawnParams);
		if (SkyLight)
		{
			TagTestRoomActor(SkyLight, false, true);
#if WITH_EDITOR
			SkyLight->SetActorLabel(TEXT("DEV_TestRoom_SkyLight"));
#endif
			if (USkyLightComponent* SkyLightComponent = SkyLight->GetLightComponent())
			{
				SkyLightComponent->SetIntensity(0.3f);
			}
		}

		SpawnPostProcessVolume(World);
	}
}
