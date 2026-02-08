// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66LobbyReadyCheckModal.generated.h"

/**
 * Modal: "Ready to start?" with Ready (green) and Not Ready (red).
 * Shown from Lobby when host presses Ready Check.
 */
UCLASS(Blueprintable)
class T66_API UT66LobbyReadyCheckModal : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66LobbyReadyCheckModal(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "LobbyReadyCheck")
	void OnReadyClicked();

	UFUNCTION(BlueprintCallable, Category = "LobbyReadyCheck")
	void OnNotReadyClicked();

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleReadyClicked();
	FReply HandleNotReadyClicked();
};
