// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"

class UStaticMeshComponent;

enum class ET66TemporaryProjectileShape : uint8
{
	Sphere,
	Cone,
	Cylinder,
	Cube
};

struct FT66TemporaryProjectileVisualSpec
{
	ET66TemporaryProjectileShape Shape = ET66TemporaryProjectileShape::Sphere;
	FVector RelativeLocation = FVector::ZeroVector;
	FRotator RelativeRotation = FRotator::ZeroRotator;
	FVector RelativeScale = FVector(0.55f);
	FLinearColor Color = FLinearColor::White;
	bool bVisible = true;
};

/**
 * Temporary combat projectile presentation layer.
 *
 * This owns the shared "clear colored shapes" pass for hero, idol, enemy, and trap
 * projectile visuals until each profile is replaced by authored meshes.
 */
struct FT66TemporaryProjectileSystem
{
	static FName ProfileHeroPierce();
	static FName ProfileHeroAOE();
	static FName ProfileHeroBounce();
	static FName ProfileHeroDOT();
	static FName ProfileHeroSingleTarget();
	static FName ProfileIdolOverlay();
	static FName ProfileEnemySpit();
	static FName ProfileTrapArrow();
	static FName ProfileHostileAccent();

	static FLinearColor HeroProjectileColor();
	static FLinearColor NoWeaponProjectileColor();
	static FLinearColor HostileProjectileColor();

	static FName GetHeroAttackProfile(ET66AttackCategory AttackCategory);
	static FT66TemporaryProjectileVisualSpec MakeSpec(FName ProfileID, const FLinearColor& Color, float ScaleMultiplier = 1.f);
	static void ApplyProfileToMesh(UStaticMeshComponent* Mesh, UObject* Outer, FName ProfileID, const FLinearColor& Color, float ScaleMultiplier = 1.f);
	static void HideMesh(UStaticMeshComponent* Mesh);
};
