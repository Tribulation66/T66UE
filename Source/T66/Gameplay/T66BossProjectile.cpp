// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossProjectile.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

static UNiagaraSystem* LoadPixelVFX_Proj()
{
	UNiagaraSystem* Sys = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	if (!Sys)
		Sys = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
	return Sys;
}

AT66BossProjectile::AT66BossProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 6.0f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetSphereRadius(25.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionSphere;

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
	CachedPixelVFX = LoadPixelVFX_Proj();
}

void AT66BossProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CachedPixelVFX || !GetWorld()) return;

	VFXAccum += DeltaSeconds;
	static constexpr float TrailInterval = 0.04f;
	if (VFXAccum < TrailInterval) return;
	VFXAccum -= TrailInterval;

	const FVector Loc = GetActorLocation();
	static constexpr int32 TrailParticles = 2;
	for (int32 i = 0; i < TrailParticles; ++i)
	{
		const FVector Jitter(FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f), FMath::FRandRange(-8.f, 8.f));
		const float R = FMath::FRandRange(0.7f, 1.f);

		UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), CachedPixelVFX, Loc + Jitter, FRotator::ZeroRotator,
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
		if (NC)
		{
			NC->SetVariableVec4(FName(TEXT("User.Tint")), FVector4(R, 0.1f, 0.1f, 1.f));
			NC->SetVariableVec2(FName(TEXT("User.SpriteSize")), FVector2D(3.0, 3.0));
		}
	}
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
		const int32 Stage = FMath::Max(1, RunState->GetCurrentStage());
		const int32 DamageHP = FMath::Max(10, FMath::RoundToInt(10.f * FMath::Pow(1.25f, static_cast<float>(Stage - 1))));
		RunState->ApplyDamage(DamageHP, GetOwner());
	}
	Destroy();
}
