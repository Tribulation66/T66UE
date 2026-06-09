// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66GameplayInteractable.h"
#include "T66WeaponAltar.generated.h"

class UBoxComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * Stage-start auto-attack weapon altar.
 * Appears only at the first stage of the selected difficulty and locks one
 * weapon branch for the run.
 */
UCLASS(Blueprintable)
class T66_API AT66WeaponAltar : public AT66GameplayInteractable
{
	GENERATED_BODY()

public:
	AT66WeaponAltar();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponAltar")
	TObjectPtr<UBoxComponent> InteractTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponAltar")
	TObjectPtr<UStaticMeshComponent> BaseRect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponAltar")
	TObjectPtr<UStaticMeshComponent> TopRect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponAltar")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponAltar")
	TSoftObjectPtr<UStaticMesh> AltarMeshOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAltar")
	float VisualScaleMultiplier = 3.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAltar|Interaction")
	FVector MinimumInteractionExtent = FVector(240.f, 240.f, 200.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponAltar|Interaction")
	FVector InteractionBoundsPadding = FVector(45.f, 45.f, 35.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponAltar")
	int32 RemainingSelections = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponAltar")
	ET66WeaponRarity WeaponOfferRarity = ET66WeaponRarity::Black;

	void ApplyVisuals();
	void LinkToTowerGateFloor(int32 FromFloorNumber);
	void NotifySelectionCommitted();

protected:
	virtual void BeginPlay() override;

private:
	void UpdateInteractionBounds();
	void ConfigureVisualCollision(UPrimitiveComponent* Primitive, bool bEnableCollision) const;

	int32 LinkedTowerGateFloorNumber = INDEX_NONE;
};
