// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66BoostInteractable.generated.h"

UCLASS(Blueprintable)
class T66_API AT66BoostInteractable : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66BoostInteractable();

	virtual void Tick(float DeltaSeconds) override;
	virtual bool Interact(APlayerController* PC) override;

	UFUNCTION(BlueprintCallable, Category = "Boost")
	void ConfigureBoost(ET66HeroStatType InStatType, int32 InBonusStatPoints, float InDurationSeconds);

	UFUNCTION(BlueprintCallable, Category = "Boost")
	void ConfigureSecondaryBoost(ET66SecondaryStatType InStatType, int32 InBonusStatPoints, float InDurationSeconds);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	ET66HeroStatType StatType = ET66HeroStatType::Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	ET66SecondaryStatType SecondaryStatType = ET66SecondaryStatType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	bool bUseSecondaryStat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	int32 BonusStatPoints = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float DurationSeconds = 10.f;

protected:
	virtual void ApplyRarityVisuals() override;
	virtual FText BuildInteractionPromptText() const override;
	virtual FText BuildInteractionPromptTargetName() const override;
	virtual float GetInteractionPromptVerticalPadding() const override { return 148.f; }
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(220.f, 220.f, 180.f); }
	virtual FVector GetInteractionBoundsPadding() const override { return FVector(120.f, 120.f, 90.f); }
	virtual FVector GetImportedVisualScale() const override { return FVector(0.82f, 0.82f, 0.82f); }

private:
	void RefreshMeshForStatType();
	FText ResolveStatDisplayName() const;
};
