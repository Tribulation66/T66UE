// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66OuroborosNPC.h"
#include "Core/T66RunStateSubsystem.h"

AT66OuroborosNPC::AT66OuroborosNPC()
{
	NPCID = FName(TEXT("Ouroboros"));
	NPCName = NSLOCTEXT("T66.NPC", "Ouroboros", "Ouroboros");
	NPCColor = FLinearColor(0.1f, 0.8f, 0.2f, 1.f);
}

void AT66OuroborosNPC::BeginPlay()
{
	// Force this for already-placed instances that may have saved the old default.
	NPCID = FName(TEXT("Ouroboros"));
	Super::BeginPlay();
}

bool AT66OuroborosNPC::Interact(APlayerController* PC)
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;
	RunState->KillPlayer();
	return true;
}

