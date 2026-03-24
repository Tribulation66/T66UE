// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroOneAttackVFX.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AT66HeroOneAttackVFX::AT66HeroOneAttackVFX()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	InitialLifeSpan = 1.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	StreakMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StreakMesh"));
	StreakMesh->SetupAttachment(SceneRoot);
	StreakMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StreakMesh->SetCastShadow(false);
	StreakMesh->SetReceivesDecals(false);
	StreakMesh->SetCanEverAffectNavigation(false);
	StreakMesh->SetMobility(EComponentMobility::Movable);
	StreakMesh->TranslucencySortPriority = 3;

	ImpactMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImpactMesh"));
	ImpactMesh->SetupAttachment(SceneRoot);
	ImpactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ImpactMesh->SetCastShadow(false);
	ImpactMesh->SetReceivesDecals(false);
	ImpactMesh->SetCanEverAffectNavigation(false);
	ImpactMesh->SetMobility(EComponentMobility::Movable);
	ImpactMesh->TranslucencySortPriority = 4;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		StreakMesh->SetStaticMesh(PlaneMesh.Object);
		ImpactMesh->SetStaticMesh(PlaneMesh.Object);
	}
}

void AT66HeroOneAttackVFX::InitEffect(const FVector& InStart, const FVector& InEnd, const FVector& InImpactLocation, const FLinearColor& InTint)
{
	StartLocation = InStart;
	EndLocation = InEnd;
	ImpactLocation = InImpactLocation;
	TintColor = InTint;
	bInitialized = true;
	ApplyEffectTransforms();
}

void AT66HeroOneAttackVFX::BeginPlay()
{
	Super::BeginPlay();

	if (!StreakBaseMaterial)
	{
		StreakBaseMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Streak.MI_Hero1_Attack_Streak"));
	}

	if (StreakBaseMaterial)
	{
		StreakMID = UMaterialInstanceDynamic::Create(StreakBaseMaterial, this, TEXT("Hero1AttackStreakMID"));
		if (StreakMID)
		{
			StreakMesh->SetMaterial(0, StreakMID);
		}
	}

	if (!ImpactBaseMaterial)
	{
		ImpactBaseMaterial = LoadObject<UMaterialInterface>(
			nullptr,
			TEXT("/Game/VFX/Hero1/MI_Hero1_Attack_Impact.MI_Hero1_Attack_Impact"));
	}

	if (ImpactBaseMaterial)
	{
		ImpactMID = UMaterialInstanceDynamic::Create(ImpactBaseMaterial, this, TEXT("Hero1AttackImpactMID"));
		if (ImpactMID)
		{
			ImpactMesh->SetMaterial(0, ImpactMID);
		}
	}

	if (bInitialized)
	{
		ApplyEffectTransforms();
		UpdateMaterialParams(0.f);
	}
}

void AT66HeroOneAttackVFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedSeconds += DeltaSeconds;
	const float Age01 = FMath::Clamp(ElapsedSeconds / FMath::Max(0.01f, LifetimeSeconds), 0.f, 1.f);
	UpdateMaterialParams(Age01);

	const float Pulse = 1.f + FMath::Sin(Age01 * PI) * 0.18f;
	const float LengthScale = FMath::Lerp(1.05f, 0.92f, Age01);
	const float WidthScale = FMath::Lerp(BaseStreakWidthScale, BaseStreakWidthScale * 1.5f, Age01);
	const float HeightScale = 1.f;
	StreakMesh->SetRelativeScale3D(FVector((TraceLength * LengthScale) / 100.f, WidthScale, HeightScale));

	const float ImpactScale = BaseImpactScale * Pulse;
	ImpactMesh->SetRelativeScale3D(FVector(ImpactScale, ImpactScale, 1.f));

	if (Age01 >= 1.f)
	{
		Destroy();
	}
}

void AT66HeroOneAttackVFX::ApplyEffectTransforms()
{
	const FVector RawDir = EndLocation - StartLocation;
	const FVector FlatDir = FVector(RawDir.X, RawDir.Y, 0.f).GetSafeNormal();
	const FVector Dir = FlatDir.IsNearlyZero() ? RawDir.GetSafeNormal() : FlatDir;
	const FRotator EffectRotation = Dir.IsNearlyZero() ? FRotator::ZeroRotator : Dir.Rotation();

	TraceLength = FMath::Max(60.f, FVector::Dist2D(StartLocation, EndLocation));
	SetActorLocation(FVector(StartLocation.X, StartLocation.Y, StartLocation.Z + 10.f));
	SetActorRotation(FRotator(0.f, EffectRotation.Yaw, 0.f));

	const FVector LocalImpact = FRotationMatrix(GetActorRotation()).InverseTransformVector(ImpactLocation - StartLocation);
	ImpactDistance = LocalImpact.X;

	StreakMesh->SetRelativeLocation(FVector(TraceLength * 0.5f, 0.f, 0.f));
	StreakMesh->SetRelativeRotation(FRotator::ZeroRotator);
	StreakMesh->SetRelativeScale3D(FVector(TraceLength / 100.f, BaseStreakWidthScale, 1.f));

	ImpactMesh->SetRelativeLocation(FVector(ImpactDistance, 0.f, 2.f));
	ImpactMesh->SetRelativeRotation(FRotator::ZeroRotator);
	ImpactMesh->SetRelativeScale3D(FVector(BaseImpactScale, BaseImpactScale, 1.f));
}

void AT66HeroOneAttackVFX::UpdateMaterialParams(float Age01) const
{
	if (StreakMID)
	{
		StreakMID->SetScalarParameterValue(TEXT("Age01"), Age01);
		StreakMID->SetVectorParameterValue(TEXT("TintColor"), TintColor);
	}

	if (ImpactMID)
	{
		ImpactMID->SetScalarParameterValue(TEXT("Age01"), Age01);
		ImpactMID->SetVectorParameterValue(TEXT("TintColor"), TintColor);
	}
}
