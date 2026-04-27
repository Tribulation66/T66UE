// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"
#include "Gameplay/T66PilotableTractor.h"
#include "T66ArcadeTruckInteractable.generated.h"

UCLASS(Blueprintable)
class T66_API AT66ArcadeTruckInteractable : public AT66PilotableTractor
{
	GENERATED_BODY()

public:
	AT66ArcadeTruckInteractable();

	const FT66ArcadeInteractableData& GetArcadeData() const { return ResolvedArcadeData; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual FVector GetImportedVisualScale() const override;

	void RefreshResolvedArcadeData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arcade")
	FName ArcadeRowID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arcade")
	FT66ArcadeInteractableData ArcadeData;

	UPROPERTY(Transient)
	FT66ArcadeInteractableData ResolvedArcadeData;

private:
	void ApplyArcadeTruckTuning();
	FText ResolvePromptVerb() const;
};
