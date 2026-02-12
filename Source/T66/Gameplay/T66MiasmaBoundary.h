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

	/** Half-extent of the safe playable rectangle (centered at 0,0). Outside this = miasma damage. 100k map = 50000. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miasma Boundary")
	float SafeHalfExtent = 50000.f;

	/** Damage interval while outside the boundary (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float DamageIntervalSeconds = 1.0f;

	/** Height of the visual wall (Z extent). Black smoke-like pass-through boundary. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float WallHeight = 2400.f;

	/** Thickness of the visual wall (no collision). Thicker = more visible black smoke look. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma Boundary")
	float WallThickness = 500.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpawnWallVisuals();
	void ApplyBoundaryDamageTick();

	FTimerHandle DamageTimerHandle;
};
