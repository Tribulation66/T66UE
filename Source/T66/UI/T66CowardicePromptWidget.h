// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "T66CowardicePromptWidget.generated.h"

class STextBlock;
class AT66CowardiceGate;

/** Small yes/no prompt for Cowardice Gate. Transparent background (world visible). */
UCLASS(Blueprintable)
class T66_API UT66CowardicePromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	void SetGate(AT66CowardiceGate* InGate);

private:
	TWeakObjectPtr<AT66CowardiceGate> Gate;
	TSharedPtr<STextBlock> StatusText;

	FReply OnYes();
	FReply OnNo();

	void ClosePrompt();
};

