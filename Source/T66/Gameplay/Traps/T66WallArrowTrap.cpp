// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66WallArrowTrap.h"

#include "Gameplay/Traps/T66TrapArrowProjectile.h"

#include "Core/T66PixelVFXSubsystem.h"
#include "Gameplay/T66ArthurSwordVisuals.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AT66WallArrowTrap::AT66WallArrowTrap()
{
	TrapTypeID = FName(TEXT("WallArrow"));
	TrapFamilyID = FName(TEXT("WallProjectile"));

	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	TrapMesh->SetupAttachment(SceneRoot);
	TrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrapMesh->SetCastShadow(false);
	TrapMesh->SetRelativeScale3D(TrapVisualScale);

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPoint->SetupAttachment(SceneRoot);
	ProjectileSpawnPoint->SetRelativeLocation(FVector(72.f, 0.f, 0.f));
}

void AT66WallArrowTrap::UpdateTrapVisuals()
{
	if (!TrapMesh)
	{
		return;
	}

	FVector ResolvedScale = TrapVisualScale;
	if (TrapMesh->GetStaticMesh() == FT66VisualUtil::GetBasicShapeCone())
	{
		ResolvedScale *= FVector(0.75f, 0.75f, 1.35f);
	}

	TrapMesh->SetRelativeScale3D(ResolvedScale);
	FT66VisualUtil::ApplyT66Color(TrapMesh, this, TrapTint);
}

void AT66WallArrowTrap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!TrapMesh)
	{
		return;
	}

	if (UStaticMesh* SwordMesh = T66ArthurSwordVisuals::LoadSwordMesh())
	{
		TrapMesh->SetStaticMesh(SwordMesh);
	}
	else if (UStaticMesh* FallbackMesh = FT66VisualUtil::GetBasicShapeCone())
	{
		TrapMesh->SetStaticMesh(FallbackMesh);
	}

	UpdateTrapVisuals();
}

void AT66WallArrowTrap::BeginPlay()
{
	Super::BeginPlay();
	if (UsesTimedActivation())
	{
		ScheduleNextFireCycle(InitialFireDelaySeconds);
	}
}

void AT66WallArrowTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireCycleTimerHandle);
	}

	PendingTargetHero.Reset();
	Super::EndPlay(EndPlayReason);
}

void AT66WallArrowTrap::ScheduleNextFireCycle(const float DelaySeconds)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FireCycleTimerHandle,
			this,
			&AT66WallArrowTrap::HandleFireCycleStart,
			FMath::Max(ScaleTrapDuration(DelaySeconds), 0.05f),
			false);
	}
}

void AT66WallArrowTrap::HandleTrapEnabledChanged()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireCycleTimerHandle);
	}

	PendingTargetHero.Reset();
	PendingAimDirection = GetActorForwardVector();

	if (bTrapEnabled && UsesTimedActivation())
	{
		ScheduleNextFireCycle(0.15f);
	}
	else
	{
		ResetTriggerLock();
	}
}

AT66HeroBase* AT66WallArrowTrap::ResolveTargetHero(FVector& OutTargetLocation) const
{
	OutTargetLocation = FVector::ZeroVector;

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> HeroActors;
	UGameplayStatics::GetAllActorsOfClass(World, AT66HeroBase::StaticClass(), HeroActors);

	const float MaxDistanceSq = FMath::Square(DetectionRange);
	AT66HeroBase* BestHero = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (AActor* HeroActor : HeroActors)
	{
		AT66HeroBase* Hero = Cast<AT66HeroBase>(HeroActor);
		if (!IsHeroTargetable(Hero))
		{
			continue;
		}

		const FVector HeroLocation = Hero->GetActorLocation() + FVector(0.f, 0.f, 55.f);
		const float DistanceSq = FVector::DistSquared(HeroLocation, GetActorLocation());
		if (DistanceSq > MaxDistanceSq || DistanceSq >= BestDistanceSq)
		{
			continue;
		}

		BestHero = Hero;
		BestDistanceSq = DistanceSq;
		OutTargetLocation = HeroLocation;
	}

	return BestHero;
}

FVector AT66WallArrowTrap::GetMuzzleLocation() const
{
	return ProjectileSpawnPoint ? ProjectileSpawnPoint->GetComponentLocation() : (GetActorLocation() + GetActorForwardVector() * 72.f);
}

void AT66WallArrowTrap::SpawnWindupBurst(const FVector& Direction) const
{
	UWorld* World = GetWorld();
	UT66PixelVFXSubsystem* PixelVFX = World ? World->GetSubsystem<UT66PixelVFXSubsystem>() : nullptr;
	if (!PixelVFX)
	{
		return;
	}

	const FVector MuzzleLocation = GetMuzzleLocation();
	for (int32 Index = 0; Index < 6; ++Index)
	{
		const FVector Offset =
			(Direction * (10.f + Index * 6.f))
			+ FVector(
				FMath::FRandRange(-10.f, 10.f),
				FMath::FRandRange(-10.f, 10.f),
				FMath::FRandRange(-8.f, 8.f));

		PixelVFX->SpawnPixelAtLocation(
			MuzzleLocation + Offset,
			WindupColor,
			FVector2D(3.2f, 3.2f),
			ET66PixelVFXPriority::Low);
	}
}

void AT66WallArrowTrap::HandleFireCycleStart()
{
	if (!IsTrapEnabled())
	{
		ScheduleNextFireCycle(FireIntervalSeconds);
		return;
	}

	FVector TargetLocation = FVector::ZeroVector;
	PendingTargetHero = ResolveTargetHero(TargetLocation);
	if (!PendingTargetHero.IsValid())
	{
		ScheduleNextFireCycle(FireIntervalSeconds);
		return;
	}

	const FVector MuzzleLocation = GetMuzzleLocation();
	PendingAimDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();
	if (PendingAimDirection.IsNearlyZero())
	{
		PendingAimDirection = GetActorForwardVector();
	}

	SpawnWindupBurst(PendingAimDirection);
	const float WindupSeconds = ScaleTrapDuration(WindupDurationSeconds);
	if (WindupSeconds <= KINDA_SMALL_NUMBER)
	{
		FireProjectile();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FireCycleTimerHandle,
			this,
			&AT66WallArrowTrap::FireProjectile,
			WindupSeconds,
			false);
	}
}

void AT66WallArrowTrap::FireProjectile()
{
	UWorld* World = GetWorld();
	if (!World || !IsTrapEnabled())
	{
		ScheduleNextFireCycle(FireIntervalSeconds);
		return;
	}

	FVector AimDirection = PendingAimDirection;
	if (AT66HeroBase* Hero = PendingTargetHero.Get())
	{
		if (IsHeroTargetable(Hero))
		{
			AimDirection = (Hero->GetActorLocation() + FVector(0.f, 0.f, 55.f) - GetMuzzleLocation()).GetSafeNormal();
		}
	}

	if (AimDirection.IsNearlyZero())
	{
		AimDirection = GetActorForwardVector();
	}

	const FVector SpawnLocation = GetMuzzleLocation();
	const FTransform SpawnTransform(AimDirection.Rotation(), SpawnLocation);
	AT66TrapArrowProjectile* Projectile = World->SpawnActorDeferred<AT66TrapArrowProjectile>(
		AT66TrapArrowProjectile::StaticClass(),
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile)
	{
		Projectile->InitializeProjectile(
			AimDirection,
			DamageHP,
			ProjectileSpeed * GetProgressionSpeedScalar(),
			ProjectileTint,
			ProjectileTrailColor);
		Projectile->FinishSpawning(SpawnTransform);
	}

	PendingTargetHero.Reset();
	ScheduleNextFireCycle(FireIntervalSeconds);
}
