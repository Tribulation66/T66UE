// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyAIController.h"
#include "Gameplay/T66EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

AT66EnemyAIController::AT66EnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AT66EnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(RepathTimerHandle, this, &AT66EnemyAIController::UpdateMoveToPlayer, RepathIntervalSeconds, true, 0.5f);
}

void AT66EnemyAIController::UpdateMoveToPlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	APawn* MyPawn = GetPawn();
	if (PlayerPawn && MyPawn)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToActor(this, PlayerPawn);
	}
}
