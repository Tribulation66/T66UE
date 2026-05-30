// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameMode_TestRoom.h"

#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Core/T66DeprecatedFeatureSettings.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Scene.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66ThemeAtmosphereData.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66LootWheelInteractable.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Gameplay/T66PerActorLightDirection.h"
#include "Gameplay/T66WorldVisualSetup.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

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

static TAutoConsoleVariable<int32> CVarT66TestRoomShowRepresentativeLineupOnly(
	TEXT("t66.TestRoom.ShowRepresentativeLineupOnly"),
	0,
	TEXT("Limits the ToonStyle TestRoom lineup to the five representative review assets when set to 1. Default 0 shows the full fixed lineup."),
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

static TAutoConsoleVariable<int32> CVarT66TestRoomEnableCombatZones(
	TEXT("t66.TestRoom.EnableCombatZones"),
	1,
	TEXT("Enables TestRoom side-room combat zones. Mobs and boss activate only while the player is inside their rooms."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66TestRoomMobSpawnIntervalSeconds(
	TEXT("t66.TestRoom.MobRoomSpawnIntervalSeconds"),
	1.15f,
	TEXT("Seconds between TestRoom mob-room spawns while the player stays in the room."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarT66TestRoomMobRoomMaxEnemies(
	TEXT("t66.TestRoom.MobRoomMaxEnemies"),
	8,
	TEXT("Maximum active TestRoom mob-room enemies."),
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

	FName MobRoomEnemyActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_MobRoomEnemy"));
		return Tag;
	}

	FName BossRoomActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_BossRoomActor"));
		return Tag;
	}

	FName LiveInteractableShowcaseActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_LiveInteractable_Showcase"));
		return Tag;
	}

	FVector PlayerStartLocation()
	{
		return FVector(0.f, 0.f, 220.f);
	}

	namespace
	{
		constexpr float TestRoomCenterHalfExtent = 5000.f;
		constexpr float TestRoomSideRoomHalfExtent = 3600.f;
		constexpr float TestRoomCorridorLength = 2600.f;
		constexpr float TestRoomCorridorHalfWidth = 1500.f;
		constexpr float TestRoomInteriorHeight = 600.f;
		constexpr float TestRoomWallThickness = 40.f;
		constexpr float TestRoomCubeSize = 100.f;
		constexpr float TestRoomSideRoomOffset = TestRoomCenterHalfExtent + TestRoomCorridorLength + TestRoomSideRoomHalfExtent;
		constexpr float TestRoomCorridorCenterOffset = TestRoomCenterHalfExtent + (TestRoomCorridorLength * 0.5f);

		FName LineupOutlineActorTag()
		{
			static const FName Tag(TEXT("T66_TestRoom_Lineup_Outline"));
			return Tag;
		}

		FBox2D MakeTestRoomBox(const FVector2D Center, const FVector2D HalfExtent)
		{
			return FBox2D(Center - HalfExtent, Center + HalfExtent);
		}

		FBox2D TestRoomMobRoomBox()
		{
			return MakeTestRoomBox(
				FVector2D(0.f, TestRoomSideRoomOffset),
				FVector2D(TestRoomSideRoomHalfExtent, TestRoomSideRoomHalfExtent));
		}

		FBox2D TestRoomBossRoomBox()
		{
			return MakeTestRoomBox(
				FVector2D(TestRoomSideRoomOffset, 0.f),
				FVector2D(TestRoomSideRoomHalfExtent, TestRoomSideRoomHalfExtent));
		}

		bool IsInsideBox2D(const FVector& Location, const FBox2D& Box)
		{
			return Location.X >= Box.Min.X
				&& Location.X <= Box.Max.X
				&& Location.Y >= Box.Min.Y
				&& Location.Y <= Box.Max.Y;
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

		FRotator LineupDisplayRotation(const FVector& Location)
		{
			FVector ToCenter = FVector::ZeroVector - Location;
			ToCenter.Z = 0.f;
			if (!ToCenter.Normalize())
			{
				return FRotator(0.f, -90.f, 0.f);
			}
			return ToCenter.Rotation();
		}

		void DestroyTestRoomActorsWithTag(UWorld* World, const FName Tag)
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

		void SpawnRectSurface(
			UWorld* World,
			UStaticMesh* CubeMesh,
			UMaterialInterface* Material,
			const FString& Label,
			const FBox2D& Rect,
			const float Z,
			const float Thickness)
		{
			const FVector Location(
				(Rect.Min.X + Rect.Max.X) * 0.5f,
				(Rect.Min.Y + Rect.Max.Y) * 0.5f,
				Z);
			const FVector Scale(
				(Rect.Max.X - Rect.Min.X) / TestRoomCubeSize,
				(Rect.Max.Y - Rect.Min.Y) / TestRoomCubeSize,
				Thickness / TestRoomCubeSize);
			SpawnCubeSurface(World, CubeMesh, Material, *Label, Location, Scale);
		}

		void SpawnHorizontalWallSegment(
			UWorld* World,
			UStaticMesh* CubeMesh,
			UMaterialInterface* WallMaterial,
			const FString& Label,
			const float XMin,
			const float XMax,
			const float Y)
		{
			if (XMax <= XMin + 1.f)
			{
				return;
			}

			const FVector Location((XMin + XMax) * 0.5f, Y, TestRoomInteriorHeight * 0.5f);
			const FVector Scale(
				(XMax - XMin) / TestRoomCubeSize,
				TestRoomWallThickness / TestRoomCubeSize,
				TestRoomInteriorHeight / TestRoomCubeSize);
			SpawnCubeSurface(World, CubeMesh, WallMaterial, *Label, Location, Scale);
		}

		void SpawnVerticalWallSegment(
			UWorld* World,
			UStaticMesh* CubeMesh,
			UMaterialInterface* WallMaterial,
			const FString& Label,
			const float X,
			const float YMin,
			const float YMax)
		{
			if (YMax <= YMin + 1.f)
			{
				return;
			}

			const FVector Location(X, (YMin + YMax) * 0.5f, TestRoomInteriorHeight * 0.5f);
			const FVector Scale(
				TestRoomWallThickness / TestRoomCubeSize,
				(YMax - YMin) / TestRoomCubeSize,
				TestRoomInteriorHeight / TestRoomCubeSize);
			SpawnCubeSurface(World, CubeMesh, WallMaterial, *Label, Location, Scale);
		}

		void SpawnHorizontalWallWithGap(
			UWorld* World,
			UStaticMesh* CubeMesh,
			UMaterialInterface* WallMaterial,
			const FString& LabelPrefix,
			const float XMin,
			const float XMax,
			const float Y,
			const float GapCenterX,
			const float GapHalfWidth)
		{
			SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, LabelPrefix + TEXT("_A"), XMin, GapCenterX - GapHalfWidth, Y);
			SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, LabelPrefix + TEXT("_B"), GapCenterX + GapHalfWidth, XMax, Y);
		}

		void SpawnVerticalWallWithGap(
			UWorld* World,
			UStaticMesh* CubeMesh,
			UMaterialInterface* WallMaterial,
			const FString& LabelPrefix,
			const float X,
			const float YMin,
			const float YMax,
			const float GapCenterY,
			const float GapHalfWidth)
		{
			SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, LabelPrefix + TEXT("_A"), X, YMin, GapCenterY - GapHalfWidth);
			SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, LabelPrefix + TEXT("_B"), X, GapCenterY + GapHalfWidth, YMax);
		}

		struct FLineupEntry
		{
			const TCHAR* ContentFolder;
			const TCHAR* AssetID;
			const TCHAR* Label;
			FVector Location;
		};

		bool IsRepresentativeLineupAsset(const TCHAR* AssetID)
		{
			return FCString::Strcmp(AssetID, TEXT("Hero_1_Chad")) == 0
				|| FCString::Strcmp(AssetID, TEXT("Hero_3_Chad")) == 0
				|| FCString::Strcmp(AssetID, TEXT("Hero_4_Stacy")) == 0
				|| FCString::Strcmp(AssetID, TEXT("Hero_5_Chad")) == 0
				|| FCString::Strcmp(AssetID, TEXT("Hero_1_Chad_Beachgoer")) == 0;
		}

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
					LineupDisplayRotation(MeshLocation),
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

			const FString OutlineSuffix = bOutline ? TEXT("_Outline") : TEXT("");
			const FString MeshPath = FString::Printf(
				TEXT("%s/SM_%s%s.SM_%s%s"),
				Entry.ContentFolder,
				Entry.AssetID,
				*OutlineSuffix,
				Entry.AssetID,
				*OutlineSuffix);
			const FString MaterialPath = FString::Printf(
				TEXT("%s/Materials/MI_%s%s.MI_%s%s"),
				Entry.ContentFolder,
				Entry.AssetID,
				*OutlineSuffix,
				Entry.AssetID,
				*OutlineSuffix);
			UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
			if (!Mesh)
			{
				UE_LOG(LogTemp, Warning, TEXT("ToonStyle TestRoom could not load lineup %s mesh %s"), bOutline ? TEXT("outline") : TEXT("shading"), *MeshPath);
				return nullptr;
			}

			UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
			if (!Material)
			{
				UE_LOG(LogTemp, Warning, TEXT("ToonStyle TestRoom could not load lineup %s material %s"), bOutline ? TEXT("outline") : TEXT("shading"), *MaterialPath);
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(),
				Entry.Location,
				LineupDisplayRotation(Entry.Location),
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
				// Male heroes - north side.
				{ TEXT("/Game/Characters/Heroes/Hero_1/Chad/Pixal3DToonStyle"), TEXT("Hero_1_Chad"), TEXT("H1 Chad"), FVector(-3200.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_2/Chad/Pixal3DToonStyle"), TEXT("Hero_2_Chad"), TEXT("H2 Chad"), FVector(-2400.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_3/Chad/Pixal3DToonStyle"), TEXT("Hero_3_Chad"), TEXT("H3 Chad"), FVector(-1600.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_4/Chad/Pixal3DToonStyle"), TEXT("Hero_4_Chad"), TEXT("H4 Chad"), FVector(-800.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_5/Chad/Pixal3DToonStyle"), TEXT("Hero_5_Chad"), TEXT("H5 Chad"), FVector(0.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_1/Chad/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_1_Chad_Beachgoer"), TEXT("H1 Chad Demo"), FVector(800.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_3/Chad/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_3_Chad_Beachgoer"), TEXT("H3 Chad Demo"), FVector(1600.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_4/Chad/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_4_Chad_Beachgoer"), TEXT("H4 Chad Demo"), FVector(2400.0f, 3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_5/Chad/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_5_Chad_Beachgoer"), TEXT("H5 Chad Demo"), FVector(3200.0f, 3950.0f, 0.f) },
				// Female heroes - south side.
				{ TEXT("/Game/Characters/Heroes/Hero_1/Stacy/Pixal3DToonStyle"), TEXT("Hero_1_Stacy"), TEXT("H1 Stacy"), FVector(-3200.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_2/Stacy/Pixal3DToonStyle"), TEXT("Hero_2_Stacy"), TEXT("H2 Stacy"), FVector(-2400.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_3/Stacy/Pixal3DToonStyle"), TEXT("Hero_3_Stacy"), TEXT("H3 Stacy"), FVector(-1600.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_4/Stacy/Pixal3DToonStyle"), TEXT("Hero_4_Stacy"), TEXT("H4 Stacy"), FVector(-800.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_5/Stacy/Pixal3DToonStyle"), TEXT("Hero_5_Stacy"), TEXT("H5 Stacy"), FVector(0.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_1/Stacy/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_1_Stacy_Beachgoer"), TEXT("H1 Stacy Demo"), FVector(800.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_3/Stacy/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_3_Stacy_Beachgoer"), TEXT("H3 Stacy Demo"), FVector(1600.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_4/Stacy/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_4_Stacy_Beachgoer"), TEXT("H4 Stacy Demo"), FVector(2400.0f, -3950.0f, 0.f) },
				{ TEXT("/Game/Characters/Heroes/Hero_5/Stacy/Beachgoer/Pixal3DToonStyle"), TEXT("Hero_5_Stacy_Beachgoer"), TEXT("H5 Stacy Demo"), FVector(3200.0f, -3950.0f, 0.f) },
				// Companions - west side.
				{ TEXT("/Game/Characters/Companions/Companion_01/Default/Pixal3DToonStyle"), TEXT("Companion_01"), TEXT("Comp 01"), FVector(-3950.0f, -2800.0f, 0.f) },
				{ TEXT("/Game/Characters/Companions/Companion_02/Default/Pixal3DToonStyle"), TEXT("Companion_02"), TEXT("Comp 02"), FVector(-3950.0f, -1200.0f, 0.f) },
				{ TEXT("/Game/Characters/Companions/Companion_03/Default/Pixal3DToonStyle"), TEXT("Companion_03"), TEXT("Comp 03"), FVector(-3950.0f, 400.0f, 0.f) },
				{ TEXT("/Game/Characters/Companions/Companion_04/Default/Pixal3DToonStyle"), TEXT("Companion_04"), TEXT("Comp 04"), FVector(-3950.0f, 2000.0f, 0.f) },
				// Easy enemies - center north rows.
				{ TEXT("/Game/Characters/Mobs/Slime"), TEXT("Slime"), TEXT("Slime"), FVector(-1600.0f, 2650.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/BoneWalker"), TEXT("BoneWalker"), TEXT("Bone Walker"), FVector(-800.0f, 2650.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/RatPack"), TEXT("RatPack"), TEXT("Rat Pack"), FVector(0.0f, 2650.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/CaveBat"), TEXT("CaveBat"), TEXT("Cave Bat"), FVector(800.0f, 2650.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/HexSlinger"), TEXT("HexSlinger"), TEXT("Hex Slinger"), FVector(1600.0f, 2650.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/TombSpider"), TEXT("TombSpider"), TEXT("Tomb Spider"), FVector(-1600.0f, 1850.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/StoneSentinel"), TEXT("StoneSentinel"), TEXT("Stone Sentinel"), FVector(-800.0f, 1850.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/MimicLure"), TEXT("MimicLure"), TEXT("Mimic Lure"), FVector(0.0f, 1850.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/BoneConjurer"), TEXT("BoneConjurer"), TEXT("Bone Conjurer"), FVector(800.0f, 1850.0f, 0.f) },
				{ TEXT("/Game/Characters/Mobs/CryptWraith"), TEXT("CryptWraith"), TEXT("Crypt Wraith"), FVector(1600.0f, 1850.0f, 0.f) },
				// World assets - east side.
				{ TEXT("/Game/World/Interactables/Arcade/Arcade_Machine"), TEXT("Arcade_Machine_Pixal3D"), TEXT("Arcade Machine"), FVector(3950.0f, -3850.0f, 0.f) },
				{ TEXT("/Game/World/Interactables/Vehicles"), TEXT("Vehicle_Pixal3D"), TEXT("Vehicle"), FVector(3950.0f, -3150.0f, 0.f) },
				{ TEXT("/Game/World/Interactables/Chests/ChestModel"), TEXT("Chest_Pixal3D"), TEXT("Chest"), FVector(3950.0f, -2450.0f, 0.f) },
				{ TEXT("/Game/World/Interactables/Fountain"), TEXT("Fountain_Pixal3D"), TEXT("Fountain"), FVector(3950.0f, -1750.0f, 0.f) },
				{ TEXT("/Game/World/Interactables/DifficultyTotem"), TEXT("DifficultyTotem_Pixal3D"), TEXT("Difficulty Totem"), FVector(3950.0f, -1050.0f, 0.f) },
				{ TEXT("/Game/World/Interactables/LootWheel"), TEXT("LootWheel_Pixal3D"), TEXT("Loot Wheel"), FVector(3950.0f, 350.0f, 0.f) },
				{ TEXT("/Game/World/LootBags/Shared"), TEXT("LootBag_Shared_Pixal3D"), TEXT("Loot Bag"), FVector(3950.0f, 1050.0f, 0.f) },
				{ TEXT("/Game/World/Interactables/IdolAltar"), TEXT("IdolAltar_Pixal3D"), TEXT("Idol Altar"), FVector(3950.0f, 1750.0f, 0.f) },
				{ TEXT("/Game/World/Interactables/WeaponAltar"), TEXT("WeaponAltar_Pixal3D"), TEXT("Weapon Altar"), FVector(3950.0f, 2450.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_DamageStrength_Pixal3D"), TEXT("Boost Damage"), FVector(3950.0f, 3150.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_AttackSpeed_Pixal3D"), TEXT("Boost Atk Speed"), FVector(3950.0f, 3850.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_AttackScale_Pixal3D"), TEXT("Boost Scale"), FVector(3350.0f, -3850.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_Armor_Pixal3D"), TEXT("Boost Armor"), FVector(3350.0f, -3150.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_Evasion_Pixal3D"), TEXT("Boost Evasion"), FVector(3350.0f, -2450.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_Luck_Pixal3D"), TEXT("Boost Luck"), FVector(3350.0f, -1750.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_Speed_Pixal3D"), TEXT("Boost Speed"), FVector(3350.0f, -1050.0f, 0.f) },
				{ TEXT("/Game/World/Boosts"), TEXT("Boost_Accuracy_Pixal3D"), TEXT("Boost Accuracy"), FVector(3350.0f, -350.0f, 0.f) },
				{ TEXT("/Game/World/Gates"), TEXT("StageGate_Pixal3D"), TEXT("Stage Gate"), FVector(3350.0f, 350.0f, 0.f) },
				{ TEXT("/Game/World/Gates"), TEXT("CowardiceGate_Pixal3D"), TEXT("Cowardice Gate"), FVector(3350.0f, 1050.0f, 0.f) },
				{ TEXT("/Game/World/VisualProps/Easy"), TEXT("WallLamp_Easy_Pixal3D"), TEXT("Wall Lamp"), FVector(3350.0f, 1750.0f, 0.f) },
				{ TEXT("/Game/World/VisualProps/Easy"), TEXT("WallTorch_Easy_Pixal3D"), TEXT("Wall Torch"), FVector(3350.0f, 2450.0f, 0.f) },
				{ TEXT("/Game/World/VisualProps/Easy"), TEXT("BrokenVase_Easy_Pixal3D"), TEXT("Broken Vase"), FVector(3350.0f, 3150.0f, 0.f) },
				{ TEXT("/Game/World/VisualProps/Easy"), TEXT("SkullRemains_Easy_Pixal3D"), TEXT("Skull Remains"), FVector(3350.0f, 3850.0f, 0.f) },
			};

			const bool bRepresentativeOnly = CVarT66TestRoomShowRepresentativeLineupOnly.GetValueOnGameThread() != 0;
			if (bRepresentativeOnly)
			{
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupGroupLabel"), TEXT("Representative Models"), FVector(0.f, 4400.f, 520.f), LineupActorTag());
			}
			else
			{
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupGroupLabel"), TEXT("Male Heroes"), FVector(0.f, 4400.f, 520.f), LineupActorTag());
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupGroupLabel"), TEXT("Female Heroes"), FVector(0.f, -4400.f, 520.f), LineupActorTag());
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupGroupLabel"), TEXT("Companions"), FVector(-4400.f, 0.f, 520.f), LineupActorTag());
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupGroupLabel"), TEXT("Easy Enemies"), FVector(0.f, 3300.f, 520.f), LineupActorTag());
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupGroupLabel"), TEXT("World Assets"), FVector(4400.f, 0.f, 520.f), LineupActorTag());
			}

			int32 SpawnedLineupCount = 0;
			for (const FLineupEntry& Entry : Entries)
			{
				if (T66DeprecatedFeatures::AreArcadeInteractablesDisabled()
					&& FCString::Stricmp(Entry.AssetID, TEXT("Arcade_Machine_Pixal3D")) == 0)
				{
					continue;
				}

				if (bRepresentativeOnly && !IsRepresentativeLineupAsset(Entry.AssetID))
				{
					continue;
				}
				SpawnLineupStaticMeshActor(World, Entry, false);
				SpawnLineupStaticMeshActor(World, Entry, true);
				SpawnTextLabel(World, TEXT("DEV_TestRoom_LineupLabel"), Entry.Label, Entry.Location + FVector(0.f, 0.f, 400.f), LineupActorTag());
				++SpawnedLineupCount;
			}

			UE_LOG(LogTemp, Display, TEXT("ToonStyle TestRoom spawned %d lineup entries (RepresentativeOnly=%d)."),
				SpawnedLineupCount,
				bRepresentativeOnly ? 1 : 0);
		}

		template<typename TInteractable>
		TInteractable* SpawnLiveInteractableShowcaseActor(
			UWorld* World,
			const TCHAR* Label,
			const FVector& Location,
			const FRotator& Rotation)
		{
			if (!World)
			{
				return nullptr;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			TInteractable* Interactable = World->SpawnActor<TInteractable>(
				TInteractable::StaticClass(),
				Location,
				Rotation,
				SpawnParams);
			if (!Interactable)
			{
				return nullptr;
			}

			Interactable->SetShowcaseReusable(true);
			Interactable->SetRarity(ET66Rarity::White);
			Interactable->Tags.AddUnique(LiveInteractableShowcaseActorTag());
			TagTestRoomActor(Interactable, false, false);

			#if WITH_EDITOR
			Interactable->SetActorLabel(Label);
			#endif

			return Interactable;
		}

		void SpawnLiveInteractableShowcase(UWorld* World)
		{
			if (!World)
			{
				return;
			}

			DestroyTestRoomActorsWithTag(World, LiveInteractableShowcaseActorTag());

			const FVector SpawnCenter = PlayerStartLocation() + FVector(900.f, 0.f, 0.f);
			const FRotator FacePlayerRotation(0.f, 180.f, 0.f);
			static constexpr float SpacingY = 520.f;

			AT66CrateInteractable* Crate = SpawnLiveInteractableShowcaseActor<AT66CrateInteractable>(
				World,
				TEXT("DEV_TestRoom_LiveCrate"),
				SpawnCenter + FVector(0.f, -SpacingY, 0.f),
				FacePlayerRotation);
			AT66LootWheelInteractable* LootWheel = SpawnLiveInteractableShowcaseActor<AT66LootWheelInteractable>(
				World,
				TEXT("DEV_TestRoom_LiveLootWheel"),
				SpawnCenter,
				FacePlayerRotation);
			AT66ChestInteractable* Chest = SpawnLiveInteractableShowcaseActor<AT66ChestInteractable>(
				World,
				TEXT("DEV_TestRoom_LiveChest"),
				SpawnCenter + FVector(0.f, SpacingY, 0.f),
				FacePlayerRotation);

			if (Chest)
			{
				Chest->bIsMimic = false;
				Chest->SetRarity(ET66Rarity::White);
			}

			UE_LOG(
				LogTemp,
				Display,
				TEXT("TestRoom spawned live interactable showcase in front of player start. Crate=%d LootWheel=%d Chest=%d"),
				Crate ? 1 : 0,
				LootWheel ? 1 : 0,
				Chest ? 1 : 0);
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

	void ScheduleCombatZones(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		static FTimerHandle CombatZoneTimerHandle;
		World->GetTimerManager().ClearTimer(CombatZoneTimerHandle);
		DestroyTestRoomActorsWithTag(World, MobRoomEnemyActorTag());
		DestroyTestRoomActorsWithTag(World, BossRoomActorTag());

		const bool bRequested = CVarT66TestRoomEnableCombatZones.GetValueOnGameThread() != 0
			&& !FParse::Param(FCommandLine::Get(), TEXT("T66DisableTestRoomCombatZones"));
		if (!bRequested)
		{
			return;
		}

		struct FCombatZoneState
		{
			bool bPlayerInMobRoom = false;
			bool bPlayerInBossRoom = false;
			double NextMobSpawnTime = 0.0;
			int32 MobSequenceIndex = 0;
#if !UE_BUILD_SHIPPING
			bool bAutoVisitCombatZones = false;
			double AutoVisitStartTime = 0.0;
			int32 AutoVisitStep = 0;
#endif
		};

		TSharedRef<FCombatZoneState> State = MakeShared<FCombatZoneState>();
		TWeakObjectPtr<UWorld> WeakWorld(World);
#if !UE_BUILD_SHIPPING
		State->bAutoVisitCombatZones = FParse::Param(FCommandLine::Get(), TEXT("T66TestRoomAutoVisitCombatZones"));
		State->AutoVisitStartTime = World->GetTimeSeconds();
#endif

		const auto CountActorsWithTag = [](UWorld* TimerWorld, const FName Tag)
		{
			int32 Count = 0;
			if (!TimerWorld || Tag.IsNone())
			{
				return Count;
			}

			for (TActorIterator<AActor> It(TimerWorld); It; ++It)
			{
				if (It->Tags.Contains(Tag))
				{
					++Count;
				}
			}
			return Count;
		};

		const auto ClearBossRunState = [](UWorld* TimerWorld)
		{
			if (UGameInstance* GI = TimerWorld ? TimerWorld->GetGameInstance() : nullptr)
			{
				if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
				{
					RunState->ResetBossState();
				}
			}
		};

		const auto SpawnMobRoomEnemy = [State, CountActorsWithTag](UWorld* TimerWorld, APawn* PlayerPawn)
		{
			const int32 MaxEnemies = FMath::Max(1, CVarT66TestRoomMobRoomMaxEnemies.GetValueOnGameThread());
			if (CountActorsWithTag(TimerWorld, MobRoomEnemyActorTag()) >= MaxEnemies)
			{
				return;
			}

			static const FName EasyMobIDs[] =
			{
				FName(TEXT("Slime")),
				FName(TEXT("BoneWalker")),
				FName(TEXT("RatPack")),
				FName(TEXT("CaveBat")),
				FName(TEXT("HexSlinger")),
				FName(TEXT("TombSpider")),
				FName(TEXT("StoneSentinel")),
				FName(TEXT("MimicLure")),
				FName(TEXT("BoneConjurer")),
				FName(TEXT("CryptWraith"))
			};

			const FName MobID = EasyMobIDs[State->MobSequenceIndex % UE_ARRAY_COUNT(EasyMobIDs)];
			const int32 SpawnIndex = State->MobSequenceIndex++;
			const float AngleRadians = (2.0f * PI * static_cast<float>(SpawnIndex % 8)) / 8.0f;
			const FVector RoomCenter(0.f, TestRoomSideRoomOffset, PlayerStartLocation().Z);
			const FVector SpawnOffset(
				FMath::Cos(AngleRadians) * (TestRoomSideRoomHalfExtent * 0.56f),
				FMath::Sin(AngleRadians) * (TestRoomSideRoomHalfExtent * 0.56f),
				0.f);
			const FVector SpawnLocation = RoomCenter + SpawnOffset;
			const FVector TargetLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : PlayerStartLocation();
			FVector ToPlayer = TargetLocation - SpawnLocation;
			ToPlayer.Z = 0.f;
			const FRotator SpawnRotation = ToPlayer.IsNearlyZero() ? FRotator::ZeroRotator : ToPlayer.Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66EnemyBase* Enemy = TimerWorld->SpawnActor<AT66EnemyBase>(
				AT66EnemyBase::StaticClass(),
				SpawnLocation,
				SpawnRotation,
				SpawnParams);
			if (!Enemy)
			{
				UE_LOG(LogTemp, Warning, TEXT("TestRoom mob room failed to spawn %s."), *MobID.ToString());
				return;
			}

			TagTestRoomActor(Enemy, false, false);
			Enemy->Tags.AddUnique(MobRoomEnemyActorTag());
			Enemy->ConfigureAsMob(MobID);
			Enemy->TouchDamageHearts = 0;
			Enemy->PointValue = 0;
			Enemy->XPValue = 0;
			Enemy->bDropsLoot = false;
			if (MobID == FName(TEXT("CaveBat")))
			{
				Enemy->EnemyFamily = ET66EnemyFamily::Flying;
			}
			else if (MobID == FName(TEXT("RatPack")) || MobID == FName(TEXT("MimicLure")))
			{
				Enemy->EnemyFamily = ET66EnemyFamily::Rush;
			}
			else if (MobID == FName(TEXT("HexSlinger")) || MobID == FName(TEXT("StoneSentinel")) || MobID == FName(TEXT("BoneConjurer")))
			{
				Enemy->EnemyFamily = ET66EnemyFamily::Ranged;
			}
			else
			{
				Enemy->EnemyFamily = ET66EnemyFamily::Melee;
			}
			if (UT66MobManagerSubsystem* MobManager = TimerWorld ? TimerWorld->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
			{
				MobManager->RecordRouteAttribution(
					Enemy->EnemyFamily,
					ET66RouteAttributionReason::RoutedRich_NonDirectorPath,
					ET66RouteAttributionChannel::NonDirector);
			}
			if (!Enemy->GetController())
			{
				Enemy->SpawnDefaultController();
			}
#if !UE_BUILD_SHIPPING
			Enemy->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 30.f);
#endif

			UE_LOG(LogTemp, Display, TEXT("TestRoom mob room spawned %s at %s."), *MobID.ToString(), *SpawnLocation.ToCompactString());
		};

		const auto SpawnBossRoomBoss = [ClearBossRunState](UWorld* TimerWorld, APawn* PlayerPawn)
		{
			DestroyTestRoomActorsWithTag(TimerWorld, BossRoomActorTag());
			ClearBossRunState(TimerWorld);

			UT66GameInstance* T66GI = TimerWorld ? Cast<UT66GameInstance>(TimerWorld->GetGameInstance()) : nullptr;
			FBossData BossData;
			if (!T66GI || !T66GI->GetBossData(FName(TEXT("Dungeon_SewerSlimeKing")), BossData))
			{
				BossData.BossID = FName(TEXT("Dungeon_SewerSlimeKing"));
				BossData.MaxHP = 1250;
				BossData.AwakenDistance = 2400.f;
				BossData.MoveSpeed = 180.f;
				BossData.FireIntervalSeconds = 2.4f;
				BossData.ProjectileSpeed = 760.f;
				BossData.ProjectileDamageHearts = 1;
				BossData.BossPartProfile = ET66BossPartProfile::Juggernaut;
				BossData.PlaceholderColor = FLinearColor(0.20f, 0.92f, 0.08f, 1.f);
			}

			BossData.AwakenDistance = 2400.f;
			BossData.MoveSpeed = FMath::Max(120.f, BossData.MoveSpeed * 0.5f);
			BossData.FireIntervalSeconds = FMath::Clamp(BossData.FireIntervalSeconds, 1.8f, 3.2f);

			const FVector RoomCenter(TestRoomSideRoomOffset, 0.f, PlayerStartLocation().Z);
			const FVector TargetLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : PlayerStartLocation();
			FVector ToPlayer = TargetLocation - RoomCenter;
			ToPlayer.Z = 0.f;
			const FRotator SpawnRotation = ToPlayer.IsNearlyZero() ? FRotator(0.f, 180.f, 0.f) : ToPlayer.Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AT66BossBase* Boss = TimerWorld->SpawnActor<AT66BossBase>(
				AT66BossBase::StaticClass(),
				RoomCenter,
				SpawnRotation,
				SpawnParams);
			if (!Boss)
			{
				UE_LOG(LogTemp, Warning, TEXT("TestRoom boss room failed to spawn Dungeon_SewerSlimeKing."));
				return;
			}

			TagTestRoomActor(Boss, false, false);
			Boss->Tags.AddUnique(BossRoomActorTag());
			Boss->InitializeBoss(BossData);
			Boss->SetActorRotation(SpawnRotation);
			Boss->ForceAwaken();

			UE_LOG(LogTemp, Display, TEXT("TestRoom boss room spawned and awakened Dungeon_SewerSlimeKing at %s."), *RoomCenter.ToCompactString());
		};

		World->GetTimerManager().SetTimer(
			CombatZoneTimerHandle,
			FTimerDelegate::CreateLambda([WeakWorld, State, SpawnMobRoomEnemy, SpawnBossRoomBoss, ClearBossRunState]()
			{
				UWorld* TimerWorld = WeakWorld.Get();
				if (!TimerWorld)
				{
					return;
				}

				APlayerController* PlayerController = TimerWorld->GetFirstPlayerController();
				APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
#if !UE_BUILD_SHIPPING
				if (State->bAutoVisitCombatZones && PlayerPawn)
				{
					const double Elapsed = TimerWorld->GetTimeSeconds() - State->AutoVisitStartTime;
					FVector AutoVisitLocation = PlayerPawn->GetActorLocation();
					bool bMoveForSmoke = false;
					if (State->AutoVisitStep == 0 && Elapsed >= 1.0)
					{
						AutoVisitLocation = FVector(0.f, TestRoomSideRoomOffset, PlayerStartLocation().Z);
						State->AutoVisitStep = 1;
						bMoveForSmoke = true;
						UE_LOG(LogTemp, Display, TEXT("TestRoom combat-zone smoke moved player to mob room."));
					}
					else if (State->AutoVisitStep == 1 && Elapsed >= 4.5)
					{
						AutoVisitLocation = PlayerStartLocation();
						State->AutoVisitStep = 2;
						bMoveForSmoke = true;
						UE_LOG(LogTemp, Display, TEXT("TestRoom combat-zone smoke moved player back to center from mob room."));
					}
					else if (State->AutoVisitStep == 2 && Elapsed >= 5.8)
					{
						AutoVisitLocation = FVector(TestRoomSideRoomOffset, 0.f, PlayerStartLocation().Z);
						State->AutoVisitStep = 3;
						bMoveForSmoke = true;
						UE_LOG(LogTemp, Display, TEXT("TestRoom combat-zone smoke moved player to boss room."));
					}
					else if (State->AutoVisitStep == 3 && Elapsed >= 7.1)
					{
						AutoVisitLocation = PlayerStartLocation();
						State->AutoVisitStep = 4;
						bMoveForSmoke = true;
						UE_LOG(LogTemp, Display, TEXT("TestRoom combat-zone smoke moved player back to center from boss room."));
					}

					if (bMoveForSmoke)
					{
						PlayerPawn->SetActorLocation(AutoVisitLocation, false, nullptr, ETeleportType::TeleportPhysics);
					}
				}
#endif
				const FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : PlayerStartLocation();
				const bool bInMobRoom = PlayerPawn && IsInsideBox2D(PlayerLocation, TestRoomMobRoomBox());
				const bool bInBossRoom = PlayerPawn && IsInsideBox2D(PlayerLocation, TestRoomBossRoomBox());

				if (bInMobRoom && !State->bPlayerInMobRoom)
				{
					State->NextMobSpawnTime = 0.0;
					UE_LOG(LogTemp, Display, TEXT("TestRoom mob room entered; mob spawning enabled."));
				}
				else if (!bInMobRoom && State->bPlayerInMobRoom)
				{
					DestroyTestRoomActorsWithTag(TimerWorld, MobRoomEnemyActorTag());
					UE_LOG(LogTemp, Display, TEXT("TestRoom mob room exited; mob spawning stopped and active mobs destroyed."));
				}

				if (bInMobRoom)
				{
					const double Now = TimerWorld->GetTimeSeconds();
					if (Now >= State->NextMobSpawnTime)
					{
						SpawnMobRoomEnemy(TimerWorld, PlayerPawn);
						const float Interval = FMath::Max(0.2f, CVarT66TestRoomMobSpawnIntervalSeconds.GetValueOnGameThread());
						State->NextMobSpawnTime = Now + Interval;
					}
				}

				if (bInBossRoom && !State->bPlayerInBossRoom)
				{
					SpawnBossRoomBoss(TimerWorld, PlayerPawn);
					UE_LOG(LogTemp, Display, TEXT("TestRoom boss room entered; boss aggro enabled."));
				}
				else if (!bInBossRoom && State->bPlayerInBossRoom)
				{
					DestroyTestRoomActorsWithTag(TimerWorld, BossRoomActorTag());
					ClearBossRunState(TimerWorld);
					UE_LOG(LogTemp, Display, TEXT("TestRoom boss room exited; boss stopped and removed."));
				}

				State->bPlayerInMobRoom = bInMobRoom;
				State->bPlayerInBossRoom = bInBossRoom;
			}),
			0.2f,
			true,
			0.2f);
	}

	void SpawnRoom(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		DestroyTestRoomActorsWithTag(World, RoomActorTag());

		UStaticMesh* CubeMesh = LoadCubeMesh();
		if (!CubeMesh)
		{
			return;
		}

		UMaterialInterface* FloorMaterial = LoadFloorMaterial();
		UMaterialInterface* WallMaterial = LoadWallMaterial();
		UMaterialInterface* CeilingMaterial = LoadCeilingMaterial();
		if (!CeilingMaterial)
		{
			CeilingMaterial = WallMaterial;
		}

		const FBox2D CenterBox = MakeTestRoomBox(
			FVector2D::ZeroVector,
			FVector2D(TestRoomCenterHalfExtent, TestRoomCenterHalfExtent));
		const FBox2D NorthRoomBox = TestRoomMobRoomBox();
		const FBox2D EastRoomBox = TestRoomBossRoomBox();
		const FBox2D SouthRoomBox = MakeTestRoomBox(
			FVector2D(0.f, -TestRoomSideRoomOffset),
			FVector2D(TestRoomSideRoomHalfExtent, TestRoomSideRoomHalfExtent));
		const FBox2D WestRoomBox = MakeTestRoomBox(
			FVector2D(-TestRoomSideRoomOffset, 0.f),
			FVector2D(TestRoomSideRoomHalfExtent, TestRoomSideRoomHalfExtent));
		const FBox2D NorthCorridorBox = MakeTestRoomBox(
			FVector2D(0.f, TestRoomCorridorCenterOffset),
			FVector2D(TestRoomCorridorHalfWidth, TestRoomCorridorLength * 0.5f));
		const FBox2D SouthCorridorBox = MakeTestRoomBox(
			FVector2D(0.f, -TestRoomCorridorCenterOffset),
			FVector2D(TestRoomCorridorHalfWidth, TestRoomCorridorLength * 0.5f));
		const FBox2D EastCorridorBox = MakeTestRoomBox(
			FVector2D(TestRoomCorridorCenterOffset, 0.f),
			FVector2D(TestRoomCorridorLength * 0.5f, TestRoomCorridorHalfWidth));
		const FBox2D WestCorridorBox = MakeTestRoomBox(
			FVector2D(-TestRoomCorridorCenterOffset, 0.f),
			FVector2D(TestRoomCorridorLength * 0.5f, TestRoomCorridorHalfWidth));

		const float FloorZ = -TestRoomWallThickness * 0.5f;
		const float CeilingZ = TestRoomInteriorHeight + (TestRoomWallThickness * 0.5f);
		const float WallOffset = TestRoomWallThickness * 0.5f;
		const float DoorHalfWidth = TestRoomCorridorHalfWidth;

		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_CenterFloor"), CenterBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_MobRoomFloor"), NorthRoomBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_BossRoomFloor"), EastRoomBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_SouthEmptyRoomFloor"), SouthRoomBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_WestEmptyRoomFloor"), WestRoomBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_NorthCorridorFloor"), NorthCorridorBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_SouthCorridorFloor"), SouthCorridorBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_EastCorridorFloor"), EastCorridorBox, FloorZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, FloorMaterial, TEXT("DEV_TestRoom_WestCorridorFloor"), WestCorridorBox, FloorZ, TestRoomWallThickness);

		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_CenterCeiling"), CenterBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_MobRoomCeiling"), NorthRoomBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_BossRoomCeiling"), EastRoomBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_SouthEmptyRoomCeiling"), SouthRoomBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_WestEmptyRoomCeiling"), WestRoomBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_NorthCorridorCeiling"), NorthCorridorBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_SouthCorridorCeiling"), SouthCorridorBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_EastCorridorCeiling"), EastCorridorBox, CeilingZ, TestRoomWallThickness);
		SpawnRectSurface(World, CubeMesh, CeilingMaterial, TEXT("DEV_TestRoom_WestCorridorCeiling"), WestCorridorBox, CeilingZ, TestRoomWallThickness);

		SpawnHorizontalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_CenterNorthWall"), CenterBox.Min.X, CenterBox.Max.X, CenterBox.Max.Y + WallOffset, 0.f, DoorHalfWidth);
		SpawnHorizontalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_CenterSouthWall"), CenterBox.Min.X, CenterBox.Max.X, CenterBox.Min.Y - WallOffset, 0.f, DoorHalfWidth);
		SpawnVerticalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_CenterEastWall"), CenterBox.Max.X + WallOffset, CenterBox.Min.Y, CenterBox.Max.Y, 0.f, DoorHalfWidth);
		SpawnVerticalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_CenterWestWall"), CenterBox.Min.X - WallOffset, CenterBox.Min.Y, CenterBox.Max.Y, 0.f, DoorHalfWidth);

		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_MobRoomNorthWall"), NorthRoomBox.Min.X, NorthRoomBox.Max.X, NorthRoomBox.Max.Y + WallOffset);
		SpawnHorizontalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_MobRoomSouthWall"), NorthRoomBox.Min.X, NorthRoomBox.Max.X, NorthRoomBox.Min.Y - WallOffset, 0.f, DoorHalfWidth);
		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_MobRoomWestWall"), NorthRoomBox.Min.X - WallOffset, NorthRoomBox.Min.Y, NorthRoomBox.Max.Y);
		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_MobRoomEastWall"), NorthRoomBox.Max.X + WallOffset, NorthRoomBox.Min.Y, NorthRoomBox.Max.Y);

		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_BossRoomEastWall"), EastRoomBox.Max.X + WallOffset, EastRoomBox.Min.Y, EastRoomBox.Max.Y);
		SpawnVerticalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_BossRoomWestWall"), EastRoomBox.Min.X - WallOffset, EastRoomBox.Min.Y, EastRoomBox.Max.Y, 0.f, DoorHalfWidth);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_BossRoomNorthWall"), EastRoomBox.Min.X, EastRoomBox.Max.X, EastRoomBox.Max.Y + WallOffset);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_BossRoomSouthWall"), EastRoomBox.Min.X, EastRoomBox.Max.X, EastRoomBox.Min.Y - WallOffset);

		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_SouthRoomSouthWall"), SouthRoomBox.Min.X, SouthRoomBox.Max.X, SouthRoomBox.Min.Y - WallOffset);
		SpawnHorizontalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_SouthRoomNorthWall"), SouthRoomBox.Min.X, SouthRoomBox.Max.X, SouthRoomBox.Max.Y + WallOffset, 0.f, DoorHalfWidth);
		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_SouthRoomWestWall"), SouthRoomBox.Min.X - WallOffset, SouthRoomBox.Min.Y, SouthRoomBox.Max.Y);
		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_SouthRoomEastWall"), SouthRoomBox.Max.X + WallOffset, SouthRoomBox.Min.Y, SouthRoomBox.Max.Y);

		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_WestRoomWestWall"), WestRoomBox.Min.X - WallOffset, WestRoomBox.Min.Y, WestRoomBox.Max.Y);
		SpawnVerticalWallWithGap(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_WestRoomEastWall"), WestRoomBox.Max.X + WallOffset, WestRoomBox.Min.Y, WestRoomBox.Max.Y, 0.f, DoorHalfWidth);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_WestRoomNorthWall"), WestRoomBox.Min.X, WestRoomBox.Max.X, WestRoomBox.Max.Y + WallOffset);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_WestRoomSouthWall"), WestRoomBox.Min.X, WestRoomBox.Max.X, WestRoomBox.Min.Y - WallOffset);

		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_NorthCorridorWestWall"), -DoorHalfWidth - WallOffset, CenterBox.Max.Y, NorthRoomBox.Min.Y);
		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_NorthCorridorEastWall"), DoorHalfWidth + WallOffset, CenterBox.Max.Y, NorthRoomBox.Min.Y);
		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_SouthCorridorWestWall"), -DoorHalfWidth - WallOffset, SouthRoomBox.Max.Y, CenterBox.Min.Y);
		SpawnVerticalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_SouthCorridorEastWall"), DoorHalfWidth + WallOffset, SouthRoomBox.Max.Y, CenterBox.Min.Y);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_EastCorridorNorthWall"), CenterBox.Max.X, EastRoomBox.Min.X, DoorHalfWidth + WallOffset);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_EastCorridorSouthWall"), CenterBox.Max.X, EastRoomBox.Min.X, -DoorHalfWidth - WallOffset);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_WestCorridorNorthWall"), WestRoomBox.Max.X, CenterBox.Min.X, DoorHalfWidth + WallOffset);
		SpawnHorizontalWallSegment(World, CubeMesh, WallMaterial, TEXT("DEV_TestRoom_WestCorridorSouthWall"), WestRoomBox.Max.X, CenterBox.Min.X, -DoorHalfWidth - WallOffset);

		SpawnTextLabel(World, TEXT("DEV_TestRoom_RoomLabel"), TEXT("MOBS"), FVector(0.f, TestRoomSideRoomOffset, 540.f), RoomActorTag());
		SpawnTextLabel(World, TEXT("DEV_TestRoom_RoomLabel"), TEXT("BOSS"), FVector(TestRoomSideRoomOffset, 0.f, 540.f), RoomActorTag());
		SpawnTextLabel(World, TEXT("DEV_TestRoom_RoomLabel"), TEXT("EMPTY"), FVector(0.f, -TestRoomSideRoomOffset, 540.f), RoomActorTag());
		SpawnTextLabel(World, TEXT("DEV_TestRoom_RoomLabel"), TEXT("EMPTY"), FVector(-TestRoomSideRoomOffset, 0.f, 540.f), RoomActorTag());

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

		DestroyTestRoomActorsWithTag(World, LightingActorTag());

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
