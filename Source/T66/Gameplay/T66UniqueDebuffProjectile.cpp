// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66UniqueDebuffProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"

AT66UniqueDebuffProjectile::AT66UniqueDebuffProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->InitSphereRadius(16.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	RootComponent = Sphere;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));
	if (UStaticMesh* SphereMesh = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(SphereMesh);
	}

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

	// Color by effect type.
	FLinearColor C(0.9f, 0.2f, 0.2f, 1.f);
	switch (EffectType)
	{
		case ET66HeroStatusEffectType::Burn:  C = FLinearColor(0.95f, 0.25f, 0.10f, 1.f); break;
		case ET66HeroStatusEffectType::Chill: C = FLinearColor(0.20f, 0.60f, 0.95f, 1.f); break;
		case ET66HeroStatusEffectType::Curse: C = FLinearColor(0.65f, 0.20f, 0.90f, 1.f); break;
		default: break;
	}
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, C);
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
			RunState->ApplyStatusBurn(EffectDurationSeconds, 0.6f); // ~1 heart every ~1.7s
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

