// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "UI/T66ScreenBase.h"
#include "T66DailyClimbScreen.generated.h"

struct FSlateBrush;
class UTexture2D;

/**
 * Full-screen Daily Climb frontend.
 * Mirrors the main menu shell while presenting a Daily-only rules panel and action flow.
 */
UCLASS(Blueprintable)
class T66_API UT66DailyClimbScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66DailyClimbScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
	FReply HandleContinueClicked();
	FReply HandleStartClicked();
	void HandleDailyClimbStatusReady(const FString& RequestTag);
	void RefreshContinueAvailability();
	int32 ComputeSeedQualityPreview(int32 RunSeed) const;
	void RequestBackgroundTexture();

	TSharedPtr<FSlateBrush> SkyBackgroundBrush;
	TStrongObjectPtr<UTexture2D> SkyBackgroundTexture;
	TSharedPtr<FSlateBrush> FireMoonBrush;
	TStrongObjectPtr<UTexture2D> FireMoonTexture;
	TSharedPtr<FSlateBrush> PyramidChadBrush;
	TStrongObjectPtr<UTexture2D> PyramidChadTexture;
	TSharedPtr<FSlateBrush> PrimaryCTAFillBrush;
	TStrongObjectPtr<UTexture2D> PrimaryCTAFillTexture;

	bool bStartRequestInFlight = false;
	int32 ContinueSaveSlotIndex = INDEX_NONE;
};
