// Copyright Tribulation 66. All Rights Reserved.

#include "VFX/T66MiniGroundTelegraphActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VFX/T66MiniVfxShared.h"

AT66MiniGroundTelegraphActor::AT66MiniGroundTelegraphActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TelegraphMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TelegraphMesh"));
	TelegraphMesh->SetupAttachment(SceneRoot);
	TelegraphMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TelegraphMesh->SetCastShadow(false);
	TelegraphMesh->SetStaticMesh(T66MiniVfx::LoadPlaneMesh());
	TelegraphMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
}

void AT66MiniGroundTelegraphActor::BeginPlay()
{
	Super::BeginPlay();
}

void AT66MiniGroundTelegraphActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	LifetimeRemaining -= DeltaSeconds;
	if (LifetimeRemaining <= 0.f)
	{
		Destroy();
		return;
	}

	PulseAccumulator += DeltaSeconds * 7.0f;
	const float PulseAlpha = 0.55f + (FMath::Sin(PulseAccumulator) * 0.25f);
	const float NormalizedRemaining = LifetimeSeconds > 0.f ? (LifetimeRemaining / LifetimeSeconds) : 0.f;
	const FLinearColor CurrentTint(Tint.R, Tint.G, Tint.B, FMath::Clamp(Tint.A * PulseAlpha * (0.45f + (NormalizedRemaining * 0.55f)), 0.f, 1.f));
	T66MiniVfx::ApplyTintedMaterial(TelegraphMesh, this, nullptr, CurrentTint);
}

void AT66MiniGroundTelegraphActor::InitializeTelegraph(
	const FVector& InWorldLocation,
	const float InRadius,
	const float InLifetimeSeconds,
	const FLinearColor& InTint)
{
	SetActorLocation(InWorldLocation + FVector(0.f, 0.f, 6.f));
	Tint = InTint;
	LifetimeSeconds = FMath::Max(0.10f, InLifetimeSeconds);
	LifetimeRemaining = LifetimeSeconds;
	const float PlaneScale = FMath::Max(0.12f, InRadius / 50.f);
	TelegraphMesh->SetRelativeScale3D(FVector(PlaneScale, PlaneScale, 1.f));
	T66MiniVfx::ApplyTintedMaterial(TelegraphMesh, this, nullptr, Tint);
}
