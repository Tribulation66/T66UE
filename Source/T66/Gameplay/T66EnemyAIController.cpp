// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyAIController.h"
#include "Gameplay/T66EnemyBase.h"
#include "Kismet/GameplayStatics.h"

AT66EnemyAIController::AT66EnemyAIController()
{
	// No per-frame tick needed; enemies use direct movement in their own Tick.
	PrimaryActorTick.bCanEverTick = false;
}

void AT66EnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	// Navmesh pathfinding removed: enemies now use direct AddMovementInput in
	// AT66EnemyBase::Tick() which is far cheaper for a top-down game with many
	// enemies. The timer that called SimpleMoveToActor every 0.5s per enemy has
	// been removed to eliminate navigation system overhead.
}

void AT66EnemyAIController::UpdateMoveToPlayer()
{
	// Intentionally empty. Direct movement is handled by AT66EnemyBase::Tick().
	// Kept for ABI compatibility (timer handle referenced in header).
}
