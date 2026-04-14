// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66MiniDataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniFrontendStateSubsystem.generated.h"

class UT66MiniDataSubsystem;
class UT66MiniRunSaveGame;

UCLASS()
class T66MINI_API UT66MiniFrontendStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ResetRunSetup();
	void BeginNewRun();
	void SeedFromRunSave(const UT66MiniRunSaveGame* RunSave);

	void SelectHero(FName HeroID);
	void SelectCompanion(FName CompanionID);
	void SelectDifficulty(FName DifficultyID);
	void SetPendingSaveSlot(int32 SlotIndex);
	void PrimeIdolOffers(const UT66MiniDataSubsystem* DataSubsystem);
	void RefreshIdolOffers(const UT66MiniDataSubsystem* DataSubsystem);
	void RerollIdolOffers(const UT66MiniDataSubsystem* DataSubsystem);
	bool AddIdolToLoadout(FName IdolID);
	void EnterIntermissionFlow(const UT66MiniDataSubsystem* DataSubsystem);
	void ResumeIntermissionFlow();
	void ExitIntermissionFlow();
	void PrimeShopOffers(const UT66MiniDataSubsystem* DataSubsystem);
	void RefillShopOffers(const UT66MiniDataSubsystem* DataSubsystem);
	void RerollShopOffers(const UT66MiniDataSubsystem* DataSubsystem);
	void ToggleShopOfferLock(FName ItemID);
	void ConsumeShopOffer(FName ItemID);
	void ClearTransientShopState();
	void WriteIntermissionStateToRunSave(UT66MiniRunSaveGame* RunSave) const;
	void SetLastRunSummary(const FT66MiniRunSummary& Summary) { LastRunSummary = Summary; }
	void ClearLastRunSummary() { LastRunSummary = FT66MiniRunSummary(); }

	FName GetSelectedHeroID() const { return SelectedHeroID; }
	FName GetSelectedCompanionID() const { return SelectedCompanionID; }
	FName GetSelectedDifficultyID() const { return SelectedDifficultyID; }
	int32 GetPendingSaveSlot() const { return PendingSaveSlot; }
	const TArray<FName>& GetSelectedIdolIDs() const { return SelectedIdolIDs; }
	const TArray<FName>& GetCurrentIdolOfferIDs() const { return CurrentIdolOfferIDs; }
	const TArray<FName>& GetCurrentShopOfferIDs() const { return CurrentShopOfferIDs; }
	const TArray<FName>& GetLockedShopOfferIDs() const { return LockedShopOfferIDs; }
	int32 GetIdolRerollCount() const { return IdolRerollCount; }
	int32 GetShopRerollCount() const { return ShopRerollCount; }
	bool IsLoadFlow() const { return bLoadFlow; }
	bool IsIntermissionFlow() const { return bIntermissionFlow; }
	bool IsShopOfferLocked(const FName ItemID) const { return LockedShopOfferIDs.Contains(ItemID); }
	const FT66MiniRunSummary& GetLastRunSummary() const { return LastRunSummary; }
	bool ConsumeShouldSkipNextShopPrime();

	bool HasSelectedHero() const { return SelectedHeroID != NAME_None; }
	bool HasSelectedCompanion() const { return SelectedCompanionID != NAME_None; }
	bool HasSelectedDifficulty() const { return SelectedDifficultyID != NAME_None; }
	bool HasCompleteIdolLoadout() const { return SelectedIdolIDs.Num() >= MaxIdolSlots; }

	static constexpr int32 MaxIdolSlots = 4;

private:
	void GenerateIdolOffers(const UT66MiniDataSubsystem* DataSubsystem, bool bCountAsReroll);
	void GenerateShopOffers(const UT66MiniDataSubsystem* DataSubsystem, bool bCountAsReroll);

	UPROPERTY()
	FName SelectedHeroID = NAME_None;

	UPROPERTY()
	FName SelectedCompanionID = NAME_None;

	UPROPERTY()
	FName SelectedDifficultyID = NAME_None;

	UPROPERTY()
	TArray<FName> SelectedIdolIDs;

	UPROPERTY()
	TArray<FName> CurrentIdolOfferIDs;

	UPROPERTY()
	int32 PendingSaveSlot = INDEX_NONE;

	UPROPERTY()
	int32 IdolRerollCount = 0;

	UPROPERTY()
	bool bLoadFlow = false;

	UPROPERTY()
	TArray<FName> CurrentShopOfferIDs;

	UPROPERTY()
	TArray<FName> LockedShopOfferIDs;

	UPROPERTY()
	int32 ShopRerollCount = 0;

	UPROPERTY()
	bool bIntermissionFlow = false;

	UPROPERTY()
	bool bSkipNextShopPrime = false;

	UPROPERTY()
	FT66MiniRunSummary LastRunSummary;
};
