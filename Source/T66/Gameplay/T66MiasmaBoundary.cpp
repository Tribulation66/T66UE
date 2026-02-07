// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaBoundary.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"

AT66MiasmaBoundary::AT66MiasmaBoundary()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AT66MiasmaBoundary::BeginPlay()
{
	Super::BeginPlay();
	SpawnWallVisuals();

	// Apply damage tick when player is outside the safe rectangle.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DamageTimerHandle, this, &AT66MiasmaBoundary::ApplyBoundaryDamageTick, DamageIntervalSeconds, true, DamageIntervalSeconds);
	}
}

void AT66MiasmaBoundary::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AT66MiasmaBoundary::SpawnWallVisuals()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!CubeMesh) return;

	const float HalfThick = WallThickness * 0.5f;
	const float HalfHeight = WallHeight * 0.5f;
	const FLinearColor WallColor(0.02f, 0.02f, 0.04f, 0.85f);

	// Cube is 100uu; we scale by (length/100, thickness/100, height/100).
	// Four walls: N/S (along X), E/W (along Y). Each runs the full extent of the opposite axis plus thickness.
	const float LengthNS = SafeHalfExtent * 2.f + WallThickness;
	const float LengthEW = SafeHalfExtent * 2.f + WallThickness;

	struct FWallDef
	{
		FVector Location;
		FVector Scale;
	};
	const TArray<FWallDef> Walls = {
		{ FVector(0.f,  SafeHalfExtent + HalfThick, HalfHeight), FVector(LengthNS / 100.f, WallThickness / 100.f, WallHeight / 100.f) },
		{ FVector(0.f, -SafeHalfExtent - HalfThick, HalfHeight), FVector(LengthNS / 100.f, WallThickness / 100.f, WallHeight / 100.f) },
		{ FVector( SafeHalfExtent + HalfThick, 0.f, HalfHeight), FVector(WallThickness / 100.f, LengthEW / 100.f, WallHeight / 100.f) },
		{ FVector(-SafeHalfExtent - HalfThick, 0.f, HalfHeight), FVector(WallThickness / 100.f, LengthEW / 100.f, WallHeight / 100.f) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = this;

	for (const FWallDef& W : Walls)
	{
		// Trace down so wall sits on landscape/terrain
		FVector WallLoc = W.Location;
		FHitResult Hit;
		const FVector TraceStart = WallLoc + FVector(0.f, 0.f, 4000.f);
		const FVector TraceEnd = WallLoc - FVector(0.f, 0.f, 8000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			WallLoc.Z = Hit.ImpactPoint.Z + HalfHeight;
		}

		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), WallLoc, FRotator::ZeroRotator, SpawnParams);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;

		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Wall->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Wall->SetActorScale3D(W.Scale);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), WallColor);
		}
	}
}

void AT66MiasmaBoundary::ApplyBoundaryDamageTick()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn);
	if (!Hero) return;

	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const FVector Loc = Hero->GetActorLocation();
	const bool bOutside = (FMath::Abs(Loc.X) > SafeHalfExtent) || (FMath::Abs(Loc.Y) > SafeHalfExtent);
	if (bOutside)
	{
		RunState->ApplyDamage(1);
	}
}
