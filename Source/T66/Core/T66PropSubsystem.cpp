// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PropSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "EngineUtils.h"
#include "UObject/SoftObjectPath.h"

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
	if (!World) return;
	ClearProps();

	UDataTable* DT = GetPropsDataTable();
	if (!DT) return;

	FRandomStream Rng(Seed + 9973);

	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 220.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> AllUsedLocs;

	auto IsInsideNoSpawnZone = [](const FVector& L) -> bool
	{
		static constexpr float StartBoxWest = -40000.f, StartBoxEast = -30909.f;
		static constexpr float StartBoxNorth = 4545.f, StartBoxSouth = -4545.f;
		static constexpr float StartMargin = 455.f;
		if (L.X >= (StartBoxWest - StartMargin) && L.X <= (StartBoxEast + StartMargin) &&
		    L.Y >= (StartBoxSouth - StartMargin) && L.Y <= (StartBoxNorth + StartMargin))
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

	TArray<FName> RowNames = DT->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		const FT66PropRow* Row = DT->FindRow<FT66PropRow>(RowName, TEXT("PropSubsystem"));
		if (!Row) continue;
		if (Row->Mesh.IsNull()) continue;

		UStaticMesh* Mesh = Row->Mesh.LoadSynchronous();
		if (!Mesh) continue;

		const int32 Count = Rng.RandRange(Row->CountMin, Row->CountMax);
		const float MinDist = Row->MinDistanceBetween;

		for (int32 i = 0; i < Count; ++i)
		{
			FVector Loc(0.f, 0.f, SpawnZ);
			FVector Normal = FVector::UpVector;
			bool bFound = false;
			for (int32 Try = 0; Try < 40; ++Try)
			{
				const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				Loc = FVector(X, Y, SpawnZ);

				if (IsInsideNoSpawnZone(Loc)) continue;

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
				for (TActorIterator<AT66HouseNPCBase> It(World); It; ++It)
				{
					const float R = (*It)->GetSafeZoneRadius() + SafeBubbleMargin;
					if (FVector::DistSquared2D(Loc, (*It)->GetActorLocation()) < (R * R))
					{
						bInNPCSafe = true;
						break;
					}
				}
				if (bInNPCSafe) continue;

				FHitResult Hit;
				const FVector Start = Loc + FVector(0.f, 0.f, 1000.f);
				const FVector End = Loc - FVector(0.f, 0.f, 4000.f);
				if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
				{
					Loc = Hit.ImpactPoint;
					Normal = Hit.ImpactNormal.GetSafeNormal(1e-4f, FVector::UpVector);
				}

				bFound = true;
				break;
			}
			if (!bFound) continue;

			AllUsedLocs.Add(Loc);

			FActorSpawnParameters SP;
			SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			const FQuat SlopeRot = FQuat::FindBetweenNormals(FVector::UpVector, Normal);
			FRotator Rot = SlopeRot.Rotator();
			if (Row->bRandomYawRotation)
			{
				Rot.Yaw += Rng.FRandRange(0.f, 360.f);
			}

			AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(
				AStaticMeshActor::StaticClass(), Loc, Rot, SP);
			if (!PropActor) continue;

			UStaticMeshComponent* SMC = PropActor->GetStaticMeshComponent();
			if (SMC)
			{
				SMC->SetStaticMesh(Mesh);
				SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SMC->SetMobility(EComponentMobility::Movable);

				const FVector Scale(
					Rng.FRandRange(Row->ScaleMin.X, Row->ScaleMax.X),
					Rng.FRandRange(Row->ScaleMin.Y, Row->ScaleMax.Y),
					Rng.FRandRange(Row->ScaleMin.Z, Row->ScaleMax.Z));
				PropActor->SetActorScale3D(Scale);
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
