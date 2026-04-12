// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PropSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66GameplayLayout.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Core/T66GameInstance.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66PilotableTractor.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Props, Log, All);

namespace
{
	static const FName T66MapPlatformTag(TEXT("T66_Map_Platform"));
	static const FName T66BarnRowName(TEXT("Barn"));
	static const FName T66BoulderRowName(TEXT("Boulder"));
	static const FName T66FenceRowName(TEXT("Fence"));
	static const FName T66Fence2RowName(TEXT("Fence2"));
	static const FName T66Fence3RowName(TEXT("Fence3"));
	static const FName T66HaybellRowName(TEXT("Haybell"));
	static const FName T66LogRowName(TEXT("Log"));
	static const FName T66RocksRowName(TEXT("Rocks"));
	static const FName T66ScarecrowRowName(TEXT("Scarecrow"));
	static const FName T66SiloRowName(TEXT("Silo"));
	static const FName T66StumpRowName(TEXT("Stump"));
	static const FName T66TractorRowName(TEXT("Tractor"));
	static const FName T66TreeRowName(TEXT("Tree"));
	static const FName T66Tree2RowName(TEXT("Tree2"));
	static const FName T66Tree3RowName(TEXT("Tree3"));
	static const FName T66TrothRowName(TEXT("Troth"));
	static const FName T66WindmillRowName(TEXT("Windmill"));
	static constexpr float T66MainMapBoulderScaleMultiplier = 5.0f;
	static constexpr int32 T66MainMapBoulderBudget = 3;
	static constexpr int32 T66MainMapFarmsteadCountMin = 4;
	static constexpr int32 T66MainMapFarmsteadCountMax = 5;
	static constexpr int32 T66MainMapGroveCountMin = 3;
	static constexpr int32 T66MainMapGroveCountMax = 4;
	static constexpr float T66MainMapClusterEdgeMarginCells = 6.0f;
	static constexpr float T66MainMapFarmsteadSeparationCells = 22.0f;
	static constexpr float T66MainMapGroveSeparationCells = 12.0f;
	static constexpr float T66MainMapClusterToSpawnKeepClearCells = 6.0f;
	static constexpr float T66MainMapClusterToBossKeepClearCells = 8.0f;
	static constexpr float T66MainMapClusterToAnchorKeepClearCells = 8.0f;
	static constexpr int32 T66MainMapGroupedSpawnAttempts = 18;

	enum class ET66MainMapPropClusterType : uint8
	{
		Farmstead,
		Grove,
	};

	struct FT66ResolvedPropRow
	{
		const FT66PropRow* Row = nullptr;
		UStaticMesh* Mesh = nullptr;
		bool bSpawnPilotableTractor = false;
		bool bOversizedMainMapBoulder = false;
	};

	struct FT66MainMapClusterAnchor
	{
		ET66MainMapPropClusterType Type = ET66MainMapPropClusterType::Farmstead;
		FIntPoint Coordinate = FIntPoint::ZeroValue;
		FVector Location = FVector::ZeroVector;
	};

	static void T66EnsureMeshHasBlockingCollision(UStaticMesh* Mesh)
	{
		if (!Mesh)
		{
			return;
		}

		if (UBodySetup* BodySetup = Mesh->GetBodySetup())
		{
			if (BodySetup->CollisionTraceFlag != CTF_UseComplexAsSimple)
			{
				BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
				BodySetup->CreatePhysicsMeshes();
			}
		}
	}

	static bool T66ShouldUsePrimitiveForGrounding(const UPrimitiveComponent* Primitive)
	{
		if (!Primitive || !Primitive->IsRegistered())
		{
			return false;
		}

		const ECollisionEnabled::Type CollisionEnabled = Primitive->GetCollisionEnabled();
		const bool bIsVisible = Primitive->IsVisible();
		const bool bHasSolidCollision =
			CollisionEnabled == ECollisionEnabled::QueryAndPhysics ||
			CollisionEnabled == ECollisionEnabled::PhysicsOnly;

		return bIsVisible || bHasSolidCollision;
	}

	static bool T66TryGetActorGroundingBottomZ(const AActor* Actor, float& OutBottomZ)
	{
		if (!Actor)
		{
			return false;
		}

		TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
		Actor->GetComponents(PrimitiveComponents);

		bool bFoundBottom = false;
		float BottomZ = 0.0f;

		for (const UPrimitiveComponent* Primitive : PrimitiveComponents)
		{
			if (!T66ShouldUsePrimitiveForGrounding(Primitive))
			{
				continue;
			}

			const FBoxSphereBounds Bounds = Primitive->CalcBounds(Primitive->GetComponentTransform());
			const float ComponentBottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
			if (!bFoundBottom || ComponentBottomZ < BottomZ)
			{
				bFoundBottom = true;
				BottomZ = ComponentBottomZ;
			}
		}

		if (!bFoundBottom)
		{
			return false;
		}

		OutBottomZ = BottomZ;
		return true;
	}

	static void T66SnapActorBottomToGroundPoint(AActor* Actor, const FVector& GroundPoint)
	{
		if (!Actor)
		{
			return;
		}

		float CurrentBottomZ = 0.0f;
		if (!T66TryGetActorGroundingBottomZ(Actor, CurrentBottomZ))
		{
			FVector NewLocation = Actor->GetActorLocation();
			NewLocation.Z = GroundPoint.Z;
			Actor->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
			return;
		}

		Actor->AddActorWorldOffset(
			FVector(0.0f, 0.0f, GroundPoint.Z - CurrentBottomZ),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
	}

	static bool T66IsRejectedFarmGroundComponent(const UPrimitiveComponent* Primitive)
	{
		if (!Primitive)
		{
			return true;
		}

		const FString ComponentName = Primitive->GetName();
		return ComponentName.StartsWith(TEXT("FarmWall"))
			|| ComponentName.StartsWith(TEXT("TerrainSafetyCatch"));
	}

	static bool T66TracePropGround(
		UWorld* World,
		const FVector& Start,
		const FVector& End,
		bool bRestrictToFarmTerrain,
		const TArray<TObjectPtr<AActor>>& IgnoredActors,
		FHitResult& OutHit)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66PropGroundTrace), false);
		for (AActor* IgnoredActor : IgnoredActors)
		{
			if (IgnoredActor)
			{
				Params.AddIgnoredActor(IgnoredActor);
			}
		}

		TArray<FHitResult> Hits;
		if (!World->LineTraceMultiByChannel(Hits, Start, End, ECC_WorldStatic, Params) &&
			!World->LineTraceMultiByChannel(Hits, Start, End, ECC_Visibility, Params))
		{
			return false;
		}

		for (const FHitResult& Hit : Hits)
		{
			const UPrimitiveComponent* HitComponent = Hit.GetComponent();
			const AActor* HitActor = Hit.GetActor();
			if (!HitComponent || !HitActor)
			{
				continue;
			}

			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			if (bRestrictToFarmTerrain)
			{
				if (!HitActor->ActorHasTag(T66MapPlatformTag))
				{
					continue;
				}

				if (T66IsRejectedFarmGroundComponent(HitComponent))
				{
					continue;
				}
			}

			OutHit = Hit;
			return true;
		}

		return false;
	}

	static bool T66TryGetBoardCellLocation(
		const T66MainMapTerrain::FBoard& Board,
		const FIntPoint& Coordinate,
		FVector& OutLocation)
	{
		return T66MainMapTerrain::TryGetCellLocation(Board, Coordinate, 0.0f, OutLocation);
	}

	static bool T66IsCoordinateWithinCellRadius(const FIntPoint& A, const FIntPoint& B, float RadiusCells)
	{
		if (A.X == INDEX_NONE || A.Y == INDEX_NONE || B.X == INDEX_NONE || B.Y == INDEX_NONE)
		{
			return false;
		}

		return FVector2D::Distance(FVector2D(A.X, A.Y), FVector2D(B.X, B.Y)) < RadiusCells;
	}
}

UDataTable* UT66PropSubsystem::GetPropsDataTable() const
{
	if (CachedPropsDataTable) return CachedPropsDataTable;

	CachedPropsDataTable = Cast<UDataTable>(StaticLoadObject(
		UDataTable::StaticClass(), nullptr,
		TEXT("/Game/Data/DT_Props.DT_Props")));
	return CachedPropsDataTable;
}

void UT66PropSubsystem::SpawnPropsForStage(UWorld* World, int32 Seed)
{
	SpawnPropsInternal(World, Seed, nullptr, 50000.f, 8000.f, -16000.f, true, FVector::ZeroVector, 0.f);
}

void UT66PropSubsystem::SpawnMainMapPropsForStage(UWorld* World, int32 Seed, const TArray<FName>& AllowedRows)
{
	if (AllowedRows.Num() == 0)
	{
		ClearProps();
		return;
	}

	const UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	const FT66MapPreset MainMapPreset = T66MainMapTerrain::BuildPresetForDifficulty(
		T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy,
		Seed);

	const T66MainMapTerrain::FSettings MainMapSettings = T66MainMapTerrain::MakeSettings(MainMapPreset);
	const float MainHalfExtent = FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.75f);
	const FVector KeepClearCenter = T66MainMapTerrain::GetPreferredSpawnLocation(MainMapPreset, 200.f);
	const float KeepClearRadius = MainMapSettings.CellSize * 1.6f;
	const float TraceStartZ = T66MainMapTerrain::GetTraceZ(MainMapPreset);
	const float TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;

	if (!SpawnMainMapGroupedProps(
		World,
		Seed,
		AllowedRows,
		MainHalfExtent,
		TraceStartZ,
		TraceEndZ,
		KeepClearCenter,
		KeepClearRadius))
	{
		UE_LOG(LogT66Props, Warning, TEXT("[PROPS] Main-map grouped placement unavailable; falling back to scatter."));
		SpawnPropsInternal(
			World,
			Seed,
			&AllowedRows,
			MainHalfExtent,
			TraceStartZ,
			TraceEndZ,
			false,
			KeepClearCenter,
			KeepClearRadius);
	}
}

bool UT66PropSubsystem::SpawnMainMapGroupedProps(
	UWorld* World,
	int32 Seed,
	const TArray<FName>& AllowedRows,
	float MainHalfExtent,
	float TraceStartZ,
	float TraceEndZ,
	const FVector& KeepClearCenter,
	float KeepClearRadius)
{
	if (!World)
	{
		return false;
	}

	ClearProps();

	UDataTable* DT = GetPropsDataTable();
	if (!DT)
	{
		return false;
	}

	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
	const FT66MapPreset MainMapPreset = T66MainMapTerrain::BuildPresetForDifficulty(
		T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy,
		Seed);

	T66MainMapTerrain::FBoard Board;
	if (!T66MainMapTerrain::Generate(MainMapPreset, Board))
	{
		return false;
	}

	FRandomStream Rng(Seed + 9973);
	static constexpr float SpawnZ = 220.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TMap<FName, FT66ResolvedPropRow> ResolvedRows;
	for (const FName& RowName : AllowedRows)
	{
		const FT66PropRow* Row = DT->FindRow<FT66PropRow>(RowName, TEXT("PropSubsystem"));
		if (!Row || Row->Mesh.IsNull())
		{
			continue;
		}

		const bool bSpawnPilotableTractor = (RowName == T66TractorRowName);
		UStaticMesh* Mesh = Row->Mesh.LoadSynchronous();
		T66EnsureMeshHasBlockingCollision(Mesh);
		if (!Mesh && !bSpawnPilotableTractor)
		{
			continue;
		}

		FT66ResolvedPropRow& Entry = ResolvedRows.Add(RowName);
		Entry.Row = Row;
		Entry.Mesh = Mesh;
		Entry.bSpawnPilotableTractor = bSpawnPilotableTractor;
		Entry.bOversizedMainMapBoulder = (RowName == T66BoulderRowName);
	}

	if (ResolvedRows.Num() == 0)
	{
		return false;
	}

	const FVector StartAreaCenter = [&Board]()
	{
		FVector Out = FVector::ZeroVector;
		return T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::StartArea, 0.0f, Out)
			? Out
			: FVector::ZeroVector;
	}();
	const bool bHasStartAreaCenter = !StartAreaCenter.IsZero();
	const FVector BossAreaCenter = [&Board]()
	{
		FVector Out = FVector::ZeroVector;
		return T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::BossArea, 0.0f, Out)
			? Out
			: FVector::ZeroVector;
	}();
	const bool bHasBossAreaCenter = !BossAreaCenter.IsZero();

	TArray<FVector> AllUsedLocs;
	int32 RemainingBoulderBudget = T66MainMapBoulderBudget;

	auto IsInsideReservedArea = [&](const FVector& Candidate) -> bool
	{
		if (FMath::Abs(Candidate.X) > MainHalfExtent || FMath::Abs(Candidate.Y) > MainHalfExtent)
		{
			return true;
		}

		if (KeepClearRadius > 0.0f &&
			FVector::DistSquared2D(Candidate, KeepClearCenter) < FMath::Square(KeepClearRadius))
		{
			return true;
		}

		return T66GameplayLayout::IsInsideReservedTraversalZone2D(Candidate, 800.0f);
	};

	auto IsInsideNPCSafeZone = [&](const FVector& Candidate) -> bool
	{
		UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC)
			{
				continue;
			}

			const float Radius = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(Candidate, NPC->GetActorLocation()) < FMath::Square(Radius))
			{
				return true;
			}
		}

		return false;
	};

	auto TrySpawnGroupedProp = [&](const FName RowName,
		const FVector& PreferredLocation,
		float SearchRadius,
		const FRotator& BaseRotation,
		bool bAllowRandomYaw,
		const FVector& ClusterScaleMultiplier,
		float MinDistanceOverride) -> bool
	{
		const FT66ResolvedPropRow* Entry = ResolvedRows.Find(RowName);
		if (!Entry || !Entry->Row || (!Entry->Mesh && !Entry->bSpawnPilotableTractor))
		{
			return false;
		}

		if (Entry->bOversizedMainMapBoulder && RemainingBoulderBudget <= 0)
		{
			return false;
		}

		const float BaseMinDistance = Entry->bOversizedMainMapBoulder
			? ((MinDistanceOverride > 0.0f)
				? MinDistanceOverride
				: FMath::Max(Entry->Row->MinDistanceBetween * 20.0f, 12000.0f))
			: Entry->Row->MinDistanceBetween;
		const float MinDistance = Entry->bOversizedMainMapBoulder
			? BaseMinDistance
			: FMath::Max(BaseMinDistance, MinDistanceOverride);

		for (int32 Try = 0; Try < T66MainMapGroupedSpawnAttempts; ++Try)
		{
			FVector Candidate = PreferredLocation;
			if (SearchRadius > KINDA_SMALL_NUMBER)
			{
				const float Angle = Rng.FRandRange(0.0f, 2.0f * PI);
				const float Radius = SearchRadius * FMath::Sqrt(Rng.FRand());
				Candidate.X += FMath::Cos(Angle) * Radius;
				Candidate.Y += FMath::Sin(Angle) * Radius;
			}
			Candidate.Z = SpawnZ;

			if (IsInsideReservedArea(Candidate) || IsInsideNPCSafeZone(Candidate))
			{
				continue;
			}

			bool bTooClose = false;
			for (const FVector& UsedLocation : AllUsedLocs)
			{
				if (FVector::DistSquared2D(Candidate, UsedLocation) < FMath::Square(MinDistance))
				{
					bTooClose = true;
					break;
				}
			}
			if (bTooClose)
			{
				continue;
			}

			FHitResult Hit;
			const FVector Start(Candidate.X, Candidate.Y, TraceStartZ);
			const FVector End(Candidate.X, Candidate.Y, TraceEndZ);
			if (!T66TracePropGround(World, Start, End, true, SpawnedProps, Hit))
			{
				continue;
			}

			const FVector GroundLocation = Hit.ImpactPoint;
			FRotator Rotation = BaseRotation;
			if (bAllowRandomYaw && Entry->Row->bRandomYawRotation)
			{
				Rotation.Yaw += Rng.FRandRange(0.0f, 360.0f);
			}

			FVector Scale(
				Rng.FRandRange(Entry->Row->ScaleMin.X, Entry->Row->ScaleMax.X),
				Rng.FRandRange(Entry->Row->ScaleMin.Y, Entry->Row->ScaleMax.Y),
				Rng.FRandRange(Entry->Row->ScaleMin.Z, Entry->Row->ScaleMax.Z));
			Scale *= ClusterScaleMultiplier;
			if (Entry->bOversizedMainMapBoulder)
			{
				Scale *= T66MainMapBoulderScaleMultiplier;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AActor* SpawnedActor = nullptr;
			if (Entry->bSpawnPilotableTractor)
			{
				if (AT66PilotableTractor* Tractor = World->SpawnActor<AT66PilotableTractor>(
					AT66PilotableTractor::StaticClass(),
					GroundLocation,
					Rotation,
					SpawnParams))
				{
					Tractor->SetActorScale3D(Scale);
					T66SnapActorBottomToGroundPoint(Tractor, GroundLocation);
					if (!FMath::IsNearlyZero(Entry->Row->PlacementZOffset))
					{
						Tractor->AddActorWorldOffset(
							FVector(0.0f, 0.0f, Entry->Row->PlacementZOffset * Scale.Z),
							false,
							nullptr,
							ETeleportType::TeleportPhysics);
					}

					SpawnedActor = Tractor;
				}
			}
			else
			{
				if (AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(
					AStaticMeshActor::StaticClass(),
					GroundLocation,
					Rotation,
					SpawnParams))
				{
					if (UStaticMeshComponent* MeshComponent = PropActor->GetStaticMeshComponent())
					{
						MeshComponent->SetMobility(EComponentMobility::Movable);
						MeshComponent->SetStaticMesh(Entry->Mesh);
						MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
						PropActor->SetActorScale3D(Scale);
						FT66VisualUtil::GroundMeshToActorOrigin(MeshComponent, Entry->Mesh);
					}

					T66SnapActorBottomToGroundPoint(PropActor, GroundLocation);
					if (!FMath::IsNearlyZero(Entry->Row->PlacementZOffset))
					{
						PropActor->AddActorWorldOffset(
							FVector(0.0f, 0.0f, Entry->Row->PlacementZOffset * Scale.Z),
							false,
							nullptr,
							ETeleportType::TeleportPhysics);
					}

					SpawnedActor = PropActor;
				}
			}

			if (!SpawnedActor)
			{
				continue;
			}

			SpawnedProps.Add(SpawnedActor);
			AllUsedLocs.Add(GroundLocation);
			if (Entry->bOversizedMainMapBoulder)
			{
				--RemainingBoulderBudget;
			}
			return true;
		}

		return false;
	};

	TArray<FIntPoint> CandidateCells;
	const float EdgeMargin = Board.Settings.CellSize * T66MainMapClusterEdgeMarginCells;
	const float SpawnKeepClearDistance = Board.Settings.CellSize * T66MainMapClusterToSpawnKeepClearCells;
	const float BossKeepClearDistance = Board.Settings.CellSize * T66MainMapClusterToBossKeepClearCells;
	for (const T66MainMapTerrain::FCell& Cell : Board.Cells)
	{
		if (!Cell.bOccupied || Cell.bSlope || Cell.Region != T66MainMapTerrain::ECellRegion::MainBoard)
		{
			continue;
		}

		if (Cell.X < T66MainMapClusterEdgeMarginCells ||
			Cell.X >= Board.Settings.BoardSize - T66MainMapClusterEdgeMarginCells ||
			Cell.Z < T66MainMapClusterEdgeMarginCells ||
			Cell.Z >= Board.Settings.BoardSize - T66MainMapClusterEdgeMarginCells)
		{
			continue;
		}

		if (T66IsCoordinateWithinCellRadius(FIntPoint(Cell.X, Cell.Z), Board.StartAnchor, T66MainMapClusterToAnchorKeepClearCells) ||
			T66IsCoordinateWithinCellRadius(FIntPoint(Cell.X, Cell.Z), Board.BossAnchor, T66MainMapClusterToAnchorKeepClearCells) ||
			T66IsCoordinateWithinCellRadius(FIntPoint(Cell.X, Cell.Z), Board.StartPathCell, T66MainMapClusterToAnchorKeepClearCells) ||
			T66IsCoordinateWithinCellRadius(FIntPoint(Cell.X, Cell.Z), Board.BossPathCell, T66MainMapClusterToAnchorKeepClearCells))
		{
			continue;
		}

		FVector CellLocation = FVector::ZeroVector;
		if (!T66TryGetBoardCellLocation(Board, FIntPoint(Cell.X, Cell.Z), CellLocation))
		{
			continue;
		}

		if (FVector::DistSquared2D(CellLocation, KeepClearCenter) < FMath::Square(SpawnKeepClearDistance))
		{
			continue;
		}

		if (bHasStartAreaCenter &&
			FVector::DistSquared2D(CellLocation, StartAreaCenter) < FMath::Square(SpawnKeepClearDistance))
		{
			continue;
		}

		if (bHasBossAreaCenter &&
			FVector::DistSquared2D(CellLocation, BossAreaCenter) < FMath::Square(BossKeepClearDistance))
		{
			continue;
		}

		if (FMath::Abs(CellLocation.X) > MainHalfExtent - EdgeMargin ||
			FMath::Abs(CellLocation.Y) > MainHalfExtent - EdgeMargin)
		{
			continue;
		}

		CandidateCells.Add(FIntPoint(Cell.X, Cell.Z));
	}

	if (CandidateCells.Num() == 0)
	{
		return false;
	}

	TArray<FT66MainMapClusterAnchor> Clusters;
	auto TryAddClusterAnchor = [&](ET66MainMapPropClusterType ClusterType, float SeparationCells) -> bool
	{
		for (int32 Attempt = 0; Attempt < 80; ++Attempt)
		{
			const FIntPoint Coordinate = CandidateCells[Rng.RandRange(0, CandidateCells.Num() - 1)];
			FVector Location = FVector::ZeroVector;
			if (!T66TryGetBoardCellLocation(Board, Coordinate, Location))
			{
				continue;
			}

			bool bTooClose = false;
			for (const FT66MainMapClusterAnchor& Existing : Clusters)
			{
				if (FVector::DistSquared2D(Location, Existing.Location) < FMath::Square(SeparationCells * Board.Settings.CellSize))
				{
					bTooClose = true;
					break;
				}
			}
			if (bTooClose)
			{
				continue;
			}

			FT66MainMapClusterAnchor& Anchor = Clusters.AddDefaulted_GetRef();
			Anchor.Type = ClusterType;
			Anchor.Coordinate = Coordinate;
			Anchor.Location = Location;
			return true;
		}

		return false;
	};

	const bool bHasFarmsteadRows =
		ResolvedRows.Contains(T66BarnRowName) ||
		ResolvedRows.Contains(T66WindmillRowName) ||
		ResolvedRows.Contains(T66SiloRowName);
	const bool bHasGroveRows =
		ResolvedRows.Contains(T66TreeRowName) ||
		ResolvedRows.Contains(T66Tree2RowName) ||
		ResolvedRows.Contains(T66Tree3RowName) ||
		ResolvedRows.Contains(T66BoulderRowName);

	const int32 TargetFarmsteadCount = bHasFarmsteadRows
		? Rng.RandRange(T66MainMapFarmsteadCountMin, T66MainMapFarmsteadCountMax)
		: 0;
	const int32 TargetGroveCount = bHasGroveRows
		? Rng.RandRange(T66MainMapGroveCountMin, T66MainMapGroveCountMax)
		: 0;

	for (int32 Index = 0; Index < TargetFarmsteadCount; ++Index)
	{
		TryAddClusterAnchor(ET66MainMapPropClusterType::Farmstead, T66MainMapFarmsteadSeparationCells);
	}
	for (int32 Index = 0; Index < TargetGroveCount; ++Index)
	{
		TryAddClusterAnchor(ET66MainMapPropClusterType::Grove, T66MainMapGroveSeparationCells);
	}

	if (Clusters.Num() == 0)
	{
		return false;
	}

	auto PickAvailableRow = [&](const TArray<FName>& Options) -> FName
	{
		TArray<FName> Available;
		for (const FName RowName : Options)
		{
			if (ResolvedRows.Contains(RowName))
			{
				Available.Add(RowName);
			}
		}

		return Available.Num() > 0
			? Available[Rng.RandRange(0, Available.Num() - 1)]
			: NAME_None;
	};

	int32 SpawnedFarmsteadCount = 0;
	int32 SpawnedGroveCount = 0;
	const float CellSize = Board.Settings.CellSize;
	for (const FT66MainMapClusterAnchor& Cluster : Clusters)
	{
		const float ClusterYaw = Rng.FRandRange(0.0f, 360.0f);
		const float YawRadians = FMath::DegreesToRadians(ClusterYaw);
		const FVector Forward(FMath::Cos(YawRadians), FMath::Sin(YawRadians), 0.0f);
		const FVector Right(-Forward.Y, Forward.X, 0.0f);
		auto MakeClusterPosition = [&](float ForwardCells, float RightCells) -> FVector
		{
			return Cluster.Location
				+ Forward * (ForwardCells * CellSize)
				+ Right * (RightCells * CellSize);
		};

		if (Cluster.Type == ET66MainMapPropClusterType::Farmstead)
		{
			const bool bSpawnedCoreStructure =
				TrySpawnGroupedProp(T66BarnRowName, MakeClusterPosition(-0.2f, -0.9f), CellSize * 0.2f, FRotator(0.0f, ClusterYaw + 180.0f, 0.0f), false, FVector(1.0f), 2600.0f) ||
				TrySpawnGroupedProp(T66WindmillRowName, MakeClusterPosition(0.0f, 1.1f), CellSize * 0.2f, FRotator(0.0f, ClusterYaw + 180.0f, 0.0f), false, FVector(1.0f), 3200.0f) ||
				TrySpawnGroupedProp(T66SiloRowName, MakeClusterPosition(0.2f, -0.3f), CellSize * 0.18f, FRotator(0.0f, ClusterYaw + 90.0f, 0.0f), false, FVector(1.0f), 2600.0f);
			if (!bSpawnedCoreStructure)
			{
				continue;
			}

			++SpawnedFarmsteadCount;
			TrySpawnGroupedProp(T66WindmillRowName, MakeClusterPosition(1.6f, 1.6f), CellSize * 0.18f, FRotator(0.0f, ClusterYaw + 160.0f, 0.0f), false, FVector(1.0f), 3200.0f);
			TrySpawnGroupedProp(T66SiloRowName, MakeClusterPosition(1.1f, -1.3f), CellSize * 0.18f, FRotator(0.0f, ClusterYaw + 90.0f, 0.0f), false, FVector(1.0f), 2600.0f);
			TrySpawnGroupedProp(T66TractorRowName, MakeClusterPosition(-1.6f, 0.7f), CellSize * 0.22f, FRotator(0.0f, ClusterYaw + 30.0f, 0.0f), false, FVector(1.0f), 2200.0f);
			TrySpawnGroupedProp(T66TrothRowName, MakeClusterPosition(-1.0f, 1.9f), CellSize * 0.2f, FRotator(0.0f, ClusterYaw + 90.0f, 0.0f), false, FVector(1.0f), 1100.0f);
			TrySpawnGroupedProp(T66ScarecrowRowName, MakeClusterPosition(2.2f, -0.8f), CellSize * 0.15f, FRotator(0.0f, ClusterYaw + 180.0f, 0.0f), false, FVector(1.0f), 1600.0f);

			const FVector2D HayOffsets[] =
			{
				FVector2D(-1.1f, -1.9f),
				FVector2D(-1.8f, -1.1f),
				FVector2D(-2.0f, -0.2f),
				FVector2D(-0.9f, 1.0f),
				FVector2D(0.8f, -2.0f),
			};
			const int32 HayCount = FMath::Min(Rng.RandRange(3, 5), static_cast<int32>(UE_ARRAY_COUNT(HayOffsets)));
			for (int32 Index = 0; Index < HayCount; ++Index)
			{
				const FVector2D Offset = HayOffsets[Index];
				TrySpawnGroupedProp(
					T66HaybellRowName,
					MakeClusterPosition(Offset.X, Offset.Y),
					CellSize * 0.18f,
					FRotator::ZeroRotator,
					true,
					FVector(1.0f),
					900.0f);
			}

			const FName FenceRow = PickAvailableRow({ T66FenceRowName, T66Fence2RowName, T66Fence3RowName });
			if (FenceRow != NAME_None)
			{
				const struct FFencePlacement
				{
					float ForwardCells;
					float RightCells;
					float YawOffset;
				} FencePlacements[] =
				{
					{ 2.2f, -1.5f, 0.0f },
					{ 2.2f, -0.2f, 0.0f },
					{ 2.2f, 1.1f, 0.0f },
					{ -2.0f, -1.6f, 0.0f },
					{ -2.0f, 1.0f, 0.0f },
					{ -1.0f, -2.2f, 90.0f },
					{ 0.3f, -2.2f, 90.0f },
					{ 1.6f, -2.2f, 90.0f },
					{ -1.0f, 2.2f, 90.0f },
					{ 0.3f, 2.2f, 90.0f },
					{ 1.6f, 2.2f, 90.0f },
				};

				for (const FFencePlacement& Placement : FencePlacements)
				{
					TrySpawnGroupedProp(
						FenceRow,
						MakeClusterPosition(Placement.ForwardCells, Placement.RightCells),
						CellSize * 0.12f,
						FRotator(0.0f, ClusterYaw + Placement.YawOffset, 0.0f),
						false,
						FVector(1.0f),
						850.0f);
				}
			}
		}
		else
		{
			const FVector2D GroveOffsets[] =
			{
				FVector2D(-2.1f, -0.9f),
				FVector2D(-1.6f, 1.4f),
				FVector2D(-0.2f, -1.9f),
				FVector2D(0.9f, 2.0f),
				FVector2D(1.7f, -1.1f),
				FVector2D(2.2f, 0.8f),
				FVector2D(0.1f, 0.6f),
				FVector2D(-2.4f, 0.5f),
			};

			TrySpawnGroupedProp(
				T66BoulderRowName,
				MakeClusterPosition(0.2f, -0.1f),
				CellSize * 0.16f,
				FRotator::ZeroRotator,
				true,
				FVector(1.0f),
				5000.0f);

			int32 TreesSpawned = 0;
			const int32 GroveTreeCount = FMath::Min(Rng.RandRange(5, 7), static_cast<int32>(UE_ARRAY_COUNT(GroveOffsets)));
			for (int32 Index = 0; Index < GroveTreeCount; ++Index)
			{
				const FName TreeRow = PickAvailableRow({ T66TreeRowName, T66Tree2RowName, T66Tree3RowName });
				if (TreeRow == NAME_None)
				{
					break;
				}

				const FVector2D Offset = GroveOffsets[Index];
				if (TrySpawnGroupedProp(
					TreeRow,
					MakeClusterPosition(Offset.X, Offset.Y),
					CellSize * 0.22f,
					FRotator::ZeroRotator,
					true,
					FVector(1.0f),
					850.0f))
				{
					++TreesSpawned;
				}
			}

			if (TreesSpawned <= 0)
			{
				continue;
			}

			++SpawnedGroveCount;
			TrySpawnGroupedProp(PickAvailableRow({ T66RocksRowName, T66StumpRowName, T66LogRowName }), MakeClusterPosition(1.2f, 1.2f), CellSize * 0.16f, FRotator::ZeroRotator, true, FVector(1.0f), 900.0f);
			TrySpawnGroupedProp(PickAvailableRow({ T66RocksRowName, T66StumpRowName, T66LogRowName }), MakeClusterPosition(-1.3f, 0.9f), CellSize * 0.16f, FRotator::ZeroRotator, true, FVector(1.0f), 900.0f);
		}
	}

	UE_LOG(
		LogT66Props,
		Log,
		TEXT("[PROPS] Grouped main-map placement spawned %d props across %d farmsteads and %d groves (seed=%d, bouldersRemaining=%d)."),
		SpawnedProps.Num(),
		SpawnedFarmsteadCount,
		SpawnedGroveCount,
		Seed,
		RemainingBoulderBudget);

	return SpawnedProps.Num() > 0;
}

void UT66PropSubsystem::SpawnPropsInternal(
	UWorld* World,
	int32 Seed,
	const TArray<FName>* AllowedRows,
	float MainHalfExtent,
	float TraceStartZ,
	float TraceEndZ,
	bool bUseLegacyNoSpawnZones,
	const FVector& KeepClearCenter,
	float KeepClearRadius)
{
	if (!World) return;
	ClearProps();

	UDataTable* DT = GetPropsDataTable();
	if (!DT) return;

	FRandomStream Rng(Seed + 9973);

	static constexpr float SpawnZ = 220.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> AllUsedLocs;

	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (!bUseLegacyNoSpawnZones)
		{
			return false;
		}

		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -22727.f, 34091.f, ArenaHalf },
			{      0.f, 34091.f, ArenaHalf },
			{      0.f, 61364.f, TutorialArenaHalf },
		};
		for (const FArena2D& A : Arenas)
		{
			if (FMath::Abs(L.X - A.X) <= (A.Half + ArenaMargin) && FMath::Abs(L.Y - A.Y) <= (A.Half + ArenaMargin))
			{
				return true;
			}
		}
		return false;
	};

	const TArray<FName> RowNames = AllowedRows ? *AllowedRows : DT->GetRowNames();
	const bool bRestrictToFarmTerrain = !bUseLegacyNoSpawnZones;
	for (const FName& RowName : RowNames)
	{
		const FT66PropRow* Row = DT->FindRow<FT66PropRow>(RowName, TEXT("PropSubsystem"));
		if (!Row) continue;
		if (Row->Mesh.IsNull()) continue;

		const bool bSpawnPilotableTractor = (RowName == FName(TEXT("Tractor")));
		const bool bOversizedMainMapBoulder = (AllowedRows != nullptr && RowName == T66BoulderRowName);

		UStaticMesh* Mesh = Row->Mesh.LoadSynchronous();
		T66EnsureMeshHasBlockingCollision(Mesh);
		if (!Mesh && !bSpawnPilotableTractor) continue;

		const int32 CountMin = bOversizedMainMapBoulder ? 1 : Row->CountMin;
		const int32 CountMax = bOversizedMainMapBoulder ? 3 : Row->CountMax;
		const int32 Count = Rng.RandRange(CountMin, CountMax);
		const float MinDist = bOversizedMainMapBoulder
			? FMath::Max(Row->MinDistanceBetween * 20.0f, 12000.0f)
			: Row->MinDistanceBetween;
		const float RowHalfExtent = FMath::Max(0.0f, MainHalfExtent - MinDist * 0.5f);

		for (int32 i = 0; i < Count; ++i)
		{
			FVector Loc(0.f, 0.f, SpawnZ);
			bool bFound = false;
			for (int32 Try = 0; Try < 40; ++Try)
			{
				const float X = Rng.FRandRange(-RowHalfExtent, RowHalfExtent);
				const float Y = Rng.FRandRange(-RowHalfExtent, RowHalfExtent);
				Loc = FVector(X, Y, SpawnZ);

				if (IsInsideNoSpawnZone(Loc)) continue;
				if (KeepClearRadius > 0.0f && FVector::DistSquared2D(Loc, KeepClearCenter) < FMath::Square(KeepClearRadius)) continue;

				bool bTooClose = false;
				for (const FVector& U : AllUsedLocs)
				{
					if (FVector::DistSquared2D(Loc, U) < (MinDist * MinDist))
					{
						bTooClose = true;
						break;
					}
				}
				if (bTooClose) continue;

				bool bInNPCSafe = false;
				UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
				const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
				for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
				{
					const AT66HouseNPCBase* NPC = WeakNPC.Get();
					if (!NPC) continue;
					const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
					if (FVector::DistSquared2D(Loc, NPC->GetActorLocation()) < (R * R))
					{
						bInNPCSafe = true;
						break;
					}
				}
				if (bInNPCSafe) continue;

				FHitResult Hit;
				const FVector Start(Loc.X, Loc.Y, TraceStartZ);
				const FVector End(Loc.X, Loc.Y, TraceEndZ);
				if (!T66TracePropGround(World, Start, End, bRestrictToFarmTerrain, SpawnedProps, Hit))
				{
					continue;
				}
				Loc = Hit.ImpactPoint;

				bFound = true;
				break;
			}
			if (!bFound) continue;

			AllUsedLocs.Add(Loc);

			FActorSpawnParameters SP;
			SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FRotator Rot = FRotator::ZeroRotator;
			if (Row->bRandomYawRotation)
			{
				Rot.Yaw += Rng.FRandRange(0.f, 360.f);
			}

			const FVector Scale(
				Rng.FRandRange(Row->ScaleMin.X, Row->ScaleMax.X),
				Rng.FRandRange(Row->ScaleMin.Y, Row->ScaleMax.Y),
				Rng.FRandRange(Row->ScaleMin.Z, Row->ScaleMax.Z));
			const FVector FinalScale = bOversizedMainMapBoulder ? (Scale * T66MainMapBoulderScaleMultiplier) : Scale;

			if (bSpawnPilotableTractor)
			{
				AT66PilotableTractor* Tractor = World->SpawnActor<AT66PilotableTractor>(
					AT66PilotableTractor::StaticClass(), Loc, Rot, SP);
				if (!Tractor) continue;

				Tractor->SetActorScale3D(FinalScale);
				T66SnapActorBottomToGroundPoint(Tractor, Loc);
				if (!FMath::IsNearlyZero(Row->PlacementZOffset))
				{
					Tractor->AddActorWorldOffset(
						FVector(0.f, 0.f, Row->PlacementZOffset * FinalScale.Z),
						false,
						nullptr,
						ETeleportType::TeleportPhysics);
				}

				SpawnedProps.Add(Tractor);
				continue;
			}

			AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(), Loc, Rot, SP);
			if (!PropActor) continue;

			UStaticMeshComponent* SMC = PropActor->GetStaticMeshComponent();
			if (SMC)
			{
				SMC->SetMobility(EComponentMobility::Movable);
				SMC->SetStaticMesh(Mesh);
				SMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				SMC->SetCollisionProfileName(TEXT("BlockAll"));
				PropActor->SetActorScale3D(FinalScale);
				FT66VisualUtil::GroundMeshToActorOrigin(SMC, Mesh);
			}

			T66SnapActorBottomToGroundPoint(PropActor, Loc);
			if (!FMath::IsNearlyZero(Row->PlacementZOffset))
			{
				PropActor->AddActorWorldOffset(
					FVector(0.f, 0.f, Row->PlacementZOffset * FinalScale.Z),
					false,
					nullptr,
					ETeleportType::TeleportPhysics);
			}

			SpawnedProps.Add(PropActor);
		}
	}

	UE_LOG(LogT66Props, Verbose, TEXT("T66PropSubsystem: Spawned %d props for stage (seed=%d)."), SpawnedProps.Num(), Seed);
}

void UT66PropSubsystem::ClearProps()
{
	for (AActor* A : SpawnedProps)
	{
		if (A) A->Destroy();
	}
	SpawnedProps.Empty();
}
