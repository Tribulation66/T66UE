// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Gameplay/T66VisualUtil.h"

AT66HeroProjectile::AT66HeroProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	// Safety: prevent unbounded projectile buildup if a projectile never overlaps anything.
	InitialLifeSpan = 10.0f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetSphereRadius(30.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	RootComponent = CollisionSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere();
	if (Sphere)
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(BaseVisualScale, BaseVisualScale, BaseVisualScale));
	}
	if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		// Engine basic-shape materials typically use "Color"; keep "BaseColor" as fallback.
		Mat->SetVectorParameterValue(TEXT("Color"), TintColor);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), TintColor);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2400.f;
	ProjectileMovement->MaxSpeed = 2400.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bIsHomingProjectile = false;
	ProjectileMovement->HomingAccelerationMagnitude = 40000.f;
}

void AT66HeroProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66HeroProjectile::OnSphereOverlap);

	// Cache MID so we can update tint when spawned.
	if (VisualMesh)
	{
		VisualMID = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (VisualMID)
		{
			VisualMID->SetVectorParameterValue(TEXT("Color"), TintColor);
			VisualMID->SetVectorParameterValue(TEXT("BaseColor"), TintColor);
		}
	}
}

void AT66HeroProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// If we have an intended target, keep steering to it and guarantee impact.
	AActor* T = TargetActor.Get();
	if (!T)
	{
		return;
	}

	if (!IsTargetAlive())
	{
		Destroy();
		return;
	}

	const FVector TargetLoc = T->GetActorLocation();
	const FVector MyLoc = GetActorLocation();
	const float DistSq = FVector::DistSquared(MyLoc, TargetLoc);
	const float HitRadius = CollisionSphere ? CollisionSphere->GetScaledSphereRadius() : 30.f;
	if (DistSq <= (HitRadius * HitRadius))
	{
		ApplyDamageToTarget(T);
		Destroy();
		return;
	}

	// Fallback steering (in case HomingTargetComponent isn't set for some target).
	if (ProjectileMovement && !ProjectileMovement->bIsHomingProjectile)
	{
		const FVector Dir = (TargetLoc - MyLoc).GetSafeNormal();
		ProjectileMovement->Velocity = Dir * ProjectileMovement->InitialSpeed;
	}
}

void AT66HeroProjectile::SetTargetLocation(const FVector& TargetLoc)
{
	FVector Dir = (TargetLoc - GetActorLocation()).GetSafeNormal();
	ProjectileMovement->Velocity = Dir * ProjectileMovement->InitialSpeed;
}

void AT66HeroProjectile::SetTargetActor(AActor* InTargetActor)
{
	TargetActor = InTargetActor;
	if (!InTargetActor || !ProjectileMovement)
	{
		return;
	}

	// Use homing so shots don't "miss" moving targets.
	if (USceneComponent* TargetComp = InTargetActor->GetRootComponent())
	{
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingTargetComponent = TargetComp;
	}
	SetTargetLocation(InTargetActor->GetActorLocation());
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

void AT66HeroProjectile::SetTintColor(const FLinearColor& InColor)
{
	TintColor = InColor;
	if (VisualMID)
	{
		VisualMID->SetVectorParameterValue(TEXT("Color"), TintColor);
		VisualMID->SetVectorParameterValue(TEXT("BaseColor"), TintColor);
	}
	else if (VisualMesh)
	{
		VisualMID = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (VisualMID)
		{
			VisualMID->SetVectorParameterValue(TEXT("Color"), TintColor);
			VisualMID->SetVectorParameterValue(TEXT("BaseColor"), TintColor);
		}
	}
}

bool AT66HeroProjectile::IsTargetAlive() const
{
	AActor* T = TargetActor.Get();
	if (!T) return false;

	if (const AT66EnemyBase* E = Cast<AT66EnemyBase>(T))
	{
		return E->CurrentHP > 0;
	}
	if (const AT66BossBase* B = Cast<AT66BossBase>(T))
	{
		return B->IsAwakened() && B->IsAlive();
	}
	return true;
}

void AT66HeroProjectile::ApplyDamageToTarget(AActor* Target)
{
	if (!Target) return;
	const FName SourceID = DamageSourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : DamageSourceID;

	if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Target))
	{
		if (Enemy->CurrentHP > 0)
		{
			Enemy->TakeDamageFromHero(Damage, SourceID, NAME_None);
		}
		return;
	}
	if (AT66BossBase* Boss = Cast<AT66BossBase>(Target))
	{
		if (Boss->IsAwakened() && Boss->IsAlive())
		{
			Boss->TakeDamageFromHeroHit(Damage, SourceID, NAME_None);
		}
		return;
	}
}

void AT66HeroProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner()) return; // ignore hero

	const FName SourceID = DamageSourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : DamageSourceID;

	// If this shot has an intended target, ignore any other overlaps so it can't "hit the wrong enemy".
	if (AActor* Intended = TargetActor.Get())
	{
		if (OtherActor != Intended)
		{
			return;
		}
		ApplyDamageToTarget(Intended);
		Destroy();
		return;
	}

	AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(OtherActor);
	if (Enemy && Enemy->CurrentHP > 0)
	{
		Enemy->TakeDamageFromHero(Damage, SourceID, NAME_None);
		Destroy();
		return;
	}

	// Boss (stage, Gambler, Vendor) damage per hit
	AT66BossBase* Boss = Cast<AT66BossBase>(OtherActor);
	if (Boss && Boss->IsAwakened() && Boss->IsAlive())
	{
		Boss->TakeDamageFromHeroHit(Damage, SourceID, NAME_None);
		Destroy();
		return;
	}
}
