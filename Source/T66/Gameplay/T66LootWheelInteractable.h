// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66LootWheelInteractable.generated.h"

UCLASS(Blueprintable)
class T66_API AT66LootWheelInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66LootWheelInteractable();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual float GetInteractionPromptVerticalPadding() const override { return 160.f; }
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(280.f, 280.f, 220.f); }
	virtual FVector GetInteractionBoundsPadding() const override { return FVector(140.f, 140.f, 100.f); }
	virtual FVector GetImportedVisualScale() const override { return FVector(0.72f, 0.72f, 0.72f); }

public:
	enum class ELockedLootWheelRewardType : uint8
	{
		Gold,
		Item,
		Boost,
	};

private:
	struct FLockedLootWheelReward
	{
		bool bLocked = false;
		bool bCommitAttempted = false;
		ELockedLootWheelRewardType RewardType = ELockedLootWheelRewardType::Gold;
		ET66Rarity WheelRarity = ET66Rarity::Black;
		int32 Gold = 0;
		int32 MinGold = 0;
		int32 MaxGold = 0;
		int32 DrawIndex = INDEX_NONE;
		int32 PreDrawSeed = 0;
		FName ItemID = NAME_None;
		ET66ItemRarity ItemRarity = ET66ItemRarity::Black;
		ET66HeroStatType BoostStatType = ET66HeroStatType::Damage;
		ET66SecondaryStatType BoostSecondaryStatType = ET66SecondaryStatType::None;
		bool bBoostUsesSecondaryStat = false;
		int32 BoostBonusStatPoints = 8;
		float BoostDurationSeconds = 10.f;
		TWeakObjectPtr<APlayerController> PlayerController;
	};

	bool LockLootWheelReward(APlayerController* PC);
	void CommitLockedLootWheelRewardIfNeeded();
	void PresentLockedLootWheelReward();
	void HandleLootWheelSpinCommit();
	void HandleLootWheelSpinFinished();
	void PresentLockedLootWheelRewardAfterSpin();
	void FinishLootWheelInteraction();

	void GrantGoldReward(APlayerController* PC);
	void GrantItemReward(APlayerController* PC);
	void GrantBoostReward(APlayerController* PC, FRandomStream& Rng);

	FLockedLootWheelReward LockedReward;
	bool bWheelResultPresented = false;
};
