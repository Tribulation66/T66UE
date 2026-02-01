// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66PlayerController.h"
#include "Data/T66DataTypes.h"

AT66GamblerNPC::AT66GamblerNPC()
{
	NPCID = FName(TEXT("Gambler"));
	NPCName = NSLOCTEXT("T66.NPC", "Gambler", "Gambler");
	NPCColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.f);
}

void AT66GamblerNPC::ApplyNPCData(const FHouseNPCData& Data)
{
	Super::ApplyNPCData(Data);
	WinGoldAmount = Data.GamblerWinGold;
}

bool AT66GamblerNPC::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;
	T66PC->OpenWorldDialogueGambler(this);
	return true;
}

