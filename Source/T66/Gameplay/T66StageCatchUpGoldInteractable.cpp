// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"

AT66StageCatchUpGoldInteractable::AT66StageCatchUpGoldInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
	Rarity = ET66Rarity::Yellow;

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(140.f, 140.f, 140.f));
	}
	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
		VisualMesh->SetRelativeScale3D(FVector(1.6f, 1.6f, 1.6f));
		if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
		{
			VisualMesh->SetStaticMesh(Cube);
		}
	}
	ApplyRarityVisuals();
}

void AT66StageCatchUpGoldInteractable::ApplyRarityVisuals()
{
	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.95f, 0.80f, 0.15f, 1.f));
}

bool AT66StageCatchUpGoldInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	RunState->AddGold(GoldAmount);
	bConsumed = true;
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
	return true;
}
