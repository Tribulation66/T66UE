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

	virtual void Tick(float DeltaSeconds) override;
	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual float GetInteractionPromptVerticalPadding() const override { return 160.f; }
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(280.f, 280.f, 220.f); }
	virtual FVector GetInteractionBoundsPadding() const override { return FVector(140.f, 140.f, 100.f); }
	virtual FVector GetImportedVisualScale() const override { return FVector(0.72f, 0.72f, 0.72f); }

private:
	void GrantGoldReward(APlayerController* PC);
	void GrantItemReward(APlayerController* PC);
	void GrantBoostReward(APlayerController* PC, FRandomStream& Rng);
};
