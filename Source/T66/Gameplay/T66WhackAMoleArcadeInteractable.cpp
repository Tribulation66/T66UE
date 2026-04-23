// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WhackAMoleArcadeInteractable.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UI/T66WhackAMoleArcadeWidget.h"

AT66WhackAMoleArcadeInteractable::AT66WhackAMoleArcadeInteractable()
{
	ArcadeData.ArcadeID = FName(TEXT("Arcade_WhackAMole"));
	ArcadeData.DisplayName = NSLOCTEXT("T66.Arcade", "WhackAMoleDisplayName", "Whack-a-Mole");
	ArcadeData.InteractionVerb = NSLOCTEXT("T66.Arcade", "WhackAMoleInteractVerb", "play Whack-a-Mole");
	ArcadeData.ArcadeClass = ET66ArcadeInteractableClass::PopupMinigame;
	ArcadeData.PopupGameType = ET66ArcadePopupGameType::WhackAMole;
	ArcadeData.PopupWidgetClass = UT66WhackAMoleArcadeWidget::StaticClass();
	ArcadeData.bConsumeOnSuccess = true;
	ArcadeData.bConsumeOnFailure = false;
	ArcadeData.DisplayMeshScale = FVector(1.3f, 1.3f, 1.0f);
	ArcadeData.DisplayMeshOffset = FVector(0.f, 0.f, 60.f);
	ArcadeData.Tint = FLinearColor(0.24f, 0.84f, 0.42f, 1.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::PopupRoundSeconds, 12.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::PopupSpawnInterval, 0.65f);
	ArcadeData.Modifiers.IntValues.Add(T66ArcadeModifierKeys::PopupTargetHits, 10);
	ArcadeData.Modifiers.IntValues.Add(T66ArcadeModifierKeys::PopupSuccessGold, 150);
	ArcadeRowID = ArcadeData.ArcadeID;

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(300.f, 300.f, 220.f));
	}

	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(ArcadeData.DisplayMeshOffset);
	}

	ApplyRarityVisuals();
}
