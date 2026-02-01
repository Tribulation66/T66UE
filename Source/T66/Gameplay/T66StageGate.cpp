// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageGate.h"
#include "Core/T66AchievementsSubsystem.h"
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
