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
#include "UObject/SoftObjectPath.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Gameplay/T66VisualUtil.h"

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

	UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetStaticMesh(CubeMesh);
	GateMesh->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	GateMesh->SetRelativeScale3D(FVector(2.f, 3.f, 2.6f));
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GateMesh->SetupAttachment(RootComponent);

	// Default expected import location (safe if missing).
	GateMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Gates/SM_CowardiceGate.SM_CowardiceGate")));
}

void AT66CowardiceGate::BeginPlay()
{
	Super::BeginPlay();

	if (GateMesh && !GateMeshOverride.IsNull())
	{
		if (UStaticMesh* M = GateMeshOverride.LoadSynchronous())
		{
			GateMesh->SetStaticMesh(M);
			GateMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
			FT66VisualUtil::GroundMeshToActorOrigin(GateMesh, M);
		}
	}

	FT66VisualUtil::SnapToGround(this, GetWorld());
}

bool AT66CowardiceGate::Interact(APlayerController* PC)
{
	UWorld* World = GetWorld();
	if (!World) return false;
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;
	// Cannot skip the difficulty boss (last stage of each difficulty block).
	const int32 CurrentStage = RunState->GetCurrentStage();
	const bool bIsDifficultyBossStage = (CurrentStage == 5 || CurrentStage == 10 || CurrentStage == 15 || CurrentStage == 20 || CurrentStage == 25 || CurrentStage == 30 || CurrentStage == 33);
	if (bIsDifficultyBossStage) return false;

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
	RunState->AddCowardiceGateTaken();

	// Advance stage (same as StageGate) and travel.
	const int32 NextStage = CurrentStage + 1;
	RunState->SetCurrentStage(NextStage);
	RunState->SetLoanSharkPending(RunState->GetCurrentDebt() > 0);
	T66GI->bIsStageTransition = true;

	// Coliseum rule: before entering a difficulty boss stage, route to Coliseum if there are owed bosses.
	const bool bIsDifficultyBossStage = (NextStage == 5 || NextStage == 10 || NextStage == 15 || NextStage == 20 || NextStage == 25 || NextStage == 30 || NextStage == 33);
	if (bIsDifficultyBossStage && RunState->GetOwedBossIDs().Num() > 0)
	{
		T66GI->bForceColiseumMode = true;
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetColiseumLevelName());
		return true;
	}

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (LevelName.IsEmpty()) return false;
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
	return true;
}

