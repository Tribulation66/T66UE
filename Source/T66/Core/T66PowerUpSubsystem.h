// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66PowerUpSubsystem.generated.h"

class UT66PowerUpSaveGame;

/**
 * Power-up progression: Power Crystals balance and unlockable stat slices.
 * Persists in its own save slot (separate from profile).
 */
UCLASS()
class T66_API UT66PowerUpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString PowerUpSaveSlotName;
	static constexpr int32 PowerUpSaveUserIndex = 0;

	/** Cost in Power Crystals to unlock one slice of a stat (or one random stat). */
	static constexpr int32 CrystalsPerSlice = 20;

	/** Max slices per stat (0..10). */
	static constexpr int32 MaxSlicesPerStat = 10;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Current Power Crystals balance. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetPowerCrystalBalance() const;

	/** Add Power Crystals (e.g. at run end from boss kills). Persists. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void AddPowerCrystals(int32 Amount);

	/** Unlock one slice for the given stat. Costs CrystalsPerSlice. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockPowerupSlice(ET66HeroStatType StatType);

	/** Unlock one random stat (infinite uses). Costs CrystalsPerSlice. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockRandomStat();

	/** Number of slices unlocked (0..10) for the given stat. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetPowerupSlicesUnlocked(ET66HeroStatType StatType) const;

	/** All powerup bonuses as flat stat points (for RunState stat getters). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	FT66HeroStatBonuses GetPowerupStatBonuses() const;

private:
	UPROPERTY()
	TObjectPtr<UT66PowerUpSaveGame> SaveData;

	void LoadOrCreateSave();
	void Save();
	int32 GetSliceCountForStat(ET66HeroStatType StatType) const;
	void SetSliceCountForStat(ET66HeroStatType StatType, int32 Count);
	int32 GetRandomBonusForStat(ET66HeroStatType StatType) const;
	void AddRandomBonusToStat(ET66HeroStatType StatType);
};
