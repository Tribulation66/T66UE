// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageCatchUpGate.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/T66GameMode.h"

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
	GateMesh->SetHiddenInGame(false, true);
	GateMesh->SetVisibility(true, true);

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		GateMesh->SetStaticMesh(Cube);
	}
	FT66VisualUtil::ApplyT66Color(GateMesh, this, FLinearColor(0.20f, 0.85f, 0.35f, 1.f));
}

void AT66StageCatchUpGate::BeginPlay()
{
	Super::BeginPlay();
	const AT66GameMode* T66GameMode = GetWorld() ? Cast<AT66GameMode>(GetWorld()->GetAuthGameMode()) : nullptr;
	if (!T66GameMode || !T66GameMode->IsUsingTowerMainMapLayout())
	{
		FT66VisualUtil::SnapToGround(this, GetWorld());
	}

	const FVector ActorLocation = GetActorLocation();
	const FVector MeshLocation = GateMesh ? GateMesh->GetComponentLocation() : FVector::ZeroVector;
	const FVector MeshScale = GateMesh ? GateMesh->GetComponentScale() : FVector::OneVector;
	const FBox MeshBounds = GateMesh ? GateMesh->Bounds.GetBox() : FBox(EForceInit::ForceInit);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("StageCatchUpGate BeginPlay: Actor=%s Mode=%s ActorLoc=(%.0f, %.0f, %.0f) MeshLoc=(%.0f, %.0f, %.0f) MeshScale=(%.2f, %.2f, %.2f) MeshBoundsMin=(%.0f, %.0f, %.0f) MeshBoundsMax=(%.0f, %.0f, %.0f) Hidden=%d Visible=%d"),
		*GetName(),
		bActsAsStageAdvanceGate ? TEXT("StageAdvance") : TEXT("CatchUp"),
		ActorLocation.X,
		ActorLocation.Y,
		ActorLocation.Z,
		MeshLocation.X,
		MeshLocation.Y,
		MeshLocation.Z,
		MeshScale.X,
		MeshScale.Y,
		MeshScale.Z,
		MeshBounds.Min.X,
		MeshBounds.Min.Y,
		MeshBounds.Min.Z,
		MeshBounds.Max.X,
		MeshBounds.Max.Y,
		MeshBounds.Max.Z,
		GateMesh ? (GateMesh->bHiddenInGame ? 1 : 0) : -1,
		GateMesh ? (GateMesh->IsVisible() ? 1 : 0) : -1);
}

bool AT66StageCatchUpGate::EnterChosenStage()
{
	UWorld* World = GetWorld();
	if (!World) return false;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return false;

	if (bActsAsStageAdvanceGate)
	{
		// Special: Coliseum mode stage gate returns to the checkpointed gameplay stage (do NOT increment).
		if (T66GI->bForceColiseumMode || T66GI->ColiseumFlowMode != ET66ColiseumFlowMode::None)
		{
			T66GI->bStageCatchUpPending = false;
			RunState->SetInStageCatchUp(false);
			T66GI->bForceColiseumMode = false;
			T66GI->ColiseumFlowMode = ET66ColiseumFlowMode::None;
			T66GI->bIsStageTransition = true;
			UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
			return true;
		}

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

		T66GI->bStageCatchUpPending = false;
		RunState->SetInStageCatchUp(false);
		RunState->SetCurrentStage(RunState->GetCurrentStage() + 1);
		RunState->SetLoanSharkPending(RunState->GetCurrentDebt() > 0);

		T66GI->bIsStageTransition = true;
		T66GI->bPendingTowerStageDropIntro = false;
		T66GI->bForceColiseumMode = false;
		T66GI->ColiseumFlowMode = ET66ColiseumFlowMode::None;

		const FString LevelName = UGameplayStatics::GetCurrentLevelName(this);
		if (LevelName.IsEmpty())
		{
			return false;
		}

		UGameplayStatics::OpenLevel(this, FName(*LevelName));
		return true;
	}

	RunState->SetInStageCatchUp(false);

	RunState->SetCurrentStage(FMath::Clamp(TargetStage, 1, 23));

	T66GI->bIsStageTransition = true;
	T66GI->bStageCatchUpPending = false;

	RunState->ResetStageTimerToFull();
	RunState->ResetBossState();

	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/Maps/GameplayLevel")));
	return true;
}
