// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BouncePadObstacle.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Gameplay/T66HeroBase.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInterface.h"

namespace
{
	static TAutoConsoleVariable<float> CVarT66BouncePadLaunchZ(
		TEXT("t66.Trap.BouncePadLaunchZ"),
		2200.0f,
		TEXT("Vertical capsule launch velocity for bounce pads (2200 clears the +500uu mesa deck at gravity 4.5)."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66BouncePadCooldown(
		TEXT("t66.Trap.BouncePadCooldown"),
		0.35f,
		TEXT("Minimum seconds between bounce pad launches."),
		ECVF_Default);
}

AT66BouncePadObstacle::AT66BouncePadObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	PadMesh->SetMobility(EComponentMobility::Movable);
	PadMesh->SetGenerateOverlapEvents(false);
	PadMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	PadMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	PadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PadMesh->SetCanEverAffectNavigation(false);
	PadMesh->SetCastShadow(false);
	PadMesh->bCastDynamicShadow = false;
	PadMesh->bCastStaticShadow = false;
	SetRootComponent(PadMesh);

	LaunchZone = CreateDefaultSubobject<UBoxComponent>(TEXT("LaunchZone"));
	LaunchZone->SetupAttachment(PadMesh);
	LaunchZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LaunchZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	LaunchZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LaunchZone->SetGenerateOverlapEvents(true);
	LaunchZone->SetCanEverAffectNavigation(false);
	LaunchZone->SetHiddenInGame(true);
	LaunchZone->OnComponentBeginOverlap.AddDynamic(this, &AT66BouncePadObstacle::OnLaunchZoneBeginOverlap);
}

void AT66BouncePadObstacle::InitPad(UStaticMesh* DiscMesh, UMaterialInterface* DiscMaterial, const float DiscRadius)
{
	const float Radius = FMath::Max(DiscRadius, 80.0f);
	if (DiscMesh && PadMesh)
	{
		PadMesh->SetStaticMesh(DiscMesh);
		const FVector NativeExtents = DiscMesh->GetBounds().BoxExtent;
		PadMesh->SetWorldScale3D(FVector(
			Radius / FMath::Max(NativeExtents.X, 1.0f),
			Radius / FMath::Max(NativeExtents.Y, 1.0f),
			(PadThickness * 0.5f) / FMath::Max(NativeExtents.Z, 1.0f)));
		if (DiscMaterial)
		{
			const int32 MaterialCount = PadMesh->GetNumMaterials();
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
			{
				PadMesh->SetMaterial(MaterialIndex, DiscMaterial);
			}
		}
	}

	if (LaunchZone)
	{
		// World-space zone: a thin sheet over the disc top so standing or landing
		// on the pad triggers the launch. (The root is scaled; compensate.)
		const FVector RootScale = PadMesh ? PadMesh->GetComponentScale() : FVector::OneVector;
		LaunchZone->SetWorldScale3D(FVector::OneVector);
		LaunchZone->SetBoxExtent(FVector(Radius * 0.9f, Radius * 0.9f, 50.0f), false);
		LaunchZone->SetWorldLocation(GetActorLocation() + FVector(0.0f, 0.0f, (PadThickness * 0.5f) + 40.0f));
		static_cast<void>(RootScale);
	}
}

void AT66BouncePadObstacle::OnLaunchZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	UWorld* World = GetWorld();
	if (!Hero || !World)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastLaunchTimeSeconds < FMath::Max(0.05f, CVarT66BouncePadCooldown.GetValueOnGameThread()))
	{
		return;
	}

	// Launch only — keep horizontal momentum, never ragdoll or disable.
	const float LaunchZ = FMath::Max(600.0f, CVarT66BouncePadLaunchZ.GetValueOnGameThread());
	Hero->LaunchCharacter(FVector(0.0f, 0.0f, LaunchZ), false, true);
	LastLaunchTimeSeconds = Now;
}
