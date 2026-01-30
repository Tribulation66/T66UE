// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LeprechaunEnemy.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66Rarity.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"

static void ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;
	if (UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")))
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, Outer ? Outer : Mesh))
		{
			Mat->SetVectorParameterValue(FName("Color"), Color);
			Mesh->SetMaterial(0, Mat);
			return;
		}
	}
	if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(FName("Color"), Color);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

AT66LeprechaunEnemy::AT66LeprechaunEnemy()
{
	// Distinct look: sphere above a cube.
	if (VisualMesh)
	{
		if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
		{
			VisualMesh->SetStaticMesh(Cube);
		}
		VisualMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.45f));
	}

	HeadSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadSphere"));
	HeadSphere->SetupAttachment(RootComponent);
	HeadSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
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
	ApplyT66Color(VisualMesh, this, C);
	ApplyT66Color(HeadSphere, this, C);
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
	// Give gold on hit (even if this hit kills it).
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->AddGold(GoldPerHit);
			}
		}
	}

	return Super::TakeDamageFromHero(Damage);
}

