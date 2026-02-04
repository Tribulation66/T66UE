// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiasmaBoundary.generated.h"

class UStaticMeshComponent;

/**
 * Wall of miasma at the playable area boundary. No collision (player can walk through).
 * When the player is on the "other side" (outside the safe rectangle), they take ticking damage.
 * Visual: thin dark planes at the four edges where ground borders empty space.
 */
UCLASS(Blueprintable)
class T66_API AT66MiasmaBoundary : public AActor
{
	GENERATED_BODY()

public:
	AT66MiasmaBoundary();

	/** Half-extent of the safe playable rectangle (centered at 0,0). Outside this = miasma damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miasma Boundary")
	float SafeHalfExtent = 18400.f;

	/** Damage interval while outside the boundary (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float DamageIntervalSeconds = 1.0f;

	/** Height of the visual wall (Z extent). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float WallHeight = 1200.f;

	/** Thickness of the visual wall (no collision). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float WallThickness = 200.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpawnWallVisuals();
	void ApplyBoundaryDamageTick();

	FTimerHandle DamageTimerHandle;
};
