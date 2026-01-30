// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LeprechaunEnemy.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66Rarity.h"
#include "Gameplay/T66VisualUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

AT66LeprechaunEnemy::AT66LeprechaunEnemy()
{
	// Distinct look: sphere above a cube.
	if (VisualMesh)
	{
		if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
		{
			VisualMesh->SetStaticMesh(Cube);
		}
		VisualMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.45f));
	}

	HeadSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadSphere"));
	HeadSphere->SetupAttachment(RootComponent);
	HeadSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		HeadSphere->SetStaticMesh(Sphere);
	}
	HeadSphere->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));
	HeadSphere->SetRelativeLocation(FVector(0.f, 0.f, 30.f));

	// Behavior: flee.
	bRunAwayFromPlayer = true;
	PointValue = 25;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 520.f;
	}

	ApplyRarityVisuals();
	RecomputeGoldFromRarity();
}

void AT66LeprechaunEnemy::SetRarity(ET66Rarity InRarity)
{
	Rarity = InRarity;
	ApplyRarityVisuals();
	RecomputeGoldFromRarity();
}

void AT66LeprechaunEnemy::ApplyRarityVisuals()
{
	const FLinearColor C = FT66RarityUtil::GetRarityColor(Rarity);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, C);
	FT66VisualUtil::ApplyT66Color(HeadSphere, this, C);
}

void AT66LeprechaunEnemy::RecomputeGoldFromRarity()
{
	switch (Rarity)
	{
	case ET66Rarity::Black: GoldPerHit = 25; break;
	case ET66Rarity::Red: GoldPerHit = 50; break;
	case ET66Rarity::Yellow: GoldPerHit = 100; break;
	case ET66Rarity::White: GoldPerHit = 200; break;
	default: GoldPerHit = 25; break;
	}
}

bool AT66LeprechaunEnemy::TakeDamageFromHero(int32 Damage)
{
	// Only award gold if this hit actually applied damage to a living target.
	if (Damage <= 0 || CurrentHP <= 0)
	{
		return false;
	}

	UT66RunStateSubsystem* RunState = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		}
	}

	const bool bApplied = Super::TakeDamageFromHero(Damage);
	if (bApplied && RunState && GoldPerHit > 0)
	{
		RunState->AddGold(GoldPerHit);
	}
	return bApplied;
}

