// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66FlyingEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"

AT66FlyingEnemy::AT66FlyingEnemy()
{
	EnemyFamily = ET66EnemyFamily::Flying;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 430.f;
		Move->SetMovementMode(MOVE_Flying);
	}
}

void AT66FlyingEnemy::ResetFamilyState()
{
	HoverAnchorZ = GetActorLocation().Z + HoverHeight;
	HoverBobTime = 0.f;
}

void AT66FlyingEnemy::TickFamilyBehavior(APawn* PlayerPawn, float DeltaSeconds, float Dist2DToPlayer, const bool bShouldRunAwayFromPlayer)
{
	if (!PlayerPawn)
	{
		return;
	}

	HoverBobTime += DeltaSeconds;
	FVector HoverLoc = GetActorLocation();
	const float DesiredZ = HoverAnchorZ + (FMath::Sin(HoverBobTime * HoverBobFrequency) * HoverBobAmplitude);
	HoverLoc.Z = FMath::FInterpTo(HoverLoc.Z, DesiredZ, DeltaSeconds, 6.f);
	SetActorLocation(HoverLoc);

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	if (Dist2DToPlayer > 10.f && ToPlayer.Normalize())
	{
		AddMovementInput(bShouldRunAwayFromPlayer ? -ToPlayer : ToPlayer, 1.f);
	}
}
