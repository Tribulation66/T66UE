// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArthurUltimateSword.h"

#include "Gameplay/T66ArthurSwordVisuals.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AT66ArthurUltimateSword::AT66ArthurUltimateSword()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	SwordMesh->SetupAttachment(SceneRoot);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SwordMesh->SetCastShadow(false);
	SwordMesh->SetReceivesDecals(false);
	SwordMesh->SetCanEverAffectNavigation(false);
	SwordMesh->SetMobility(EComponentMobility::Movable);
	SwordMesh->SetRelativeRotation(FRotator::ZeroRotator);
}

void AT66ArthurUltimateSword::BeginPlay()
{
	Super::BeginPlay();

	if (SwordMesh && !SwordMesh->GetStaticMesh())
	{
		SwordMesh->SetStaticMesh(T66ArthurSwordVisuals::LoadSwordMesh());
	}

	if (!SwordMesh || !SwordMesh->GetStaticMesh())
	{
		Destroy();
		return;
	}

	if (bInitialized)
	{
		UpdateSwordTransform(0.f);
	}
}

void AT66ArthurUltimateSword::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInitialized)
	{
		return;
	}

	ElapsedSeconds += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, TravelDurationSeconds), 0.f, 1.f);
	UpdateSwordTransform(Alpha);

	if (Alpha >= 1.f)
	{
		Destroy();
	}
}

void AT66ArthurUltimateSword::InitSwordFlight(const FVector& InStart, const FVector& InEnd)
{
	StartLocation = InStart;
	EndLocation = InEnd;
	ElapsedSeconds = 0.f;

	const FVector TravelDirection = (EndLocation - StartLocation).GetSafeNormal();
	SwordRotation = TravelDirection.IsNearlyZero() ? FRotator::ZeroRotator : TravelDirection.Rotation();

	const float TravelDistance = FVector::Dist(StartLocation, EndLocation);
	TravelDurationSeconds = FMath::Clamp(TravelDistance / SwordTravelSpeed, 0.16f, 0.65f);
	bInitialized = true;

	SetActorLocation(StartLocation);
	SetActorRotation(SwordRotation);
	UpdateSwordTransform(0.f);
}

void AT66ArthurUltimateSword::UpdateSwordTransform(const float Alpha)
{
	if (!SwordMesh)
	{
		return;
	}

	const float SmoothedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);
	const float Pulse = 1.f + (FMath::Sin(Alpha * PI) * 0.08f);
	const float FinalScale = BaseSwordScale * UltimateSwordScaleMultiplier * Pulse;

	SetActorLocation(FMath::Lerp(StartLocation, EndLocation, SmoothedAlpha));
	SetActorRotation(SwordRotation);
	SwordMesh->SetRelativeLocation(FVector::ZeroVector);
	SwordMesh->SetRelativeScale3D(FVector(FinalScale));
}
