// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66CasinoInteractable.generated.h"

/** One-use casino kiosk: opens the gambler-only casino games overlay. */
UCLASS(Blueprintable)
class T66_API AT66CasinoInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66CasinoInteractable();

	virtual bool Interact(APlayerController* PC) override;

	int32 GetWinGoldAmount() const { return WinGoldAmount; }
	void HandleCasinoGambleCompleted();

protected:
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual FVector GetImportedVisualScale() const override;
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(320.f, 320.f, 260.f); }

private:
	UPROPERTY(VisibleAnywhere, Category = "SafeZone")
	TObjectPtr<class UT66SafeZoneComponent> SafeZoneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Casino", meta = (AllowPrivateAccess = "true"))
	int32 WinGoldAmount = 10;
};
