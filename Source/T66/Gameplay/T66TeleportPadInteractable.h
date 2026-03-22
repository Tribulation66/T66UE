// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66TeleportPadInteractable.generated.h"

class UCapsuleComponent;

UCLASS(Blueprintable)
class T66_API AT66TeleportPadInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66TeleportPadInteractable();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
	virtual float GetInteractionPromptWorldSize() const override { return 88.f; }
	virtual float GetInteractionPromptVerticalPadding() const override { return 180.f; }

private:
	AT66TeleportPadInteractable* ChooseDestinationPad() const;
};
