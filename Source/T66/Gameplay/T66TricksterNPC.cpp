// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TricksterNPC.h"

AT66TricksterNPC::AT66TricksterNPC()
{
	NPCID = NAME_None; // no DT row required for now
	NPCName = FText::FromString(TEXT("Trickster"));
	NPCColor = FLinearColor(0.55f, 0.2f, 0.9f, 1.f);
	SafeZoneRadius = 650.f;
}

bool AT66TricksterNPC::Interact(APlayerController* PC)
{
	// Trickster itself doesn't do anything in v0; Cowardice Gate handles interaction.
	return false;
}

