// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66Rarity.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

AT66GoblinThiefEnemy::AT66GoblinThiefEnemy()
{
	CharacterVisualID = FName(TEXT("GoblinThief"));

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
	ApplyRarityVisuals();
	RecomputeGoldFromRarity();
}

void AT66GoblinThiefEnemy::ApplyRarityVisuals()
{
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

