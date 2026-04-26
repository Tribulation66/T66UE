// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArcadeMachineInteractable.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AT66ArcadeMachineInteractable::AT66ArcadeMachineInteractable()
{
	ArcadeData.ArcadeID = FName(TEXT("Arcade_Machine"));
	ArcadeData.DisplayName = NSLOCTEXT("T66.Arcade", "MachineDisplayName", "Arcade Cabinet");
	ArcadeData.InteractionVerb = NSLOCTEXT("T66.Arcade", "MachineInteractVerb", "play arcade");
	ArcadeData.ArcadeClass = ET66ArcadeInteractableClass::PopupArcade;
	ArcadeData.ArcadeGameType = ET66ArcadeGameType::Random;
	ArcadeData.RandomGameTypes = { ET66ArcadeGameType::WhackAMole, ET66ArcadeGameType::Topwar };
	ArcadeData.bConsumeOnSuccess = true;
	ArcadeData.bConsumeOnFailure = true;
	ArcadeData.DisplayMeshScale = FVector(1.1f, 0.75f, 1.85f);
	ArcadeData.DisplayMeshOffset = FVector(0.f, 0.f, 95.f);
	ArcadeData.Tint = FLinearColor(0.14f, 0.36f, 0.92f, 1.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::ArcadeRoundSeconds, 10.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::ArcadeStartInterval, 0.70f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::ArcadeEndInterval, 0.24f);
	ArcadeRowID = ArcadeData.ArcadeID;

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(320.f, 320.f, 260.f));
	}

	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(ArcadeData.DisplayMeshOffset);
	}

	ApplyRarityVisuals();
}
