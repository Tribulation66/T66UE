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
			// Ground to actor origin using bounds (handles pivots already at the base).
			const FBoxSphereBounds B = M->GetBounds();
			const float BottomZ = (B.Origin.Z - B.BoxExtent.Z);
			GateMesh->SetRelativeLocation(FVector(0.f, 0.f, -BottomZ));
		}
	}

	// Snap to ground so the gate never floats.
	if (UWorld* World = GetWorld())
	{
		FHitResult Hit;
		const FVector Here = GetActorLocation();
		const FVector Start = Here + FVector(0.f, 0.f, 2000.f);
		const FVector End = Here - FVector(0.f, 0.f, 12000.f);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66CowardiceGateSnap), false, this);
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		if (World->LineTraceSingleByObjectType(Hit, Start, End, ObjParams, Params))
		{
			SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
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
	RunState->AddCowardiceGateTaken();

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

