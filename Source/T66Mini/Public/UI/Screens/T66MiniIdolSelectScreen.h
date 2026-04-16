// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniIdolSelectScreen.generated.h"

class STextBlock;
struct FT66MiniIdolDefinition;
struct FSlateBrush;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniIdolSelectScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniIdolSelectScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
	FReply HandleRerollClicked();
	FReply HandleTakeIdolClicked(FName IdolID);
	FReply HandleContinueClicked();
	void HandleSessionStateChanged();
	void SyncToSharedPartyScreen();
	void SetStatus(const FText& InText);
	void RebuildIdolBrushes(const TArray<FT66MiniIdolDefinition>& Idols);
	const FSlateBrush* FindIdolBrush(FName IdolID) const;

	TSharedPtr<STextBlock> StatusTextBlock;
	TMap<FName, TSharedPtr<FSlateBrush>> IdolBrushes;
	FText CurrentStatusText;
	FDelegateHandle SessionStateChangedHandle;
};
