// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniCircusSubsystem.generated.h"

class UT66MiniDataSubsystem;
class UT66MiniRunSaveGame;

UCLASS()
class T66MINI_API UT66MiniCircusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ResetCircusState();
	void SeedFromRunSave(const UT66MiniRunSaveGame* RunSave);
	void WriteToRunSave(UT66MiniRunSaveGame* RunSave) const;
	void PrimeVendorOffers(const UT66MiniDataSubsystem* DataSubsystem);
	void ToggleVendorOfferLock(FName ItemID);
	bool IsVendorOfferLocked(FName ItemID) const;
	bool TryBuyOffer(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FName ItemID, FString& OutResult);
	bool TryStealOffer(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FName ItemID, FString& OutResult);
	bool TrySellOwnedItem(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FName ItemID, FString& OutResult);
	bool TryBuybackItem(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FName ItemID, FString& OutResult);
	bool TryRerollVendor(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult);
	bool TryBorrowGold(UT66MiniRunSaveGame* ActiveRun, int32 Amount, FString& OutResult);
	bool TryPayDebt(UT66MiniRunSaveGame* ActiveRun, int32 Amount, FString& OutResult);
	bool TryPlayGame(FName GameID, UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult);
	bool TryAlchemyTransmute(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult);
	bool TryAlchemyDissolveOldest(UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem, FString& OutResult);

	const TArray<FName>& GetCurrentVendorOfferIDs() const { return CurrentVendorOfferIDs; }
	const TArray<FName>& GetLockedVendorOfferIDs() const { return LockedVendorOfferIDs; }
	const TArray<FName>& GetBuybackItemIDs() const { return BuybackItemIDs; }
	int32 GetDebt() const { return CircusDebt; }
	float GetAnger01() const { return CircusAnger01; }
	int32 GetVendorRerollCount() const { return VendorRerollCount; }

private:
	void GenerateVendorOffers(const UT66MiniDataSubsystem* DataSubsystem, bool bCountAsReroll);
	void AddAnger(float Amount);
	void HandleBacklash(UT66MiniRunSaveGame* ActiveRun, FString& InOutResult);
	int32 CalculateStealSupportBonus(const UT66MiniRunSaveGame* ActiveRun, const UT66MiniDataSubsystem* DataSubsystem) const;

	UPROPERTY()
	TArray<FName> CurrentVendorOfferIDs;

	UPROPERTY()
	TArray<FName> LockedVendorOfferIDs;

	UPROPERTY()
	TArray<FName> BuybackItemIDs;

	UPROPERTY()
	int32 CircusDebt = 0;

	UPROPERTY()
	float CircusAnger01 = 0.f;

	UPROPERTY()
	int32 VendorRerollCount = 0;
};
