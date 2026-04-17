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
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66ActorRegistrySubsystem.h"

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
	UStaticMesh* CubeMesh = FT66VisualUtil::GetBasicShapeCube();
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

	// [GOLD] Register with the actor registry (replaces TActorIterator for map markers).
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterStageGate(this);
		}
	}

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

void AT66StageGate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// [GOLD] Unregister from the actor registry.
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterStageGate(this);
		}
	}
	Super::EndPlay(EndPlayReason);
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

	// Special: Coliseum mode stage gate returns to the checkpointed gameplay stage (do NOT increment).
	if (T66GI->bForceColiseumMode || T66GI->ColiseumFlowMode != ET66ColiseumFlowMode::None)
	{
		T66GI->bForceColiseumMode = false;
		T66GI->ColiseumFlowMode = ET66ColiseumFlowMode::None;
		T66GI->bIsStageTransition = true;
		// Return to GameplayLevel (checkpoint stage is already set before entering Coliseum).
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
		return true;
	}

	// Companion Union + achievement: clearing stages with a companion increases Union and notifies stage cleared.
	if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
	{
		Ach->NotifyStageCleared(1);
		if (!T66GI->SelectedHeroID.IsNone())
		{
			Ach->AddHeroUnityStagesCleared(T66GI->SelectedHeroID, 1);
		}
		if (!T66GI->SelectedCompanionID.IsNone())
		{
			Ach->AddCompanionUnionStagesCleared(T66GI->SelectedCompanionID, 1);
		}
	}

	const int32 NextStage = RunState->GetCurrentStage() + 1;
	RunState->SetCurrentStage(NextStage);

	// If leaving with unpaid debt, schedule loan shark for next stage (spawns when timer starts).
	RunState->SetLoanSharkPending(RunState->GetCurrentDebt() > 0);

	T66GI->bIsStageTransition = true;
	T66GI->bPendingTowerStageDropIntro = false;
	T66GI->bForceColiseumMode = false;
	T66GI->ColiseumFlowMode = ET66ColiseumFlowMode::None;

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
	if (LevelName.IsEmpty()) return false;
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
	return true;
}
