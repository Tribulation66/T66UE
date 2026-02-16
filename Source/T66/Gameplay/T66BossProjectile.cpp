// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossProjectile.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Gameplay/T66VisualUtil.h"

AT66BossProjectile::AT66BossProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	// Safety: prevent unbounded projectile buildup if a projectile never overlaps anything.
	InitialLifeSpan = 6.0f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetSphereRadius(25.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(0.24f, 0.24f, 0.24f));
	}
	if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.9f, 0.1f, 0.1f, 1.f));
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 900.f;
	ProjectileMovement->MaxSpeed = 900.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AT66BossProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66BossProjectile::OnSphereOverlap);
}

void AT66BossProjectile::SetTargetLocation(const FVector& TargetLoc, float Speed)
{
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	const FVector Dir = (TargetLoc - GetActorLocation()).GetSafeNormal();
	ProjectileMovement->Velocity = Dir * Speed;
}

void AT66BossProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;
	if (!Cast<AT66HeroBase>(OtherActor)) return;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		// Numerical damage: 10 base, +25% per stage. Pass owner (boss) as Attacker for reflect/crush.
		const int32 Stage = FMath::Max(1, RunState->GetCurrentStage());
		const int32 DamageHP = FMath::Max(10, FMath::RoundToInt(10.f * FMath::Pow(1.25f, static_cast<float>(Stage - 1))));
		RunState->ApplyDamage(DamageHP, GetOwner());
	}
	Destroy();
}

