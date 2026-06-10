// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/Animation/T66AnimationSequence.h"
#include "Input/Reply.h"
#include "UI/WidgetGames/T66WidgetGameSession.h"
#include "T66GuessCupGameWidget.generated.h"

class SBox;
class SImage;
class STextBlock;

UCLASS(Blueprintable)
class T66_API UT66GuessCupGameWidget : public UUserWidget, public IT66WidgetGameSession
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
	void RevealResult(int32 ChosenCup, int32 WinningCup, int32 PayoutGold);

private:
	static constexpr int32 CupCount = 3;

	struct FCupVisual
	{
		TSharedPtr<SBox> Box;
		TSharedPtr<SImage> Image;
		FVector2D Translation = FVector2D::ZeroVector;
		FVector2D Scale = FVector2D(1.f, 1.f);
		float AngleDeg = 0.f;
		float HoverLift = 0.f;
		int32 Slot = 0;
	};

	FReply OnBackClicked();
	FReply OnCupClicked(int32 SlotIndex);

	void BuildShuffleSequence();
	void BuildRevealSequence();
	void HandleSequenceMarkers(const TArray<FT66AnimationMarkerEvent>& MarkerEvents);
	void SnapCupsToSlots();
	void ApplyStageTransforms();
	void EnsureStageTimer();
	void ClearStageTimer();
	EActiveTimerReturnType HandleStageTick(double CurrentTime, float DeltaTime);
	static float SlotX(int32 SlotIndex);

	FT66WidgetGameHostContext WidgetGameHostContext;
	TFunction<void(int32)> ChoiceCallback;
	TFunction<void()> ReturnCallback;
	TFunction<void()> RevealCompleteCallback;

	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<STextBlock> WagerText;
	TSharedPtr<STextBlock> PromptText;
	TSharedPtr<STextBlock> BannerText;
	TSharedPtr<SWidget> StageRoot;
	TSharedPtr<SBox> TokenBox;
	TSharedPtr<SBox> GlowBox;
	FCupVisual Cups[CupCount];
	TSharedPtr<SWidget> HitZones[CupCount];

	TSharedPtr<FActiveTimerHandle> StageTimerHandle;

	FT66AnimationSequence ActiveSequence;
	bool bSequenceActive = false;
	bool bChoiceEnabled = false;
	bool bRoundArmed = false;

	float TokenTranslateX = 0.f;
	float TokenTranslateY = 0.f;
	float TokenOpacity = 0.f;
	float GlowTranslateX = 0.f;
	float GlowOpacity = 0.f;
	float GlowScale = 1.f;
	float BannerOpacity = 0.f;
	float BannerScale = 1.f;
	float PromptPulseTime = 0.f;

	int32 RevealChosenCup = INDEX_NONE;
	int32 RevealWinningCup = INDEX_NONE;
	int32 RevealPayoutGold = 0;
};
