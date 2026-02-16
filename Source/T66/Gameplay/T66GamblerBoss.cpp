// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AT66GamblerBoss::AT66GamblerBoss()
{
	BossID = FName(TEXT("GamblerBoss"));
	MaxHP = 1000;
	FireIntervalSeconds = 0.6f;
	ProjectileSpeed = 1100.f;
	ProjectileDamageHearts = 1;

	GroundAOEIntervalSeconds = 4.f;
	GroundAOERadius = 250.f;
	GroundAOEWarningSeconds = 1.0f;
	GroundAOEBaseDamageHP = 20;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 520.f;
	}
}

void AT66GamblerBoss::BeginPlay()
{
	Super::BeginPlay();

	// Placeholder mesh: smaller sphere, casino yellow (overridden by ApplyCharacterVisual if mapped).
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(4.f, 4.f, 4.f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 112.f));
	}
	if (UMaterialInterface* ColorMat = FT66VisualUtil::GetPlaceholderColorMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this))
		{
			Mat->SetVectorParameterValue(FName("Color"), FLinearColor(0.85f, 0.75f, 0.15f, 1.f));
			VisualMesh->SetMaterial(0, Mat);
		}
	}

	// Gambler boss must use the same mesh as Gambler NPC (per rule).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(FName(TEXT("GamblerBoss")), GetMesh(), VisualMesh, true);
			if (!bApplied && GetMesh())
			{
				GetMesh()->SetVisibility(false, true);
			}
		}
	}

	ForceAwaken();
}

void AT66GamblerBoss::Die()
{
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ResetGamblerAnger();
		}
	}
	Super::Die();
}
