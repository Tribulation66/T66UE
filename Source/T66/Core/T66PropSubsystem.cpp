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
#include "UObject/SoftObjectPath.h"

namespace
{
	static const FName T66MapPlatformTag(TEXT("T66_Map_Platform"));

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
		const TArray<AActor*>& IgnoredActors,
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

	SpawnPropsInternal(
		World,
		Seed,
		&AllowedRows,
		MainHalfExtent,
		T66MainMapTerrain::GetTraceZ(MainMapPreset),
		T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight,
		false,
		KeepClearCenter,
		KeepClearRadius);
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

		UStaticMesh* Mesh = Row->Mesh.LoadSynchronous();
		if (!Mesh && !bSpawnPilotableTractor) continue;

		const int32 Count = Rng.RandRange(Row->CountMin, Row->CountMax);
		const float MinDist = Row->MinDistanceBetween;
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

			if (bSpawnPilotableTractor)
			{
				AT66PilotableTractor* Tractor = World->SpawnActor<AT66PilotableTractor>(
					AT66PilotableTractor::StaticClass(), Loc, Rot, SP);
				if (!Tractor) continue;

				Tractor->SetActorScale3D(Scale);
				T66SnapActorBottomToGroundPoint(Tractor, Loc);
				if (!FMath::IsNearlyZero(Row->PlacementZOffset))
				{
					Tractor->AddActorWorldOffset(
						FVector(0.f, 0.f, Row->PlacementZOffset * Scale.Z),
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
				PropActor->SetActorScale3D(Scale);
				FT66VisualUtil::GroundMeshToActorOrigin(SMC, Mesh);
			}

			T66SnapActorBottomToGroundPoint(PropActor, Loc);
			if (!FMath::IsNearlyZero(Row->PlacementZOffset))
			{
				PropActor->AddActorWorldOffset(
					FVector(0.f, 0.f, Row->PlacementZOffset * Scale.Z),
					false,
					nullptr,
					ETeleportType::TeleportPhysics);
			}

			SpawnedProps.Add(PropActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("T66PropSubsystem: Spawned %d props for stage (seed=%d)."), SpawnedProps.Num(), Seed);
}

void UT66PropSubsystem::ClearProps()
{
	for (AActor* A : SpawnedProps)
	{
		if (A) A->Destroy();
	}
	SpawnedProps.Empty();
}
