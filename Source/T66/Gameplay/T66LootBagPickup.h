// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66Rarity.h"
#include "GameFramework/Actor.h"
#include "T66LootBagPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UStaticMesh;
class AT66PlayerController;

UCLASS(Blueprintable)
class T66_API AT66LootBagPickup : public AActor
{
	GENERATED_BODY()

public:
	AT66LootBagPickup();

	UPROPERTY(BlueprintReadWrite, Category = "Loot")
	FName ItemID;

	UPROPERTY(BlueprintReadWrite, Category = "Loot")
	ET66Rarity LootRarity = ET66Rarity::Black;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetItemID(FName InItemID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	FName GetItemID() const { return ItemID; }

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetLootRarity(ET66Rarity InRarity);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Loot")
	ET66Rarity GetLootRarity() const { return LootRarity; }

	/** Destroy the loot bag after accept/reject. */
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void ConsumeAndDestroy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** Optional rarity meshes (when imported). If null/unloaded, we fall back to the colored cube. */
	UPROPERTY(EditDefaultsOnly, Category = "Visuals|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshBlack;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshRed;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshYellow;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals|Meshes")
	TSoftObjectPtr<UStaticMesh> MeshWhite;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdateVisualsFromRarity();
};

