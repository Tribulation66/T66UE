// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66QuitConfirmationModal.generated.h"

/**
 * Quit Confirmation Modal
 */
UCLASS(Blueprintable)
class T66_API UT66QuitConfirmationModal : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66QuitConfirmationModal(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Quit")
	void OnStayClicked();

	UFUNCTION(BlueprintCallable, Category = "Quit")
	void OnQuitClicked();

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleStayClicked();
	FReply HandleQuitClicked();
};
