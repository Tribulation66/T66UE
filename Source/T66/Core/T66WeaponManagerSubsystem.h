// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66WeaponManagerSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnT66WeaponStateChanged);

/**
 * Owns the run's equipped auto-attack weapon and the current weapon altar offers.
 */
UCLASS()
class T66_API UT66WeaponManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	FOnT66WeaponStateChanged OnWeaponStateChanged;

	void ResetForNewRun(FName HeroID);
	void RestoreState(FName InEquippedWeaponID);

	FName GetEquippedWeaponID() const { return EquippedWeaponID; }
	bool GetEquippedWeaponData(FWeaponData& OutWeaponData) const;

	const TArray<FName>& GetWeaponOfferIDs() const { return WeaponOfferIDs; }
	ET66WeaponRarity GetCurrentOfferRarity() const { return CurrentOfferRarity; }

	void BuildWeaponOffers(FName HeroID, ET66WeaponRarity Rarity);
	bool SelectWeapon(FName WeaponID);
	bool IsWeaponOfferSelected(FName WeaponID) const { return !WeaponID.IsNone() && EquippedWeaponID == WeaponID; }

	static FName MakeStarterWeaponID(FName HeroID);
	static FName MakeWeaponID(FName HeroID, ET66WeaponRarity Rarity, ET66AttackCategory Branch);
	static ET66WeaponRarity GetUpgradeRarityForClearedDifficulty(ET66Difficulty Difficulty);
	static ET66WeaponRarity GetUpgradeRarityForStartingDifficulty(ET66Difficulty Difficulty);
	static FString WeaponRarityToString(ET66WeaponRarity Rarity);
	static FString AttackBranchToString(ET66AttackCategory Branch);
	static FLinearColor GetWeaponRarityColor(ET66WeaponRarity Rarity);

private:
	void BroadcastWeaponStateChanged();

	UPROPERTY(Transient)
	FName EquippedWeaponID = NAME_None;

	UPROPERTY(Transient)
	TArray<FName> WeaponOfferIDs;

	UPROPERTY(Transient)
	ET66WeaponRarity CurrentOfferRarity = ET66WeaponRarity::Black;
};
