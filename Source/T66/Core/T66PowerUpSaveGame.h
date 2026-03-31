// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66PowerUpSaveGame.generated.h"

/**
 * Persistent power-up progression: Power Coupons balance and per-stat unlock steps.
 * Runtime now treats each stat as 10 binary fill steps (0=Locked, 1=Unlocked).
 * The saved array names remain from the old wedge system so legacy saves still deserialize cleanly.
 * Stored in its own save slot (separate from profile).
 */
UCLASS(BlueprintType)
class T66_API UT66PowerUpSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** SaveVersion 4 = 10 fill steps; 3 = 6 body-part unlocks; 2 = 10-slot wedge tiers; 1 = legacy slice counts. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 SaveVersion = 4;

	/** Current Power Coupons balance. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerCrystalBalance = 1000;

	/** Saved as legacy-named arrays, interpreted at runtime as fill-step states (0=Locked, 1=Unlocked). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersAttackSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersAttackScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersArmor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersEvasion;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersLuck;

	/** Legacy (v1): slice counts from before per-slot state was introduced. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp", meta = (DeprecatedProperty))
	int32 PowerupSlicesDamage = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp", meta = (DeprecatedProperty))
	int32 PowerupSlicesAttackSpeed = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp", meta = (DeprecatedProperty))
	int32 PowerupSlicesAttackScale = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp", meta = (DeprecatedProperty))
	int32 PowerupSlicesArmor = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp", meta = (DeprecatedProperty))
	int32 PowerupSlicesEvasion = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp", meta = (DeprecatedProperty))
	int32 PowerupSlicesLuck = 0;

	/** Extra flat bonuses that do not map to visible fill. Also stores overflow above the 10 visible steps. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusDamage = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusAttackSpeed = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusAttackScale = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusArmor = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusEvasion = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusLuck = 0;
};
