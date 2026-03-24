// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66CircusInteractable.generated.h"

class UT66RunStateSubsystem;

UCLASS(Blueprintable)
class T66_API AT66CircusInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66CircusInteractable();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ApplyRarityVisuals() override;
	virtual bool ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const override;
	virtual float GetInteractionPromptWorldSize() const override { return 92.f; }
	virtual float GetInteractionPromptVerticalPadding() const override { return 360.f; }

private:
	UFUNCTION()
	void HandleBossStateChanged();

	UT66RunStateSubsystem* GetRunState() const;
};

