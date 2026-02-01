// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TricksterNPC.h"

AT66TricksterNPC::AT66TricksterNPC()
{
	// Use DT_CharacterVisuals (and optionally DT_HouseNPCs later) to drive the imported mesh.
	NPCID = FName(TEXT("Trickster"));
	NPCName = NSLOCTEXT("T66.NPC", "Trickster", "Trickster");
	NPCColor = FLinearColor(0.55f, 0.2f, 0.9f, 1.f);
	SafeZoneRadius = 650.f;
}

void AT66TricksterNPC::BeginPlay()
{
	// Force this for already-placed instances that may have saved the old default (None).
	NPCID = FName(TEXT("Trickster"));
	Super::BeginPlay();
}

bool AT66TricksterNPC::Interact(APlayerController* PC)
{
	// Trickster itself doesn't do anything in v0; Cowardice Gate handles interaction.
	return false;
}

