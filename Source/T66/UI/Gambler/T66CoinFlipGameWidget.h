// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "Styling/SlateBrush.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66CoinFlipGameWidget.generated.h"

class SImage;
class STextBlock;

UCLASS(Blueprintable)
class T66_API UT66CoinFlipGameWidget : public UUserWidget, public IT66WidgetGameSession
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	virtual void ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext) override;
	virtual void DeactivateWidgetGame() override;
	virtual void PauseWidgetGame() override {}
	virtual void ResumeWidgetGame() override {}
	virtual void RequestWidgetGameExit() override;

	void SetChoiceCallback(TFunction<void(bool)> InChoiceCallback);
	void SetReturnCallback(TFunction<void()> InReturnCallback);

	void ResetForOpen();
	void SetStatus(const FText& Message, const FLinearColor& Color);
	void SetWagerAmount(int32 WagerAmount);
	void SetResultText(const FText& ResultText);
	void StartSpin(bool bResultHeads, float DurationSeconds);

private:
	enum class ECoinFace : uint8
	{
		Heads,
		Tails,
		Side,
	};

	FReply OnBackClicked();
	FReply OnHeadsClicked();
	FReply OnTailsClicked();

	void TickCoinSpin();
	void FinishCoinSpin();
	void SetCoinFace(ECoinFace Face);

	FT66WidgetGameHostContext WidgetGameHostContext;
	TFunction<void(bool)> ChoiceCallback;
	TFunction<void()> ReturnCallback;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> ResultText;
	TSharedPtr<STextBlock> WagerText;
	TSharedPtr<SImage> CoinImage;

	FSlateBrush CoinBrushHeads;
	FSlateBrush CoinBrushTails;
	FSlateBrush CoinBrushSide;
	FTimerHandle CoinSpinTimerHandle;
	float CoinSpinElapsed = 0.f;
	float CoinSpinLastTickTimeSeconds = 0.f;
	float CoinSpinDuration = 2.f;
	bool bCoinSpinActive = false;
	bool bCoinSpinResultHeads = false;
};
