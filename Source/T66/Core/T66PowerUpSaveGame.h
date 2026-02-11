// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66PowerUpSaveGame.generated.h"

/**
 * Persistent power-up progression: Power Crystals balance and unlocked slices per stat.
 * Stored in its own save slot (separate from profile).
 */
UCLASS(BlueprintType)
class T66_API UT66PowerUpSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 SaveVersion = 1;

	/** Current Power Crystals balance (earned from killing stage bosses). New saves start with 1000 for testing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerCrystalBalance = 1000;

	/** Unlocked slice count (0..10) per stat: Damage, AttackSpeed, AttackScale, Armor, Evasion, Luck. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerupSlicesDamage = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerupSlicesAttackSpeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerupSlicesAttackScale = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerupSlicesArmor = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerupSlicesEvasion = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerupSlicesLuck = 0;

	/** Random-stat unlock bonuses (no cap; each UnlockRandomStat adds 1 to a random stat). */
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
