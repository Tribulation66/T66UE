// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66PowerUpSubsystem.generated.h"

class UT66PowerUpSaveGame;

/** Wedge tier: 0=Empty, 1=Black, 2=Red, 3=Yellow, 4=White. */
enum class ET66PowerUpWedgeTier : uint8
{
	Empty = 0,
	Black = 1,
	Red = 2,
	Yellow = 3,
	White = 4
};

/**
 * Power-up progression: Power Coupons balance and per-wedge tiers (Black/Red/Yellow/White).
 * Persists in its own save slot (separate from profile).
 */
UCLASS()
class T66_API UT66PowerUpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString PowerUpSaveSlotName;
	static constexpr int32 PowerUpSaveUserIndex = 0;

	/** Max wedges per stat. */
	static constexpr int32 MaxSlicesPerStat = 10;

	/** Cost in Power Coupons per tier step: Empty->Black 1, Black->Red 3, Red->Yellow 5, Yellow->White 10. */
	static int32 GetCostForTierStep(uint8 FromTier);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Current Power Coupons balance. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetPowerCrystalBalance() const;

	/** Add Power Coupons (e.g. at run end from boss kills). Persists. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void AddPowerCrystals(int32 Amount);

	/** Get wedge tier (0..4) for the given stat and wedge index (0..9). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetWedgeTier(ET66HeroStatType StatType, int32 WedgeIndex) const;

	/** Cost in PC for the next upgrade on this stat (0 if maxed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetCostForNextUpgrade(ET66HeroStatType StatType) const;

	/** Unlock next wedge upgrade for the given stat (lowest-index wedge not yet White). Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockNextWedgeUpgrade(ET66HeroStatType StatType);

	/** Unlock one random stat: add one Black wedge to first empty wedge of a random stat. Cost 1 PC. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockRandomStat();

	/** True if all 10 wedges for this stat are White. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	bool IsStatMaxed(ET66HeroStatType StatType) const;

	/** All powerup bonuses as flat stat points (for RunState stat getters). Each wedge tier >= Black counts as +1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	FT66HeroStatBonuses GetPowerupStatBonuses() const;

private:
	UPROPERTY()
	TObjectPtr<UT66PowerUpSaveGame> SaveData;

	void LoadOrCreateSave();
	void MigrateV1ToV2WedgeTiers();
	void Save();
	TArray<uint8>* GetWedgeTiersForStat(ET66HeroStatType StatType);
	const TArray<uint8>* GetWedgeTiersForStat(ET66HeroStatType StatType) const;
	void EnsureWedgeTiersSize(TArray<uint8>& Arr);
	int32 GetRandomBonusForStat(ET66HeroStatType StatType) const;
	void AddRandomBonusToStat(ET66HeroStatType StatType);
};
