// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SaintNPC.h"
#include "Gameplay/T66PlayerController.h"

AT66SaintNPC::AT66SaintNPC()
{
	NPCID = FName(TEXT("Saint"));
	NPCName = NSLOCTEXT("T66.NPC", "Saint", "Saint");
	NPCColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.f);
}

bool AT66SaintNPC::Interact(APlayerController* PC)
{
	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
	{
		T66PC->OpenWorldDialogueSaint(this);
		return true;
	}
	return false;
}

