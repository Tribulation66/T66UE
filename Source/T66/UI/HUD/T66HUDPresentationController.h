// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Animation/T66AnimationTimeline.h"
#include "TimerManager.h"
#include "UI/Animation/T66AnimationMarkerDispatch.h"
#include "Core/T66Rarity.h"
#include "Data/T66DataTypes.h"
#include "UI/T66LootWheelPresentationTypes.h"

class UT66GameplayHUDWidget;
class UT66CrateOverlayWidget;
class UT66LootWheelOverlayWidget;
enum class ET66ItemRarity : uint8;

class FT66HUDPresentationController
{
public:
	explicit FT66HUDPresentationController(UT66GameplayHUDWidget& InOwner);

	void Tick(float InDeltaTime);
	void Reset();

	void HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs);
	void ShowNextAchievementNotification();
	void HideAchievementNotificationAndShowNext();

	void StartCrateOpen(ET66Rarity SourceCrateRarity);
	bool StartLootWheelSpin(FT66LootWheelPresentationParams Params);
	bool StartChestReward(ET66Rarity ChestRarity, int32 GoldAmount, TFunction<void()> OnCommit = nullptr, TFunction<void()> OnFinished = nullptr);
	bool TrySkipActivePresentation();
	void ClearActiveCratePresentation(UT66CrateOverlayWidget* Overlay);
	void ClearActiveLootWheelPresentation(UT66LootWheelOverlayWidget* Overlay);

	int32 GetChestRewardRevealThresholdGold(ET66Rarity Rarity) const;
	ET66Rarity ResolveChestRewardDisplayedRarity(int32 DisplayedGold) const;
	void RefreshChestRewardVisualState();
	void TickChestRewardPresentation(float InDeltaTime);
	void HideChestReward();
	void TryShowQueuedPresentation();

	void ShowPickupItemCard(FName ItemID, ET66ItemRarity ItemRarity);
	void ShowLootBagItemReveal(FName ItemID, ET66ItemRarity ItemRarity);
	void HidePickupCard();
	void TickLootBagRevealPresentation(float InDeltaTime);
	void HideLootBagReveal();

	bool IsPickupCardVisible() const { return bPickupCardVisible; }
	bool IsChestRewardVisible() const { return bChestRewardVisible; }
	bool HasPendingPresentationWork() const { return ActiveCrateOverlay.IsValid() || ActiveLootWheelOverlay.IsValid() || bChestRewardVisible || bLootBagRevealVisible || bPickupCardVisible || QueuedPresentations.Num() > 0; }

private:
	enum class EQueuedPresentationType : uint8
	{
		CrateOpen,
		LootWheelSpin,
		ChestReward,
		PickupCard,
		LootBagReveal,
	};

	struct FQueuedPresentation
	{
		EQueuedPresentationType Type = EQueuedPresentationType::PickupCard;
		ET66Rarity Rarity = ET66Rarity::Black;
		int32 GoldAmount = 0;
		FT66LootWheelPresentationParams LootWheelParams;
		FName ItemID = NAME_None;
		ET66ItemRarity ItemRarity = ET66ItemRarity::Black;
		TFunction<void()> OnCommit;
		TFunction<void()> OnFinished;
	};

	bool IsRewardPresentationBusy() const;
	void PopulatePickupCardContent(FName ItemID, ET66ItemRarity ItemRarity, bool bPopulateLootBagProxy);
	void CompleteLootBagRevealToPickupCard();
	void ResetLootBagRevealWidgets();
	void DispatchChestRewardCommitIfNeeded();
	void DispatchChestRewardFinishedIfNeeded();
	void DrainQueuedPresentationsForTeardown();

	UT66GameplayHUDWidget& Owner;

	bool bPickupCardVisible = false;
	bool bChestRewardVisible = false;
	bool bLootBagRevealVisible = false;
	bool bLootBagRevealHandoffComplete = false;
	float ChestRewardRemainingSeconds = 0.f;
	float ChestRewardElapsedSeconds = 0.f;
	float LootBagRevealElapsedSeconds = 0.f;
	float LootBagRevealRemainingSeconds = 0.f;
	bool bResettingPresentations = false;
	FName ActivePickupCardItemID = NAME_None;
	ET66ItemRarity ActivePickupCardRarity = ET66ItemRarity::Black;
	FName ActiveLootBagRevealItemID = NAME_None;
	ET66ItemRarity ActiveLootBagRevealRarity = ET66ItemRarity::Black;
	ET66Rarity ActiveChestRewardRarity = ET66Rarity::Black;
	int32 ChestRewardTargetGold = 0;
	int32 ChestRewardDisplayedGold = 0;
	int32 ChestRewardMinimumDisplayedGold = 0;
	bool bChestRewardCommitDispatched = false;
	bool bChestRewardCountTimelineStarted = false;
	FT66AnimationTimeline ChestRewardCountTimeline;
	FT66AnimationMarkerDispatcher ChestRewardMarkerDispatcher;
	TFunction<void()> ActiveChestRewardCommitCallback;
	TFunction<void()> ActiveChestRewardFinishedCallback;
	TArray<FQueuedPresentation> QueuedPresentations;

	TWeakObjectPtr<UT66CrateOverlayWidget> ActiveCrateOverlay;
	TWeakObjectPtr<UT66LootWheelOverlayWidget> ActiveLootWheelOverlay;

	TArray<FName> AchievementNotificationQueue;
	FTimerHandle AchievementNotificationTimerHandle;
};
