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
		// Support "flee" enemies (e.g. Leprechaun) by moving away from the player.
		if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(MyPawn))
		{
			if (Enemy->bRunAwayFromPlayer)
			{
				FVector Away = MyPawn->GetActorLocation() - PlayerPawn->GetActorLocation();
				Away.Z = 0.f;
				if (!Away.Normalize())
				{
					Away = FVector(1.f, 0.f, 0.f);
				}
				const FVector TargetLoc = MyPawn->GetActorLocation() + Away * 1600.f;
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, TargetLoc);
				return;
			}
		}

		UAIBlueprintHelperLibrary::SimpleMoveToActor(this, PlayerPawn);
	}
}
