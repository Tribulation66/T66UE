// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"
#include "T66ArcadePopupWidget.generated.h"

class AT66ArcadeInteractableBase;

UCLASS(Abstract, Blueprintable)
class T66_API UT66ArcadePopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeArcadePopup(const FT66ArcadeInteractableData& InArcadeData, AT66ArcadeInteractableBase* InSourceInteractable);

	const FT66ArcadeInteractableData& GetArcadeData() const { return ArcadeData; }
	AT66ArcadeInteractableBase* GetSourceInteractable() const { return SourceInteractable.Get(); }
	virtual bool ReportsArcadeResult() const { return true; }

protected:
	void StartCloseSequence(bool bSucceeded, int32 FinalScore = 0);

	UPROPERTY(Transient)
	FT66ArcadeInteractableData ArcadeData;

	TWeakObjectPtr<AT66ArcadeInteractableBase> SourceInteractable;

private:
	bool bCloseSequenceStarted = false;
};
