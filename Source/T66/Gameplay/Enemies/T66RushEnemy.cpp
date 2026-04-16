// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66RushEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"

AT66RushEnemy::AT66RushEnemy()
{
	EnemyFamily = ET66EnemyFamily::Rush;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 330.f;
	}
}

void AT66RushEnemy::ResetFamilyState()
{
	RushCooldownRemaining = 0.8f;
	RushSecondsRemaining = 0.f;
	RushDirection = FVector::ZeroVector;
}

void AT66RushEnemy::TickFamilyBehavior(APawn* PlayerPawn, const float DeltaSeconds, const float Dist2DToPlayer, const bool bShouldRunAwayFromPlayer)
{
	if (!PlayerPawn)
	{
		return;
	}

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const bool bHasDirection = ToPlayer.Normalize();

	if (bShouldRunAwayFromPlayer)
	{
		RushCooldownRemaining = FMath::Max(0.f, RushCooldownRemaining - DeltaSeconds);
		RushSecondsRemaining = 0.f;
		if (bHasDirection)
		{
			AddMovementInput(-ToPlayer, 1.f);
		}
		return;
	}

	if (RushSecondsRemaining > 0.f)
	{
		RushSecondsRemaining = FMath::Max(0.f, RushSecondsRemaining - DeltaSeconds);
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->MaxWalkSpeed = FMath::Max(Move->MaxWalkSpeed, GetBaseWalkSpeed() * RushSpeedMultiplier);
		}

		if (!RushDirection.IsNearlyZero())
		{
			AddMovementInput(RushDirection, 1.f);
		}

		if (RushSecondsRemaining <= 0.f)
		{
			RushCooldownRemaining = RushIntervalSeconds;
		}
		return;
	}

	RushCooldownRemaining = FMath::Max(0.f, RushCooldownRemaining - DeltaSeconds);
	if (RushCooldownRemaining <= 0.f && bHasDirection && Dist2DToPlayer <= RushTriggerDistance)
	{
		RushDirection = ToPlayer;
		RushSecondsRemaining = RushDurationSeconds;
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->MaxWalkSpeed = FMath::Max(Move->MaxWalkSpeed, GetBaseWalkSpeed() * RushSpeedMultiplier);
		}
		AddMovementInput(RushDirection, 1.f);
		return;
	}

	if (bHasDirection)
	{
		AddMovementInput(ToPlayer, 1.f);
	}
}
