// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "UObject/StrongObjectPtr.h"
#include "T66TDMainMenuScreen.generated.h"

class UTexture2D;

UCLASS(Blueprintable)
class T66TD_API UT66TDMainMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66TDMainMenuScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackToMainMenuClicked();
	FReply HandleNewGameClicked();
	FReply HandleMapBrowserClicked();
	void RequestMenuTextures();
	void ReleaseRetainedSlateState();

	TSharedPtr<FSlateBrush> BackdropBrush;
	TStrongObjectPtr<UTexture2D> BackdropTexture;
	TSharedPtr<FSlateBrush> ForegroundBrush;
	TStrongObjectPtr<UTexture2D> ForegroundTexture;
	TSharedPtr<FSlateBrush> PrimaryCTAFillBrush;
	TStrongObjectPtr<UTexture2D> PrimaryCTAFillTexture;
};
