// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LeprechaunEnemy.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66Rarity.h"
#include "Gameplay/T66VisualUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

static FName T66_GetLeprechaunVisualIdForRarity(ET66Rarity R)
{
	switch (R)
	{
	case ET66Rarity::Black:  return FName(TEXT("Leprechaun_Black"));
	case ET66Rarity::Red:    return FName(TEXT("Leprechaun_Red"));
	case ET66Rarity::Yellow: return FName(TEXT("Leprechaun_Yellow"));
	case ET66Rarity::White:  return FName(TEXT("Leprechaun_White"));
	default:                 return FName(TEXT("Leprechaun_Black"));
	}
}

AT66LeprechaunEnemy::AT66LeprechaunEnemy()
{
	// Default to Black; Director will call SetRarity shortly after spawn.
	CharacterVisualID = T66_GetLeprechaunVisualIdForRarity(Rarity);

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

void AT66LeprechaunEnemy::BeginPlay()
{
	Super::BeginPlay();

	// If an imported skeletal mesh is in use, hide placeholder parts.
	if (bUsingCharacterVisual)
	{
		if (HeadSphere) HeadSphere->SetVisibility(false, true);
	}
}

void AT66LeprechaunEnemy::SetRarity(ET66Rarity InRarity)
{
	Rarity = InRarity;
	CharacterVisualID = T66_GetLeprechaunVisualIdForRarity(Rarity);

	// Re-apply imported visuals now (SetRarity is typically called AFTER BeginPlay).
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
			{
				bUsingCharacterVisual = Visuals->ApplyCharacterVisual(CharacterVisualID, GetMesh(), VisualMesh, true);
			}
		}
	}
	if (bUsingCharacterVisual && HeadSphere)
	{
		HeadSphere->SetVisibility(false, true);
		HeadSphere->SetHiddenInGame(true, true);
	}

	ApplyRarityVisuals();
	RecomputeGoldFromRarity();
}

void AT66LeprechaunEnemy::ApplyRarityVisuals()
{
	// If we have a real imported mesh, do not tint placeholder parts.
	if (bUsingCharacterVisual)
	{
		return;
	}
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

bool AT66LeprechaunEnemy::TakeDamageFromHero(int32 Damage, FName DamageSourceID)
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

	const bool bApplied = Super::TakeDamageFromHero(Damage, DamageSourceID);
	if (bApplied && RunState && GoldPerHit > 0)
	{
		RunState->AddGold(GoldPerHit);
	}
	return bApplied;
}

