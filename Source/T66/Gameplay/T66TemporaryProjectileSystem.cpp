// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TemporaryProjectileSystem.h"

#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

namespace
{
	UStaticMesh* T66ResolveTemporaryProjectileMesh(const ET66TemporaryProjectileShape Shape)
	{
		switch (Shape)
		{
		case ET66TemporaryProjectileShape::Cone:
			return FT66VisualUtil::GetBasicShapeCone();
		case ET66TemporaryProjectileShape::Cylinder:
			return FT66VisualUtil::GetBasicShapeCylinder();
		case ET66TemporaryProjectileShape::Cube:
			return FT66VisualUtil::GetBasicShapeCube();
		case ET66TemporaryProjectileShape::Sphere:
		default:
			return FT66VisualUtil::GetBasicShapeSphere();
		}
	}

	FLinearColor T66ResolveTemporaryProjectileColor(const FName ProfileID, const FLinearColor& RequestedColor)
	{
		if (ProfileID == FT66TemporaryProjectileSystem::ProfileHeroPierce()
			|| ProfileID == FT66TemporaryProjectileSystem::ProfileHeroAOE()
			|| ProfileID == FT66TemporaryProjectileSystem::ProfileHeroBounce()
			|| ProfileID == FT66TemporaryProjectileSystem::ProfileHeroDOT()
			|| ProfileID == FT66TemporaryProjectileSystem::ProfileIdolOverlay())
		{
			return FT66TemporaryProjectileSystem::HeroProjectileColor();
		}

		if (ProfileID == FT66TemporaryProjectileSystem::ProfileHeroSingleTarget())
		{
			return FT66TemporaryProjectileSystem::NoWeaponProjectileColor();
		}

		if (ProfileID == FT66TemporaryProjectileSystem::ProfileEnemySpit()
			|| ProfileID == FT66TemporaryProjectileSystem::ProfileTrapArrow()
			|| ProfileID == FT66TemporaryProjectileSystem::ProfileHostileAccent())
		{
			return FT66TemporaryProjectileSystem::HostileProjectileColor();
		}

		return RequestedColor;
	}
}

FName FT66TemporaryProjectileSystem::ProfileHeroPierce()
{
	static const FName Profile(TEXT("HeroPierce"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileHeroAOE()
{
	static const FName Profile(TEXT("HeroAOE"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileHeroBounce()
{
	static const FName Profile(TEXT("HeroBounce"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileHeroDOT()
{
	static const FName Profile(TEXT("HeroDOT"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileHeroSingleTarget()
{
	static const FName Profile(TEXT("HeroSingleTarget"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileIdolOverlay()
{
	static const FName Profile(TEXT("IdolOverlay"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileEnemySpit()
{
	static const FName Profile(TEXT("EnemySpit"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileTrapArrow()
{
	static const FName Profile(TEXT("TrapArrow"));
	return Profile;
}

FName FT66TemporaryProjectileSystem::ProfileHostileAccent()
{
	static const FName Profile(TEXT("HostileAccent"));
	return Profile;
}

FLinearColor FT66TemporaryProjectileSystem::HeroProjectileColor()
{
	return FLinearColor(0.08f, 0.52f, 1.f, 1.f);
}

FLinearColor FT66TemporaryProjectileSystem::NoWeaponProjectileColor()
{
	return FLinearColor(0.92f, 0.95f, 1.f, 1.f);
}

FLinearColor FT66TemporaryProjectileSystem::HostileProjectileColor()
{
	return FLinearColor(1.f, 0.04f, 0.02f, 1.f);
}

FName FT66TemporaryProjectileSystem::GetHeroAttackProfile(const ET66AttackCategory AttackCategory)
{
	switch (AttackCategory)
	{
	case ET66AttackCategory::Pierce:
		return ProfileHeroPierce();
	case ET66AttackCategory::Bounce:
		return ProfileHeroBounce();
	case ET66AttackCategory::DOT:
		return ProfileHeroDOT();
	case ET66AttackCategory::SingleTarget:
		return ProfileHeroSingleTarget();
	case ET66AttackCategory::AOE:
	default:
		return ProfileHeroAOE();
	}
}

FT66TemporaryProjectileVisualSpec FT66TemporaryProjectileSystem::MakeSpec(
	const FName ProfileID,
	const FLinearColor& Color,
	const float ScaleMultiplier)
{
	FT66TemporaryProjectileVisualSpec Spec;
	Spec.Color = T66ResolveTemporaryProjectileColor(ProfileID, Color);

	const float S = FMath::Clamp(ScaleMultiplier, 0.25f, 6.f);
	if (ProfileID == ProfileHeroPierce())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Cone;
		Spec.RelativeRotation = FRotator(-90.f, 0.f, 0.f);
		Spec.RelativeScale = FVector(0.56f, 0.56f, 1.08f) * S;
	}
	else if (ProfileID == ProfileHeroBounce())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Cube;
		Spec.RelativeRotation = FRotator(0.f, 0.f, 45.f);
		Spec.RelativeScale = FVector(0.54f, 0.54f, 0.54f) * S;
	}
	else if (ProfileID == ProfileHeroDOT())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Cylinder;
		Spec.RelativeRotation = FRotator(90.f, 0.f, 0.f);
		Spec.RelativeScale = FVector(0.62f, 0.62f, 0.50f) * S;
	}
	else if (ProfileID == ProfileHeroSingleTarget())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Sphere;
		Spec.RelativeScale = FVector(0.82f, 0.82f, 0.82f) * S;
	}
	else if (ProfileID == ProfileIdolOverlay())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Cube;
		Spec.RelativeLocation = FVector(0.f, 0.f, 30.f * S);
		Spec.RelativeRotation = FRotator(0.f, 45.f, 45.f);
		Spec.RelativeScale = FVector(0.30f, 0.30f, 0.30f) * S;
	}
	else if (ProfileID == ProfileEnemySpit())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Sphere;
		Spec.RelativeScale = FVector(1.10f, 1.10f, 1.10f) * S;
	}
	else if (ProfileID == ProfileTrapArrow())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Cone;
		Spec.RelativeRotation = FRotator(-90.f, 0.f, 0.f);
		Spec.RelativeScale = FVector(0.72f, 0.72f, 1.70f) * S;
	}
	else if (ProfileID == ProfileHostileAccent())
	{
		Spec.Shape = ET66TemporaryProjectileShape::Cone;
		Spec.RelativeLocation = FVector(34.f * S, 0.f, 0.f);
		Spec.RelativeRotation = FRotator(-90.f, 0.f, 0.f);
		Spec.RelativeScale = FVector(0.45f, 0.45f, 0.72f) * S;
	}
	else
	{
		Spec.Shape = ET66TemporaryProjectileShape::Sphere;
		Spec.RelativeScale = FVector(0.70f, 0.70f, 0.70f) * S;
	}

	return Spec;
}

void FT66TemporaryProjectileSystem::ApplyProfileToMesh(
	UStaticMeshComponent* Mesh,
	UObject* Outer,
	const FName ProfileID,
	const FLinearColor& Color,
	const float ScaleMultiplier)
{
	if (!Mesh || ProfileID.IsNone())
	{
		HideMesh(Mesh);
		return;
	}

	const FT66TemporaryProjectileVisualSpec Spec = MakeSpec(ProfileID, Color, ScaleMultiplier);
	if (UStaticMesh* ResolvedMesh = T66ResolveTemporaryProjectileMesh(Spec.Shape))
	{
		Mesh->SetStaticMesh(ResolvedMesh);
	}

	Mesh->SetRelativeLocation(Spec.RelativeLocation);
	Mesh->SetRelativeRotation(Spec.RelativeRotation);
	Mesh->SetRelativeScale3D(Spec.RelativeScale);
	Mesh->SetVisibility(Spec.bVisible, true);
	Mesh->SetHiddenInGame(!Spec.bVisible, true);
	Mesh->SetRenderInMainPass(Spec.bVisible);
	Mesh->SetCullDistance(0.f);
	Mesh->BoundsScale = FMath::Max(Mesh->BoundsScale, 2.0f);
	Mesh->SetCastShadow(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FT66VisualUtil::ApplyT66Color(Mesh, Outer, Spec.Color);
}

void FT66TemporaryProjectileSystem::HideMesh(UStaticMeshComponent* Mesh)
{
	if (!Mesh)
	{
		return;
	}

	Mesh->SetVisibility(false, true);
	Mesh->SetHiddenInGame(true, true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
