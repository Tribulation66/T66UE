// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/Animation/T66AnimationSequence.h"
#include "Input/Reply.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66StickPickGameWidget.generated.h"

class SBox;
class SImage;
class STextBlock;

UCLASS(Blueprintable)
class T66_API UT66StickPickGameWidget : public UUserWidget, public IT66WidgetGameSession
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

	void SetChoiceCallback(TFunction<void(int32)> InChoiceCallback);
	void SetReturnCallback(TFunction<void()> InReturnCallback);
	void SetRevealCompleteCallback(TFunction<void()> InRevealCompleteCallback);

	void ResetForOpen();
	void NotifyRoundArmed();
	bool IsRevealSequencePending() const { return bSequenceActive; }
	void SetStatus(const FText& Message, const FLinearColor& Color);
	void SetWagerAmount(int32 WagerAmount);
	void SetTargetShortest(bool bShortest);
	void RevealResult(int32 ChosenStick, int32 TargetStick, bool bTargetShortest, int32 PayoutGold);

private:
	static constexpr int32 StickCount = 5;

	struct FStickVisual
	{
		TSharedPtr<SBox> Box;          // Sized to the stick's revealed length.
		TSharedPtr<SWidget> Body;      // Transform target (cap + shaft).
		TSharedPtr<SImage> CapImage;
		TSharedPtr<SImage> ShaftImage;
		float LengthPx = 150.f;
		float Translation = 0.f;       // Y offset; positive sits deeper in the cauldron.
		float RestTranslation = 0.f;
		float PullAlpha = 0.f;         // 0 = hidden in cauldron, 1 = fully drawn.
	};

	FReply OnBackClicked();
	FReply OnStickClicked(int32 StickIndex);

	void BuildIntroSequence();
	void BuildRevealSequence();
	void HandleSequenceMarkers(const TArray<FT66AnimationMarkerEvent>& MarkerEvents);
	void AssignRevealLengths();
	void ApplyStageTransforms();
	void EnsureStageTimer();
	void ClearStageTimer();
	EActiveTimerReturnType HandleStageTick(double CurrentTime, float DeltaTime);
	static float StickSlotX(int32 StickIndex);

	FT66WidgetGameHostContext WidgetGameHostContext;
	TFunction<void(int32)> ChoiceCallback;
	TFunction<void()> ReturnCallback;
	TFunction<void()> RevealCompleteCallback;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> WagerText;
	TSharedPtr<STextBlock> TargetText;
	TSharedPtr<STextBlock> BannerText;
	TSharedPtr<SWidget> StageRoot;
	TSharedPtr<SBox> GlowBox;
	TSharedPtr<SWidget> CauldronWidget;
	FStickVisual Sticks[StickCount];

	TSharedPtr<FActiveTimerHandle> StageTimerHandle;

	FT66AnimationSequence ActiveSequence;
	bool bSequenceActive = false;
	bool bChoiceEnabled = false;
	bool bRoundArmed = false;

	float CauldronTranslateY = 0.f;
	float GlowTranslateX = 0.f;
	float GlowOpacity = 0.f;
	float GlowScale = 1.f;
	float BannerOpacity = 0.f;
	float BannerScale = 1.f;

	bool bPendingTargetShortest = false;
	int32 RevealChosenStick = INDEX_NONE;
	int32 RevealTargetStick = INDEX_NONE;
	bool bRevealTargetShortest = false;
	int32 RevealPayoutGold = 0;
};
