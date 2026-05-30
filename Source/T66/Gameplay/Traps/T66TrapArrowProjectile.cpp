// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66TrapArrowProjectile.h"

#include "Gameplay/Traps/T66TrapBase.h"
#include "Gameplay/Traps/T66TrapDamageUtils.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66TrapTuningConfig.h"
#include "Gameplay/T66ArthurSwordVisuals.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66CombatShared.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66TrapProjectile, Log, All);

namespace
{
	int32 GT66ActiveTrapProjectileCount = 0;

	FString T66TrapProjectileVectorForLog(const FVector& Vector)
	{
		return Vector.ToCompactString();
	}

	UStaticMesh* LoadTrapArrowMesh()
	{
		const FT66TrapVisualAssetConfig& TrapAssets = UT66TrapTuningConfig::GetRuntimeTrapAssets();
		return UT66TrapTuningConfig::LoadConfiguredTrapStaticMesh(TrapAssets.ArrowProjectileMesh, TEXT("TrapAssets.ArrowProjectileMesh"));
	}
}

int32 AT66TrapArrowProjectile::GetActiveTrapProjectileCount()
{
	return FMath::Max(0, GT66ActiveTrapProjectileCount);
}

void AT66TrapArrowProjectile::UpdateVisuals()
{
	FT66TemporaryProjectileSystem::ApplyProfileToMesh(VisualMesh, this, FT66TemporaryProjectileSystem::ProfileTrapArrow(), FT66TemporaryProjectileSystem::HostileProjectileColor());
	FT66TemporaryProjectileSystem::ApplyProfileToMesh(AccentMesh, this, FT66TemporaryProjectileSystem::ProfileHostileAccent(), FT66TemporaryProjectileSystem::HostileProjectileColor(), 0.85f);
}

AT66TrapArrowProjectile::AT66TrapArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 5.0f;

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetBoxExtent(FVector(92.f, 26.f, 26.f));
	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBox->SetGenerateOverlapEvents(true);
	RootComponent = DamageBox;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCastShadow(true);

	AccentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccentMesh"));
	AccentMesh->SetupAttachment(RootComponent);
	AccentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AccentMesh->SetCastShadow(false);

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
	ProjectileMovement->UpdatedComponent = DamageBox;
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

	UE_LOG(
		LogT66TrapProjectile,
		VeryVerbose,
		TEXT("[ProjectileFired] Type=TrapProjectile Projectile=%s Owner=%s DamageHP=%d Loc=%s BoxLoc=%s Direction=%s Speed=%.1f Extent=%s Visual=%s VisualVisible=%d Accent=%s AccentVisible=%d"),
		*GetName(),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		DamageHP,
		*T66TrapProjectileVectorForLog(GetActorLocation()),
		DamageBox ? *T66TrapProjectileVectorForLog(DamageBox->GetComponentLocation()) : TEXT("None"),
		*T66TrapProjectileVectorForLog(Direction.GetSafeNormal()),
		ProjectileSpeed,
		DamageBox ? *T66TrapProjectileVectorForLog(DamageBox->GetScaledBoxExtent()) : TEXT("None"),
		VisualMesh ? *VisualMesh->GetName() : TEXT("None"),
		(VisualMesh && VisualMesh->IsVisible() && !VisualMesh->bHiddenInGame) ? 1 : 0,
		AccentMesh ? *AccentMesh->GetName() : TEXT("None"),
		(AccentMesh && AccentMesh->IsVisible() && !AccentMesh->bHiddenInGame) ? 1 : 0);
}

void AT66TrapArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	++GT66ActiveTrapProjectileCount;
	bCountedAsActiveProjectile = true;

	if (DamageBox)
	{
		DamageBox->OnComponentBeginOverlap.AddDynamic(this, &AT66TrapArrowProjectile::OnDamageBoxOverlap);
	}

	UpdateVisuals();
}

void AT66TrapArrowProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bCountedAsActiveProjectile)
	{
		GT66ActiveTrapProjectileCount = FMath::Max(0, GT66ActiveTrapProjectileCount - 1);
		bCountedAsActiveProjectile = false;
	}

	Super::EndPlay(EndPlayReason);
}

void AT66TrapArrowProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	T66CombatDebugDraw::DrawDamageBox(DamageBox, TEXT("Trap Projectile Damage"), DamageHP > 0);
}

void AT66TrapArrowProjectile::OnDamageBoxOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OtherBodyIndex;

	if (!OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor))
	{
		if (!T66CombatShared::IsHeroHurtboxComponent(Hero, OtherComp))
		{
			UE_LOG(
				LogT66TrapProjectile,
				VeryVerbose,
				TEXT("[ProjectileImpact] Type=TrapProjectile Result=RejectedNonHeroHurtbox Projectile=%s Owner=%s Hero=%s OtherComp=%s ProjectileLoc=%s BoxLoc=%s HeroLoc=%s HeroDist2D=%.1f BoxToHeroDist2D=%.1f OverlapComp=%s bFromSweep=%d SweepLoc=%s ImpactPoint=%s Velocity=%s"),
				*GetName(),
				GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
				*Hero->GetName(),
				*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
				*T66TrapProjectileVectorForLog(GetActorLocation()),
				DamageBox ? *T66TrapProjectileVectorForLog(DamageBox->GetComponentLocation()) : TEXT("None"),
				*T66TrapProjectileVectorForLog(Hero->GetActorLocation()),
				FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation()),
				DamageBox ? FVector::Dist2D(DamageBox->GetComponentLocation(), Hero->GetActorLocation()) : -1.f,
				*T66CombatShared::DescribePrimitiveComponentForCombatLog(OverlappedComponent),
				bFromSweep ? 1 : 0,
				*T66TrapProjectileVectorForLog(SweepResult.Location),
				*T66TrapProjectileVectorForLog(SweepResult.ImpactPoint),
				ProjectileMovement ? *T66TrapProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
			return;
		}
	}

	AT66TrapBase* OwningTrap = Cast<AT66TrapBase>(GetOwner());
	if (!OwningTrap)
	{
		Destroy();
		return;
	}

	const bool bAppliedDamage = FT66TrapDamageUtils::ApplyTrapDamageToActor(OwningTrap, OtherActor, DamageHP, FName(TEXT("TrapProjectile")), this);
	UE_LOG(
		LogT66TrapProjectile,
		VeryVerbose,
		TEXT("[ProjectileImpact] Type=TrapProjectile Result=%s Projectile=%s Owner=%s OtherActor=%s OtherComp=%s DamageHP=%d ProjectileLoc=%s BoxLoc=%s OtherLoc=%s Dist2D=%.1f bFromSweep=%d SweepLoc=%s ImpactPoint=%s Normal=%s Velocity=%s"),
		bAppliedDamage ? TEXT("AppliedDamage") : TEXT("NoDamage"),
		*GetName(),
		GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
		*OtherActor->GetName(),
		*T66CombatShared::DescribePrimitiveComponentForCombatLog(OtherComp),
		DamageHP,
		*T66TrapProjectileVectorForLog(GetActorLocation()),
		DamageBox ? *T66TrapProjectileVectorForLog(DamageBox->GetComponentLocation()) : TEXT("None"),
		*T66TrapProjectileVectorForLog(OtherActor->GetActorLocation()),
		FVector::Dist2D(GetActorLocation(), OtherActor->GetActorLocation()),
		bFromSweep ? 1 : 0,
		*T66TrapProjectileVectorForLog(SweepResult.Location),
		*T66TrapProjectileVectorForLog(SweepResult.ImpactPoint),
		*T66TrapProjectileVectorForLog(SweepResult.ImpactNormal),
		ProjectileMovement ? *T66TrapProjectileVectorForLog(ProjectileMovement->Velocity) : TEXT("None"));
	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Trap.Arrow.Impact")), GetActorLocation(), OwningTrap);
	Destroy();
}
