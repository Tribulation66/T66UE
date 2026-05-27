// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66ArcadePopupWidget.generated.h"

class AT66ArcadeInteractableBase;

UCLASS(Abstract, Blueprintable)
class T66_API UT66ArcadePopupWidget : public UUserWidget, public IT66WidgetGameSession
{
	GENERATED_BODY()

public:
	void InitializeArcadePopup(const FT66ArcadeInteractableData& InArcadeData, AT66ArcadeInteractableBase* InSourceInteractable);
	virtual void ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext) override;
	virtual void DeactivateWidgetGame() override;
	virtual void PauseWidgetGame() override;
	virtual void ResumeWidgetGame() override;
	virtual void RequestWidgetGameExit() override;

	const FT66ArcadeInteractableData& GetArcadeData() const { return ArcadeData; }
	AT66ArcadeInteractableBase* GetSourceInteractable() const { return SourceInteractable.Get(); }
	virtual bool ReportsArcadeResult() const { return true; }

protected:
	void StartCloseSequence(bool bSucceeded, int32 FinalScore = 0);

	UPROPERTY(Transient)
	FT66ArcadeInteractableData ArcadeData;

	TWeakObjectPtr<AT66ArcadeInteractableBase> SourceInteractable;

private:
	FT66WidgetGameHostContext WidgetGameHostContext;
	bool bCloseSequenceStarted = false;
};
