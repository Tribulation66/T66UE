// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66TrapArrowProjectile.h"

#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66ArthurSwordVisuals.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

namespace
{
	UNiagaraSystem* LoadTrapPixelVFX()
	{
		static TObjectPtr<UNiagaraSystem> CachedSystem = nullptr;
		static TObjectPtr<UNiagaraSystem> CachedFallbackSystem = nullptr;
		if (!CachedSystem)
		{
			CachedSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
		}
		if (!CachedSystem && !CachedFallbackSystem)
		{
			CachedFallbackSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
		}
		return CachedSystem ? CachedSystem.Get() : CachedFallbackSystem.Get();
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
	VisualMesh->SetCastShadow(false);
	VisualMesh->SetRelativeScale3D(FVector(0.45f));

	if (UStaticMesh* SwordMesh = T66ArthurSwordVisuals::LoadSwordMesh())
	{
		VisualMesh->SetStaticMesh(SwordMesh);
	}
	else if (UStaticMesh* FallbackMesh = FT66VisualUtil::GetBasicShapeCone())
	{
		VisualMesh->SetStaticMesh(FallbackMesh);
		VisualMesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.52f));
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AT66TrapArrowProjectile::InitializeProjectile(const FVector& Direction, int32 InDamageHP, float InProjectileSpeed)
{
	DamageHP = InDamageHP;
	ProjectileSpeed = InProjectileSpeed;

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = ProjectileSpeed;
		ProjectileMovement->MaxSpeed = ProjectileSpeed;
		ProjectileMovement->Velocity = Direction.GetSafeNormal() * ProjectileSpeed;
	}

	SetActorRotation(Direction.Rotation());
}

void AT66TrapArrowProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66TrapArrowProjectile::OnSphereOverlap);
	}

	CachedPixelVFX = LoadTrapPixelVFX();
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
			FLinearColor(1.f, 0.78f, 0.25f, 0.95f),
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

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		return;
	}

	if (Hero->IsInSafeZone())
	{
		Destroy();
		return;
	}

	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ApplyDamage(DamageHP, GetOwner() ? GetOwner() : this);
		}
	}

	Destroy();
}
