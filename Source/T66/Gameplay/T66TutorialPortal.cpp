// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TutorialPortal.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

#include "Gameplay/T66PlayerController.h"

AT66TutorialPortal::AT66TutorialPortal()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetSphereRadius(220.f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetGenerateOverlapEvents(true);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerSphere;

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootComponent);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		PortalMesh->SetStaticMesh(Cylinder);
		PortalMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.5f));
		// Cylinder is 100 units tall; scale Z=2.5 => 250 tall, half-height=125. Sit on ground plane.
		PortalMesh->SetRelativeLocation(FVector(0.f, 0.f, 125.f));
	}
}

bool AT66TutorialPortal::Interact(AT66PlayerController* PC)
{
	if (!PC) return false;
	UWorld* World = GetWorld();
	if (!World) return false;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return false;

	// Snap the target to ground and teleport without loading.
	FVector Dest = TargetLocation;
	{
		FHitResult Hit;
		const FVector Start = Dest + FVector(0.f, 0.f, 2500.f);
		const FVector End = Dest + FVector(0.f, 0.f, -8000.f);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66TutorialPortalTrace), false);
		Params.AddIgnoredActor(Pawn);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			float HalfHeight = 90.f;
			if (ACharacter* C = Cast<ACharacter>(Pawn))
			{
				if (UCapsuleComponent* Capsule = C->GetCapsuleComponent())
				{
					HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}
			}
			Dest.Z = Hit.ImpactPoint.Z + HalfHeight + 5.f;
		}
	}

	Pawn->SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics);

	// Persist tutorial completion (profile) and clear any active tutorial hint.
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->MarkTutorialCompleted();
		}
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ClearTutorialHint();
		}
	}

	if (bDestroyOnUse)
	{
		Destroy();
	}
	return true;
}

