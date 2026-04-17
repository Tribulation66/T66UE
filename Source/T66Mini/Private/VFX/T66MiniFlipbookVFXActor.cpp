// Copyright Tribulation 66. All Rights Reserved.

#include "VFX/T66MiniFlipbookVFXActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "VFX/T66MiniVfxShared.h"

AT66MiniFlipbookVFXActor::AT66MiniFlipbookVFXActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	VfxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VfxMesh"));
	VfxMesh->SetupAttachment(SceneRoot);
	VfxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VfxMesh->SetCastShadow(false);
	VfxMesh->SetStaticMesh(T66MiniVfx::LoadPlaneMesh());
	VfxMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
}

void AT66MiniFlipbookVFXActor::BeginPlay()
{
	Super::BeginPlay();
}

void AT66MiniFlipbookVFXActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	LifetimeRemaining -= DeltaSeconds;
	if (LifetimeRemaining <= 0.f)
	{
		DeactivateVfx();
		return;
	}

	const float NormalizedRemaining = LifetimeSeconds > 0.f ? (LifetimeRemaining / LifetimeSeconds) : 0.f;
	const float Progress = 1.f - FMath::Clamp(NormalizedRemaining, 0.f, 1.f);
	const float PulseScale = 1.f + (Progress * GrowthFactor);
	VfxMesh->SetRelativeScale3D(BaseScale * PulseScale);
	T66MiniVfx::ApplyTintedMaterial(
		VfxMesh,
		this,
		nullptr,
		FLinearColor(Tint.R, Tint.G, Tint.B, Tint.A * NormalizedRemaining));
}

void AT66MiniFlipbookVFXActor::DeactivateVfx()
{
	bVfxActive = false;
	LifetimeRemaining = 0.f;
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
}

void AT66MiniFlipbookVFXActor::InitializeVfx(
	const FVector& InWorldLocation,
	const FVector& InRelativeScale,
	const float InLifetimeSeconds,
	const FLinearColor& InTint,
	UTexture* InTexture,
	const float InGrowthFactor)
{
	SetActorLocation(InWorldLocation);
	BaseScale = InRelativeScale;
	Tint = InTint;
	LifetimeSeconds = FMath::Max(0.05f, InLifetimeSeconds);
	LifetimeRemaining = LifetimeSeconds;
	GrowthFactor = InGrowthFactor;
	bVfxActive = true;
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	VfxMesh->SetRelativeScale3D(BaseScale);
	T66MiniVfx::ApplyTintedMaterial(VfxMesh, this, InTexture, Tint);
}
