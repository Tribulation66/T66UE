// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/T66DataTypes.h"
#include "T66HeroBase.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;

/**
 * Base class for all playable heroes in Tribulation 66
 *
 * For prototyping: uses a colored cylinder mesh
 * For production: replace mesh with skeletal mesh from FBX
 */
UCLASS(Blueprintable)
class T66_API AT66HeroBase : public ACharacter
{
	GENERATED_BODY()

public:
	AT66HeroBase();

	/** The hero ID this actor represents (set at spawn time) */
	UPROPERTY(BlueprintReadWrite, Category = "Hero")
	FName HeroID;

	/** Cached hero data (populated on spawn) */
	UPROPERTY(BlueprintReadOnly, Category = "Hero")
	FHeroData HeroData;

	/** The placeholder cylinder mesh (visible during prototyping) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	/**
	 * Initialize this hero with data from the DataTable
	 * Called after spawning to set up visuals and stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Hero")
	void InitializeHero(const FHeroData& InHeroData);

	/**
	 * Set the placeholder color (for prototyping)
	 */
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void SetPlaceholderColor(FLinearColor Color);

protected:
	virtual void BeginPlay() override;

	/** Dynamic material instance for color changes */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PlaceholderMaterial;
};
