// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"

AT66CowardiceGate::AT66CowardiceGate()
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
	GateMesh->SetRelativeScale3D(FVector(2.f, 3.f, 2.6f));
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GateMesh->SetupAttachment(RootComponent);
}

bool AT66CowardiceGate::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;
	T66PC->OpenCowardicePrompt(this);
	return true;
}

bool AT66CowardiceGate::ConfirmCowardice()
{
	UWorld* World = GetWorld();
	if (!World) return false;

	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	if (!RunState || !T66GI) return false;

	// Determine the current stage's boss ID and mark it owed.
	const int32 CurrentStage = RunState->GetCurrentStage();
	FStageData StageData;
	if (!T66GI->GetStageData(CurrentStage, StageData))
	{
		StageData.BossID = FName(*FString::Printf(TEXT("Boss_%02d"), CurrentStage));
	}
	if (!StageData.BossID.IsNone())
	{
		RunState->AddOwedBoss(StageData.BossID);
	}

	// Advance stage (same as StageGate) and travel.
	const int32 NextStage = CurrentStage + 1;
	RunState->SetCurrentStage(NextStage);
	RunState->SetLoanSharkPending(RunState->GetCurrentDebt() > 0);
	T66GI->bIsStageTransition = true;

	const bool bIsCheckpointStage = (NextStage % 5) == 0;
	if (bIsCheckpointStage && RunState->GetOwedBossIDs().Num() > 0)
	{
		T66GI->bForceColiseumMode = true;
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/GameplayLevel")));
		return true;
	}

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (LevelName.IsEmpty()) return false;
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
	return true;
}

