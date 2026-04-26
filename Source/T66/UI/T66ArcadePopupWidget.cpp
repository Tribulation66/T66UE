// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ArcadePopupWidget.h"

#include "Gameplay/T66ArcadeInteractableBase.h"
#include "Gameplay/T66PlayerController.h"

void UT66ArcadePopupWidget::InitializeArcadePopup(
	const FT66ArcadeInteractableData& InArcadeData,
	AT66ArcadeInteractableBase* InSourceInteractable)
{
	ArcadeData = InArcadeData;
	SourceInteractable = InSourceInteractable;
}

void UT66ArcadePopupWidget::StartCloseSequence(const bool bSucceeded, const int32 FinalScore)
{
	if (bCloseSequenceStarted)
	{
		return;
	}

	bCloseSequenceStarted = true;

	if (AT66PlayerController* T66PC = GetOwningPlayer<AT66PlayerController>())
	{
		T66PC->HandleArcadePopupResult(this, bSucceeded, FinalScore);
		return;
	}

	if (AT66ArcadeInteractableBase* SourceInteractablePtr = SourceInteractable.Get())
	{
		SourceInteractablePtr->HandleArcadePopupClosed(bSucceeded, FinalScore);
	}

	RemoveFromParent();
}
