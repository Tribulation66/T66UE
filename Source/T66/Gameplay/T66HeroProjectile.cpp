// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66VendorBoss.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

AT66HeroProjectile::AT66HeroProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	// Safety: prevent unbounded projectile buildup if a projectile never overlaps anything.
	InitialLifeSpan = 5.0f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetSphereRadius(30.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	RootComponent = CollisionSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere)
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(BaseVisualScale, BaseVisualScale, BaseVisualScale));
	}
	if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.2f, 0.2f, 1.f));
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.f;
	ProjectileMovement->MaxSpeed = 1200.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AT66HeroProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66HeroProjectile::OnSphereOverlap);
}

void AT66HeroProjectile::SetTargetLocation(const FVector& TargetLoc)
{
	FVector Dir = (TargetLoc - GetActorLocation()).GetSafeNormal();
	ProjectileMovement->Velocity = Dir * ProjectileMovement->InitialSpeed;
}

void AT66HeroProjectile::SetScaleMultiplier(float InScaleMultiplier)
{
	ScaleMultiplier = FMath::Clamp(InScaleMultiplier, 0.1f, 10.f);
	if (VisualMesh)
	{
		const float S = BaseVisualScale * ScaleMultiplier;
		VisualMesh->SetRelativeScale3D(FVector(S, S, S));
	}
	// Keep collision roughly in sync with visuals for fairness.
	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(30.f * ScaleMultiplier);
	}
}

void AT66HeroProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner()) return; // ignore hero
	AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(OtherActor);
	if (Enemy && Enemy->CurrentHP > 0)
	{
		Enemy->TakeDamageFromHero(Damage);
		Destroy();
		return;
	}

	// Boss takes fixed 20 damage per hero projectile hit (only after awakening)
	AT66BossBase* Boss = Cast<AT66BossBase>(OtherActor);
	if (Boss && Boss->IsAwakened() && Boss->IsAlive())
	{
		Boss->TakeDamageFromHeroHit(Damage);
		Destroy();
		return;
	}

	// Gambler boss damage per hit
	if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(OtherActor))
	{
		GB->TakeDamageFromHeroHit(Damage);
		Destroy();
		return;
	}

	// Vendor boss damage per hit
	if (AT66VendorBoss* VB = Cast<AT66VendorBoss>(OtherActor))
	{
		VB->TakeDamageFromHeroHit(Damage);
		Destroy();
		return;
	}
}
