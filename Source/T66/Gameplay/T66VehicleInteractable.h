// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66VehicleInteractableTypes.h"
#include "T66VehicleInteractable.generated.h"

UCLASS(Blueprintable)
class T66_API AT66VehicleInteractable : public AT66PilotableTractor
{
	GENERATED_BODY()

public:
	AT66VehicleInteractable();

	const FT66VehicleInteractableData& GetVehicleData() const { return ResolvedVehicleData; }
	void SetVehicleRowID(FName InVehicleRowID);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual FVector GetImportedVisualScale() const override;

	void RefreshResolvedVehicleData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle")
	FName VehicleRowID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle")
	FT66VehicleInteractableData VehicleData;

	UPROPERTY(Transient)
	FT66VehicleInteractableData ResolvedVehicleData;

private:
	void ApplyVehicleTuning();
	FText ResolvePromptVerb() const;
};
