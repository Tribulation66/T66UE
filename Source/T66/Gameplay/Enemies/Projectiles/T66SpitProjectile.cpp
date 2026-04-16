// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/Projectiles/T66SpitProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AT66SpitProjectile::AT66SpitProjectile()
{
	HitDamageHearts = 1;
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = 2400.f;
		ProjectileMovement->MaxSpeed = 2400.f;
	}

	if (Sphere)
	{
		Sphere->InitSphereRadius(18.f);
	}
}
