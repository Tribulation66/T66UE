// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66EnemyHealthBarWidget.generated.h"

class SBox;

/** Minimal health bar for enemies (WidgetComponent). */
UCLASS(Blueprintable)
class T66_API UT66EnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthPercent(float InPct);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetLocked(bool bInLocked);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	static constexpr float BarWidth = 90.f;
	static constexpr float BarHeight = 10.f;

	TSharedPtr<SBox> FillWidthBox;
	TSharedPtr<SBox> LockRingBox;
	TSharedPtr<SWidget> RootSlate;

	float Pct = 1.f;
	bool bLocked = false;
};

