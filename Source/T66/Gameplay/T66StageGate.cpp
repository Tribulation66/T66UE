// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageGate.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AT66StageGate::AT66StageGate()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(120.f, 220.f, 180.f)); // large enough to stand in front and press F
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	// Big 3D rectangle (cube scaled): wide and tall
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetStaticMesh(CubeMesh);
	GateMesh->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	GateMesh->SetRelativeScale3D(FVector(2.f, 4.f, 3.f)); // 200 x 400 x 300 units
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GateMesh->SetupAttachment(RootComponent);
}

void AT66StageGate::BeginPlay()
{
	Super::BeginPlay();
}

bool AT66StageGate::AdvanceToNextStage()
{
	UWorld* World = GetWorld();
	if (!World) return false;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return false;

	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	if (!RunState) return false;

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!T66GI) return false;

	const int32 NextStage = RunState->GetCurrentStage() + 1;
	RunState->SetCurrentStage(NextStage);

	T66GI->bIsStageTransition = true;

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (LevelName.IsEmpty()) return false;

	UGameplayStatics::OpenLevel(this, FName(*LevelName));
	return true;
}
