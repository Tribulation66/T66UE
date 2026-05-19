// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerLighting.h"

#include "Gameplay/T66ThemeAtmosphereData.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/StrongObjectPtr.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66TowerLighting, Log, All);

namespace
{
	static const FName T66AtmosphereSparedTag(TEXT("T66_AtmosphereSpared"));
	static const FName T66TowerTorchLightingTag(TEXT("T66_TowerTorchLighting"));
	static constexpr float T66TorchWallOffsetUU = 80.0f;

	static TAutoConsoleVariable<int32> CVarT66TorchPlaceholderMesh(
		TEXT("t66.TorchPlaceholderMesh"),
		1,
		TEXT("Enables tiny placeholder cube meshes at procedural torch light locations."));

	static FName T66MakeFloorLightingTag(const int32 FloorNumber)
	{
		return FName(*FString::Printf(TEXT("T66_TowerTorchLighting_Floor_%02d"), FloorNumber));
	}

	static bool T66BoxesOverlap(const FBox2D& A, const FBox2D& B)
	{
		return A.Min.X <= B.Max.X
			&& A.Max.X >= B.Min.X
			&& A.Min.Y <= B.Max.Y
			&& A.Max.Y >= B.Min.Y;
	}

	static bool T66IntersectsDoorwayHeader(const FBox2D& WallBox, const TArray<FBox2D>& DoorwayHeaderBoxes)
	{
		for (const FBox2D& HeaderBox : DoorwayHeaderBoxes)
		{
			if (T66BoxesOverlap(WallBox, HeaderBox))
			{
				return true;
			}
		}
		return false;
	}

	static const T66TowerMapTerrain::FGridCell* T66FindNearestWalkableCell(
		const T66TowerMapTerrain::FFloor& Floor,
		const FVector2D& Position)
	{
		const T66TowerMapTerrain::FGridCell* BestCell = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		for (const T66TowerMapTerrain::FGridCell& Cell : Floor.GridCells)
		{
			if (Cell.Semantic == T66TowerMapTerrain::ET66TowerGridCellSemantic::Unused)
			{
				continue;
			}

			const FVector2D CellCenter(Cell.WorldCenter.X, Cell.WorldCenter.Y);
			const float DistSq = FVector2D::DistSquared(Position, CellCenter);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestCell = &Cell;
			}
		}
		return BestCell;
	}

	static FVector2D T66ResolveCorridorNormal(const T66TowerMapTerrain::FFloor& Floor, const FBox2D& WallBox)
	{
		const FVector2D WallCenter = WallBox.GetCenter();
		if (const T66TowerMapTerrain::FGridCell* NearestCell = T66FindNearestWalkableCell(Floor, WallCenter))
		{
			const FVector2D CellCenter(NearestCell->WorldCenter.X, NearestCell->WorldCenter.Y);
			FVector2D Normal = CellCenter - WallCenter;
			if (Normal.SizeSquared() > KINDA_SMALL_NUMBER)
			{
				Normal.Normalize();
				return Normal;
			}
		}

		const FVector2D Size = WallBox.GetSize();
		return Size.X >= Size.Y ? FVector2D(0.0f, 1.0f) : FVector2D(1.0f, 0.0f);
	}

	static FString T66FloorRoleToString(const T66TowerMapTerrain::ET66TowerFloorRole Role)
	{
		switch (Role)
		{
		case T66TowerMapTerrain::ET66TowerFloorRole::Start:
			return TEXT("Start");
		case T66TowerMapTerrain::ET66TowerFloorRole::Gameplay:
			return TEXT("Gameplay");
		case T66TowerMapTerrain::ET66TowerFloorRole::Boss:
			return TEXT("Boss");
		default:
			return TEXT("Unknown");
		}
	}

	struct FT66TorchPlacementResult
	{
		TArray<FVector> Positions;
		int32 MazeTorchCount = 0;
		int32 ShellTorchCount = 0;
		bool bCapReached = false;
	};

	static FVector2D T66ResolveShellWallNormal(const T66TowerMapTerrain::FFloor& Floor, const FBox2D& WallBox)
	{
		const FVector2D FloorCenter(Floor.Center.X, Floor.Center.Y);
		FVector2D Normal = FloorCenter - WallBox.GetCenter();
		if (Normal.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			Normal.Normalize();
			return Normal;
		}

		return T66ResolveCorridorNormal(Floor, WallBox);
	}

	static bool T66TryAppendTorchForWallBox(
		const T66TowerMapTerrain::FFloor& Floor,
		const FBox2D& WallBox,
		const FT66ThemeAtmosphereSpec& Spec,
		const bool bOuterShell,
		FT66TorchPlacementResult& Result)
	{
		const int32 MaxTorchLights = FMath::Max(0, Spec.TorchMaxPerFloor);
		if (Result.Positions.Num() >= MaxTorchLights)
		{
			Result.bCapReached = MaxTorchLights > 0;
			return false;
		}
		if (T66IntersectsDoorwayHeader(WallBox, Floor.DoorwayHeaderBoxes))
		{
			return true;
		}

		const FVector2D WallCenter = WallBox.GetCenter();
		const FVector2D WallNormal = bOuterShell
			? T66ResolveShellWallNormal(Floor, WallBox)
			: T66ResolveCorridorNormal(Floor, WallBox);
		const FVector2D OffsetPosition = WallCenter + WallNormal * T66TorchWallOffsetUU;
		Result.Positions.Add(FVector(OffsetPosition.X, OffsetPosition.Y, Floor.SurfaceZ + Spec.TorchVerticalOffset));
		if (bOuterShell)
		{
			++Result.ShellTorchCount;
		}
		else
		{
			++Result.MazeTorchCount;
		}

		if (Result.Positions.Num() >= MaxTorchLights)
		{
			Result.bCapReached = true;
			return false;
		}
		return true;
	}

	static FT66TorchPlacementResult T66BuildTorchPositionsForFloor(
		const T66TowerMapTerrain::FFloor& Floor,
		const FT66ThemeAtmosphereSpec& Spec)
	{
		FT66TorchPlacementResult Result;
		const TArray<FBox2D>& MazeSourceBoxes = Floor.TrapEligibleWallBoxes.Num() > 0
			? Floor.TrapEligibleWallBoxes
			: Floor.MazeWallBoxes;

		for (const FBox2D& WallBox : MazeSourceBoxes)
		{
			if (!T66TryAppendTorchForWallBox(Floor, WallBox, Spec, false, Result))
			{
				return Result;
			}
		}

		for (const FBox2D& WallBox : Floor.OuterShellWallBoxes)
		{
			if (!T66TryAppendTorchForWallBox(Floor, WallBox, Spec, true, Result))
			{
				return Result;
			}
		}

		return Result;
	}

	static UStaticMesh* T66GetTorchPlaceholderMesh()
	{
		static TStrongObjectPtr<UStaticMesh> Mesh;
		if (!Mesh.IsValid())
		{
			Mesh.Reset(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
		}
		return Mesh.Get();
	}

	static UMaterialInterface* T66GetTorchPlaceholderMaterial()
	{
		static TStrongObjectPtr<UMaterialInterface> Material;
		if (!Material.IsValid())
		{
			Material.Reset(LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_GLB_Unlit.M_GLB_Unlit")));
		}
		return Material.Get();
	}

	static UTexture* T66GetWhiteTexture()
	{
		static TStrongObjectPtr<UTexture> Texture;
		if (!Texture.IsValid())
		{
			Texture.Reset(LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")));
		}
		if (!Texture.IsValid())
		{
			Texture.Reset(LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture")));
		}
		return Texture.Get();
	}

	static UMaterialInterface* T66GetTorchLightFunction()
	{
		static TStrongObjectPtr<UMaterialInterface> Material;
		if (!Material.IsValid())
		{
			Material.Reset(LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_TorchFlicker_LightFn.M_TorchFlicker_LightFn")));
		}
		return Material.Get();
	}

	static void T66AddTorchPlaceholderMesh(AT66TowerLightingActor* LightingActor, const FVector& RelativeLocation, int32 Index)
	{
		if (!LightingActor || CVarT66TorchPlaceholderMesh.GetValueOnAnyThread() == 0)
		{
			return;
		}

		UStaticMesh* Mesh = T66GetTorchPlaceholderMesh();
		if (!Mesh)
		{
			return;
		}

		const FName ComponentName = FName(*FString::Printf(TEXT("TorchMarker_%02d"), Index));
		UStaticMeshComponent* Marker = NewObject<UStaticMeshComponent>(LightingActor, ComponentName);
		if (!Marker)
		{
			return;
		}

		Marker->SetupAttachment(LightingActor->GetRootComponent());
		Marker->SetRelativeLocation(RelativeLocation);
		Marker->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.4f));
		Marker->SetMobility(EComponentMobility::Movable);
		Marker->SetStaticMesh(Mesh);
		Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Marker->SetCastShadow(false);
		Marker->bReceivesDecals = false;

		if (UMaterialInterface* BaseMaterial = T66GetTorchPlaceholderMaterial())
		{
			if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, LightingActor))
			{
				if (UTexture* WhiteTexture = T66GetWhiteTexture())
				{
					MID->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
					MID->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
				}
				MID->SetVectorParameterValue(TEXT("Tint"), FLinearColor(1.0f, 0.6f, 0.2f, 1.0f));
				MID->SetScalarParameterValue(TEXT("Brightness"), 2.5f);
				Marker->SetMaterial(0, MID);
			}
		}

		LightingActor->AddInstanceComponent(Marker);
		Marker->RegisterComponent();
	}
}

AT66TowerLightingActor::AT66TowerLightingActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);
}

void AT66TowerLightingActor::SetFloorNumber(const int32 InFloorNumber)
{
	FloorNumber = InFloorNumber;
	Tags.AddUnique(T66MakeFloorLightingTag(FloorNumber));
}

AT66TowerLightingActor* T66TowerLighting::SpawnFloorTorchLights(
	UWorld* World,
	const T66TowerMapTerrain::FFloor& Floor,
	const T66TowerMapTerrain::FLayout& Layout,
	const T66TowerMapTerrain::ET66TowerGameplayLevelTheme Theme,
	AActor* AttachOwner)
{
	(void)Layout;

	if (!World)
	{
		return nullptr;
	}

	DestroyFloorTorchLights(World, Floor);
	const FString FloorRoleString = T66FloorRoleToString(Floor.FloorRole);
	if (Theme != T66TowerMapTerrain::ET66TowerGameplayLevelTheme::Dungeon)
	{
		UE_LOG(
			LogT66TowerLighting,
			Display,
			TEXT("[ATMOSPHERE] Skipped torch placement for floor %d (role=%s, reason=%s)"),
			Floor.FloorNumber,
			*FloorRoleString,
			TEXT("non-dungeon-theme"));
		return nullptr;
	}
	if (Floor.FloorRole == T66TowerMapTerrain::ET66TowerFloorRole::Boss)
	{
		UE_LOG(
			LogT66TowerLighting,
			Display,
			TEXT("[ATMOSPHERE] Skipped torch placement for floor %d (role=%s, reason=%s)"),
			Floor.FloorNumber,
			*FloorRoleString,
			TEXT("boss-floor"));
		return nullptr;
	}
	if (Floor.FloorRole != T66TowerMapTerrain::ET66TowerFloorRole::Start
		&& Floor.FloorRole != T66TowerMapTerrain::ET66TowerFloorRole::Gameplay)
	{
		UE_LOG(
			LogT66TowerLighting,
			Display,
			TEXT("[ATMOSPHERE] Skipped torch placement for floor %d (role=%s, reason=%s)"),
			Floor.FloorNumber,
			*FloorRoleString,
			TEXT("unsupported-floor-role"));
		return nullptr;
	}

	const FT66ThemeAtmosphereSpec& Spec = T66ThemeAtmosphereData::GetSpecForTheme(Theme);

	const FT66TorchPlacementResult Placement = T66BuildTorchPositionsForFloor(Floor, Spec);
	if (Placement.Positions.Num() <= 0)
	{
		UE_LOG(
			LogT66TowerLighting,
			Display,
			TEXT("[ATMOSPHERE] Skipped torch placement for floor %d (role=%s, reason=%s)"),
			Floor.FloorNumber,
			*FloorRoleString,
			TEXT("no-eligible-wall-boxes"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AT66TowerLightingActor* LightingActor = World->SpawnActor<AT66TowerLightingActor>(
		AT66TowerLightingActor::StaticClass(),
		Floor.Center,
		FRotator::ZeroRotator,
		SpawnParams);
	if (!LightingActor)
	{
		return nullptr;
	}

	LightingActor->Tags.AddUnique(T66AtmosphereSparedTag);
	LightingActor->Tags.AddUnique(T66TowerTorchLightingTag);
	LightingActor->SetFloorNumber(Floor.FloorNumber);
	if (!Floor.FloorTag.IsNone())
	{
		LightingActor->Tags.AddUnique(Floor.FloorTag);
	}
	if (AttachOwner)
	{
		LightingActor->AttachToActor(AttachOwner, FAttachmentTransformRules::KeepWorldTransform);
	}

	UMaterialInterface* LightFunction = T66GetTorchLightFunction();
	for (int32 Index = 0; Index < Placement.Positions.Num(); ++Index)
	{
		const FVector RelativeLocation = Placement.Positions[Index] - LightingActor->GetActorLocation();
		const FName ComponentName = FName(*FString::Printf(TEXT("TorchLight_%02d"), Index));
		UPointLightComponent* Torch = NewObject<UPointLightComponent>(LightingActor, ComponentName);
		if (!Torch)
		{
			continue;
		}

		Torch->SetupAttachment(LightingActor->GetRootComponent());
		Torch->SetRelativeLocation(RelativeLocation);
		Torch->SetMobility(EComponentMobility::Movable);
		Torch->SetIntensity(Spec.TorchIntensity);
		Torch->SetLightColor(Spec.TorchColor);
		Torch->SetAttenuationRadius(Spec.TorchAttenuationRadius);
		Torch->SetCastShadows(false);
		Torch->SetUseInverseSquaredFalloff(false);
		Torch->SetLightFalloffExponent(Spec.TorchFalloffExponent);
		if (LightFunction)
		{
			Torch->SetLightFunctionMaterial(LightFunction);
		}
		LightingActor->AddInstanceComponent(Torch);
		Torch->RegisterComponent();

		T66AddTorchPlaceholderMesh(LightingActor, RelativeLocation, Index);
	}

	UE_LOG(
		LogT66TowerLighting,
		Display,
		TEXT("[ATMOSPHERE] Spawned %d Dungeon torch light(s) for floor %d ")
		TEXT("(maze=%d shell=%d, intensity=%.1f radius=%.1f color=%s falloff=%.2f vOffset=%.1f cap=%d) %s"),
		Placement.Positions.Num(),
		Floor.FloorNumber,
		Placement.MazeTorchCount,
		Placement.ShellTorchCount,
		Spec.TorchIntensity,
		Spec.TorchAttenuationRadius,
		*Spec.TorchColor.ToString(),
		Spec.TorchFalloffExponent,
		Spec.TorchVerticalOffset,
		Spec.TorchMaxPerFloor,
		Placement.bCapReached ? TEXT("(CAP REACHED)") : TEXT(""));

	return LightingActor;
}

void T66TowerLighting::DestroyFloorTorchLights(UWorld* World, const T66TowerMapTerrain::FFloor& Floor)
{
	if (!World)
	{
		return;
	}

	const FName FloorLightingTag = T66MakeFloorLightingTag(Floor.FloorNumber);
	TArray<AT66TowerLightingActor*> ActorsToDestroy;
	for (TActorIterator<AT66TowerLightingActor> It(World); It; ++It)
	{
		if (AT66TowerLightingActor* Actor = *It; Actor && Actor->ActorHasTag(FloorLightingTag))
		{
			ActorsToDestroy.Add(Actor);
		}
	}

	for (AT66TowerLightingActor* Actor : ActorsToDestroy)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}
