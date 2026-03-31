// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66PowerUpSubsystem.generated.h"

class UT66PowerUpSaveGame;

/** Fill-step state: 0=Locked, 1=Unlocked. */
enum class ET66PowerUpFillStepState : uint8
{
	Locked = 0,
	Unlocked = 1
};

/**
 * Power-up progression: Power Coupons balance and per-stat statue fill unlocks.
 * Persists in its own save slot (separate from profile).
 */
UCLASS()
class T66_API UT66PowerUpSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const FString PowerUpSaveSlotName;
	static constexpr int32 PowerUpSaveUserIndex = 0;

	/** Visible fill steps per statue: 10 slices from bottom to top. */
	static constexpr int32 MaxFillStepsPerStat = 10;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Current Power Coupons balance. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetPowerCrystalBalance() const;

	/** Add Power Coupons (e.g. at run end from boss kills). Persists. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void AddPowerCrystals(int32 Amount);

	/** Spend Power Coupons. Returns true if balance was sufficient and amount was deducted. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool SpendPowerCrystals(int32 Amount);

	/** Get fill-step state (0=Locked, 1=Unlocked) for the given stat and step index (0..9). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetFillStepState(ET66HeroStatType StatType, int32 StepIndex) const;

	/** Number of visible fill steps currently unlocked for this stat. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetUnlockedFillStepCount(ET66HeroStatType StatType) const;

	/** Flat stat bonus from this stat's fill steps plus any non-visual overflow bonus. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetTotalStatBonus(ET66HeroStatType StatType) const;

	/** Cost in PC for the next visible fill-step unlock on this stat (0 if maxed). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetCostForNextFillStepUnlock(ET66HeroStatType StatType) const;

	/** Unlock the next locked fill step for the given stat. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockNextFillStep(ET66HeroStatType StatType);

	/** Unlock one random stat: reveal its next locked fill step. Cost 1 PC. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockRandomStat();

	/** True if all 10 visible fill steps for this stat are unlocked. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	bool IsStatMaxed(ET66HeroStatType StatType) const;

	/** All powerup bonuses as flat stat points (for RunState stat getters). Each unlocked fill step counts as +1. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	FT66HeroStatBonuses GetPowerupStatBonuses() const;

private:
	static constexpr int32 LegacyV2SlotsPerStat = 10;

	UPROPERTY()
	TObjectPtr<UT66PowerUpSaveGame> SaveData;

	void LoadOrCreateSave();
	void MigrateV1ToV2WedgeTiers();
	void MigrateV2ToV3BodyParts();
	void MigrateV3ToV4FillSteps();
	void Save();
	TArray<uint8>* GetFillStepStatesForStat(ET66HeroStatType StatType);
	const TArray<uint8>* GetFillStepStatesForStat(ET66HeroStatType StatType) const;
	void EnsureFillStepStatesSize(TArray<uint8>& Arr);
	int32 GetRandomBonusForStat(ET66HeroStatType StatType) const;
};
