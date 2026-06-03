// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66FindJokerGameWidget.generated.h"

class SBorder;
class STextBlock;

UCLASS(Blueprintable)
class T66_API UT66FindJokerGameWidget : public UUserWidget, public IT66WidgetGameSession
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext) override;
	virtual void DeactivateWidgetGame() override;
	virtual void PauseWidgetGame() override {}
	virtual void ResumeWidgetGame() override {}
	virtual void RequestWidgetGameExit() override;

	void SetChoiceCallback(TFunction<void(int32)> InChoiceCallback);
	void SetReturnCallback(TFunction<void()> InReturnCallback);

	void ResetForOpen();
	void SetStatus(const FText& Message, const FLinearColor& Color);
	void SetWagerAmount(int32 WagerAmount);
	void RevealResult(int32 ChosenCard, int32 JokerCard, int32 PayoutGold);

private:
	FReply OnBackClicked();
	FReply OnCardClicked(int32 CardIndex);

	FT66WidgetGameHostContext WidgetGameHostContext;
	TFunction<void(int32)> ChoiceCallback;
	TFunction<void()> ReturnCallback;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> WagerText;
	TSharedPtr<STextBlock> ResultText;
	TArray<TSharedPtr<SBorder>> CardBorders;
};
