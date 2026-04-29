// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66ArcadeInteractableBase.generated.h"

UCLASS(Abstract, Blueprintable)
class T66_API AT66ArcadeInteractableBase : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66ArcadeInteractableBase();

	virtual bool Interact(APlayerController* PC) override;

	const FT66ArcadeInteractableData& GetArcadeData() const { return ResolvedArcadeData; }
	TArray<FT66ArcadeInteractableData> BuildArcadeSelectionOptions() const;
	bool BuildArcadeSessionDataForGame(ET66ArcadeGameType GameType, FT66ArcadeInteractableData& OutData) const;
	void HandleArcadePopupClosed(bool bSucceeded, int32 FinalScore);
	void HandleArcadePopupDismissedWithoutResult();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void ApplyRarityVisuals() override;
	virtual bool ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual FVector GetImportedVisualScale() const override;

	void RefreshResolvedArcadeData();
	FT66ArcadeInteractableData BuildArcadeSessionData() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arcade")
	FName ArcadeRowID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arcade")
	FT66ArcadeInteractableData ArcadeData;

	bool bArcadeSessionActive = false;

	UPROPERTY(Transient)
	FT66ArcadeInteractableData ResolvedArcadeData;

private:
	FText ResolveInteractionVerb() const;
	ET66ArcadeGameType ResolveRandomGameType(const FT66ArcadeInteractableData& Data) const;
	void SpawnCompletionRewards(int32 FinalScore);
	void SpawnLootBagReward(const FVector& SpawnLocation, FRandomStream& Rng);
	void SpawnChestReward(const FVector& SpawnLocation);
	void SpawnWeaponCrateReward(const FVector& SpawnLocation, FRandomStream& Rng);
	void SpawnAmplifierReward(const FVector& SpawnLocation, FRandomStream& Rng, int32 BonusStatPoints, float DurationSeconds);
	FVector BuildRewardSpawnLocation(int32 RewardIndex, int32 RewardCount) const;
	void PlayCompletionEffects(int32 FinalScore);
};
