// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CashTruckMimicEnemy.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"

AT66CashTruckMimicEnemy::AT66CashTruckMimicEnemy()
{
	// Disappear after 5 seconds regardless.
	PrimaryActorTick.bCanEverTick = true; // uses base movement tick
	InitialLifeSpan = 5.0f;

	// Heavy touch damage (tunable).
	TouchDamageHearts = 3;
	// TouchDamageCooldown is a static constexpr on the base class (0.5s) for now.

	// Beefier move speed so it feels like a threat.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 650.f;
	}

	// Replace base enemy cylinder with a truck-ish cube.
	if (VisualMesh)
	{
		if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
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

	WheelMeshes.SetNum(4);
	for (int32 i = 0; i < WheelMeshes.Num(); ++i)
	{
		const FName Name = FName(*FString::Printf(TEXT("MimicWheel_%d"), i));
		WheelMeshes[i] = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		WheelMeshes[i]->SetupAttachment(RootComponent);
		WheelMeshes[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		for (UStaticMeshComponent* W : WheelMeshes)
		{
			if (!W) continue;
			W->SetStaticMesh(Cylinder);
			W->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.08f));
			W->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
			if (UMaterialInstanceDynamic* Mat = W->CreateAndSetMaterialInstanceDynamic(0))
			{
				Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.05f, 0.05f, 0.05f, 1.f));
			}
		}
	}

	static const FVector Offsets[4] =
	{
		FVector( 120.f,  70.f, -95.f),
		FVector( 120.f, -70.f, -95.f),
		FVector(-120.f,  70.f, -95.f),
		FVector(-120.f, -70.f, -95.f),
	};
	for (int32 i = 0; i < WheelMeshes.Num(); ++i)
	{
		if (WheelMeshes[i])
		{
			WheelMeshes[i]->SetRelativeLocation(Offsets[i]);
		}
	}
}

void AT66CashTruckMimicEnemy::OnDeath()
{
	// Mimics do not drop items / score in v0. They simply vanish.
	Destroy();
}

