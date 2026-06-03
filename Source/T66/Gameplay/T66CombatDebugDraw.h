// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66CombatTargetTypes.h"

class UBoxComponent;
class UCapsuleComponent;
class USphereComponent;
class UWorld;

namespace T66CombatDebugDraw
{
	enum class ERole : uint8
	{
		Hurtbox,
		DamageVolume,
		PlayerHurtbox
	};

	bool ShouldDrawHitboxes();
	bool ShouldDrawDamageVolumes();

	void DrawHitZone(const USphereComponent* Zone, ET66HitZoneType HitZoneType, bool bActive, const FString& Label);
	void DrawDamageSphere(const USphereComponent* Sphere, const FString& Label, bool bActive = true);
	void DrawDamageSphere(UWorld* World, const FVector& Center, float Radius, const FString& Label, bool bActive = true);
	void DrawDamageSector(UWorld* World, const FVector& Center, const FVector& Forward, float Radius, float HalfAngleDegrees, const FString& Label, bool bActive = true, float InnerRadius = 0.f);
	void DrawDamageCapsule(UWorld* World, const FVector& Center, const FQuat& Rotation, float HalfHeight, float Radius, const FString& Label, bool bActive = true);
	void DrawDamageBox(const UBoxComponent* Box, const FString& Label, bool bActive = true);
	void DrawDamageCapsule(const UCapsuleComponent* Capsule, const FString& Label, bool bActive = true);
	void DrawTriggerBox(const UBoxComponent* Box, const FString& Label, bool bArmed = true);
	void DrawPlayerHurtCapsule(const UCapsuleComponent* Capsule, const FString& Label);
}
