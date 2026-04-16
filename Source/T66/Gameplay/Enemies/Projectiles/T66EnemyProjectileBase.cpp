// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"

#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AT66EnemyProjectileBase::AT66EnemyProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->InitSphereRadius(14.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	RootComponent = Sphere;
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AT66EnemyProjectileBase::OnSphereBeginOverlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 4.f;
}

void AT66EnemyProjectileBase::FireInDirection(const FVector& Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
	}

	SetActorRotation(Direction.Rotation());
}

void AT66EnemyProjectileBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		Destroy();
		return;
	}

	if (Hero->IsInSafeZone())
	{
		Destroy();
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		Destroy();
		return;
	}

	if (HitDamageHearts > 0)
	{
		RunState->ApplyDamage(HitDamageHearts * 20, GetOwner());
	}

	HandleHeroHit(Hero, RunState);
	Destroy();
}

void AT66EnemyProjectileBase::HandleHeroHit(AT66HeroBase* Hero, UT66RunStateSubsystem* RunState)
{
}
