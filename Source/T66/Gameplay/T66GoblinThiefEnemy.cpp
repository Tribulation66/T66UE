// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66Rarity.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

static FName T66_GetGoblinThiefVisualIdForRarity(ET66Rarity R)
{
	switch (R)
	{
	case ET66Rarity::Black:  return FName(TEXT("GoblinThief_Black"));
	case ET66Rarity::Red:    return FName(TEXT("GoblinThief_Red"));
	case ET66Rarity::Yellow: return FName(TEXT("GoblinThief_Yellow"));
	case ET66Rarity::White:  return FName(TEXT("GoblinThief_White"));
	default:                 return FName(TEXT("GoblinThief_Black"));
	}
}

AT66GoblinThiefEnemy::AT66GoblinThiefEnemy()
{
	// Default to Black; Director will call SetRarity shortly after spawn.
	CharacterVisualID = T66_GetGoblinThiefVisualIdForRarity(Rarity);

	// Distinct look: pyramid (cone) shape.
	if (VisualMesh)
	{
		UStaticMesh* Cone = FT66VisualUtil::GetBasicShapeCone();
		if (!Cone) Cone = FT66VisualUtil::GetBasicShapeCylinder();
		if (Cone)
		{
			VisualMesh->SetStaticMesh(Cone);
		}
		VisualMesh->SetRelativeScale3D(FVector(0.70f, 0.70f, 0.85f));
	}

	// No heart damage on touch; we steal gold instead.
	TouchDamageHearts = 0;
	PointValue = 20;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 430.f;
	}

	ApplyRarityVisuals();
	RecomputeGoldFromRarity();
}

void AT66GoblinThiefEnemy::SetRarity(ET66Rarity InRarity)
{
	Rarity = InRarity;
	CharacterVisualID = T66_GetGoblinThiefVisualIdForRarity(Rarity);

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

	ApplyRarityVisuals();
	RecomputeGoldFromRarity();
}

void AT66GoblinThiefEnemy::ApplyRarityVisuals()
{
	// If we have a real imported mesh, do not tint the placeholder cone.
	if (bUsingCharacterVisual)
	{
		return;
	}
	const FLinearColor C = FT66RarityUtil::GetRarityColor(Rarity);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, C);
}

void AT66GoblinThiefEnemy::RecomputeGoldFromRarity()
{
	switch (Rarity)
	{
	case ET66Rarity::Black: GoldStolenPerHit = 25; break;
	case ET66Rarity::Red: GoldStolenPerHit = 50; break;
	case ET66Rarity::Yellow: GoldStolenPerHit = 100; break;
	case ET66Rarity::White: GoldStolenPerHit = 200; break;
	default: GoldStolenPerHit = 25; break;
	}
}

void AT66GoblinThiefEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Replace base overlap handler so we steal gold instead of applying heart damage.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->OnComponentBeginOverlap.RemoveAll(this);
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66GoblinThiefEnemy::OnCapsuleBeginOverlapThief);
	}
}

void AT66GoblinThiefEnemy::OnCapsuleBeginOverlapThief(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (Hero->IsInSafeZone()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;
	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	if (!RunState) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (Now - LastTouchDamageTime < TouchDamageCooldown) return;
	LastTouchDamageTime = Now;

	// Steal gold without damaging hearts.
	RunState->TrySpendGold(GoldStolenPerHit);
}

