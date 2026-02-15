// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66PowerUpSaveGame.generated.h"

/**
 * Persistent power-up progression: Power Coupons balance and per-wedge tiers per stat.
 * Each stat has 10 wedges; each wedge has tier 0=Empty, 1=Black, 2=Red, 3=Yellow, 4=White.
 * Costs: Empty->Black 1 PC, Black->Red 3, Red->Yellow 5, Yellow->White 10.
 * Stored in its own save slot (separate from profile).
 */
UCLASS(BlueprintType)
class T66_API UT66PowerUpSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** SaveVersion 2 = wedge tiers; 1 = legacy slice counts (migrated on load). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 SaveVersion = 2;

	/** Current Power Coupons balance. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerCrystalBalance = 1000;

	/** Per-wedge tier (0=Empty, 1=Black, 2=Red, 3=Yellow, 4=White). 10 wedges per stat. */
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

	/** Legacy (v1): migrated to WedgeTiers* as Black on first load of v2. */
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

	/** Random-stat unlock bonuses (UnlockRandomStat adds 1 Black wedge to a random stat). */
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
