// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66IdolAltar.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class ET66AltarOfferMode : uint8
{
	Idols UMETA(DisplayName = "Idols"),
	Weapons UMETA(DisplayName = "Weapons"),
};

/**
 * Stage-entry Idol Altar.
 * Visual: 3 stacked rectangles (pyramid feel).
 * Interaction: Press Interact near it to open the Idol Altar overlay.
 */
UCLASS(Blueprintable)
class T66_API AT66IdolAltar : public AActor
{
	GENERATED_BODY()

public:
	AT66IdolAltar();

	/** Interact radius/trigger. We use PlayerController sphere overlap, so this just needs to overlap Pawn. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UBoxComponent> InteractTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> BaseRect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> MidRect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> TopRect;

	/** Optional imported altar mesh (if set, we hide the 3-rect placeholder stack). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IdolAltar")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, Category = "IdolAltar")
	TSoftObjectPtr<UStaticMesh> AltarMeshOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IdolAltar")
	float VisualScaleMultiplier = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IdolAltar|Interaction")
	FVector MinimumInteractionExtent = FVector(240.f, 240.f, 200.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IdolAltar|Interaction")
	FVector InteractionBoundsPadding = FVector(45.f, 45.f, 35.f);

	/** Remaining selections granted by this altar visit. The altar destroys itself when this hits zero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	int32 RemainingSelections = 1;

	/** Whether this altar offers idols or auto-attack weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	ET66AltarOfferMode OfferMode = ET66AltarOfferMode::Idols;

	/** Rarity used when OfferMode is Weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	ET66WeaponRarity WeaponOfferRarity = ET66WeaponRarity::Black;

	/** Extra selections granted to catch up with skipped starting stages on higher difficulties. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	int32 CatchUpSelectionsRemaining = 0;

	/** Tutorial mode: expose a single authored idol instead of the shared altar stock. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	bool bUseTutorialSingleOffer = false;

	/** Idol offered when tutorial single-offer mode is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IdolAltar")
	FName TutorialOfferedIdolID = FName(TEXT("Idol_Electric"));

	bool DidSelectedStockSlotConsumeCatchUp(int32 SlotIndex) const
	{
		return SelectedStockSlotsConsumedCatchUp.IsValidIndex(SlotIndex) && SelectedStockSlotsConsumedCatchUp[SlotIndex];
	}

	void SetSelectedStockSlotConsumedCatchUp(int32 SlotIndex, bool bConsumed)
	{
		if (SlotIndex < 0)
		{
			return;
		}

		if (!SelectedStockSlotsConsumedCatchUp.IsValidIndex(SlotIndex))
		{
			SelectedStockSlotsConsumedCatchUp.SetNumZeroed(SlotIndex + 1);
		}

		SelectedStockSlotsConsumedCatchUp[SlotIndex] = bConsumed;
	}

	bool DidTutorialOfferConsumeCatchUp() const { return bTutorialOfferConsumedCatchUp; }
	void SetTutorialOfferConsumedCatchUp(bool bConsumed) { bTutorialOfferConsumedCatchUp = bConsumed; }

	/** Apply simple placeholder visuals (color/material). */
	void ApplyVisuals();

protected:
	virtual void BeginPlay() override;

private:
	void UpdateInteractionBounds();
	void ConfigureVisualCollision(UPrimitiveComponent* Primitive, bool bEnableCollision) const;

	TArray<bool> SelectedStockSlotsConsumedCatchUp;
	bool bTutorialOfferConsumedCatchUp = false;
};

