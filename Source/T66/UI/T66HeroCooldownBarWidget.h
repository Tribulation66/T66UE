// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66HeroCooldownBarWidget.generated.h"

class SBox;
class STextBlock;

/** Cooldown bar (fills 0->1) plus range units text, shown below the hero's feet. */
UCLASS(Blueprintable)
class T66_API UT66HeroCooldownBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set cooldown progress 0 = just fired, 1 = ready. Bar fills left to right. */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void SetProgress(float Pct);

	/** Set the attack range in units (shown below the bar). */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void SetRangeUnits(int32 Units);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	static constexpr float BarWidth = 100.f;
	static constexpr float BarHeight = 8.f;

	TSharedPtr<SBox> FillWidthBox;
	TSharedPtr<STextBlock> RangeTextBlock;
	float Pct = 0.f;
	int32 RangeUnits = 0;
};
