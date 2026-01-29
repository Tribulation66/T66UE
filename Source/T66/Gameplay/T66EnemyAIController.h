// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "T66EnemyAIController.generated.h"

UCLASS(Blueprintable)
class T66_API AT66EnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AT66EnemyAIController();

	/** How often to update MoveToActor (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float RepathIntervalSeconds = 0.5f;

protected:
	virtual void BeginPlay() override;

	void UpdateMoveToPlayer();

	FTimerHandle RepathTimerHandle;
};
