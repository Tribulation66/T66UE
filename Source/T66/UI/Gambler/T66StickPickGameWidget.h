// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66StickPickGameWidget.generated.h"

class SBorder;
class STextBlock;

UCLASS(Blueprintable)
class T66_API UT66StickPickGameWidget : public UUserWidget, public IT66WidgetGameSession
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
	void SetTargetShortest(bool bShortest);
	void RevealResult(int32 ChosenStick, int32 TargetStick, bool bTargetShortest, int32 PayoutGold);

private:
	FReply OnBackClicked();
	FReply OnStickClicked(int32 StickIndex);

	FT66WidgetGameHostContext WidgetGameHostContext;
	TFunction<void(int32)> ChoiceCallback;
	TFunction<void()> ReturnCallback;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> WagerText;
	TSharedPtr<STextBlock> TargetText;
	TSharedPtr<STextBlock> ResultText;
	TArray<TSharedPtr<SBorder>> StickBorders;
};
