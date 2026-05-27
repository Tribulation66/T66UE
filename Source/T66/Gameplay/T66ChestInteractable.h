// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66ChestInteractable.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/**
 * Treasure Chest: reward rarity is rolled on open, or it spawns a Mimic.
 */
UCLASS(Blueprintable)
class T66_API AT66ChestInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66ChestInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	bool bIsMimic = false;

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ApplyRarityVisuals() override;

private:
	struct FLockedChestReward
	{
		bool bLocked = false;
		bool bCommitAttempted = false;
		ET66Rarity RewardRarity = ET66Rarity::Black;
		int32 Gold = 0;
		int32 MinGold = 0;
		int32 MaxGold = 0;
		int32 DrawIndex = INDEX_NONE;
		int32 PreDrawSeed = 0;
		TWeakObjectPtr<APlayerController> PlayerController;
	};

	bool LockChestReward(APlayerController* PC);
	void PresentLockedChestReward();
	void CommitLockedChestRewardIfNeeded();
	void FinishChestInteraction();

	FLockedChestReward LockedReward;
	bool bChestRewardPresentationStarted = false;
};
