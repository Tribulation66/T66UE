// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "Data/T66DataTypes.h"
#include "T66PetCaptureInteractable.generated.h"

/** Cute post-boss interactable that performs guaranteed pet capture. */
UCLASS(Blueprintable)
class T66_API AT66PetCaptureInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66PetCaptureInteractable();

	UPROPERTY(BlueprintReadOnly, Category = "Pet Capture")
	FName PetID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Pet Capture")
	FName SourceBossID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Pet Capture")
	FPetData PetData;

	UFUNCTION(BlueprintCallable, Category = "Pet Capture")
	void InitializePetCapture(const FPetData& InPetData);

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(210.f, 210.f, 150.f); }
	virtual FVector GetImportedVisualScale() const override { return FVector(0.42f); }

private:
	void ApplyPetCaptureVisual();
};
