// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Styling/SlateBrush.h"
#include "T66GameplayHUDWidget.generated.h"

class UT66RunStateSubsystem;
class UT66DamageLogSubsystem;
class UT66PlayerSettingsSubsystem;
class UT66MediaViewerSubsystem;
class UT66CrateOverlayWidget;
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
class AActor;
class AT66LootBagPickup;
enum class ET66ItemRarity : uint8;
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

	UFUNCTION()
	void MarkHUDDirty();

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

	/** Show the item card popup for a just-picked-up item (above inventory, skippable, then fades out). */
	void ShowPickupItemCard(FName ItemID, ET66ItemRarity ItemRarity);
	bool TrySkipActivePresentation();
	void ClearActiveCratePresentation(UT66CrateOverlayWidget* Overlay);

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

	/** Toggle hit-test visibility so tooltips work when the mouse is free. */
	void SetInteractive(bool bInteractive);

	/** TikTok placeholder toggle (O / ToggleTikTok). */
	void ToggleTikTokPlaceholder();
	bool IsTikTokPlaceholderVisible() const;

	/** Wheel spin: show HUD animation + award gold (no overlay). */
	void StartWheelSpin(ET66Rarity WheelRarity);

	/** Crate open: show CS:GO-style item reveal overlay. */
	void StartCrateOpen();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual TSharedRef<SWidget> BuildSlateUI();

	UT66RunStateSubsystem* GetRunState() const;
	UT66DamageLogSubsystem* GetDamageLog() const;
	void RefreshCooldownBar();
	void RefreshDPS();
	void RefreshHeroStats();

	void RefreshMapData();
	void RefreshFPS();
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
	TSharedPtr<STextBlock> DPSText;
	TSharedPtr<STextBlock> StageText;
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
	TSharedPtr<SBox> PickupCardBox;
	TSharedPtr<SBorder> PickupCardTileBorder;
	TSharedPtr<SBorder> PickupCardIconBorder;
	TSharedPtr<FSlateBrush> PickupCardIconBrush;
	TSharedPtr<SImage> PickupCardIconImage;
	TSharedPtr<STextBlock> PickupCardNameText;
	TSharedPtr<STextBlock> PickupCardDescText;
	TSharedPtr<STextBlock> PickupCardSkipText;
	static constexpr float PickupCardWidth = 260.f;
	static constexpr float PickupCardHeight = 460.f;
	static constexpr float PickupCardDisplaySeconds = 5.f;
	static constexpr float PickupCardFadeOutSeconds = 0.6f;
	void HidePickupCard();
	TSharedPtr<SBorder> TutorialHintBorder;
	TSharedPtr<STextBlock> TutorialHintLine1Text;
	TSharedPtr<STextBlock> TutorialHintLine2Text;
	TSharedPtr<SBox> CenterCrosshairBox;
	TSharedPtr<SBorder> ScopedSniperOverlayBorder;
	TSharedPtr<STextBlock> ScopedUltTimerText;
	TSharedPtr<STextBlock> ScopedShotCooldownText;
	TArray<TSharedPtr<SBorder>> IdolLevelDotBorders; // legacy; rarity is now shown via sprite + border color
	TSharedPtr<ST66RingWidget> LevelRingWidget;
	TSharedPtr<STextBlock> LevelText;
	TSharedPtr<SBorder> PassiveBorder;
	TSharedPtr<SImage> PassiveImage;
	TSharedPtr<SBox> PassiveStackBadgeBox;
	TSharedPtr<STextBlock> PassiveStackText;
	TSharedPtr<SBorder> UltimateBorder;
	TSharedPtr<SBorder> UltimateCooldownOverlay;
	TSharedPtr<STextBlock> UltimateText;
	TSharedPtr<SImage> UltimateImage;
	TSharedPtr<FSlateBrush> UltimateBrush;
	TArray<TSharedPtr<ST66DotWidget>> StatusEffectDots;
	TArray<TSharedPtr<SBox>> StatusEffectDotBoxes;
	TSharedPtr<SBorder> CurseOverlayBorder;
	TSharedPtr<SBorder> FullMapOverlayBorder;
	TArray<TSharedPtr<SBorder>> HeartBorders;
	TArray<TSharedPtr<SImage>> HeartImages;
	TSharedPtr<FSlateBrush> HeartBrush;
	TArray<TSharedPtr<SBorder>> DifficultyBorders;
	TArray<TSharedPtr<SImage>> DifficultyImages;
	TSharedPtr<FSlateBrush> SkullBrush;
	TArray<TSharedPtr<SImage>> ClownImages;
	TSharedPtr<FSlateBrush> ClownBrush;
	TSharedPtr<SButton> ImmortalityButton;
	TSharedPtr<STextBlock> ImmortalityButtonText;
	TSharedPtr<SButton> PowerButton;
	TSharedPtr<STextBlock> PowerButtonText;
	TArray<TSharedPtr<SBorder>> IdolSlotBorders;
	TArray<TSharedPtr<SBox>> IdolSlotContainers;
	TArray<TSharedPtr<SImage>> IdolSlotImages;
	TArray<TSharedPtr<FSlateBrush>> IdolSlotBrushes;
	TSharedPtr<SBorder> PortraitBorder;
	TSharedPtr<SImage> PortraitImage;
	TSharedPtr<FSlateBrush> PortraitBrush;
	TSharedPtr<SBox> CooldownBarFillBox;
	TSharedPtr<STextBlock> CooldownTimeText;
	static constexpr float CooldownBarWidth = 200.f;
	static constexpr float CooldownBarHeight = 6.f;
	TSharedPtr<SBox> PortraitStatPanelBox;
	TSharedPtr<STextBlock> StatDamageText;
	TSharedPtr<STextBlock> StatAttackSpeedText;
	TSharedPtr<STextBlock> StatAttackScaleText;
	TSharedPtr<STextBlock> StatArmorText;
	TSharedPtr<STextBlock> StatEvasionText;
	TSharedPtr<STextBlock> StatLuckText;
	TSharedPtr<STextBlock> StatSpeedText;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TArray<TSharedPtr<SBox>> InventorySlotContainers;
	TArray<TSharedPtr<SImage>> InventorySlotImages;
	TArray<TSharedPtr<FSlateBrush>> InventorySlotBrushes;
	/** Cached idol/item IDs per slot so we only update tooltips when slot content changes (avoids tooltip flash from RefreshHUD). */
	TArray<FName> CachedIdolSlotIDs;
	TArray<FName> CachedInventorySlotIDs;
	TSharedPtr<STextBlock> FPSText;
	TSharedPtr<SBox> InventoryPanelBox;
	TSharedPtr<SBox> BottomLeftHUDBox;
	TSharedPtr<SBox> IdolSlotsPanelBox;
	TSharedPtr<SBox> MinimapPanelBox;
	TSharedPtr<SBox> TikTokPlaceholderBox;
	TSharedPtr<SBox> TikTokContentBox;
	TSharedPtr<SBox> MediaViewerVideoBox; // Inner video area (WebView2 syncs to this, not the full panel)
	TSharedPtr<SBox> WheelSpinBox;
	TSharedPtr<SImage> WheelSpinDisk;
	TSharedPtr<STextBlock> WheelSpinText;
	FSlateBrush WheelTextureBrush;
	TSharedPtr<ST66WorldMapWidget> MinimapWidget;
	TSharedPtr<ST66WorldMapWidget> FullMapWidget;
	TSharedPtr<SBox> WorldDialogueBox;
	TArray<TSharedPtr<SBorder>> WorldDialogueOptionBorders;
	TArray<TSharedPtr<STextBlock>> WorldDialogueOptionTexts;

	bool bFullMapOpen = false;
	bool bHUDDirty = false;
	bool bPickupCardVisible = false;
	float DPSRefreshAccumSeconds = 0.f;
	float PickupCardRemainingSeconds = 0.f;
	static constexpr float DPSRefreshIntervalSeconds = 0.2f;
	int32 LastDisplayedCooldownBarWidthPx = -1;
	int32 LastDisplayedCooldownRemainingCs = -1;
	int32 LastDisplayedDPS = -1;
	FLinearColor LastDisplayedDPSColor = FLinearColor::Transparent;
	int32 LastDisplayedSpeedRunTotalCs = -1;
	bool bPortraitStateInitialized = false;
	bool bLastPortraitHasRef = false;
	FName LastPortraitHeroID = NAME_None;
	ET66BodyType LastPortraitBodyType = ET66BodyType::TypeA;
	ET66HeroPortraitVariant LastPortraitVariant = ET66HeroPortraitVariant::Half;

	/** Map/minimap: cache actor refs + static marker data; full TActorIterator only every MapCacheRefreshIntervalSeconds. */
	struct FMapCacheEntry { TWeakObjectPtr<AActor> Actor; FLinearColor Color; FText Label; };
	TArray<FMapCacheEntry> MapCache;
	float MapCacheLastRefreshTime = -1.f;
	TWeakObjectPtr<UWorld> MapCacheWorld;
	static constexpr float MapCacheRefreshIntervalSeconds = 1.5f;

	FTimerHandle MapRefreshTimerHandle;
	FTimerHandle FPSTimerHandle;
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
	TWeakObjectPtr<UT66CrateOverlayWidget> ActiveCrateOverlay;

	// Achievement unlock notification (above inventory, one at a time, tier-colored border)
	TArray<FName> AchievementNotificationQueue;
	FTimerHandle AchievementNotificationTimerHandle;
	TSharedPtr<SBox> AchievementNotificationBox;
	TSharedPtr<SBorder> AchievementNotificationBorder;
	TSharedPtr<STextBlock> AchievementNotificationTitleText;
	static constexpr float AchievementNotificationDisplaySeconds = 3.5f;
	UFUNCTION()
	void HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs);
	void ShowNextAchievementNotification();
	void HideAchievementNotificationAndShowNext();

	// ============================================================
	// Slate texture lifetime
	//
	// IMPORTANT: FSlateBrush does NOT keep UObject resources alive.
	// We rely on UT66UITexturePoolSubsystem as the central owner for any UTexture2D
	// used by Slate brushes (async-loaded from soft references).
	// ============================================================
};
