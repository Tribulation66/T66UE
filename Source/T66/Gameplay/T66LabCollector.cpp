// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LabCollector.h"
#include "Core/T66GameInstance.h"
#include "Gameplay/T66PlayerController.h"

AT66LabCollector::AT66LabCollector()
{
	NPCID = FName(TEXT("Collector"));
	NPCName = NSLOCTEXT("T66.Lab", "CollectorName", "The Collector");
	NPCColor = FLinearColor(0.45f, 0.25f, 0.65f, 1.f);  // Purple-ish
}

bool AT66LabCollector::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC) return false;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(T66PC->GetGameInstance()); T66GI && !T66GI->IsCollectorPlayable())
	{
		return false;
	}
	T66PC->OpenCollectorOverlay();
	return true;
}
