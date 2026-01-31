// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TutorialManager.h"

#include "Gameplay/T66TutorialPromptActor.h"
#include "Engine/World.h"

AT66TutorialManager::AT66TutorialManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AT66TutorialManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Place prompts along the natural Start Area path toward the gate.
	const FVector MoveLoc(-9000.f, 0.f, 0.f);
	const FVector JumpLoc(-8200.f, 0.f, 0.f);

	PromptMove = World->SpawnActor<AT66TutorialPromptActor>(AT66TutorialPromptActor::StaticClass(), MoveLoc, FRotator::ZeroRotator, P);
	PromptJump = World->SpawnActor<AT66TutorialPromptActor>(AT66TutorialPromptActor::StaticClass(), JumpLoc, FRotator::ZeroRotator, P);

	if (PromptMove)
	{
		PromptMove->PromptText = NSLOCTEXT("T66.Tutorial", "MovePrompt", "WASD to move");
		PromptMove->SetActive(true);
		PromptMove->OnPassed.AddDynamic(this, &AT66TutorialManager::HandleMovePassed);
	}
	if (PromptJump)
	{
		PromptJump->PromptText = NSLOCTEXT("T66.Tutorial", "JumpPrompt", "Space bar to jump");
		PromptJump->SetActive(false);
	}
}

void AT66TutorialManager::HandleMovePassed()
{
	if (PromptJump)
	{
		PromptJump->SetActive(true);
	}
}

