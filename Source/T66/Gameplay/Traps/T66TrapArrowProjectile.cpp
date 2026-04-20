// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66TrapArrowProjectile.h"

#include "Gameplay/Traps/T66TrapBase.h"
#include "Gameplay/Traps/T66TrapDamageUtils.h"

#include "Core/T66PixelVFXSubsystem.h"
#include "Gameplay/T66ArthurSwordVisuals.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	UNiagaraSystem* LoadTrapPixelVFX()
	{
		static TSoftObjectPtr<UNiagaraSystem> PixelSystem(FSoftObjectPath(TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle")));
		if (UNiagaraSystem* System = PixelSystem.LoadSynchronous())
		{
			return System;
		}

		static TSoftObjectPtr<UNiagaraSystem> FallbackSystem(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));
		return FallbackSystem.LoadSynchronous();
	}

	UStaticMesh* LoadTrapArrowMesh()
	{
		static TSoftObjectPtr<UStaticMesh> Mesh(FSoftObjectPath(TEXT("/Game/Stylized_VFX_StPack/Meshes/SM_Arrows_PickUp.SM_Arrows_PickUp")));
		return Mesh.LoadSynchronous();
	}
}

void AT66TrapArrowProjectile::UpdateVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	const UStaticMesh* CurrentMesh = VisualMesh->GetStaticMesh();
	if (CurrentMesh == LoadTrapArrowMesh())
	{
		VisualMesh->EmptyOverrideMaterials();
		VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
		VisualMesh->SetRelativeScale3D(FVector(0.22f));
	}
	else if (CurrentMesh == T66ArthurSwordVisuals::LoadSwordMesh())
	{
		VisualMesh->EmptyOverrideMaterials();
		VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
		VisualMesh->SetRelativeScale3D(FVector(0.36f));
	}
	else
	{
		VisualMesh->SetRelativeRotation(FRotator(0.f, 0.f, -90.f));
		VisualMesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.52f));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, ProjectileTint);
	}
}

AT66TrapArrowProjectile::AT66TrapArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 5.0f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(24.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionSphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCastShadow(true);

	if (UStaticMesh* ArrowMesh = LoadTrapArrowMesh())
	{
		VisualMesh->SetStaticMesh(ArrowMesh);
	}
	else if (UStaticMesh* SwordMesh = T66ArthurSwordVisuals::LoadSwordMesh())
	{
		VisualMesh->SetStaticMesh(SwordMesh);
	}
	else if (UStaticMesh* FallbackMesh = FT66VisualUtil::GetBasicShapeCone())
	{
		VisualMesh->SetStaticMesh(FallbackMesh);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	UpdateVisuals();
}

void AT66TrapArrowProjectile::InitializeProjectile(
	const FVector& Direction,
	const int32 InDamageHP,
	const float InProjectileSpeed,
	const FLinearColor& InProjectileTint,
	const FLinearColor& InTrailColor)
{
	DamageHP = InDamageHP;
	ProjectileSpeed = InProjectileSpeed;
	ProjectileTint = InProjectileTint;
	TrailColor = InTrailColor;

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ProjectileSpeed;
		ProjectileMovement->MaxSpeed = ProjectileSpeed;
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * ProjectileSpeed;
	}

	SetActorRotation(Direction.Rotation());
	UpdateVisuals();
}

void AT66TrapArrowProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66TrapArrowProjectile::OnSphereOverlap);
	}

	CachedPixelVFX = LoadTrapPixelVFX();
	UpdateVisuals();
}

void AT66TrapArrowProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CachedPixelVFX || !GetWorld())
	{
		return;
	}

	VFXAccum += DeltaSeconds;
	if (VFXAccum < 0.045f)
	{
		return;
	}
	VFXAccum = 0.f;

	if (UT66PixelVFXSubsystem* PixelVFX = GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>())
	{
		PixelVFX->SpawnPixelAtLocation(
			GetActorLocation(),
			TrailColor,
			FVector2D(3.2f, 3.2f),
			ET66PixelVFXPriority::Low,
			FRotator::ZeroRotator,
			FVector(1.f),
			CachedPixelVFX);
	}
}

void AT66TrapArrowProjectile::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	AT66TrapBase* OwningTrap = Cast<AT66TrapBase>(GetOwner());
	if (!OwningTrap)
	{
		Destroy();
		return;
	}

	FT66TrapDamageUtils::ApplyTrapDamageToActor(OwningTrap, OtherActor, DamageHP);
	Destroy();
}
