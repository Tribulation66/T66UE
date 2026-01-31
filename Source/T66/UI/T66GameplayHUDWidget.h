// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66GameplayHUDWidget.generated.h"

class UT66RunStateSubsystem;
class UT66PlayerSettingsSubsystem;
class STextBlock;
class SBorder;
class SBox;
class ST66RingWidget;
class ST66DotWidget;
class AT66LootBagPickup;

/**
 * In-run HUD: hearts (5 icons), gold, toggleable inventory bar (1x5) and minimap placeholder.
 * T toggles panels; hearts and gold always visible. Event-driven updates via RunState.
 */
UCLASS(Blueprintable)
class T66_API UT66GameplayHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Call when RunState changes (hearts, gold, inventory, panel visibility). */
	UFUNCTION()
	void RefreshHUD();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI();

	UT66RunStateSubsystem* GetRunState() const;

	/** Cached Slate widgets for updates (set in BuildSlateUI via SAssignNew) */
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> ScoreText;
	TSharedPtr<STextBlock> StageText;
	TSharedPtr<STextBlock> TimerText;
	TSharedPtr<STextBlock> SpeedRunText;
	TSharedPtr<STextBlock> SpeedRunTargetText;
	TSharedPtr<SBox> BossBarContainerBox;
	TSharedPtr<SBox> BossBarFillBox;
	TSharedPtr<STextBlock> BossBarText;
	TSharedPtr<SBox> LootPromptBox;
	TSharedPtr<SBorder> LootPromptBorder;
	TSharedPtr<STextBlock> LootPromptText;
	TSharedPtr<SBox> SurvivalBarFillBox;
	TSharedPtr<SBorder> SurvivalBarFillBorder;
	TSharedPtr<ST66RingWidget> LevelRingWidget;
	TSharedPtr<STextBlock> LevelText;
	TSharedPtr<SBorder> UltimateBorder;
	TSharedPtr<STextBlock> UltimateText;
	TArray<TSharedPtr<ST66DotWidget>> StatusEffectDots;
	TArray<TSharedPtr<SBox>> StatusEffectDotBoxes;
	TSharedPtr<SBorder> CurseOverlayBorder;
	TArray<TSharedPtr<SBorder>> HeartBorders;
	TArray<TSharedPtr<SBorder>> DifficultyBorders;
	TArray<TSharedPtr<SBorder>> IdolSlotBorders;
	TSharedPtr<SBorder> PortraitBorder;
	TArray<TSharedPtr<STextBlock>> StatLineTexts;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TSharedPtr<SBox> InventoryPanelBox;
	TSharedPtr<SBox> MinimapPanelBox;
};
