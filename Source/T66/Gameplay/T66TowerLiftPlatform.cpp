// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerLiftPlatform.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

namespace
{
	// FT66VisualUtil::GetBasicShapeCube native edge length.
	constexpr float T66LiftCubeNativeSize = 100.0f;
}

AT66TowerLiftPlatform::AT66TowerLiftPlatform()
{
	// The slab must move before character movement consumes its base transform.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	LiftCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LiftCollision"));
	LiftCollision->SetMobility(EComponentMobility::Movable);
	LiftCollision->SetGenerateOverlapEvents(false);
	LiftCollision->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	// Course geometry never collides with the camera boom (occluder fade handles see-through).
	LiftCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	LiftCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LiftCollision->SetCanEverAffectNavigation(false);
	SetRootComponent(LiftCollision);

	LiftVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LiftVisual"));
	LiftVisual->SetupAttachment(LiftCollision);
	LiftVisual->SetMobility(EComponentMobility::Movable);
	LiftVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LiftVisual->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Runtime tower geometry is fully unlit and only serves traversal/readability.
	LiftVisual->SetCastShadow(false);
	LiftVisual->bCastDynamicShadow = false;
	LiftVisual->bCastStaticShadow = false;
	LiftVisual->bAffectDistanceFieldLighting = false;
	LiftVisual->bAffectDynamicIndirectLighting = false;
	LiftVisual->bReceivesDecals = false;
	LiftVisual->SetCanEverAffectNavigation(false);
}

void AT66TowerLiftPlatform::InitLift(
	UStaticMesh* SlabMesh,
	UMaterialInterface* SlabMaterial,
	const FVector2D& DeckCenter,
	const FVector2D& DeckHalfExtents,
	const float InDeckBottomZ,
	const float InDeckTopZ,
	const float InTravelSeconds,
	const float InDwellSeconds,
	const float PhaseFraction)
{
	CachedDeckCenter = DeckCenter;
	DeckBottomZ = InDeckBottomZ;
	DeckTopZ = FMath::Max(InDeckTopZ, InDeckBottomZ + 1.0f);
	TravelSeconds = FMath::Max(InTravelSeconds, 0.25f);
	DwellSeconds = FMath::Max(InDwellSeconds, 0.25f);
	CycleSeconds = FMath::Frac(FMath::Max(PhaseFraction, 0.0f)) * GetCyclePeriod();

	LiftCollision->SetBoxExtent(FVector(DeckHalfExtents.X, DeckHalfExtents.Y, SlabThickness * 0.5f), false);

	if (SlabMesh)
	{
		LiftVisual->SetStaticMesh(SlabMesh);
		LiftVisual->SetWorldScale3D(FVector(
			(DeckHalfExtents.X * 2.0f) / T66LiftCubeNativeSize,
			(DeckHalfExtents.Y * 2.0f) / T66LiftCubeNativeSize,
			SlabThickness / T66LiftCubeNativeSize));
		LiftVisual->SetRelativeLocation(FVector::ZeroVector);
		if (SlabMaterial)
		{
			LiftVisual->SetMaterial(0, SlabMaterial);
		}
	}

	ApplyDeckTopZ(ResolveDeckTopZ(CycleSeconds));
}

float AT66TowerLiftPlatform::GetCyclePeriod() const
{
	return (DwellSeconds + TravelSeconds) * 2.0f;
}

float AT66TowerLiftPlatform::ResolveDeckTopZ(const float InCycleSeconds) const
{
	const float Period = GetCyclePeriod();
	float Time = FMath::Fmod(InCycleSeconds, Period);
	if (Time < 0.0f)
	{
		Time += Period;
	}

	// dwell bottom -> eased rise -> dwell top -> eased descent
	if (Time < DwellSeconds)
	{
		return DeckBottomZ;
	}
	Time -= DwellSeconds;
	if (Time < TravelSeconds)
	{
		const float Eased = 0.5f - 0.5f * FMath::Cos(PI * (Time / TravelSeconds));
		return FMath::Lerp(DeckBottomZ, DeckTopZ, Eased);
	}
	Time -= TravelSeconds;
	if (Time < DwellSeconds)
	{
		return DeckTopZ;
	}
	Time -= DwellSeconds;
	const float Eased = 0.5f - 0.5f * FMath::Cos(PI * (Time / TravelSeconds));
	return FMath::Lerp(DeckTopZ, DeckBottomZ, Eased);
}

void AT66TowerLiftPlatform::ApplyDeckTopZ(const float InDeckTopZ)
{
	// No sweep: the slab is kinematic; riders follow through based movement and
	// anything underneath resolves by depenetration (30uu bottom rest height
	// keeps the slab from pinning a capsule against the floor).
	SetActorLocation(
		FVector(CachedDeckCenter.X, CachedDeckCenter.Y, InDeckTopZ - (SlabThickness * 0.5f)),
		false,
		nullptr,
		ETeleportType::None);
}

void AT66TowerLiftPlatform::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Period = GetCyclePeriod();
	CycleSeconds = FMath::Fmod(CycleSeconds + DeltaSeconds, Period);
	ApplyDeckTopZ(ResolveDeckTopZ(CycleSeconds));
}
