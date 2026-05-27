// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66RangedEnemy.h"

#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/Enemies/Projectiles/T66SpitProjectile.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RangedEnemy, Log, All);

AT66RangedEnemy::AT66RangedEnemy()
{
	EnemyFamily = ET66EnemyFamily::Ranged;
	ProjectileClass = AT66SpitProjectile::StaticClass();
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 320.f;
	}
}

void AT66RangedEnemy::ResetFamilyState()
{
	FireCooldownRemaining = 0.6f;
}

void AT66RangedEnemy::TickFamilyBehavior(APawn* PlayerPawn, const float DeltaSeconds, const float Dist2DToPlayer, const bool bShouldRunAwayFromPlayer)
{
	if (!PlayerPawn)
	{
		return;
	}

	FireCooldownRemaining = FMath::Max(0.f, FireCooldownRemaining - DeltaSeconds);

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const bool bHasDirection = ToPlayer.Normalize();

	if (bShouldRunAwayFromPlayer)
	{
		if (bHasDirection)
		{
			AddMovementInput(-ToPlayer, 1.f);
		}
		return;
	}

	if (Dist2DToPlayer < DesiredMinRange)
	{
		if (bHasDirection)
		{
			AddMovementInput(-ToPlayer, 1.f);
		}
	}
	else if (Dist2DToPlayer > DesiredMaxRange)
	{
		if (bHasDirection)
		{
			AddMovementInput(ToPlayer, 1.f);
		}
	}
	else if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}

	if (FireCooldownRemaining <= 0.f)
	{
		UT66MobManagerSubsystem* MobManager = GetWorld() ? GetWorld()->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
		if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(PlayerPawn))
		{
			if (Hero->IsInSafeZone())
			{
				if (MobManager)
				{
					MobManager->RecordRangedFireSkippedSafeZone(false, MobID, Dist2DToPlayer);
				}
				return;
			}
		}

		const float MaxFireRange = FMath::Max(DesiredMaxRange, 1.f) + FMath::Max(0.f, FireRangeGrace);
		if (Dist2DToPlayer > MaxFireRange)
		{
			if (MobManager)
			{
				MobManager->RecordRangedFireSkippedOutOfRange(false, MobID, Dist2DToPlayer, MaxFireRange);
			}
			FireCooldownRemaining = 0.25f;
		}
		else
		{
			if (MobManager)
			{
				MobManager->RecordRangedDistancePassed(false, MobID, Dist2DToPlayer, DesiredMinRange, DesiredMaxRange, MaxFireRange);
			}
			if (FireProjectileAtPlayer(PlayerPawn))
			{
				FireCooldownRemaining = FireIntervalSeconds;
			}
			else
			{
				FireCooldownRemaining = 0.25f;
			}
		}
	}
	else if (UT66MobManagerSubsystem* MobManager = GetWorld() ? GetWorld()->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
	{
		if (UT66MobManagerSubsystem::IsRangedDiagnosticLoggingEnabled())
		{
			MobManager->RecordRangedCooldownBlocked(false, MobID, Dist2DToPlayer, FireCooldownRemaining);
		}
	}
}

bool AT66RangedEnemy::FireProjectileAtPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn || !ProjectileClass)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>();

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, ProjectileSpawnHeight);
	const FVector Target = PlayerPawn->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const float Dist2D = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());
	UE_CLOG(
		UT66MobManagerSubsystem::IsRangedDiagnosticLoggingEnabled(),
		LogT66RangedDiagnostics,
		VeryVerbose,
		TEXT("[RangedFireDecision] Stage=Entry Path=Rich MobID=%s Enemy=%s Player=%s WorldTime=%.2f Dist2D=%.1f CooldownRemaining=%.3f Start=%s Target=%s ProjectileClass=%s"),
		MobID.IsNone() ? TEXT("None") : *MobID.ToString(),
		*GetName(),
		*GetNameSafe(PlayerPawn),
		World->GetTimeSeconds(),
		Dist2D,
		FireCooldownRemaining,
		*Start.ToCompactString(),
		*Target.ToCompactString(),
		*GetNameSafe(ProjectileClass));
	if (MobManager)
	{
		MobManager->RecordRangedFireAttempt(false, MobID, Dist2D);
	}
	FString LOSBlockerName;
	if (!HasProjectileLineOfSightToPlayer(PlayerPawn, Start, Target, LOSBlockerName))
	{
		if (MobManager)
		{
			MobManager->RecordRangedLosBlocked(false, MobID, Dist2D, LOSBlockerName);
		}
		UE_LOG(
			LogT66RangedEnemy,
			VeryVerbose,
			TEXT("[EnemyRange] BlockedShot Enemy=%s MobID=%s Player=%s Dist2D=%.1f Start=%s Target=%s Blocker=%s"),
			*GetName(),
			*MobID.ToString(),
			*PlayerPawn->GetName(),
			Dist2D,
			*Start.ToCompactString(),
			*Target.ToCompactString(),
			*LOSBlockerName);
		return false;
	}
	if (MobManager)
	{
		MobManager->RecordRangedLosPassed(false, MobID, Dist2D);
	}

	FVector ShotDirection = Target - Start;
	if (!ShotDirection.Normalize())
	{
		if (MobManager)
		{
			MobManager->RecordRangedZeroDirectionShot(false, MobID);
		}
		return false;
	}
	if (MobManager)
	{
		MobManager->RecordRangedDispatchReached(false, MobID, Dist2D);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66EnemyProjectileBase* Projectile = World->SpawnActor<AT66EnemyProjectileBase>(ProjectileClass, Start, ShotDirection.Rotation(), SpawnParams))
	{
		Projectile->FireInDirection(ShotDirection);
		if (MobManager)
		{
			MobManager->RecordRangedProjectileSpawned(false, MobID);
		}
		UE_LOG(
			LogT66RangedEnemy,
			VeryVerbose,
			TEXT("[EnemyRange] FiredShot Enemy=%s MobID=%s Projectile=%s Player=%s Dist2D=%.1f Start=%s Target=%s"),
			*GetName(),
			*MobID.ToString(),
			*Projectile->GetName(),
			*PlayerPawn->GetName(),
			Dist2D,
			*Start.ToCompactString(),
			*Target.ToCompactString());
		return true;
	}

	if (MobManager)
	{
		MobManager->RecordRangedProjectileSpawnFailed(false, MobID);
	}
	return false;
}

bool AT66RangedEnemy::HasProjectileLineOfSightToPlayer(const APawn* PlayerPawn, const FVector& Start, const FVector& End, FString& OutBlockerName) const
{
	OutBlockerName = TEXT("None");
	UWorld* World = GetWorld();
	if (!World || !PlayerPawn)
	{
		OutBlockerName = TEXT("NoWorldOrPlayer");
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66RangedEnemyProjectileLOS), false, this);
	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return true;
	}

	const AActor* HitActor = Hit.GetActor();
	if (HitActor == PlayerPawn)
	{
		return true;
	}

	OutBlockerName = HitActor
		? FString::Printf(TEXT("%s/%s"), *HitActor->GetName(), HitActor->GetClass() ? *HitActor->GetClass()->GetName() : TEXT("None"))
		: FString(TEXT("WorldStatic"));
	return false;
}
