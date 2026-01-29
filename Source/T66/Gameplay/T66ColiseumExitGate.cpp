// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ColiseumExitGate.h"
#include "Core/T66GameInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AT66ColiseumExitGate::AT66ColiseumExitGate()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(140.f, 220.f, 180.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionObjectType(ECC_Pawn);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetStaticMesh(CubeMesh);
	GateMesh->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	GateMesh->SetRelativeScale3D(FVector(2.f, 4.f, 3.f));
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GateMesh->SetupAttachment(RootComponent);
}

bool AT66ColiseumExitGate::Interact(APlayerController* PC)
{
	UWorld* World = GetWorld();
	if (!World) return false;
	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!T66GI) return false;

	// Keep progress and go back to GameplayLevel for the current stage.
	T66GI->bIsStageTransition = true;
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
	return true;
}

