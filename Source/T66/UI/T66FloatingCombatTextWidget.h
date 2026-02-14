// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66FloatingCombatTextWidget.generated.h"

class STextBlock;

/** Single-line floating text for damage numbers (sides) and status events (above head). Different fonts per type. */
UCLASS(Blueprintable)
class T66_API UT66FloatingCombatTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set content to a damage number. Optional EventType for style (e.g. Crit = larger, different color). */
	UFUNCTION(BlueprintCallable, Category = "FloatingCombatText")
	void SetDamageNumber(int32 Amount, FName EventType = NAME_None);

	/** Set content to a status/event label (e.g. "CRIT!", "DoT"). Uses event-specific font and color. */
	UFUNCTION(BlueprintCallable, Category = "FloatingCombatText")
	void SetStatusEvent(FName EventType);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void UpdateTextBlock(const FText& InText, int32 FontSize, const FLinearColor& Color);

	TSharedPtr<class STextBlock> TextBlock;
	FText CachedText;
	int32 CachedFontSize = 18;
	FLinearColor CachedColor = FLinearColor(1.f, 0.95f, 0.7f, 1.f);
};
