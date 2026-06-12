// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Shutdown/T66ShutdownSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66BuffSubsystem.generated.h"

class UT66BuffSaveGame;

USTRUCT(BlueprintType)
struct T66_API FT66SurgeryDefinition
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Surgery")
	FName SurgeryID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Surgery")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Surgery")
	ET66HeroStatType BaseStatType = ET66HeroStatType::Damage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Surgery")
	ET66StatType StatType = ET66StatType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Surgery")
	bool bUsesStat = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Surgery")
	int32 BonusStatPoints = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Surgery")
	int32 CostCC = 10;
};

/** Fill-step state: 0=Locked, 1=Unlocked. */
enum class ET66BuffFillStepState : uint8
{
	Locked = 0,
	Unlocked = 1
};

/**
 * Buff progression: permanent per-stat unlocks plus owned/selected temporary buff stacks.
 * Persists in its own save slot (separate from profile).
 */
UCLASS()
class T66_API UT66BuffSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Legacy slot name retained so existing buff progression still loads after the class rename. */
	static const FString BuffSaveSlotName;
	static constexpr int32 BuffSaveUserIndex = 0;

	/** Permanent Surgery progression has four visible rarity tiers. Legacy wedge arrays still deserialize older saves. */
	static constexpr int32 MaxFillStepsPerStat = 4;
	static constexpr int32 MaxSurgeryRarityTier = 4;
	static constexpr int32 PermanentBuffUnlockCostCC = 10;
	static constexpr int32 SurgeryUnlockCostCC = 10;
	static constexpr int32 SurgeryPermanentBonusStatPoints = 1;
	static constexpr int32 SingleUseBuffCostCC = 1;
	static constexpr int32 MaxSelectedSingleUseBuffs = 4;
	static constexpr float SingleUseStatBuffMultiplier = 1.10f;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Buffs")
	int32 GetChadCouponBalance() const;

	/** Legacy wallet accessor kept so older callers still read the shared Chad Coupons balance. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetPowerCrystalBalance() const;

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	void AddChadCoupons(int32 Amount);

	/** Legacy wallet add kept so older callers still add to the shared Chad Coupons balance. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void AddPowerCrystals(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Buffs")
	bool SpendChadCoupons(int32 Amount);

	/** Legacy wallet spend kept so older callers still spend from the shared Chad Coupons balance. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool SpendPowerCrystals(int32 Amount);

	/** Get fill-step state (0=Locked, 1=Unlocked) for the given stat and step index. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetFillStepState(ET66HeroStatType StatType, int32 StepIndex) const;

	/** Number of visible fill steps currently unlocked for this stat. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetUnlockedFillStepCount(ET66HeroStatType StatType) const;

	/** Permanent diploma primary-stat bonus for this stat. Purchases remain disabled by gate, but owned bonuses apply. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetTotalStatBonus(ET66HeroStatType StatType) const;

	/** Permanent stat upgrade purchases are currently disabled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetCostForNextFillStepUnlock(ET66HeroStatType StatType) const;

	/** Purchase path is currently disabled by gate. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockNextFillStep(ET66HeroStatType StatType);

	/** Random purchase path is currently disabled by gate. */
	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool UnlockRandomStat();

	/** True if the primary-stat Surgery for this stat is owned. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	bool IsStatMaxed(ET66HeroStatType StatType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Demo")
	bool IsDemoDiplomaUpgradeLimitReached(ET66HeroStatType StatType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Buffs")
	FT66HeroStatBonuses GetPermanentBuffStatBonuses() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Surgeries")
	bool IsSurgeryOwned(FName SurgeryID) const;

	UFUNCTION(BlueprintCallable, Category = "PowerUp|Surgeries")
	bool PurchaseSurgery(FName SurgeryID);

	UFUNCTION(BlueprintCallable, Category = "PowerUp|Surgeries")
	bool RefundSurgery(FName SurgeryID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Surgeries")
	int32 GetSurgeryCost(FName SurgeryID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Surgeries")
	int32 GetSurgeryTierValue(FName SurgeryID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Surgeries")
	ET66ItemRarity GetSurgeryRarity(FName SurgeryID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Surgeries")
	bool IsSurgeryMaxTier(FName SurgeryID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Surgeries")
	int32 GetSurgeryBaseStatBonus(ET66HeroStatType StatType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Surgeries")
	int32 GetSurgeryStatBonus(ET66StatType StatType) const;

	static const TArray<FT66SurgeryDefinition>& GetAllSurgeryDefinitions();

	/** Compatibility alias for permanent diploma primary-stat bonuses. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	FT66HeroStatBonuses GetPowerupStatBonuses() const;

	/** Returns true if at least one copy of this temporary buff is owned and available to be selected. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	bool HasPendingSingleUseBuff(ET66StatType StatType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetOwnedSingleUseBuffCount(ET66StatType StatType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	bool IsSingleUseBuffSelected(ET66StatType StatType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetSelectedSingleUseBuffCountForStat(ET66StatType StatType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetSelectedSingleUseBuffCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	TArray<ET66StatType> GetOwnedSingleUseBuffs() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	TArray<ET66StatType> GetSelectedSingleUseBuffs() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	TArray<ET66StatType> GetSelectedSingleUseBuffSlots() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	ET66StatType GetSelectedSingleUseBuffSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool SetSelectedSingleUseBuffSlot(int32 SlotIndex, ET66StatType StatType);

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool ClearSelectedSingleUseBuffSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	bool IsSelectedSingleUseBuffSlotOwned(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetSelectedSingleUseBuffSlotAssignedCountForStat(ET66StatType StatType) const;

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool PurchaseSelectedSingleUseBuffSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void SetSelectedSingleUseBuffEditSlotIndex(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetSelectedSingleUseBuffEditSlotIndex() const;

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void BeginHeroSelectionSingleUseBuffEdit(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	void EndHeroSelectionSingleUseBuffEdit();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	bool IsHeroSelectionSingleUseBuffEditActive() const { return bHeroSelectionSingleUseBuffEditActive; }

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool SetSingleUseBuffSelected(ET66StatType StatType, bool bSelected);

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool AddSelectedSingleUseBuff(ET66StatType StatType);

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool RemoveSelectedSingleUseBuff(ET66StatType StatType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp")
	int32 GetSingleUseBuffCost() const { return SingleUseBuffCostCC; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PowerUp|Demo")
	bool AreSingleUseBuffPurchasesAllowed() const;

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool PurchaseSingleUseBuff(ET66StatType StatType);

	UFUNCTION(BlueprintCallable, Category = "PowerUp")
	bool RefundSingleUseBuff(ET66StatType StatType);

	TMap<ET66StatType, float> GetPendingSingleUseBuffMultipliers() const;

	TMap<ET66StatType, float> ConsumePendingSingleUseBuffMultipliers();

	static const TArray<ET66StatType>& GetAllSingleUseBuffTypes();

#if !UE_BUILD_SHIPPING
	void DebugSetDiplomaUnlockedSteps(ET66HeroStatType StatType, int32 Count);
	void DebugGrantSingleUseBuff(ET66StatType StatType, int32 Count, bool bSelectForNextRun);
#endif

private:
	static constexpr int32 LegacyV2SlotsPerStat = 10;
	static constexpr int32 SingleUseBuffCount = 28;
	static constexpr int32 SelectedSingleUseBuffSlotCount = MaxSelectedSingleUseBuffs;

	UPROPERTY()
	TObjectPtr<UT66BuffSaveGame> SaveData;

	int32 ActiveSelectedSingleUseBuffEditSlotIndex = 0;
	bool bHeroSelectionSingleUseBuffEditActive = false;

	void LoadOrCreateSave();
	void MigrateV1ToV2WedgeTiers();
	void MigrateV2ToV3BodyParts();
	void MigrateV3ToV4FillSteps();
	void MigrateV4ToV5UnifiedBuffs();
	void MigrateV5ToV6StatSingleUseBuffs();
	void MigrateV6ToV7SelectedSingleUseBuffs();
	void MigrateV7ToV8TemporaryBuffPresets();
	void MigrateV8ToV9PrimaryAccuracy();
	void MigrateV9ToV10SingleLoadoutSlots();
	void MigrateV10ToV11PrimarySpeed();
	void MigrateV11ToV12Surgeries();
	void MigrateV12ToV13SurgeryTiers();
	void Save();
	bool HandleShutdown(const FT66ShutdownContext& Context);
	bool FlushPendingDurableState(const TCHAR* Reason);
	TArray<uint8>* GetFillStepStatesForStat(ET66HeroStatType StatType);
	const TArray<uint8>* GetFillStepStatesForStat(ET66HeroStatType StatType) const;
	void EnsureFillStepStatesSize(TArray<uint8>& Arr);
	TArray<uint8>* GetPendingSingleUseStates();
	const TArray<uint8>* GetPendingSingleUseStates() const;
	void EnsurePendingSingleUseStatesSize(TArray<uint8>& Arr) const;
	TArray<uint8>* GetSelectedSingleUseStates();
	const TArray<uint8>* GetSelectedSingleUseStates() const;
	void EnsureSelectedSingleUseStatesSize(TArray<uint8>& Arr) const;
	void SanitizeSelectedSingleUseStates(TArray<uint8>& SelectedStates, const TArray<uint8>& OwnedStates) const;
	void EnsureSelectedSingleUseBuffSlotsSize(TArray<ET66StatType>& Slots) const;
	void BuildSelectedSingleUseStateSnapshot(const TArray<uint8>& OwnedStates, TArray<uint8>& OutSelectedStates) const;
	bool EnsureSelectedSingleUseBuffLoadoutValid();
	bool RebuildSelectedSingleUseStatesFromLoadout();
	int32 GetRandomBonusForStat(ET66HeroStatType StatType) const;
	int32 GetStatIndex(ET66HeroStatType StatType) const;
	FName GetSurgeryIDForBaseStat(ET66HeroStatType StatType) const;
	const FT66SurgeryDefinition* FindSurgeryDefinition(FName SurgeryID) const;
	bool EnsureSurgeryOwnershipValid();
	int32 GetSingleUseBuffIndex(ET66StatType StatType) const;
	void AddBonusForStat(FT66HeroStatBonuses& Bonuses, ET66HeroStatType StatType, int32 Amount) const;

	bool bBuffSaveFlushNeeded = false;
	int64 BuffSaveAsyncSequence = 0;
	int64 PendingBuffSaveSequence = 0;
	FT66ShutdownParticipantHandle ShutdownParticipantHandle;
};
