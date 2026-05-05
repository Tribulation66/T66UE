// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Styling/SlateBrush.h"
#include "Templates/UniquePtr.h"
#include "UI/HUD/T66HUDPresentationController.h"
#include "T66GameplayHUDWidget.generated.h"

class UT66RunStateSubsystem;
class UT66DamageLogSubsystem;
class UT66PlayerSettingsSubsystem;
class UT66MediaViewerSubsystem;
class UT66CrateOverlayWidget;
class STextBlock;
class SBorder;
class SBox;
class SVerticalBox;
class SButton;
class SImage;
class ST66CrosshairWidget;
class UTexture2D;
struct FSlateBrush;
class ST66RingWidget;
class ST66DotWidget;
class ST66WorldMapWidget;
class AActor;
class AT66LootBagPickup;
enum class ET66ItemRarity : uint8;
enum class ET66Rarity : uint8;

struct FT66BossPartBarRow
{
	FName PartID = NAME_None;
	ET66HitZoneType HitZoneType = ET66HitZoneType::None;
	TSharedPtr<SBox> FillBox;
	TSharedPtr<SBorder> FillBorder;
	TSharedPtr<STextBlock> Text;
	int32 LastCurrentHP = INDEX_NONE;
	int32 LastMaxHP = INDEX_NONE;
	bool bLastAlive = false;
};

struct FT66HUDInteractionPromptEntry
{
	TWeakObjectPtr<AActor> SourceActor;
	FText TargetName = FText::GetEmpty();
};

/**
 * In-run HUD: hearts (5 icons), gold, toggleable inventory bar (1x5) and minimap placeholder.
 * T toggles panels; hearts and gold always visible. Event-driven updates via RunState.
 */
UCLASS(Blueprintable)
class T66_API UT66GameplayHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	friend class FT66HUDPresentationController;

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

	void RefreshBeatTargets();

	UFUNCTION()
	void RefreshBossBar();
	void RebuildBossPartBars(const TArray<FT66BossPartSnapshot>& BossParts, const FLinearColor& BossBarBackgroundColor);
	bool DoesBossPartBarTopologyMatch(const TArray<FT66BossPartSnapshot>& BossParts) const;

	UFUNCTION()
	void RefreshTutorialHint();

	UFUNCTION()
	void RefreshTutorialSubtitle();

	UFUNCTION()
	void RefreshHearts();

	UFUNCTION()
	void RefreshQuickReviveState();

	UFUNCTION()
	void RefreshStatusEffects();

	UFUNCTION()
	void RefreshLootPrompt();

	/** Show the item card popup for a just-picked-up item above inventory. */
	void ShowPickupItemCard(FName ItemID, ET66ItemRarity ItemRarity);

	/** Show the chest gold reward presentation in the same above-inventory lane. */
	void StartChestReward(ET66Rarity ChestRarity, int32 GoldAmount);
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

	/** HUD-rendered interaction prompt for nearby world interactables. */
	void ShowInteractionPrompt(AActor* SourceActor, const FText& TargetName);
	void HideInteractionPrompt(AActor* SourceActor);

	/** Toggle hit-test visibility so tooltips work when the mouse is free. */
	void SetInteractive(bool bInteractive);

	/** Enlarge the inventory panel for mouse-hover inspection. */
	void SetInventoryInspectMode(bool bEnabled);
	bool IsInventoryInspectMode() const { return bInventoryInspectMode; }

	/** TikTok placeholder toggle (O / ToggleTikTok). */
	void ToggleTikTokPlaceholder();
	bool IsTikTokPlaceholderVisible() const;

	/** Wheel spin: show HUD animation + award gold (no overlay). */
	void StartWheelSpin(ET66Rarity WheelRarity);

	/** Crate open: show CS:GO-style item reveal overlay. */
	void StartCrateOpen();

	static constexpr float MinimapPanelWidth = 164.f;
	static constexpr float BottomRightInventoryPanelWidth = 378.f;
	static constexpr float BottomRightInventoryPanelHeight = 167.f;
	static constexpr float BottomRightPauseAchievementPanelWidth = 472.f;
	static constexpr float BottomRightPresentationGap = 8.f;
	static constexpr float BottomRightInventorySlotSize = 50.f;
	static constexpr float BottomRightInventoryInspectScale = 2.f;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual TSharedRef<SWidget> BuildSlateUI();

	UT66RunStateSubsystem* GetRunState() const;
	UT66DamageLogSubsystem* GetDamageLog() const;
	void RefreshDPS();
	void RefreshHeroStats();

	void RefreshMapData();
	void UpdateTowerMapReveal(const FVector& PlayerLocation);
	bool IsTowerMapRevealPointVisible(int32 FloorNumber, const FVector2D& WorldXY) const;
	void RefreshFPS();
	bool IsPausePresentationActive() const;
	void RefreshPausePresentation();
	TSharedRef<SWidget> BuildPauseStatsPanel() const;
	TSharedRef<SWidget> BuildPauseAchievementsPanel() const;
	void UpdateTikTokVisibility(bool bForce = false);
	bool IsMediaViewerOpen() const;

	void RequestTikTokWebView2OverlaySync();
	void SyncTikTokWebView2OverlayToPlaceholder();

	UFUNCTION()
	void HandleMediaViewerOpenChanged(bool bIsOpen);

	void HandleBackendLeaderboardDataReady(const FString& Key);
	void HandleBackendRunSummaryReady(const FString& EntryId);

	// Wheel spin animation (HUD-side; no overlay)
	void TickWheelSpin();
	void ResolveWheelSpin();
	void CloseWheelSpin();
	int32 GetChestRewardRevealThresholdGold(ET66Rarity Rarity) const;
	ET66Rarity ResolveChestRewardDisplayedRarity(int32 DisplayedGold) const;
	void RefreshChestRewardVisualState();
	void TickChestRewardPresentation(float InDeltaTime);
	void HideChestReward();
	void TryShowQueuedPresentation();

	FReply OnToggleImmortality();
	FReply OnTogglePower();

	/** Cached Slate widgets for updates (set in BuildSlateUI via SAssignNew) */
	TSharedPtr<STextBlock> NetWorthText;
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> DebtText;
	TSharedPtr<STextBlock> ScoreText;
	TSharedPtr<STextBlock> ScorePacingText;
	TSharedPtr<STextBlock> ScoreTargetText;
	TSharedPtr<STextBlock> ScoreMultiplierText;
	TSharedPtr<STextBlock> DPSText;
	TSharedPtr<STextBlock> DifficultyAreaNameText;
	TSharedPtr<STextBlock> StageText;
	TSharedPtr<STextBlock> SpeedRunText;
	TSharedPtr<STextBlock> SpeedRunPacingText;
	TSharedPtr<STextBlock> SpeedRunTargetText;
	TSharedPtr<SBox> BossBarContainerBox;
	TSharedPtr<SBox> BossBarFillBox;
	TSharedPtr<STextBlock> BossBarText;
	TSharedPtr<SVerticalBox> BossPartBarsBox;
	TArray<FT66BossPartBarRow> BossPartBarRows;
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
	TSharedPtr<SBox> ChestRewardBox;
	TSharedPtr<SBorder> ChestRewardTileBorder;
	TSharedPtr<SBox> ChestRewardClosedBox;
	TSharedPtr<SBox> ChestRewardOpenBox;
	TSharedPtr<FSlateBrush> ChestRewardClosedBrush;
	TSharedPtr<FSlateBrush> ChestRewardOpenBrush;
	TSharedPtr<FSlateBrush> GoldCurrencyBrush;
	TSharedPtr<FSlateBrush> DebtCurrencyBrush;
	TSharedPtr<SImage> ChestRewardClosedImage;
	TSharedPtr<SImage> ChestRewardOpenImage;
	TSharedPtr<STextBlock> ChestRewardCounterText;
	TSharedPtr<STextBlock> ChestRewardSkipText;
	TArray<TSharedPtr<SBox>> ChestRewardCoinBoxes;
	TArray<TSharedPtr<SImage>> ChestRewardCoinImages;
	static constexpr float PickupCardWidth = MinimapPanelWidth;
	static constexpr float PickupCardHeight = 216.f;
	static constexpr float ChestRewardDisplaySeconds = 2.4f;
	static constexpr float ChestRewardFadeOutSeconds = 0.35f;
	static constexpr int32 ChestRewardCoinCount = 7;
	void HidePickupCard();
	void ApplyInventoryInspectMode();
	TSharedPtr<SBorder> TutorialHintBorder;
	TSharedPtr<STextBlock> TutorialHintLine1Text;
	TSharedPtr<STextBlock> TutorialHintLine2Text;
	TSharedPtr<SBorder> TutorialSubtitleBorder;
	TSharedPtr<STextBlock> TutorialSubtitleSpeakerText;
	TSharedPtr<STextBlock> TutorialSubtitleBodyText;
	TSharedPtr<ST66CrosshairWidget> CenterCrosshairWidget;
	TSharedPtr<SBox> CenterCrosshairBox;
	TSharedPtr<SBorder> ScopedSniperOverlayBorder;
	TSharedPtr<STextBlock> ScopedUltTimerText;
	TSharedPtr<STextBlock> ScopedShotCooldownText;
	TSharedPtr<SBorder> QuickReviveDownedOverlayBorder;
	TSharedPtr<STextBlock> QuickReviveDownedText;
	TArray<TSharedPtr<SBorder>> IdolLevelDotBorders; // legacy; rarity is now shown via sprite + border color
	TSharedPtr<ST66RingWidget> LevelRingWidget;
	TSharedPtr<STextBlock> LevelText;
	TSharedPtr<SBorder> PassiveBorder;
	TSharedPtr<SImage> PassiveImage;
	TSharedPtr<FSlateBrush> PassiveBrush;
	TSharedPtr<SBox> PassiveStackBadgeBox;
	TSharedPtr<STextBlock> PassiveStackText;
	TSharedPtr<SBorder> UltimateBorder;
	TSharedPtr<SBorder> UltimateCooldownOverlay;
	TSharedPtr<STextBlock> UltimateText;
	TSharedPtr<STextBlock> UltimateInputHintText;
	TSharedPtr<SImage> UltimateImage;
	TSharedPtr<FSlateBrush> UltimateBrush;
	TArray<TSharedPtr<ST66DotWidget>> StatusEffectDots;
	TArray<TSharedPtr<SBox>> StatusEffectDotBoxes;
	TSharedPtr<SBorder> CurseOverlayBorder;
	TSharedPtr<SBorder> FullMapOverlayBorder;
	TSharedPtr<SBorder> PauseBackdropBorder;
	TArray<TSharedPtr<SBorder>> HeartBorders;
	TArray<TSharedPtr<SBox>> HeartFillBoxes;
	TArray<TSharedPtr<SImage>> HeartImages;
	TSharedPtr<FSlateBrush> HeartBrush;
	TSharedPtr<FSlateBrush> HeartBlessingBrush;
	TArray<TSharedPtr<FSlateBrush>> HeartTierBrushes;
	TSharedPtr<SBox> QuickReviveIconRowBox;
	TSharedPtr<SImage> QuickReviveIconImage;
	TSharedPtr<FSlateBrush> QuickReviveBrush;
	TSharedPtr<SBox> DifficultyRowBox;
	TArray<TSharedPtr<SBorder>> DifficultyBorders;
	TArray<TSharedPtr<SImage>> DifficultyImages;
	TSharedPtr<FSlateBrush> SkullBrush;
	TSharedPtr<SBox> CowardiceRowBox;
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
	TSharedPtr<STextBlock> PortraitPlaceholderText;
	TSharedPtr<SBox> PortraitStatPanelBox;
	TSharedPtr<STextBlock> StatDamageText;
	TSharedPtr<STextBlock> StatAttackSpeedText;
	TSharedPtr<STextBlock> StatAttackScaleText;
	TSharedPtr<STextBlock> StatArmorText;
	TSharedPtr<STextBlock> StatEvasionText;
	TSharedPtr<STextBlock> StatLuckText;
	TArray<TSharedPtr<SBorder>> InventorySlotBorders;
	TArray<TSharedPtr<SBox>> InventorySlotContainers;
	TArray<TSharedPtr<SImage>> InventorySlotImages;
	TArray<TSharedPtr<FSlateBrush>> InventorySlotBrushes;
	/** Cached idol/item IDs per slot so we only update tooltips when slot content changes (avoids tooltip flash from RefreshHUD). */
	TArray<FName> CachedIdolSlotIDs;
	TArray<FName> CachedInventorySlotIDs;
	TSharedPtr<STextBlock> ElevationText;
	TSharedPtr<STextBlock> FPSText;
	TSharedPtr<SBox> InventoryPanelBox;
	TSharedPtr<SBox> BottomLeftHUDBox;
	TSharedPtr<SBox> IdolSlotsPanelBox;
	TSharedPtr<SBox> MinimapPanelBox;
	TSharedPtr<SBox> PauseStatsPanelBox;
	TSharedPtr<SBox> PauseAchievementsPanelBox;
	TSharedPtr<SBox> TikTokPlaceholderBox;
	TSharedPtr<SBox> TikTokContentBox;
	TSharedPtr<SBox> MediaViewerVideoBox; // Inner video area (WebView2 syncs to this, not the full panel)
	TSharedPtr<SBox> WheelSpinBox;
	TSharedPtr<SImage> WheelSpinDisk;
	TSharedPtr<STextBlock> WheelSpinText;
	TSharedPtr<STextBlock> WheelSpinSkipText;
	FSlateBrush WheelTextureBrush;
	TSharedPtr<ST66WorldMapWidget> MinimapWidget;
	TSharedPtr<ST66WorldMapWidget> FullMapWidget;
	TSharedPtr<SBox> InteractionPromptBox;
	TSharedPtr<STextBlock> InteractionPromptTargetText;
	TArray<FT66HUDInteractionPromptEntry> InteractionPromptEntries;
	TSharedPtr<SBox> WorldDialogueBox;
	TArray<TSharedPtr<SBorder>> WorldDialogueOptionBorders;
	TArray<TSharedPtr<STextBlock>> WorldDialogueOptionTexts;

	bool bFullMapOpen = false;
	bool bInventoryInspectMode = false;
	bool bHUDDirty = false;
	float DPSRefreshAccumSeconds = 0.f;
	static constexpr float DPSRefreshIntervalSeconds = 0.2f;
	int32 LastDisplayedDPS = -1;
	FLinearColor LastDisplayedDPSColor = FLinearColor::Transparent;
	int32 LastDisplayedSpeedRunTotalCs = -1;
	int32 LastDisplayedBossCurrentHP = INDEX_NONE;
	int32 LastDisplayedBossMaxHP = INDEX_NONE;
	bool bLastBossBarVisible = false;
	bool bPortraitStateInitialized = false;
	bool bLastPortraitHasRef = false;
	FName LastPortraitHeroID = NAME_None;
	ET66BodyType LastPortraitBodyType = ET66BodyType::Chad;
	ET66HeroPortraitVariant LastPortraitVariant = ET66HeroPortraitVariant::Half;
	bool bAbilityStateInitialized = false;
	FName LastAbilityHeroID = NAME_None;
	FName LastWeaponID = NAME_None;
	ET66UltimateType LastUltimateType = ET66UltimateType::None;
	ET66PassiveType LastPassiveType = ET66PassiveType::None;
	bool bLastCrosshairLocked = false;
	bool bLastScopedHudVisible = false;
	int32 LastScopedUltDisplayTenths = INDEX_NONE;
	int32 LastScopedShotDisplayCentis = INDEX_NONE;
	bool bLastScopedShotReady = false;
	bool bLastPausePresentationActive = false;
	bool bHasAppliedMediaViewerOpenState = false;
	bool bLastAppliedMediaViewerOpenState = false;

	/** Map/minimap: cache actor refs + static marker data; full TActorIterator only every MapCacheRefreshIntervalSeconds. */
	enum class EMapCacheMarkerType : uint8
	{
		NPC,
		Gate,
		Enemy,
		Miasma,
		POI,
	};

	struct FMapCacheEntry
	{
		TWeakObjectPtr<AActor> Actor;
		EMapCacheMarkerType Type = EMapCacheMarkerType::NPC;
		FLinearColor Color = FLinearColor::White;
		FText Label = FText::GetEmpty();
		FName MarkerKey = NAME_None;
	};
	TArray<FMapCacheEntry> MapCache;
	float MapCacheLastRefreshTime = -1.f;
	TWeakObjectPtr<UWorld> MapCacheWorld;
	static constexpr float MapCacheRefreshIntervalSeconds = 1.5f;
	TMap<int32, TArray<FVector2D>> TowerRevealPointsByFloor;
	int32 LastTowerRevealFloorNumber = INDEX_NONE;
	float TowerRevealAccumSeconds = 0.f;
	static constexpr float TowerRevealIntervalSeconds = 0.10f;

	FTimerHandle MapRefreshTimerHandle;
	FTimerHandle FPSTimerHandle;
	FTimerHandle TikTokOverlaySyncHandle;
	TSharedPtr<SBox> AchievementNotificationBox;
	TSharedPtr<SBorder> AchievementNotificationBorder;
	TSharedPtr<STextBlock> AchievementNotificationTitleText;
	static constexpr float AchievementNotificationDisplaySeconds = 3.5f;
	UFUNCTION()
	void HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs);
	void ShowNextAchievementNotification();
	void HideAchievementNotificationAndShowNext();
	void RefreshInteractionPromptWidget();
	FT66HUDPresentationController& GetPresentationController();
	const FT66HUDPresentationController& GetPresentationController() const;
	TUniquePtr<FT66HUDPresentationController> PresentationController;

	// ============================================================
	// Slate texture lifetime
	//
	// IMPORTANT: FSlateBrush does NOT keep UObject resources alive.
	// We rely on UT66UITexturePoolSubsystem as the central owner for any UTexture2D
	// used by Slate brushes (async-loaded from soft references).
	// ============================================================
};
