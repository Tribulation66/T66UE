// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/T66Rarity.h"
#include "Input/Reply.h"
#include "T66WheelOverlayWidget.generated.h"

class STextBlock;
class SBorder;
class SButton;

/** Simple non-pausing wheel popup: press Spin to roll a gold payout. */
UCLASS(Blueprintable)
class T66_API UT66WheelOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetWheelRarity(ET66Rarity InRarity) { WheelRarity = InRarity; }

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

private:
	ET66Rarity WheelRarity = ET66Rarity::Black;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SBorder> WheelDisk;
	TSharedPtr<SButton> SpinButton;
	TSharedPtr<SButton> BackButton;

	FTimerHandle SpinTickHandle;
	FTimerHandle ResolveHandle;
	FTimerHandle CloseHandle;

	bool bSpinning = false;
	float SpinElapsed = 0.f;
	float SpinDuration = 2.0f;
	float StartAngleDeg = 0.f;
	float TotalAngleDeg = 0.f;
	float LastSpinTickTimeSeconds = 0.f;
	int32 PendingGold = 0;

	void TickSpin();
	void ResolveSpin();
	void CloseAfterResolve();

	FReply OnSpin();
	FReply OnBack();
};

