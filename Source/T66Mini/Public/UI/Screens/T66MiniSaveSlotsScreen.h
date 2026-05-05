// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniSaveSlotsScreen.generated.h"

class STextBlock;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniSaveSlotsScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniSaveSlotsScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
	FReply HandleLoadSlotClicked(int32 SlotIndex);
	void SetStatus(const FText& InText);

	TSharedPtr<STextBlock> StatusTextBlock;
};
