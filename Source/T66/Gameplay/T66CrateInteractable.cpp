// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/SoftObjectPath.h"

AT66CrateInteractable::AT66CrateInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Crate.Crate")));
	ApplyRarityVisuals();
}

void AT66CrateInteractable::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh()) return;

	if (!VisualMesh) return;
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.45f, 0.30f, 0.15f, 1.f));
}

bool AT66CrateInteractable::Interact(APlayerController* PC)
{
	if (!PC || bConsumed) return false;

	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
	{
		T66PC->StartCrateOpenHUD();
	}

	bConsumed = true;
	Destroy();
	return true;
}
