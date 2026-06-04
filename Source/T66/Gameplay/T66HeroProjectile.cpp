// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Gameplay/T66VisualUtil.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

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
	VisualMesh->SetCastShadow(false);

	AccentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccentMesh"));
	AccentMesh->SetupAttachment(RootComponent);
	AccentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AccentMesh->SetCastShadow(false);
	FT66TemporaryProjectileSystem::ApplyProfileToMesh(VisualMesh, this, FT66TemporaryProjectileSystem::ProfileHeroAOE(), FT66TemporaryProjectileSystem::HeroProjectileColor());
	FT66TemporaryProjectileSystem::HideMesh(AccentMesh);

	TrailVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailVFX"));
	TrailVFXComponent->SetupAttachment(VisualMesh);
	TrailVFXComponent->SetAutoActivate(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionSphere);
	ProjectileMovement->InitialSpeed = 2400.f;
	ProjectileMovement->MaxSpeed = 2400.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
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

	if (!bVisualOnly && Damage > 0)
	{
		T66CombatDebugDraw::DrawDamageSphere(CollisionSphere, TEXT("Hero Projectile Damage"), true);
	}

	// Deterministic visual-only travel: interpolate Start->End by accumulated game time
	// so the authored slash carrier occupies the whole hero->target path across frames.
	// This cannot collapse into a single tick the way speed-based movement could when a
	// capture hitch produced a large DeltaSeconds.
	if (bVisualOnly && bTimedVisualTravel)
	{
		// Visual-only Bounce links are proof/readability carriers, not damage authority.
		// Screenshot capture can deliver one large first DeltaSeconds after the projectile
		// is spawned, which collapses the whole link to its impact point before any travel
		// frame is sampled. Cap only this visual interpolation step so hitches cannot skip
		// the visible hero->target path.
		constexpr float MaxVisualTravelStepSeconds = 0.04f;
		VisualTravelElapsed += FMath::Min(FMath::Max(0.f, DeltaSeconds), MaxVisualTravelStepSeconds);
		const float Alpha = FMath::Clamp(VisualTravelElapsed / VisualTravelDuration, 0.f, 1.f);
		const FVector NewLoc = FMath::Lerp(VisualTravelStart, TargetLocation, Alpha);
		SetActorLocation(NewLoc);
		if (DrivenCarrierComponent)
		{
			DrivenCarrierComponent->SetWorldLocation(NewLoc);
		}
		if (Alpha >= 1.f)
		{
			if (DrivenCarrierComponent)
			{
				DrivenCarrierComponent->SetWorldLocation(TargetLocation);
			}
			if (VisualArrivalCallback)
			{
				TFunction<void()> ArrivalCallback = MoveTemp(VisualArrivalCallback);
				ArrivalCallback();
			}
			Destroy();
		}
		return;
	}

	// If we have an intended target, keep steering to it and guarantee impact.
	AActor* T = TargetActor.Get();
	if (T && IsTargetAlive())
	{
		TargetLocation = T->GetActorLocation();
		bHasTargetLocation = true;
	}
	else if (!bVisualOnly)
	{
		Destroy();
		return;
	}

	if (!bHasTargetLocation)
	{
		Destroy();
		return;
	}

	const FVector MyLoc = GetActorLocation();
	const float DistSq = FVector::DistSquared(MyLoc, TargetLocation);
	const float HitRadius = CollisionSphere ? CollisionSphere->GetScaledSphereRadius() : 30.f;
	const float Dist = FMath::Sqrt(DistSq);
	const bool bReachedTarget = Dist <= HitRadius;
	const float VisualStepThisTick = (bVisualOnly && ProjectileMovement)
		? ProjectileMovement->InitialSpeed * FMath::Max(0.f, DeltaSeconds)
		: 0.f;
	const bool bVisualWillReachTargetThisTick = bVisualOnly
		&& VisualStepThisTick > KINDA_SMALL_NUMBER
		&& VisualStepThisTick >= FMath::Max(0.f, Dist - HitRadius);
	if (bReachedTarget || bVisualWillReachTargetThisTick)
	{
		if (bVisualOnly)
		{
			SetActorLocation(TargetLocation);
			// Leave the free-spawned carrier at the impact point so its short remaining
			// playback completes at the target after this mover is destroyed.
			if (DrivenCarrierComponent)
			{
				DrivenCarrierComponent->SetWorldLocation(TargetLocation);
			}
		}
		if (!bVisualOnly)
		{
			ApplyDamageToTarget(T);
		}
		else if (VisualArrivalCallback)
		{
			TFunction<void()> ArrivalCallback = MoveTemp(VisualArrivalCallback);
			ArrivalCallback();
		}
		Destroy();
		return;
	}

	// Fallback steering (in case HomingTargetComponent isn't set for some target).
	if (ProjectileMovement && (!ProjectileMovement->bIsHomingProjectile || bVisualOnly))
	{
		const FVector Dir = (TargetLocation - MyLoc).GetSafeNormal();
		ProjectileMovement->Velocity = Dir * ProjectileMovement->InitialSpeed;
		if (bVisualOnly && (!T || !IsTargetAlive()))
		{
			ProjectileMovement->bIsHomingProjectile = false;
			ProjectileMovement->HomingTargetComponent = nullptr;
		}
	}

	// Drive the authored carrier along the path so the readable horizontal slash
	// travels with this mover (rotation/scale are fixed at spawn by the combat component).
	if (DrivenCarrierComponent)
	{
		DrivenCarrierComponent->SetWorldLocation(GetActorLocation());
	}
}

void AT66HeroProjectile::SetTargetLocation(const FVector& TargetLoc)
{
	TargetLocation = TargetLoc;
	bHasTargetLocation = true;
	FVector Dir = (TargetLoc - GetActorLocation()).GetSafeNormal();
	ProjectileMovement->Velocity = Dir * ProjectileMovement->InitialSpeed;
}

void AT66HeroProjectile::SetTargetActor(AActor* InTargetActor)
{
	TargetActor = InTargetActor;
	if (InTargetActor)
	{
		TargetLocation = InTargetActor->GetActorLocation();
		bHasTargetLocation = true;
	}
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
	// Keep collision roughly in sync with visuals for fairness.
	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(30.f * ScaleMultiplier);
	}
}

void AT66HeroProjectile::SetProjectileMesh(UStaticMesh* InMesh)
{
	if (VisualMesh && InMesh)
	{
		VisualMesh->SetStaticMesh(InMesh);
	}
}

void AT66HeroProjectile::SetProjectileSpeed(float InSpeed)
{
	if (!ProjectileMovement)
	{
		return;
	}

	const float ResolvedSpeed = FMath::Max(1.f, InSpeed);
	ProjectileMovement->InitialSpeed = ResolvedSpeed;
	ProjectileMovement->MaxSpeed = ResolvedSpeed;
	if (bHasTargetLocation)
	{
		SetTargetLocation(TargetLocation);
	}
}

void AT66HeroProjectile::SetTrailVFX(UNiagaraSystem* InTrailSystem, const FLinearColor& InTrailColor)
{
	if (!TrailVFXComponent || !InTrailSystem)
	{
		return;
	}

	static_cast<void>(InTrailColor);
	const FLinearColor TrailColor = FT66TemporaryProjectileSystem::HeroProjectileColor();
	TrailVFXComponent->SetAsset(InTrailSystem);
	TrailVFXComponent->SetVariableLinearColor(FName(TEXT("User.Color")), TrailColor);
	TrailVFXComponent->SetVariableLinearColor(FName(TEXT("User.Tint")), TrailColor);
	TrailVFXComponent->SetVariableLinearColor(FName(TEXT("Color")), TrailColor);
	TrailVFXComponent->Activate(true);
}

void AT66HeroProjectile::SetVisualOnly(bool bInVisualOnly)
{
	bVisualOnly = bInVisualOnly;
}

void AT66HeroProjectile::SetVisualArrivalCallback(TFunction<void()>&& InCallback)
{
	VisualArrivalCallback = MoveTemp(InCallback);
}

void AT66HeroProjectile::SetTimedVisualTravel(const FVector& StartLoc, const FVector& EndLoc, float DurationSeconds)
{
	VisualTravelStart = StartLoc;
	TargetLocation = EndLoc;
	bHasTargetLocation = true;
	VisualTravelDuration = FMath::Max(KINDA_SMALL_NUMBER, DurationSeconds);
	VisualTravelElapsed = 0.f;
	bTimedVisualTravel = true;
	SetActorLocation(StartLoc);

	// The carrier is positioned explicitly each tick, so the projectile movement
	// component must not also drive the root or it would fight the interpolation.
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Velocity = FVector::ZeroVector;
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
	}
}

void AT66HeroProjectile::SetDrivenCarrierComponent(UNiagaraComponent* InComponent)
{
	if (!InComponent)
	{
		return;
	}

	// The authored Niagara slash (spawned by the combat component via the proven
	// SpawnSystemAtLocation path) becomes the visible silhouette; the temporary cube
	// profile meshes are hidden so they are not the accepted primary carrier.
	FT66TemporaryProjectileSystem::HideMesh(VisualMesh);
	FT66TemporaryProjectileSystem::HideMesh(AccentMesh);

	DrivenCarrierComponent = InComponent;
	DrivenCarrierComponent->SetWorldLocation(GetActorLocation());
}

void AT66HeroProjectile::ConfigureTemporaryProjectileVisual(
	const FName ProfileID,
	const FLinearColor& CoreColor,
	const float CoreScaleMultiplier,
	const FName OverlayProfileID,
	const FLinearColor& OverlayColor,
	const float OverlayScaleMultiplier)
{
	TintColor = CoreColor;
	ScaleMultiplier = FMath::Clamp(CoreScaleMultiplier, 0.1f, 10.f);
	FT66TemporaryProjectileSystem::ApplyProfileToMesh(VisualMesh, this, ProfileID, CoreColor, ScaleMultiplier);
	if (!OverlayProfileID.IsNone())
	{
		FT66TemporaryProjectileSystem::ApplyProfileToMesh(AccentMesh, this, OverlayProfileID, OverlayColor, ScaleMultiplier * FMath::Max(0.1f, OverlayScaleMultiplier));
	}
	else
	{
		FT66TemporaryProjectileSystem::HideMesh(AccentMesh);
	}

	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(32.f * FMath::Max(1.f, ScaleMultiplier));
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

bool AT66HeroProjectile::CanDamageTargetOnTowerFloor(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	UWorld* World = GetWorld();
	const AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	return !GameMode || GameMode->ShouldApplyTowerFloorDamage(GetOwner(), GetActorLocation(), Target);
}

void AT66HeroProjectile::ApplyDamageToTarget(AActor* Target)
{
	if (!Target) return;
	if (Damage <= 0) return;
	if (!CanDamageTargetOnTowerFloor(Target)) return;
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

	if (bVisualOnly)
	{
		if (AActor* Intended = TargetActor.Get(); Intended && OtherActor == Intended)
		{
			if (VisualArrivalCallback)
			{
				TFunction<void()> ArrivalCallback = MoveTemp(VisualArrivalCallback);
				ArrivalCallback();
			}
			Destroy();
		}
		return;
	}

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
	if (Enemy && Enemy->CurrentHP > 0 && CanDamageTargetOnTowerFloor(Enemy))
	{
		Enemy->TakeDamageFromHero(Damage, SourceID, NAME_None);
		Destroy();
		return;
	}

	// Boss (stage, Gambler, Shop) damage per hit
	AT66BossBase* Boss = Cast<AT66BossBase>(OtherActor);
	if (Boss && Boss->IsAwakened() && Boss->IsAlive() && CanDamageTargetOnTowerFloor(Boss))
	{
		Boss->TakeDamageFromHeroHit(Damage, SourceID, NAME_None);
		Destroy();
		return;
	}
}
