// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SaintNPC.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"

AT66SaintNPC::AT66SaintNPC()
{
	NPCID = FName(TEXT("Saint"));
	NPCName = NSLOCTEXT("T66.NPC", "Saint", "Saint");
	NPCColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.f);
}

bool AT66SaintNPC::Interact(APlayerController* PC)
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;
	RunState->HealToFull();
	return true;
}

