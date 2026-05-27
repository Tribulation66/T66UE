// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"

#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66EnemyProjectile, Log, All);

namespace
{
	static int32 GT66ActiveEnemyProjectileCount = 0;

	FString T66ProjectileVectorForLog(const FVector& Vector)
	{
		return Vector.ToCompactString();
	}
}

AT66EnemyProjectileBase::AT66EnemyProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->InitSphereRadius(14.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	RootComponent = Sphere;
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AT66EnemyProjectileBase::OnSphereBeginOverlap);

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
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	ConfigureTemporaryProjectileVisual(
		FT66TemporaryProjectileSystem::ProfileEnemySpit(),
		FT66TemporaryProjectileSystem::HostileProjectileColor(),
		FT66TemporaryProjectileSystem::ProfileHostileAccent(),
		FT66TemporaryProjectileSystem::HostileProjectileColor());

	InitialLifeSpan = 4.f;
}

int32 AT66EnemyProjectileBase::GetActiveEnemyProjectileCount()
{
	return FMath::Max(0, GT66ActiveEnemyProjectileCount);
}

void AT66EnemyProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	if (!bVisualOnly)
	{
		++GT66ActiveEnemyProjectileCount;
		bCountedAsActiveProjectile = true;
	}
}

void AT66EnemyProjectileBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bCountedAsActiveProjectile)
	{
		GT66ActiveEnemyProjectileCount = FMath::Max(0, GT66ActiveEnemyProjectileCount - 1);
		bCountedAsActiveProjectile = false;
	}
	Super::EndPlay(EndPlayReason);
}

void AT66EnemyProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bVisualOnly)
	{
		T66CombatDebugDraw::DrawDamageSphere(Sphere, TEXT("Enemy Projectile Damage"), HitDamageHearts > 0);
	}
}

void AT66EnemyProjectileBase::FireInDirection(const FVector& Direction)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
	}

	SetActorRotation(Direction.Rotation());

	UE_LOG(
		LogT66EnemyProjectile,
		VeryVerbose,
		TEXT("[ProjectileFired] Type=EnemyProjectile Projectile=%s Owner=%s OwnerClass=%s Loc=%s SphereLoc=%s Direction=%s Speed=%.1f Radius=%.1f Visual=%s VisualVisible=%d Accent=%s AccentVisible=%d"),
		*GetName(),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		GetOwner() && GetOwner()->GetClass() ? *GetOwner()->GetClass()->GetName() : TEXT("None"),
		*T66ProjectileVectorForLog(GetActorLocation()),
		Sphere ? *T66ProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
		*T66ProjectileVectorForLog(Direction),
		ProjectileMovement ? ProjectileMovement->InitialSpeed : 0.f,
		Sphere ? Sphere->GetScaledSphereRadius() : 0.f,
		VisualMesh ? *VisualMesh->GetName() : TEXT("None"),
		(VisualMesh && VisualMesh->IsVisible() && !VisualMesh->bHiddenInGame) ? 1 : 0,
		AccentMesh ? *AccentMesh->GetName() : TEXT("None"),
		(AccentMesh && AccentMesh->IsVisible() && !AccentMesh->bHiddenInGame) ? 1 : 0);
}

void AT66EnemyProjectileBase::ConfigureTemporaryProjectileVisual(
	const FName ProfileID,
	const FLinearColor& CoreColor,
	const FName AccentProfileID,
	const FLinearColor& AccentColor)
{
	FT66TemporaryProjectileSystem::ApplyProfileToMesh(VisualMesh, this, ProfileID, CoreColor);
	if (!AccentProfileID.IsNone())
	{
		FT66TemporaryProjectileSystem::ApplyProfileToMesh(AccentMesh, this, AccentProfileID, AccentColor);
	}
	else
	{
		FT66TemporaryProjectileSystem::HideMesh(AccentMesh);
	}
}

void AT66EnemyProjectileBase::SetVisualOnly(const bool bInVisualOnly)
{
	bVisualOnly = bInVisualOnly;
	if (bVisualOnly)
	{
		if (bCountedAsActiveProjectile)
		{
			GT66ActiveEnemyProjectileCount = FMath::Max(0, GT66ActiveEnemyProjectileCount - 1);
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

void AT66EnemyProjectileBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	(void)OtherBodyIndex;

	if (bVisualOnly)
	{
		return;
	}

	const AActor* ProjectileOwner = GetOwner();
	UT66MobManagerSubsystem* MobManager = GetWorld() ? GetWorld()->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
	if (OtherActor && OtherActor == ProjectileOwner && ProjectileOwner && !Cast<APawn>(ProjectileOwner))
	{
		if (MobManager)
		{
			MobManager->RecordEnemyProjectileOwnerIgnored(this, ProjectileOwner);
		}
		UE_LOG(
			LogT66EnemyProjectile,
			VeryVerbose,
			TEXT("[ProjectileImpact] Type=EnemyProjectile Result=IgnoredOwner Projectile=%s Owner=%s OtherComp=%s ProjectileLoc=%s SphereLoc=%s OverlapComp=%s"),
			*GetName(),
			*ProjectileOwner->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66ProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66ProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OverlappedComponent));
		return;
	}

	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		if (MobManager)
		{
			MobManager->RecordEnemyProjectileNonHeroImpact(this, ProjectileOwner, OtherActor);
		}
		UE_LOG(
			LogT66EnemyProjectile,
			Verbose,
			TEXT("[ProjectileImpact] Type=EnemyProjectile Result=NonHero Projectile=%s Owner=%s OtherActor=%s OtherComp=%s ProjectileLoc=%s SphereLoc=%s OverlapComp=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			OtherActor ? *OtherActor->GetName() : TEXT("None"),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66ProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66ProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OverlappedComponent),
			bFromSweep ? 1 : 0,
			*T66ProjectileVectorForLog(SweepResult.Location),
			*T66ProjectileVectorForLog(SweepResult.ImpactPoint),
			ProjectileMovement ? *T66ProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		Destroy();
		return;
	}

	if (!T66CombatShared::IsHeroHurtboxComponent(Hero, OtherComp))
	{
		if (MobManager)
		{
			MobManager->RecordEnemyProjectileHeroHurtboxReject(this, ProjectileOwner, Hero);
		}
		UE_LOG(
			LogT66EnemyProjectile,
			VeryVerbose,
			TEXT("[ProjectileImpact] Type=EnemyProjectile Result=RejectedNonHeroHurtbox Projectile=%s Owner=%s Hero=%s OtherComp=%s ProjectileLoc=%s SphereLoc=%s HeroLoc=%s HeroDist2D=%.1f SphereToHeroDist2D=%.1f OverlapComp=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66ProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66ProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66ProjectileVectorForLog(Hero->GetActorLocation()),
			FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation()),
			Sphere ? FVector::Dist2D(Sphere->GetComponentLocation(), Hero->GetActorLocation()) : -1.f,
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OverlappedComponent),
			bFromSweep ? 1 : 0,
			*T66ProjectileVectorForLog(SweepResult.Location),
			*T66ProjectileVectorForLog(SweepResult.ImpactPoint),
			ProjectileMovement ? *T66ProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		return;
	}

	if (Hero->IsInSafeZone())
	{
		if (MobManager)
		{
			MobManager->RecordEnemyProjectileSafeZoneReject(this, ProjectileOwner, Hero);
		}
		UE_LOG(
			LogT66EnemyProjectile,
			VeryVerbose,
			TEXT("[ProjectileImpact] Type=EnemyProjectile Result=RejectedSafeZone Projectile=%s Owner=%s Hero=%s HeroComp=%s ProjectileLoc=%s SphereLoc=%s HeroLoc=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			*T66ProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66ProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66ProjectileVectorForLog(Hero->GetActorLocation()),
			bFromSweep ? 1 : 0,
			*T66ProjectileVectorForLog(SweepResult.Location),
			*T66ProjectileVectorForLog(SweepResult.ImpactPoint));
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
		const int32 DamageHP = HitDamageHearts * 20;
		if (MobManager)
		{
			MobManager->RecordEnemyProjectileHeroHit(this, ProjectileOwner, Hero, DamageHP);
		}
		UE_LOG(
			LogT66EnemyProjectile,
			Log,
			TEXT("[ProjectileImpact] Type=EnemyProjectile Result=HeroHit Projectile=%s Owner=%s OwnerClass=%s Hero=%s HeroComp=%s DamageHP=%d ProjectileLoc=%s SphereLoc=%s HeroLoc=%s HeroDist2D=%.1f SphereToHeroDist2D=%.1f bFromSweep=%d SweepLoc=%s ImpactPoint=%s Normal=%s Velocity=%s"),
			*GetName(),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			GetOwner() && GetOwner()->GetClass() ? *GetOwner()->GetClass()->GetName() : TEXT("None"),
			*Hero->GetName(),
			*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
			DamageHP,
			*T66ProjectileVectorForLog(GetActorLocation()),
			Sphere ? *T66ProjectileVectorForLog(Sphere->GetComponentLocation()) : TEXT("None"),
			*T66ProjectileVectorForLog(Hero->GetActorLocation()),
			FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation()),
			Sphere ? FVector::Dist2D(Sphere->GetComponentLocation(), Hero->GetActorLocation()) : -1.f,
			bFromSweep ? 1 : 0,
			*T66ProjectileVectorForLog(SweepResult.Location),
			*T66ProjectileVectorForLog(SweepResult.ImpactPoint),
			*T66ProjectileVectorForLog(SweepResult.ImpactNormal),
			ProjectileMovement ? *T66ProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
		RunState->ApplyDamage(DamageHP, GetOwner(), FName(TEXT("EnemyProjectile")), this);
	}

	HandleHeroHit(Hero, RunState);
	Destroy();
}

void AT66EnemyProjectileBase::HandleHeroHit(AT66HeroBase* Hero, UT66RunStateSubsystem* RunState)
{
}
