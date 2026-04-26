// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "UI/T66ArcadePopupWidget.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "T66TopwarArcadeWidget.generated.h"

class SBorder;
class STextBlock;
class UTexture2D;

UCLASS(Blueprintable)
class T66_API UT66TopwarArcadeWidget : public UT66ArcadePopupWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void StartRound();
	void ClearActiveTimers();
	void ScheduleNextGate();
	void HandleRoundTick();
	void HandleGateTick();
	void HandleAutoClose();
	void CompleteRound();
	void RollGateChoices();
	void RefreshHud();
	void RefreshGateVisuals();
	float ResolveRoundDurationSeconds() const;
	float ResolveStartGateIntervalSeconds() const;
	float ResolveEndGateIntervalSeconds() const;
	float ResolveCurrentGateIntervalSeconds() const;
	int32 ResolveStartingSquad() const;
	int32 ResolveChoiceScore() const;
	int32 ResolveSquadGain() const;
	FReply HandleGateClicked(bool bLeftGate);
	FReply HandlePrimaryActionClicked();
	FText GetStatLabel(ET66HeroStatType StatType) const;
	FLinearColor GetStatColor(ET66HeroStatType StatType) const;
	FLinearColor GetSquadMarkerColor(int32 MarkerIndex) const;
	FLinearColor GetEnemyMarkerColor(int32 MarkerIndex) const;
	float GetGateUrgencyAlpha() const;
	float GetChoiceFeedbackAlpha(bool bLeftGate) const;
	void LoadArtworkBrush();
	TSharedRef<SWidget> BuildArtworkLayer(float Opacity) const;

	TSharedPtr<STextBlock> TimerTextBlock;
	TSharedPtr<STextBlock> ScoreTextBlock;
	TSharedPtr<STextBlock> SquadTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
	TSharedPtr<STextBlock> LeftGateTextBlock;
	TSharedPtr<STextBlock> RightGateTextBlock;
	TSharedPtr<STextBlock> PrimaryActionTextBlock;
	TSharedPtr<SBorder> LeftGateBorder;
	TSharedPtr<SBorder> RightGateBorder;
	TArray<TSharedPtr<SBorder>> SquadMarkers;
	TArray<TSharedPtr<SBorder>> EnemyMarkers;

	FTimerHandle RoundTickHandle;
	FTimerHandle GateTickHandle;
	FTimerHandle AutoCloseHandle;
	TStrongObjectPtr<UTexture2D> ArtworkTexture;
	FSlateBrush ArtworkBrush;

	float RemainingSeconds = 0.f;
	float RoundDurationSeconds = 10.f;
	float StartGateIntervalSeconds = 0.90f;
	float EndGateIntervalSeconds = 0.28f;
	double RoundEndTimeSeconds = 0.0;
	int32 Score = 0;
	int32 SquadPower = 4;
	int32 ChoiceScore = 9;
	int32 SquadGain = 2;
	double LastGateRollTimeSeconds = 0.0;
	double LastChoiceTimeSeconds = -9999.0;
	bool bLastChoiceWasLeftGate = true;
	ET66HeroStatType LeftGateStat = ET66HeroStatType::Damage;
	ET66HeroStatType RightGateStat = ET66HeroStatType::Speed;
	bool bRoundEnded = false;
	bool bRoundSucceeded = false;
};
