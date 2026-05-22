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
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/T66ThemeAtmosphereData.h"
#include "Gameplay/T66EnemyBase.h"
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

static TAutoConsoleVariable<int32> CVarT66TestRoomSpawnVATSlimeChase(
	TEXT("t66.TestRoom.SpawnVATSlimeChase"),
	1,
	TEXT("Spawns a VAT Slime chase target in TestRoom after the player spawns. Disable with t66.TestRoom.SpawnVATSlimeChase=0."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66TestRoomVATSlimeDelaySeconds(
	TEXT("t66.TestRoom.VATSlimeDelaySeconds"),
	2.0f,
	TEXT("Delay before spawning the TestRoom VAT Slime chase target."),
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

	FName VATSlimeChaseActorTag()
	{
		static const FName Tag(TEXT("T66_TestRoom_VATSlimeChase"));
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
				{ TEXT("/Game/World/Interactables/Vending"), TEXT("QuickReviveVending_Pixal3D"), TEXT("Quick Revive"), FVector(3950.0f, -350.0f, 0.f) },
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

	void ScheduleVATSlimeChase(UWorld* World)
	{
		if (!World)
		{
			return;
		}

		const bool bRequested = CVarT66TestRoomSpawnVATSlimeChase.GetValueOnGameThread() != 0
			|| FParse::Param(FCommandLine::Get(), TEXT("T66TestRoomSpawnVATSlime"));
		if (!bRequested)
		{
			return;
		}

		DestroyTestRoomActorsWithTag(World, VATSlimeChaseActorTag());

		FTimerHandle SpawnTimerHandle;
		TWeakObjectPtr<UWorld> WeakWorld(World);
		const float DelaySeconds = FMath::Max(0.f, CVarT66TestRoomVATSlimeDelaySeconds.GetValueOnGameThread());
		World->GetTimerManager().SetTimer(
			SpawnTimerHandle,
			FTimerDelegate::CreateLambda([WeakWorld]()
			{
				UWorld* TimerWorld = WeakWorld.Get();
				if (!TimerWorld)
				{
					return;
				}

				DestroyTestRoomActorsWithTag(TimerWorld, VATSlimeChaseActorTag());

				APlayerController* PlayerController = TimerWorld->GetFirstPlayerController();
				APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
				const FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : PlayerStartLocation();
				FVector SpawnLocation = PlayerLocation + FVector(850.f, 0.f, 0.f);
				SpawnLocation.Z = PlayerStartLocation().Z;

				FVector ToPlayer = PlayerLocation - SpawnLocation;
				ToPlayer.Z = 0.f;
				const FRotator SpawnRotation = ToPlayer.IsNearlyZero() ? FRotator::ZeroRotator : ToPlayer.Rotation();

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AT66EnemyBase* Slime = TimerWorld->SpawnActor<AT66EnemyBase>(
					AT66EnemyBase::StaticClass(),
					SpawnLocation,
					SpawnRotation,
					SpawnParams);
				if (!Slime)
				{
					UE_LOG(LogTemp, Warning, TEXT("TestRoom VAT Slime QA spawn failed."));
					return;
				}

				TagTestRoomActor(Slime, false, false);
				Slime->Tags.AddUnique(VATSlimeChaseActorTag());
				Slime->ConfigureAsMob(FName(TEXT("Slime")));
				Slime->TouchDamageHearts = 0;
				Slime->PointValue = 0;
				Slime->XPValue = 0;
				Slime->bDropsLoot = false;
				Slime->EnemyFamily = ET66EnemyFamily::Melee;
				if (!Slime->GetController())
				{
					Slime->SpawnDefaultController();
				}
#if !UE_BUILD_SHIPPING
				Slime->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 30.f);
#endif

				UE_LOG(LogTemp, Display, TEXT("TestRoom VAT Slime QA spawned at %.1f %.1f %.1f and configured to chase the player."),
					SpawnLocation.X,
					SpawnLocation.Y,
					SpawnLocation.Z);
			}),
			DelaySeconds,
			false);
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
