// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66LootPinata.generated.h"

class AT66LootBagPickup;
class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * Attack-opened world interactable.
 *
 * This is intentionally not an auto-attack target. The player must manually
 * lock it with attack-lock/left click, then normal weapon range and line-of-sight
 * checks decide whether attacks can hit it.
 */
UCLASS(Blueprintable)
class T66_API AT66LootPinata : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66LootPinata();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinata|Combat", meta = (ClampMin = "1"))
	int32 MaxHealth = 80;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pinata|Combat")
	int32 CurrentHealth = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinata|Loot", meta = (ClampMin = "1"))
	int32 LootBagCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinata|Loot", meta = (ClampMin = "0.0"))
	float LootExplosionRadius = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinata|Loot", meta = (ClampMin = "0.0"))
	float LootSpawnHeight = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinata|Loot")
	bool bRollLootRarity = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinata|Loot")
	ET66Rarity FixedLootRarity = ET66Rarity::Yellow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pinata|Visuals")
	TObjectPtr<UStaticMeshComponent> LockIndicatorMesh;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pinata|Combat")
	bool IsAliveAndTargetable() const;

	UFUNCTION(BlueprintCallable, Category = "Pinata|Combat")
	bool TakeDamageFromHero(int32 DamageAmount, FName DamageSourceID = NAME_None, FName EventType = NAME_None);

	bool TakeDamageFromHeroHitZone(int32 DamageAmount, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID = NAME_None, FName EventType = NAME_None);

	FT66CombatTargetHandle ResolveCombatTargetHandle(UPrimitiveComponent* PreferredComponent = nullptr, ET66HitZoneType PreferredHitZone = ET66HitZoneType::Body) const;

	void SetLockedIndicator(bool bLocked);

	virtual bool Interact(APlayerController* PC) override;

protected:
	virtual void BeginPlay() override;
	virtual void ApplyRarityVisuals() override;
	virtual bool ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const override;
	virtual FVector GetMinimumInteractionExtent() const override { return FVector(180.f, 180.f, 180.f); }
	virtual FVector GetInteractionBoundsPadding() const override { return FVector(80.f, 80.f, 80.f); }

private:
	void OpenPinata();
	void SpawnLootBags();
	ET66Rarity RollLootRarity(FRandomStream& Stream) const;
};
