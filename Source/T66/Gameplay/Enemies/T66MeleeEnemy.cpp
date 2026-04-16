// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66MeleeEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"

AT66MeleeEnemy::AT66MeleeEnemy()
{
	EnemyFamily = ET66EnemyFamily::Melee;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 350.f;
	}
}
