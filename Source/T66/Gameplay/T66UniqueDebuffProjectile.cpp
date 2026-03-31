// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66UniqueDebuffProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

static UNiagaraSystem* LoadPixelVFX_Debuff()
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

AT66UniqueDebuffProjectile::AT66UniqueDebuffProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->InitSphereRadius(16.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	RootComponent = Sphere;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 3200.f;
	ProjectileMovement->MaxSpeed = 3200.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 3.0f;
}

void AT66UniqueDebuffProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (Sphere)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &AT66UniqueDebuffProjectile::OnSphereBeginOverlap);
	}

	switch (EffectType)
	{
		case ET66HeroStatusEffectType::Burn:  TrailColor = FLinearColor(0.95f, 0.25f, 0.10f, 1.f); break;
		case ET66HeroStatusEffectType::Chill: TrailColor = FLinearColor(0.20f, 0.60f, 0.95f, 1.f); break;
		case ET66HeroStatusEffectType::Curse: TrailColor = FLinearColor(0.65f, 0.20f, 0.90f, 1.f); break;
		default: break;
	}

	CachedPixelVFX = LoadPixelVFX_Debuff();
}

void AT66UniqueDebuffProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CachedPixelVFX || !GetWorld()) return;

	VFXAccum += DeltaSeconds;
	static constexpr float TrailInterval = 0.04f;
	if (VFXAccum < TrailInterval) return;
	VFXAccum -= TrailInterval;

	const FVector Loc = GetActorLocation();
	static constexpr int32 TrailParticles = 2;
	UT66PixelVFXSubsystem* PixelVFX = GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>();
	for (int32 i = 0; i < TrailParticles; ++i)
	{
		const FVector Jitter(FMath::FRandRange(-6.f, 6.f), FMath::FRandRange(-6.f, 6.f), FMath::FRandRange(-6.f, 6.f));
		if (PixelVFX)
		{
			PixelVFX->SpawnPixelAtLocation(
				Loc + Jitter,
				TrailColor,
				FVector2D(3.0f, 3.0f),
				ET66PixelVFXPriority::Low,
				FRotator::ZeroRotator,
				FVector(1.f),
				CachedPixelVFX);
		}
	}
}

void AT66UniqueDebuffProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (Hero->IsInSafeZone()) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) { Destroy(); return; }

	if (HitDamageHearts > 0)
	{
		RunState->ApplyDamage(HitDamageHearts * 20);
	}

	switch (EffectType)
	{
		case ET66HeroStatusEffectType::Burn:
			RunState->ApplyStatusBurn(EffectDurationSeconds, 0.6f);
			break;
		case ET66HeroStatusEffectType::Chill:
			RunState->ApplyStatusChill(EffectDurationSeconds, 0.60f);
			break;
		case ET66HeroStatusEffectType::Curse:
			RunState->ApplyStatusCurse(EffectDurationSeconds);
			break;
		default:
			break;
	}

	if (UT66FloatingCombatTextSubsystem* FCT = GI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
	{
		FName EventType = NAME_None;
		switch (EffectType)
		{
			case ET66HeroStatusEffectType::Burn:  EventType = UT66FloatingCombatTextSubsystem::EventType_Burn;  break;
			case ET66HeroStatusEffectType::Chill: EventType = UT66FloatingCombatTextSubsystem::EventType_Chill; break;
			case ET66HeroStatusEffectType::Curse: EventType = UT66FloatingCombatTextSubsystem::EventType_Curse; break;
			default: break;
		}
		if (!EventType.IsNone())
			FCT->ShowStatusEvent(Hero, EventType);
	}

	Destroy();
}
