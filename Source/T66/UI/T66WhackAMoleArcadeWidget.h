// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ArcadePopupWidget.h"
#include "Styling/SlateBrush.h"
#include "T66WhackAMoleArcadeWidget.generated.h"

class SButton;
class SHorizontalBox;
class SImage;
class STextBlock;
class UTexture2D;

UCLASS(Blueprintable)
class T66_API UT66WhackAMoleArcadeWidget : public UT66ArcadePopupWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	static constexpr int32 GridWidth = 3;
	static constexpr int32 GridHeight = 3;
	static constexpr int32 CellCount = GridWidth * GridHeight;

	enum class EWhackAMoleGameState : uint8
	{
		Ready,
		Playing,
		Paused,
		Finished
	};

	enum class EWhackAMoleSlotState : uint8
	{
		Hidden,
		Rising,
		Visible,
		Hit,
		Retreating
	};

	enum class EWhackAMoleType : uint8
	{
		Normal,
		Golden,
		Bomb
	};

	struct FWhackAMoleSlotRuntime
	{
		int32 SlotIndex = INDEX_NONE;
		EWhackAMoleSlotState CurrentState = EWhackAMoleSlotState::Hidden;
		EWhackAMoleType MoleType = EWhackAMoleType::Normal;
		float StateTimer = 0.f;
		float StateDuration = 0.f;
		float VisibleDuration = 0.f;
		float EffectTimer = 0.f;
		float ScorePopupTimer = 0.f;
		int32 ScorePopupValue = 0;
		FName EffectSpriteName = NAME_None;
		FName ScorePopupSpriteName = NAME_None;
		bool bCanBeHit = false;
	};

	void StartRound();
	void ClearActiveTimers();
	void HandleRoundTick();
	void CompleteRound();
	void TickSlots(float DeltaSeconds);
	void TrySpawnMole();
	void SpawnMoleInSlot(int32 SlotIndex, EWhackAMoleType MoleType);
	void ExpireVisibleMole(FWhackAMoleSlotRuntime& MoleSlot);
	void RefreshBoardVisuals();
	void RefreshHud();
	float ResolveRoundDurationSeconds() const;
	float ResolveStartSpawnIntervalSeconds() const;
	float ResolveEndSpawnIntervalSeconds() const;
	float ResolveCurrentSpawnIntervalSeconds() const;
	float ResolveVisibleDurationSeconds() const;
	float ResolveVisibleStartSeconds() const;
	float ResolveVisibleEndSeconds() const;
	float ResolveDifficultyAlpha() const;
	float ResolveGoldenChance() const;
	float ResolveBombChance() const;
	int32 ResolveTargetScore() const;
	int32 ResolveScorePerHit() const;
	int32 ResolveGoldenScore() const;
	int32 ResolveBombPenalty() const;
	int32 ResolveStartingLives() const;
	int32 ResolveMaxActiveMoles() const;
	int32 GetComboMultiplier() const;
	int32 CountActiveMoles() const;
	FText BuildRewardSummaryText() const;
	FText BuildStatusText() const;
	FText BuildPrimaryActionText() const;
	FText BuildSlotScorePopupText(const FWhackAMoleSlotRuntime& MoleSlot) const;
	FName ResolveMoleSpriteName(const FWhackAMoleSlotRuntime& MoleSlot) const;
	FName ResolveEffectSpriteName(const FWhackAMoleSlotRuntime& MoleSlot) const;
	const FSlateBrush* FindOrLoadSpriteBrush(FName SpriteName);
	TSharedRef<SWidget> BuildSpriteImage(FName SpriteName, const FVector2D& ImageSize, float Opacity = 1.f);
	TSharedRef<SWidget> BuildHudPanel(FName IconSprite, const FText& Label, const TSharedRef<SWidget>& ValueWidget);
	TSharedRef<SWidget> BuildLivesWidget();
	TSharedRef<SWidget> BuildSlotWidget(int32 SlotIndex);
	void ShowHammerAtSlot(int32 SlotIndex);
	void ApplyEmptyClick();
	FReply HandleCellClicked(int32 CellIndex);
	void HandleCellHovered(int32 CellIndex);
	void HandleCellUnhovered(int32 CellIndex);
	FReply HandlePrimaryActionClicked();

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UTexture2D>> SpriteTextures;

	TMap<FName, FSlateBrush> SpriteBrushes;
	TArray<FWhackAMoleSlotRuntime> MoleSlots;
	TArray<TSharedPtr<SButton>> SlotButtons;
	TArray<TSharedPtr<SImage>> HoleImages;
	TArray<TSharedPtr<SImage>> MoleImages;
	TArray<TSharedPtr<SImage>> HoverImages;
	TArray<TSharedPtr<SImage>> EffectImages;
	TArray<TSharedPtr<SImage>> ScorePopupImages;
	TArray<TSharedPtr<SImage>> HammerImages;
	TArray<TSharedPtr<STextBlock>> ScorePopupTextBlocks;
	TArray<TSharedPtr<SImage>> LifeImages;
	TSharedPtr<STextBlock> TimerTextBlock;
	TSharedPtr<STextBlock> ScoreTextBlock;
	TSharedPtr<STextBlock> ComboTextBlock;
	TSharedPtr<STextBlock> MultiplierTextBlock;
	TSharedPtr<STextBlock> BestComboTextBlock;
	TSharedPtr<STextBlock> StatusTextBlock;
	TSharedPtr<STextBlock> RewardTextBlock;
	TSharedPtr<STextBlock> PrimaryActionTextBlock;

	FTimerHandle RoundTickHandle;

	float RemainingSeconds = 0.f;
	float RoundDurationSeconds = 30.f;
	float StartSpawnIntervalSeconds = 1.0f;
	float EndSpawnIntervalSeconds = 0.35f;
	float VisibleStartSeconds = 1.2f;
	float VisibleEndSeconds = 0.45f;
	float SpawnAccumulator = 0.f;
	float HammerTimer = 0.f;
	int32 TargetScore = 2500;
	int32 ScorePerHit = 100;
	int32 GoldenScore = 500;
	int32 BombPenalty = 250;
	int32 Score = 0;
	int32 Hits = 0;
	int32 Misses = 0;
	int32 Combo = 0;
	int32 BestCombo = 0;
	int32 Lives = 3;
	int32 HoveredCellIndex = INDEX_NONE;
	int32 HammerCellIndex = INDEX_NONE;
	double LastTickTimeSeconds = 0.0;
	EWhackAMoleGameState GameState = EWhackAMoleGameState::Ready;
	bool bRoundSucceeded = false;
};
