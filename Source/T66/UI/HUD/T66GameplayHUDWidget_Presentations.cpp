// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66GameplayHUDWidget_Private.h"

void UT66GameplayHUDWidget::HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs)
{
	GetPresentationController().HandleAchievementsUnlocked(NewlyUnlockedIDs);
}


void UT66GameplayHUDWidget::ShowNextAchievementNotification()
{
	GetPresentationController().ShowNextAchievementNotification();
}


void UT66GameplayHUDWidget::HideAchievementNotificationAndShowNext()
{
	GetPresentationController().HideAchievementNotificationAndShowNext();
}


void UT66GameplayHUDWidget::StartCrateOpen(const ET66Rarity SourceCrateRarity)
{
	GetPresentationController().StartCrateOpen(SourceCrateRarity);
}


bool UT66GameplayHUDWidget::StartChestReward(
	const ET66Rarity ChestRarity,
	const int32 GoldAmount,
	TFunction<void()> OnCommit,
	TFunction<void()> OnFinished)
{
	return GetPresentationController().StartChestReward(ChestRarity, GoldAmount, MoveTemp(OnCommit), MoveTemp(OnFinished));
}

bool UT66GameplayHUDWidget::StartLootWheelSpin(FT66LootWheelPresentationParams Params)
{
	return GetPresentationController().StartLootWheelSpin(MoveTemp(Params));
}


bool UT66GameplayHUDWidget::TrySkipActivePresentation()
{
	return GetPresentationController().TrySkipActivePresentation();
}


void UT66GameplayHUDWidget::ClearActiveCratePresentation(UT66CrateOverlayWidget* Overlay)
{
	GetPresentationController().ClearActiveCratePresentation(Overlay);
}

void UT66GameplayHUDWidget::ClearActiveLootWheelPresentation(UT66LootWheelOverlayWidget* Overlay)
{
	GetPresentationController().ClearActiveLootWheelPresentation(Overlay);
}


int32 UT66GameplayHUDWidget::GetChestRewardRevealThresholdGold(const ET66Rarity Rarity) const
{
	return GetPresentationController().GetChestRewardRevealThresholdGold(Rarity);
}


ET66Rarity UT66GameplayHUDWidget::ResolveChestRewardDisplayedRarity(const int32 DisplayedGold) const
{
	return GetPresentationController().ResolveChestRewardDisplayedRarity(DisplayedGold);
}


void UT66GameplayHUDWidget::RefreshChestRewardVisualState()
{
	GetPresentationController().RefreshChestRewardVisualState();
}


void UT66GameplayHUDWidget::TickChestRewardPresentation(const float InDeltaTime)
{
	GetPresentationController().TickChestRewardPresentation(InDeltaTime);
}


void UT66GameplayHUDWidget::HideChestReward()
{
	GetPresentationController().HideChestReward();
}


void UT66GameplayHUDWidget::TryShowQueuedPresentation()
{
	GetPresentationController().TryShowQueuedPresentation();
}


void UT66GameplayHUDWidget::ShowPickupItemCard(FName ItemID, ET66ItemRarity ItemRarity)
{
	GetPresentationController().ShowPickupItemCard(ItemID, ItemRarity);
}


void UT66GameplayHUDWidget::ShowLootBagItemReveal(FName ItemID, ET66ItemRarity ItemRarity)
{
	GetPresentationController().ShowLootBagItemReveal(ItemID, ItemRarity);
}


void UT66GameplayHUDWidget::HidePickupCard()
{
	GetPresentationController().HidePickupCard();
}

