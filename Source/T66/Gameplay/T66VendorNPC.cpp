// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorNPC.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"

AT66VendorNPC::AT66VendorNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	NPCID = FName(TEXT("Vendor"));
	NPCName = FText::FromString(TEXT("Vendor"));
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
	return TrySellFirstItem();
}
