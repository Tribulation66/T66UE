// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ChestMimicEnemy.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"

AT66ChestMimicEnemy::AT66ChestMimicEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 5.0f;

	TouchDamageHearts = 3;
	PointValue = 0;
	bDropsLoot = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 650.f;
	}

	if (VisualMesh)
	{
		if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
		{
			VisualMesh->SetStaticMesh(Cube);
		}
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
		VisualMesh->SetRelativeScale3D(FVector(1.8f, 1.0f, 0.9f));
		if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.35f, 0.10f, 0.55f, 1.f));
		}
	}
}

void AT66ChestMimicEnemy::OnDeath()
{
	Super::OnDeath();
}
