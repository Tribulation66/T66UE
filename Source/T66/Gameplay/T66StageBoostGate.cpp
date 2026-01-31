// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageBoostGate.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Kismet/GameplayStatics.h"

AT66StageBoostGate::AT66StageBoostGate()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(140.f, 240.f, 180.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(RootComponent);
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GateMesh->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	GateMesh->SetRelativeScale3D(FVector(2.5f, 4.5f, 3.0f));

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		GateMesh->SetStaticMesh(Cube);
	}
	FT66VisualUtil::ApplyT66Color(GateMesh, this, FLinearColor(0.20f, 0.85f, 0.35f, 1.f));
}

void AT66StageBoostGate::BeginPlay()
{
	Super::BeginPlay();
}

bool AT66StageBoostGate::EnterChosenStage()
{
	UWorld* World = GetWorld();
	if (!World) return false;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return false;

	RunState->SetInStageBoost(false);

	// Start at the chosen stage number (e.g. 11/21/31...). Clamp to [1..66].
	RunState->SetCurrentStage(FMath::Clamp(TargetStage, 1, 66));

	// New stage load should be treated as a stage transition (keep progress).
	T66GI->bIsStageTransition = true;
	T66GI->bStageBoostPending = false;

	// Stage timer remains frozen at full until the start gate is crossed.
	RunState->ResetStageTimerToFull();
	RunState->ResetBossState();

	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/GameplayLevel")));
	return true;
}

