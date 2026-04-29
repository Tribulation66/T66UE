// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Core/T66Rarity.h"
#include "Data/T66DataTypes.h"

class UT66GameplayHUDWidget;
class UT66CrateOverlayWidget;
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

	void StartWheelSpin(ET66Rarity WheelRarity);
	void StartCrateOpen();
	void StartChestReward(ET66Rarity ChestRarity, int32 GoldAmount);
	bool TrySkipActivePresentation();
	void ClearActiveCratePresentation(UT66CrateOverlayWidget* Overlay);

	void TickWheelSpin();
	void ResolveWheelSpin();
	void CloseWheelSpin();

	int32 GetChestRewardRevealThresholdGold(ET66Rarity Rarity) const;
	ET66Rarity ResolveChestRewardDisplayedRarity(int32 DisplayedGold) const;
	void RefreshChestRewardVisualState();
	void TickChestRewardPresentation(float InDeltaTime);
	void HideChestReward();
	void TryShowQueuedPresentation();

	void ShowPickupItemCard(FName ItemID, ET66ItemRarity ItemRarity);
	void HidePickupCard();

	bool IsWheelPanelOpen() const { return bWheelPanelOpen; }
	bool IsPickupCardVisible() const { return bPickupCardVisible; }
	bool IsChestRewardVisible() const { return bChestRewardVisible; }

private:
	struct FQueuedChestReward
	{
		ET66Rarity Rarity = ET66Rarity::Black;
		int32 GoldAmount = 0;
	};

	struct FQueuedPickupCard
	{
		FName ItemID = NAME_None;
		ET66ItemRarity ItemRarity = ET66ItemRarity::Black;
	};

	void QueueActivePickupCardToFront();

	UT66GameplayHUDWidget& Owner;

	bool bPickupCardVisible = false;
	bool bChestRewardVisible = false;
	float ChestRewardRemainingSeconds = 0.f;
	float ChestRewardElapsedSeconds = 0.f;
	FName ActivePickupCardItemID = NAME_None;
	ET66ItemRarity ActivePickupCardRarity = ET66ItemRarity::Black;
	ET66Rarity ActiveChestRewardRarity = ET66Rarity::Black;
	int32 ChestRewardTargetGold = 0;
	int32 ChestRewardDisplayedGold = 0;
	int32 ChestRewardMinimumDisplayedGold = 0;
	TArray<FQueuedChestReward> QueuedChestRewards;
	TArray<FQueuedPickupCard> QueuedPickupCards;

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

	TArray<FName> AchievementNotificationQueue;
	FTimerHandle AchievementNotificationTimerHandle;
};
