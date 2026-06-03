// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66VendorInteractable.generated.h"

/** Guaranteed floor vendor: opens the vendor-only shop overlay. */
UCLASS(Blueprintable)
class T66_API AT66VendorInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66VendorInteractable();

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual FVector GetImportedVisualScale() const override;
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(320.f, 320.f, 260.f); }

private:
	UPROPERTY(VisibleAnywhere, Category = "SafeZone")
	TObjectPtr<class UT66SafeZoneComponent> SafeZoneComponent;
};
