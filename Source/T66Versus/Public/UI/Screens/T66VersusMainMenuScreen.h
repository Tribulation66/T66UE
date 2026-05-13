// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "UI/Style/T66Style.h"
#include "T66VersusMainMenuScreen.generated.h"

UCLASS(Blueprintable)
class T66VERSUS_API UT66VersusMainMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66VersusMainMenuScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleHostClicked();
	FReply HandleJoinClicked();
	FReply HandlePracticeClicked();
	FReply HandleBackClicked();
	void HandleSessionStateChanged();
	FText GetStatusText() const;
	TSharedRef<SWidget> MakeVersusPanel(const TSharedRef<SWidget>& Content, const FMargin& ContentPadding, const FLinearColor& Accent, FName Tag = NAME_None) const;
	TSharedRef<SWidget> MakeVersusButton(const FText& Text, const FOnClicked& Handler, ET66ButtonType Type, bool bEnabled = true) const;
	TSharedRef<SWidget> MakeInfoRow(const FText& Label, const FText& Body) const;

	FText StatusText;
	FDelegateHandle SessionStateChangedHandle;
};
