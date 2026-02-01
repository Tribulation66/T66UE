// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorNPC.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66PlayerController.h"

AT66VendorNPC::AT66VendorNPC()
{
	NPCID = FName(TEXT("Vendor"));
	NPCName = NSLOCTEXT("T66.NPC", "Vendor", "Vendor");
	NPCColor = FLinearColor(0.05f, 0.05f, 0.05f, 1.f);
}

bool AT66VendorNPC::TrySellFirstItem()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;
	return RunState->SellFirstItem();
}

bool AT66VendorNPC::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;
	T66PC->OpenWorldDialogueVendor(this);
	return true;
}
