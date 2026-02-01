// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageGate.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

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

	// Default expected import location (safe if missing).
	GateMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Gates/SM_StageGate.SM_StageGate")));
}

void AT66StageGate::BeginPlay()
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
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66StageGateSnap), false, this);
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		if (World->LineTraceSingleByObjectType(Hit, Start, End, ObjParams, Params))
		{
			SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
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

	// Special: Coliseum mode stage gate returns to the checkpoint stage (do NOT increment).
	if (T66GI->bForceColiseumMode)
	{
		T66GI->bForceColiseumMode = false;
		T66GI->bIsStageTransition = true;
		// Return to GameplayLevel (checkpoint stage is already set before entering Coliseum).
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/GameplayLevel")));
		return true;
	}

	// Companion Union: clearing stages with a companion increases Union for that companion (profile progression).
	if (!T66GI->SelectedCompanionID.IsNone())
	{
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->AddCompanionUnionStagesCleared(T66GI->SelectedCompanionID, 1);
		}
	}

	const int32 NextStage = RunState->GetCurrentStage() + 1;
	RunState->SetCurrentStage(NextStage);

	// If leaving with unpaid debt, schedule loan shark for next stage (spawns when timer starts).
	RunState->SetLoanSharkPending(RunState->GetCurrentDebt() > 0);

	T66GI->bIsStageTransition = true;

	// Coliseum rule: before checkpoint stages (5/10/15...), route to Coliseum mode if there are owed bosses.
	const bool bIsCheckpointStage = (NextStage % 5) == 0;
	if (bIsCheckpointStage && RunState->GetOwedBossIDs().Num() > 0)
	{
		T66GI->bIsStageTransition = true;
		T66GI->bForceColiseumMode = true;
		UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/GameplayLevel")));
		return true;
	}

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (LevelName.IsEmpty()) return false;
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
	return true;
}
