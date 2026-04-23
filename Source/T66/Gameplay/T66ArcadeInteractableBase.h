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
	void HandleArcadePopupClosed(bool bSucceeded);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void ApplyRarityVisuals() override;
	virtual bool ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FVector GetImportedVisualScale() const override;

	void RefreshResolvedArcadeData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arcade")
	FName ArcadeRowID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arcade")
	FT66ArcadeInteractableData ArcadeData;

	bool bArcadeSessionActive = false;

	UPROPERTY(Transient)
	FT66ArcadeInteractableData ResolvedArcadeData;

private:
	FText ResolveInteractionVerb() const;
};
