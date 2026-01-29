// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66DifficultyTotem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AT66DifficultyTotem::AT66DifficultyTotem()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(90.f, 90.f, 220.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionObjectType(ECC_Pawn);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		VisualMesh->SetStaticMesh(Cube);
		// Tall skinny rectangle
		VisualMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 4.0f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	}
}

bool AT66DifficultyTotem::Interact(APlayerController* PC)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;
	RunState->IncreaseDifficultyTier();
	return true;
}

