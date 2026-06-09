// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TutorialGate.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66TutorialGate::AT66TutorialGate()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetSphereRadius(220.f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetGenerateOverlapEvents(true);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerSphere;

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(RootComponent);
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		GateMesh->SetStaticMesh(Cylinder);
		GateMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.5f));
		GateMesh->SetRelativeLocation(FVector(0.f, 0.f, 125.f));
	}

	GateMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Gates/SM_TutorialGate_Pixal3D.SM_TutorialGate_Pixal3D")));
}

void AT66TutorialGate::BeginPlay()
{
	Super::BeginPlay();

	if (GateMesh && !GateMeshOverride.IsNull())
	{
		if (UStaticMesh* ImportedMesh = GateMeshOverride.LoadSynchronous())
		{
			GateMesh->EmptyOverrideMaterials();
			GateMesh->SetStaticMesh(ImportedMesh);
			GateMesh->SetRelativeScale3D(FVector::OneVector);
			GateMesh->SetRelativeRotation(FRotator::ZeroRotator);
			FT66VisualUtil::GroundMeshToActorOrigin(GateMesh, ImportedMesh);
		}
	}
}

bool AT66TutorialGate::Interact(AT66PlayerController* PC)
{
	if (!PC) return false;
	UWorld* World = GetWorld();
	if (!World) return false;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return false;

	FVector Dest = TargetLocation;
	{
		FHitResult Hit;
		const FVector Start = Dest + FVector(0.f, 0.f, 2500.f);
		const FVector End = Dest + FVector(0.f, 0.f, -8000.f);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66TutorialGateTrace), false);
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

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			T66GI->SelectedRunMode = ET66RunMode::Regular;
			T66GI->SelectedRunCategory = ET66RunCategory::Tower;
			T66GI->ApplyConfiguredMainMapLayoutVariant();
		}
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->MarkTutorialCompleted();
		}
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ClearTutorialHint();
		}
	}

	UT66GameInstance::TransitionToFrontendLevel(this);
	return true;
}
