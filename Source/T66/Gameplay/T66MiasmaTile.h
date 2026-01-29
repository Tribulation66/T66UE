// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiasmaTile.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** Single thin miasma sheet tile that damages the hero while standing on it. */
UCLASS(Blueprintable)
class T66_API AT66MiasmaTile : public AActor
{
	GENERATED_BODY()

public:
	AT66MiasmaTile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	/** Size of tile in world units (square). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float TileSize = 750.f;

	/** Damage interval while overlapping (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float DamageIntervalSeconds = 1.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyMiasmaDamageTick();

	bool bHeroOverlapping = false;
	FTimerHandle DamageTimerHandle;
};

