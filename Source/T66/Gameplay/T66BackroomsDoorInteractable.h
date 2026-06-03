// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66BackroomsDoorInteractable.generated.h"

class AT66HeroBase;

/** Texture-backed door used for the Backrooms entrance, exit, and closed return door. */
UCLASS(Blueprintable)
class T66_API AT66BackroomsDoorInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66BackroomsDoorInteractable();

	void InitializeBackroomsDoor(bool bInExitDoor, bool bInVisualOnlyClosedDoor = false);
	bool IsExitDoor() const { return bExitDoor; }
	bool IsVisualOnlyClosedDoor() const { return bVisualOnlyClosedDoor; }
	void SetDoorConsumed();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
	virtual bool ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual float GetInteractionPromptWorldSize() const override { return 72.f; }
	virtual float GetInteractionPromptVerticalPadding() const override { return 210.f; }
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(300.f, 170.f, 300.f); }
	virtual FVector GetInteractionBoundsPadding() const override { return FVector(70.f, 70.f, 90.f); }

private:
	UPROPERTY(Transient)
	bool bExitDoor = false;

	UPROPERTY(Transient)
	bool bVisualOnlyClosedDoor = false;
};
