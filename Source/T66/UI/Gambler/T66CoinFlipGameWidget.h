// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/Animation/T66AnimationSequence.h"
#include "Input/Reply.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66CoinFlipGameWidget.generated.h"

class SBox;
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
	void SetRevealCompleteCallback(TFunction<void()> InRevealCompleteCallback);

	void ResetForOpen();
	void NotifyRoundArmed();
	bool IsRevealSequencePending() const { return bSequenceActive; }
	void SetStatus(const FText& Message, const FLinearColor& Color);
	void SetWagerAmount(int32 WagerAmount);

	/** Plays the full physical coin toss; result presentation lands when the coin settles. */
	void StartSpin(bool bResultHeads, bool bWin, int32 PayoutGold);

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

	void BuildSpinSequence();
	void HandleSequenceMarkers(const TArray<FT66AnimationMarkerEvent>& MarkerEvents);
	void SetCoinFace(ECoinFace Face);
	void ApplyStageTransforms();
	void EnsureStageTimer();
	void ClearStageTimer();
	EActiveTimerReturnType HandleStageTick(double CurrentTime, float DeltaTime);

	FT66WidgetGameHostContext WidgetGameHostContext;
	TFunction<void(bool)> ChoiceCallback;
	TFunction<void()> ReturnCallback;
	TFunction<void()> RevealCompleteCallback;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> WagerText;
	TSharedPtr<STextBlock> BannerText;
	TSharedPtr<SWidget> StageRoot;
	TSharedPtr<SBox> CoinBox;
	TSharedPtr<SImage> CoinImage;
	TSharedPtr<SBox> ShadowBox;
	TSharedPtr<SBox> GlowBox;
	TSharedPtr<SBox> BurstBox;

	TSharedPtr<FActiveTimerHandle> StageTimerHandle;

	FT66AnimationSequence SpinSequence;
	bool bSequenceActive = false;
	bool bRoundArmed = false;

	// Animated state, applied every stage tick.
	float CoinTranslateY = 0.f;
	float CoinScaleX = 1.f;
	float CoinScaleY = 1.f;
	float CoinAngleDeg = 0.f;
	float ShadowScale = 1.f;
	float ShadowOpacity = 0.85f;
	float GlowOpacity = 0.f;
	float GlowScale = 1.f;
	float BurstOpacity = 0.f;
	float BurstScale = 1.f;
	float BannerOpacity = 0.f;
	float BannerScale = 1.f;
	float IdleTimeSeconds = 0.f;
	ECoinFace CurrentFace = ECoinFace::Heads;

	bool bSpinResultHeads = false;
	bool bSpinWin = false;
	int32 SpinPayoutGold = 0;
};
