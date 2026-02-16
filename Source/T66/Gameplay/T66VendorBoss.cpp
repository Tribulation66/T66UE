// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorBoss.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AT66VendorBoss::AT66VendorBoss()
{
	BossID = FName(TEXT("VendorBoss"));
	MaxHP = 1000;
	FireIntervalSeconds = 0.8f;
	ProjectileSpeed = 950.f;
	ProjectileDamageHearts = 1;

	GroundAOEIntervalSeconds = 6.f;
	GroundAOERadius = 350.f;
	GroundAOEWarningSeconds = 1.2f;
	GroundAOEBaseDamageHP = 30;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 520.f;
	}
}

void AT66VendorBoss::BeginPlay()
{
	Super::BeginPlay();

	// Placeholder mesh: cube, dark (overridden by ApplyCharacterVisual if mapped).
	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(4.f, 4.f, 4.f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 112.f));
	}
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.08f, 0.08f, 0.10f, 1.f));

	// Vendor boss must use the same mesh as Vendor NPC (per rule).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(FName(TEXT("VendorBoss")), GetMesh(), VisualMesh, true);
			if (!bApplied && GetMesh())
			{
				GetMesh()->SetVisibility(false, true);
			}
		}
	}

	ForceAwaken();
}

void AT66VendorBoss::Die()
{
	UWorld* World = GetWorld();
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ResetVendorAnger();
		}
	}

	// Respawn Vendor NPC so the house becomes interactable again this stage.
	if (World)
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, P);
	}

	Super::Die();
}
