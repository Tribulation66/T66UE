// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageCatchUpGate.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Kismet/GameplayStatics.h"

AT66StageCatchUpGate::AT66StageCatchUpGate()
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

void AT66StageCatchUpGate::BeginPlay()
{
	Super::BeginPlay();
	FT66VisualUtil::SnapToGround(this, GetWorld());
}

bool AT66StageCatchUpGate::EnterChosenStage()
{
	UWorld* World = GetWorld();
	if (!World) return false;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return false;

	RunState->SetInStageCatchUp(false);

	RunState->SetCurrentStage(FMath::Clamp(TargetStage, 1, 33));

	T66GI->bIsStageTransition = true;
	T66GI->bStageCatchUpPending = false;

	RunState->ResetStageTimerToFull();
	RunState->ResetBossState();

	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/GameplayLevel")));
	return true;
}
