// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ArcadePopupWidget.h"

#include "Gameplay/T66ArcadeInteractableBase.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/WidgetGames/T66WidgetGameRegistry.h"
#include "UI/WidgetGames/T66WidgetGameResult.h"

void UT66ArcadePopupWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
}

void UT66ArcadePopupWidget::DeactivateWidgetGame()
{
	WidgetGameHostContext = FT66WidgetGameHostContext();
}

void UT66ArcadePopupWidget::PauseWidgetGame()
{
}

void UT66ArcadePopupWidget::ResumeWidgetGame()
{
}

void UT66ArcadePopupWidget::RequestWidgetGameExit()
{
}

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

	if (ReportsArcadeResult())
	{
		FT66WidgetGameResult Result;
		Result.GameID = ArcadeData.ArcadeID.IsNone()
			? T66WidgetGames::Registry::GetArcadeRowID(ArcadeData.ArcadeGameType)
			: ArcadeData.ArcadeID;
		Result.ExitReason = bSucceeded ? ET66WidgetGameExitReason::Completed : ET66WidgetGameExitReason::PlayerCancelled;
		Result.FinalScore = FinalScore;
		Result.bHasFinalScore = true;
		Result.bSuccessful = bSucceeded;
		Result.ResultID = Result.GameID;
		WidgetGameHostContext.ReportResult(Result);
	}

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
