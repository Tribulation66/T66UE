// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66LobbyBackConfirmModal.generated.h"

/**
 * Modal: "Are you sure you want to leave the lobby?" with Stay (green) and Leave (red).
 */
UCLASS(Blueprintable)
class T66_API UT66LobbyBackConfirmModal : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66LobbyBackConfirmModal(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "LobbyBackConfirm")
	void OnStayClicked();

	UFUNCTION(BlueprintCallable, Category = "LobbyBackConfirm")
	void OnLeaveClicked();

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleStayClicked();
	FReply HandleLeaveClicked();
};
