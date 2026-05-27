// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66UniqueDebuffProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66UniqueDebuffProjectile, Log, All);

namespace
{
	static int32 GT66ActiveUniqueDebuffProjectileCount = 0;

	FString T66UniqueProjectileVectorForLog(const FVector& Vector)
	{
		return Vector.ToCompactString();
	}
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

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCastShadow(false);

	AccentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccentMesh"));
	AccentMesh->SetupAttachment(RootComponent);
	AccentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AccentMesh->SetCastShadow(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 3200.f;
	ProjectileMovement->MaxSpeed = 3200.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	FT66TemporaryProjectileSystem::ApplyProfileToMesh(VisualMesh, this, FT66TemporaryProjectileSystem::ProfileUniqueDebuff(), FT66TemporaryProjectileSystem::HostileProjectileColor());
	FT66TemporaryProjectileSystem::ApplyProfileToMesh(AccentMesh, this, FT66TemporaryProjectileSystem::ProfileHostileAccent(), FT66TemporaryProjectileSystem::HostileProjectileColor(), 0.85f);

	InitialLifeSpan = 3.0f;
}

int32 AT66UniqueDebuffProjectile::GetActiveEnemyProjectileCount()
{
	return FMath::Max(0, GT66ActiveUniqueDebuffProjectileCount);
}

void AT66UniqueDebuffProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (!bVisualOnly)
	{
		++GT66ActiveUniqueDebuffProjectileCount;
		bCountedAsActiveProjectile = true;
	}
	if (Sphere)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &AT66UniqueDebuffProjectile::OnSphereBeginOverlap);
	}

	FT66TemporaryProjectileSystem::ApplyProfileToMesh(VisualMesh, this, FT66TemporaryProjectileSystem::ProfileUniqueDebuff(), FT66TemporaryProjectileSystem::HostileProjectileColor());
	FT66TemporaryProjectileSystem::ApplyProfileToMesh(AccentMesh, this, FT66TemporaryProjectileSystem::ProfileHostileAccent(), FT66TemporaryProjectileSystem::HostileProjectileColor(), 0.85f);
}

void AT66UniqueDebuffProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bCountedAsActiveProjectile)
	{
		GT66ActiveUniqueDebuffProjectileCount = FMath::Max(0, GT66ActiveUniqueDebuffProjectileCount - 1);
		bCountedAsActiveProjectile = false;
	}
	Super::EndPlay(EndPlayReason);
}

void AT66UniqueDebuffProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bVisualOnly)
	{
		T66CombatDebugDraw::DrawDamageSphere(Sphere, TEXT("Unique Projectile Damage"), HitDamageHearts > 0);
	}

}

void AT66UniqueDebuffProjectile::SetVisualOnly(const bool bInVisualOnly)
{
	bVisualOnly = bInVisualOnly;
	if (bVisualOnly)
	{
		if (bCountedAsActiveProjectile)
		{
			GT66ActiveUniqueDebuffProjectileCount = FMath::Max(0, GT66ActiveUniqueDebuffProjectileCount - 1);
			bCountedAsActiveProjectile = false;
		}
		SetActorEnableCollision(false);
		if (Sphere)
		{
			Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (ProjectileMovement)
		{
			ProjectileMovement->InitialSpeed = 0.f;
			ProjectileMovement->MaxSpeed = 0.f;
			ProjectileMovement->Velocity = FVector::ZeroVector;
		}
	}
}

void AT66UniqueDebuffProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	(void)OtherBodyIndex;

	if (bVisualOnly)
	{
		return;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (!T66CombatShared::IsHeroHurtboxComponent(Hero, OtherComp))
	{
		UE_LOG(
			LogT66UniqueDebuffProjectile,
			Log,
			TEXT("[ProjectileImpact] Type=UniqueDebuffProjectile Result=RejectedNonHeroHurtbox Projectile=%s Owner=%s Hero=%s OtherComp=%s ProjectileLoc=%s SphereLoc=%s HeroLoc=%s HeroDist2D=%.1f SphereToHeroDist2D=%.1f OverlapComp=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66UniqueProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66UniqueProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66UniqueProjectileVectorForLog(Hero->GetActorLocation()),
			FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation()),
			Sphere ? FVector::Dist2D(Sphere->GetComponentLocation(), Hero->GetActorLocation()) : -1.f,
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OverlappedComponent),
			bFromSweep ? 1 : 0,
			*T66UniqueProjectileVectorForLog(SweepResult.Location),
			*T66UniqueProjectileVectorForLog(SweepResult.ImpactPoint),
			ProjectileMovement ? *T66UniqueProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		return;
	}

	if (Hero->IsInSafeZone())
	{
		UE_LOG(
			LogT66UniqueDebuffProjectile,
			Log,
			TEXT("[ProjectileImpact] Type=UniqueDebuffProjectile Result=RejectedSafeZone Projectile=%s Owner=%s Hero=%s HeroComp=%s ProjectileLoc=%s SphereLoc=%s HeroLoc=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66UniqueProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66UniqueProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66UniqueProjectileVectorForLog(Hero->GetActorLocation()),
			bFromSweep ? 1 : 0,
			*T66UniqueProjectileVectorForLog(SweepResult.Location),
			*T66UniqueProjectileVectorForLog(SweepResult.ImpactPoint));
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) { Destroy(); return; }

	if (HitDamageHearts > 0)
	{
		const int32 DamageHP = HitDamageHearts * 20;
		UE_LOG(
			LogT66UniqueDebuffProjectile,
			Log,
			TEXT("[ProjectileImpact] Type=UniqueDebuffProjectile Result=HeroHit Projectile=%s Owner=%s OwnerClass=%s Hero=%s HeroComp=%s DamageHP=%d Effect=%d ProjectileLoc=%s SphereLoc=%s HeroLoc=%s HeroDist2D=%.1f SphereToHeroDist2D=%.1f bFromSweep=%d SweepLoc=%s ImpactPoint=%s Normal=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			GetOwner() && GetOwner()->GetClass() ? *GetOwner()->GetClass()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			DamageHP,
			static_cast<int32>(EffectType),
			*T66UniqueProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66UniqueProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66UniqueProjectileVectorForLog(Hero->GetActorLocation()),
			FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation()),
			Sphere ? FVector::Dist2D(Sphere->GetComponentLocation(), Hero->GetActorLocation()) : -1.f,
			bFromSweep ? 1 : 0,
			*T66UniqueProjectileVectorForLog(SweepResult.Location),
			*T66UniqueProjectileVectorForLog(SweepResult.ImpactPoint),
			*T66UniqueProjectileVectorForLog(SweepResult.ImpactNormal),
			ProjectileMovement ? *T66UniqueProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		RunState->ApplyDamage(DamageHP, GetOwner(), FName(TEXT("UniqueDebuffProjectile")), this);
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
