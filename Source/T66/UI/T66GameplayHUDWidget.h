// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "T66GameplayHUDWidget.generated.h"

class UT66RunStateSubsystem;
class UT66PlayerSettingsSubsystem;
class UT66MediaViewerSubsystem;
class STextBlock;
class SBorder;
class SBox;
class SButton;
class SImage;
class UTexture2D;
struct FSlateBrush;
class ST66RingWidget;
class ST66DotWidget;
class ST66WorldMapWidget;
class AT66LootBagPickup;
enum class ET66Rarity : uint8;

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

	// Targeted refreshes (avoid calling full RefreshHUD for every tiny change).
	UFUNCTION()
	void RefreshEconomy();

	UFUNCTION()
	void RefreshStageAndTimer();

	UFUNCTION()
	void RefreshSpeedRunTimers();

	UFUNCTION()
	void RefreshBossBar();

	UFUNCTION()
	void RefreshTutorialHint();

	UFUNCTION()
	void RefreshHearts();

	UFUNCTION()
	void RefreshStatusEffects();

	UFUNCTION()
	void RefreshLootPrompt();

	/** Full-screen map overlay (M / OpenFullMap). */
	void SetFullMapOpen(bool bOpen);
	bool IsFullMapOpen() const { return bFullMapOpen; }
	void ToggleFullMap();

	// ============================================================
	// In-world NPC dialogue (Vendor/Gambler) - HUD-rendered
	// ============================================================
	void ShowWorldDialogue(const TArray<FText>& Options, int32 SelectedIndex);
	void HideWorldDialogue();
	void SetWorldDialogueSelection(int32 SelectedIndex);
	void SetWorldDialogueScreenPosition(const FVector2D& ScreenPos);
	bool IsWorldDialogueVisible() const;

	/** TikTok placeholder toggle (O / ToggleTikTok). */
	void ToggleTikTokPlaceholder();
	bool IsTikTokPlaceholderVisible() const;

	/** Wheel spin: show HUD animation + award gold (no overlay). */
	void StartWheelSpin(ET66Rarity WheelRarity);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual TSharedRef<SWidget> BuildSlateUI();

	UT66RunStateSubsystem* GetRunState() const;

	void RefreshMapData();
	void UpdateTikTokVisibility();
	bool IsMediaViewerOpen() const;

	void RequestTikTokWebView2OverlaySync();
	void SyncTikTokWebView2OverlayToPlaceholder();

	UFUNCTION()
	void HandleMediaViewerOpenChanged(bool bIsOpen);

	// Wheel spin animation (HUD-side; no overlay)
	void TickWheelSpin();
	void ResolveWheelSpin();
	void CloseWheelSpin();

	FReply OnToggleImmortality();
	FReply OnTogglePower();

	/** Cached Slate widgets for updates (set in BuildSlateUI via SAssignNew) */
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> ScoreText;
	TSharedPtr<STextBlock> ScoreMultiplierText;
	TSharedPtr<STextBlock> StageText;
	TSharedPtr<STextBlock> TimerText;
	TSharedPtr<STextBlock> SpeedRunText;
	TSharedPtr<STextBlock> SpeedRunTargetText;
	TSharedPtr<SBox> BossBarContainerBox;
	TSharedPtr<SBox> BossBarFillBox;
	TSharedPtr<STextBlock> BossBarText;
	TSharedPtr<SBox> LootPromptBox;
	TSharedPtr<SBorder> LootPromptBorder;
	TSharedPtr<SImage> LootPromptIconImage;
	TSharedPtr<FSlateBrush> LootPromptIconBrush;
	TSharedPtr<STextBlock> LootPromptText;
	TSharedPtr<SBorder> TutorialHintBorder;
	TSharedPtr<STextBlock> TutorialHintLine1Text;
	TSharedPtr<STextBlock> TutorialHintLine2Text;
	TArray<TSharedPtr<SBorder>> IdolLevelDotBorders; // 10 per idol slot
	TSharedPtr<ST66RingWidget> LevelRingWidget;
	TSharedPtr<STextBlock> LevelText;
	TSharedPtr<SBorder> UltimateBorder;
	TSharedPtr<STextBlock> UltimateText;
	TArray<TSharedPtr<ST66DotWidget>> StatusEffectDots;
	TArray<TSharedPtr<SBox>> StatusEffectDotBoxes;
	TSharedPtr<SBorder> CurseOverlayBorder;
	TSharedPtr<SBorder> FullMapOverlayBorder;
	TArray<TSharedPtr<SBorder>> HeartBorders;
	TArray<TSharedPtr<SBorder>> DifficultyBorders;
	TSharedPtr<SButton> ImmortalityButton;
	TSharedPtr<STextBlock> ImmortalityButtonText;
	TSharedPtr<SButton> PowerButton;
	TSharedPtr<STextBlock> PowerButtonText;
	TArray<TSharedPtr<SBorder>> IdolSlotBorders;
	TArray<TSharedPtr<SImage>> IdolSlotImages;
	TArray<TSharedPtr<FSlateBrush>> IdolSlotBrushes;
	TSharedPtr<SBorder> PortraitBorder;
	TSharedPtr<SImage> PortraitImage;
	TSharedPtr<FSlateBrush> PortraitBrush;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TArray<TSharedPtr<SImage>> InventorySlotImages;
	TArray<TSharedPtr<FSlateBrush>> InventorySlotBrushes;
	TSharedPtr<SBox> InventoryPanelBox;
	TSharedPtr<SBox> MinimapPanelBox;
	TSharedPtr<SBox> TikTokPlaceholderBox;
	TSharedPtr<SBox> TikTokContentBox;
	TSharedPtr<SBox> WheelSpinBox;
	TSharedPtr<SBorder> WheelSpinDisk;
	TSharedPtr<STextBlock> WheelSpinText;
	TSharedPtr<ST66WorldMapWidget> MinimapWidget;
	TSharedPtr<ST66WorldMapWidget> FullMapWidget;
	TSharedPtr<SBox> WorldDialogueBox;
	TArray<TSharedPtr<SBorder>> WorldDialogueOptionBorders;
	TArray<TSharedPtr<STextBlock>> WorldDialogueOptionTexts;

	bool bFullMapOpen = false;
	FTimerHandle MapRefreshTimerHandle;
	FTimerHandle TikTokOverlaySyncHandle;

	// Wheel spin state
	bool bWheelPanelOpen = false;
	bool bWheelSpinning = false;
	float WheelSpinElapsed = 0.f;
	float WheelSpinDuration = 2.0f;
	float WheelStartAngleDeg = 0.f;
	float WheelTotalAngleDeg = 0.f;
	float WheelLastTickTimeSeconds = 0.f;
	int32 WheelPendingGold = 0;
	ET66Rarity ActiveWheelRarity = static_cast<ET66Rarity>(0);
	FTimerHandle WheelSpinTickHandle;
	FTimerHandle WheelResolveHandle;
	FTimerHandle WheelCloseHandle;

	// ============================================================
	// Slate texture lifetime
	//
	// IMPORTANT: FSlateBrush does NOT keep UObject resources alive.
	// We rely on UT66UITexturePoolSubsystem as the central owner for any UTexture2D
	// used by Slate brushes (async-loaded from soft references).
	// ============================================================
};
