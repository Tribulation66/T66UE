// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/T66DataTypes.h"
#include "T66BuffSaveGame.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66TemporaryBuffPreset
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	FString PresetName = TEXT("Preset 1");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<ET66SecondaryStatType> SlotBuffs;

	FT66TemporaryBuffPreset()
	{
		SlotBuffs.Init(ET66SecondaryStatType::None, 5);
	}
};

/**
 * Persistent buff progression: permanent per-stat unlock steps plus pending single-use buffs.
 * Runtime now treats each stat as 10 binary fill steps (0=Locked, 1=Unlocked).
 * The saved array names remain from the old wedge system so legacy saves still deserialize cleanly.
 * Stored in its own save slot (separate from profile).
 */
UCLASS(BlueprintType)
class T66_API UT66BuffSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** SaveVersion 9 = added primary Accuracy progression; 8 = named temp-buff presets; 7 = selected secondary single-use buffs; 6 = secondary-stat single-use buffs; 5 = unified Chad Coupons buffs; 4 = 10 fill steps; 3 = 6 body-part unlocks; 2 = 10-slot wedge tiers; 1 = legacy slice counts. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 SaveVersion = 9;

	/** @deprecated Replaced by the shared Chad Coupons wallet in SaveVersion 5. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 PowerCrystalBalance = 0;

	/** Saved as legacy-named arrays, interpreted at runtime as fill-step states (0=Locked, 1=Unlocked). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersAttackSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersAttackScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> WedgeTiersAccuracy;

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
	int32 PowerupSlicesAccuracy = 0;
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
	int32 RandomBonusAccuracy = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusArmor = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusEvasion = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 RandomBonusLuck = 0;

	/** Owned temporary buff stack counts aligned to the 30 live secondary shop stats. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> PendingSingleUseBuffStates;

	/** Selected temporary buff stack counts that will actually activate for the next run. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<uint8> SelectedSingleUseBuffStates;

	/** Named preset loadouts for the 5 temporary-buff slots shown on hero selection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	TArray<FT66TemporaryBuffPreset> TemporaryBuffPresets;

	/** Which preset currently drives the active temporary-buff selection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
	int32 ActiveTemporaryBuffPresetIndex = 0;
};
