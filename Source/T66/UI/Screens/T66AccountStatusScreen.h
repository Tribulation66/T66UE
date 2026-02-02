// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66AccountStatusScreen.generated.h"

class SMultiLineEditableTextBox;
class SEditableTextBox;

/**
 * Account Status Panel (Suspension / Appeal)
 * Modal overlay opened from the Main Menu when the account has a flagged run.
 *
 * Bible: T66_Bible.md -> "1.4 ACCOUNT STATUS PANEL (SUSPENSION / APPEAL)"
 */
UCLASS(Blueprintable)
class T66_API UT66AccountStatusScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	// UI elements (draft state)
	TSharedPtr<SMultiLineEditableTextBox> AppealTextBox;
	TSharedPtr<SEditableTextBox> EvidenceUrlTextBox;

	FString DraftAppealMessage;
	FString DraftEvidenceUrl;

	// Button handlers
	FReply HandleBackClicked();
	FReply HandleRunInQuestionClicked();
	FReply HandleSubmitAppealClicked();
};

