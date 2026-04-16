// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66RangedEnemy.h"

#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/Enemies/Projectiles/T66SpitProjectile.h"
#include "Gameplay/T66HeroBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

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
		if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(PlayerPawn))
		{
			if (Hero->IsInSafeZone())
			{
				return;
			}
		}

		FireProjectileAtPlayer(PlayerPawn);
		FireCooldownRemaining = FireIntervalSeconds;
	}
}

void AT66RangedEnemy::FireProjectileAtPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn || !ProjectileClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, ProjectileSpawnHeight);
	FVector ShotDirection = PlayerPawn->GetActorLocation() + FVector(0.f, 0.f, 60.f) - Start;
	if (!ShotDirection.Normalize())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AT66EnemyProjectileBase* Projectile = World->SpawnActor<AT66EnemyProjectileBase>(ProjectileClass, Start, ShotDirection.Rotation(), SpawnParams))
	{
		Projectile->FireInDirection(ShotDirection);
	}
}
