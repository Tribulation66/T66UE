// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TutorialManager.generated.h"

class AT66TutorialPromptActor;

/** Stage 1 tutorial flow (v0): WASD prompt then Space prompt. */
UCLASS(Blueprintable)
class T66_API AT66TutorialManager : public AActor
{
	GENERATED_BODY()

public:
	AT66TutorialManager();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<AT66TutorialPromptActor> PromptMove;

	UPROPERTY()
	TObjectPtr<AT66TutorialPromptActor> PromptJump;

	UFUNCTION()
	void HandleMovePassed();
};

