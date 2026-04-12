// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66BuffSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BuffSaveGame.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Buff, Log, All);

const FString UT66BuffSubsystem::BuffSaveSlotName(TEXT("T66_PowerUp"));

namespace
{
	static const ET66SecondaryStatType GSingleUseBuffStats[31] = {
		ET66SecondaryStatType::AoeDamage,
		ET66SecondaryStatType::BounceDamage,
		ET66SecondaryStatType::PierceDamage,
		ET66SecondaryStatType::DotDamage,
		ET66SecondaryStatType::CritDamage,
		ET66SecondaryStatType::AoeSpeed,
		ET66SecondaryStatType::BounceSpeed,
		ET66SecondaryStatType::PierceSpeed,
		ET66SecondaryStatType::DotSpeed,
		ET66SecondaryStatType::CritChance,
		ET66SecondaryStatType::AoeScale,
		ET66SecondaryStatType::BounceScale,
		ET66SecondaryStatType::PierceScale,
		ET66SecondaryStatType::DotScale,
		ET66SecondaryStatType::AttackRange,
		ET66SecondaryStatType::Taunt,
		ET66SecondaryStatType::DamageReduction,
		ET66SecondaryStatType::ReflectDamage,
		ET66SecondaryStatType::HpRegen,
		ET66SecondaryStatType::Crush,
		ET66SecondaryStatType::EvasionChance,
		ET66SecondaryStatType::CounterAttack,
		ET66SecondaryStatType::LifeSteal,
		ET66SecondaryStatType::Invisibility,
		ET66SecondaryStatType::Assassinate,
		ET66SecondaryStatType::TreasureChest,
		ET66SecondaryStatType::Cheating,
		ET66SecondaryStatType::Stealing,
		ET66SecondaryStatType::LootCrate,
		ET66SecondaryStatType::Alchemy,
		ET66SecondaryStatType::Accuracy
	};
}

const TArray<ET66SecondaryStatType>& UT66BuffSubsystem::GetAllSingleUseBuffTypes()
{
	static const TArray<ET66SecondaryStatType> BuffTypes = []()
	{
		TArray<ET66SecondaryStatType> Out;
		Out.Reserve(UE_ARRAY_COUNT(GSingleUseBuffStats));
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(GSingleUseBuffStats); ++Index)
		{
			if (T66IsLiveSecondaryStatType(GSingleUseBuffStats[Index]))
			{
				Out.Add(GSingleUseBuffStats[Index]);
			}
		}
		return Out;
	}();

	return BuffTypes;
}

void UT66BuffSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreateSave();
}

void UT66BuffSubsystem::LoadOrCreateSave()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(BuffSaveSlotName, BuffSaveUserIndex);
	SaveData = Cast<UT66BuffSaveGame>(Loaded);
	bool bNeedsSave = false;
	if (!SaveData)
	{
		SaveData = NewObject<UT66BuffSaveGame>(this);
		UE_LOG(LogT66Buff, Log, TEXT("[Buffs] LoadOrCreateSave: Created fresh save (no file found)."));
	}
	else
	{
		UE_LOG(LogT66Buff, Log, TEXT("[Buffs] LoadOrCreateSave: Loaded existing save, legacyBalance=%d"), SaveData->PowerCrystalBalance);
	}

	if (SaveData->SaveVersion < 2)
	{
		MigrateV1ToV2WedgeTiers();
		SaveData->SaveVersion = 2;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 3)
	{
		MigrateV2ToV3BodyParts();
		SaveData->SaveVersion = 3;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 4)
	{
		MigrateV3ToV4FillSteps();
		SaveData->SaveVersion = 4;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 5)
	{
		MigrateV4ToV5UnifiedBuffs();
		SaveData->SaveVersion = 5;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 6)
	{
		MigrateV5ToV6SecondarySingleUseBuffs();
		SaveData->SaveVersion = 6;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 7)
	{
		MigrateV6ToV7SelectedSingleUseBuffs();
		SaveData->SaveVersion = 7;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 8)
	{
		MigrateV7ToV8TemporaryBuffPresets();
		SaveData->SaveVersion = 8;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 9)
	{
		MigrateV8ToV9PrimaryAccuracy();
		SaveData->SaveVersion = 9;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 10)
	{
		MigrateV9ToV10SingleLoadoutSlots();
		SaveData->SaveVersion = 10;
		bNeedsSave = true;
	}

	EnsurePendingSingleUseStatesSize(SaveData->PendingSingleUseBuffStates);
	EnsureSelectedSingleUseStatesSize(SaveData->SelectedSingleUseBuffStates);
	SanitizeSelectedSingleUseStates(SaveData->SelectedSingleUseBuffStates, SaveData->PendingSingleUseBuffStates);
	bNeedsSave |= EnsureSelectedSingleUseBuffLoadoutValid();
	bNeedsSave |= RebuildSelectedSingleUseStatesFromLoadout();

	if (bNeedsSave)
	{
		Save();
	}
}

void UT66BuffSubsystem::MigrateV1ToV2WedgeTiers()
{
	if (!SaveData) return;
	auto Migrate = [](TArray<uint8>& Tiers, int32 LegacyCount)
	{
		Tiers.SetNumZeroed(UT66BuffSubsystem::LegacyV2SlotsPerStat);
		for (int32 i = 0; i < LegacyCount && i < UT66BuffSubsystem::LegacyV2SlotsPerStat; ++i)
		{
			Tiers[i] = static_cast<uint8>(ET66BuffFillStepState::Unlocked);
		}
	};
	Migrate(SaveData->WedgeTiersDamage, SaveData->PowerupSlicesDamage);
	Migrate(SaveData->WedgeTiersAttackSpeed, SaveData->PowerupSlicesAttackSpeed);
	Migrate(SaveData->WedgeTiersAttackScale, SaveData->PowerupSlicesAttackScale);
	Migrate(SaveData->WedgeTiersAccuracy, SaveData->PowerupSlicesAccuracy);
	Migrate(SaveData->WedgeTiersArmor, SaveData->PowerupSlicesArmor);
	Migrate(SaveData->WedgeTiersEvasion, SaveData->PowerupSlicesEvasion);
	Migrate(SaveData->WedgeTiersLuck, SaveData->PowerupSlicesLuck);
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v1 slice counts to v2 wedge tiers."));
}

void UT66BuffSubsystem::MigrateV2ToV3BodyParts()
{
	if (!SaveData) return;

	auto CountLegacyUnlocked = [](const TArray<uint8>& States) -> int32
	{
		int32 Count = 0;
		for (int32 i = 0; i < States.Num() && i < UT66BuffSubsystem::LegacyV2SlotsPerStat; ++i)
		{
			if (States[i] > 0)
			{
				++Count;
			}
		}
		return Count;
	};

	auto Convert = [this, &CountLegacyUnlocked](TArray<uint8>& States, int32& RandomBonus)
	{
		const int32 LegacyUnlocked = CountLegacyUnlocked(States);
		const int32 VisibleUnlocked = FMath::Clamp(LegacyUnlocked, 0, 6);
		const int32 OverflowBonus = FMath::Max(0, LegacyUnlocked - 6);

		States.SetNumZeroed(6);
		for (int32 i = 0; i < VisibleUnlocked; ++i)
		{
			States[i] = static_cast<uint8>(ET66BuffFillStepState::Unlocked);
		}

		RandomBonus = FMath::Max(0, RandomBonus) + OverflowBonus;
	};

	Convert(SaveData->WedgeTiersDamage, SaveData->RandomBonusDamage);
	Convert(SaveData->WedgeTiersAttackSpeed, SaveData->RandomBonusAttackSpeed);
	Convert(SaveData->WedgeTiersAttackScale, SaveData->RandomBonusAttackScale);
	Convert(SaveData->WedgeTiersAccuracy, SaveData->RandomBonusAccuracy);
	Convert(SaveData->WedgeTiersArmor, SaveData->RandomBonusArmor);
	Convert(SaveData->WedgeTiersEvasion, SaveData->RandomBonusEvasion);
	Convert(SaveData->WedgeTiersLuck, SaveData->RandomBonusLuck);

	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v2 wedge tiers to v3 six-part statues."));
}

void UT66BuffSubsystem::MigrateV3ToV4FillSteps()
{
	if (!SaveData) return;

	auto Convert = [](TArray<uint8>& States, int32& RandomBonus)
	{
		int32 UnlockedVisible = 0;
		for (int32 i = 0; i < States.Num(); ++i)
		{
			if (States[i] > 0)
			{
				++UnlockedVisible;
			}
		}

		const int32 TotalBonus = FMath::Max(0, UnlockedVisible) + FMath::Max(0, RandomBonus);
		const int32 VisibleUnlocked = FMath::Clamp(TotalBonus, 0, UT66BuffSubsystem::MaxFillStepsPerStat);
		const int32 OverflowBonus = FMath::Max(0, TotalBonus - UT66BuffSubsystem::MaxFillStepsPerStat);

		States.SetNumZeroed(UT66BuffSubsystem::MaxFillStepsPerStat);
		for (int32 i = 0; i < VisibleUnlocked; ++i)
		{
			States[i] = static_cast<uint8>(ET66BuffFillStepState::Unlocked);
		}

		RandomBonus = OverflowBonus;
	};

	Convert(SaveData->WedgeTiersDamage, SaveData->RandomBonusDamage);
	Convert(SaveData->WedgeTiersAttackSpeed, SaveData->RandomBonusAttackSpeed);
	Convert(SaveData->WedgeTiersAttackScale, SaveData->RandomBonusAttackScale);
	Convert(SaveData->WedgeTiersAccuracy, SaveData->RandomBonusAccuracy);
	Convert(SaveData->WedgeTiersArmor, SaveData->RandomBonusArmor);
	Convert(SaveData->WedgeTiersEvasion, SaveData->RandomBonusEvasion);
	Convert(SaveData->WedgeTiersLuck, SaveData->RandomBonusLuck);

	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v3 six-part statues to v4 ten-step fill progression."));
}

void UT66BuffSubsystem::MigrateV4ToV5UnifiedBuffs()
{
	if (!SaveData)
	{
		return;
	}

	SaveData->PowerCrystalBalance = 0;
	SaveData->PendingSingleUseBuffStates.SetNumZeroed(SingleUseBuffCount);
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v4 progression to unified Chad Coupons + single-use buffs."));
}

void UT66BuffSubsystem::MigrateV5ToV6SecondarySingleUseBuffs()
{
	if (!SaveData)
	{
		return;
	}

	SaveData->PendingSingleUseBuffStates.SetNumZeroed(SingleUseBuffCount);
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v5 primary single-use buffs to v6 secondary single-use buffs."));
}

void UT66BuffSubsystem::MigrateV6ToV7SelectedSingleUseBuffs()
{
	if (!SaveData)
	{
		return;
	}

	EnsurePendingSingleUseStatesSize(SaveData->PendingSingleUseBuffStates);
	SaveData->SelectedSingleUseBuffStates.SetNumZeroed(SingleUseBuffCount);

	int32 SelectedCount = 0;
	for (ET66SecondaryStatType StatType : GetAllSingleUseBuffTypes())
	{
		const int32 StatIndex = GetSingleUseBuffIndex(StatType);
		if (StatIndex == INDEX_NONE
			|| !SaveData->PendingSingleUseBuffStates.IsValidIndex(StatIndex)
			|| !SaveData->SelectedSingleUseBuffStates.IsValidIndex(StatIndex)
			|| SaveData->PendingSingleUseBuffStates[StatIndex] == 0)
		{
			continue;
		}

		if (SelectedCount >= MaxSelectedSingleUseBuffs)
		{
			break;
		}

		SaveData->SelectedSingleUseBuffStates[StatIndex] = 1;
		++SelectedCount;
	}

	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v6 owned single-use buffs to v7 selected buff loadout (%d selected)."), SelectedCount);
}

void UT66BuffSubsystem::MigrateV7ToV8TemporaryBuffPresets()
{
	if (!SaveData)
	{
		return;
	}

	EnsurePendingSingleUseStatesSize(SaveData->PendingSingleUseBuffStates);
	EnsureSelectedSingleUseStatesSize(SaveData->SelectedSingleUseBuffStates);
	SanitizeSelectedSingleUseStates(SaveData->SelectedSingleUseBuffStates, SaveData->PendingSingleUseBuffStates);

	FT66TemporaryBuffPreset DefaultPreset;
	DefaultPreset.PresetName = TEXT("Preset 1");
	EnsureSelectedSingleUseBuffSlotsSize(DefaultPreset.SlotBuffs);

	int32 SlotIndex = 0;
	for (ET66SecondaryStatType StatType : GetAllSingleUseBuffTypes())
	{
		const int32 StatArrayIndex = GetSingleUseBuffIndex(StatType);
		if (StatArrayIndex == INDEX_NONE || !SaveData->SelectedSingleUseBuffStates.IsValidIndex(StatArrayIndex))
		{
			continue;
		}

		const int32 SelectedCount = FMath::Max(0, static_cast<int32>(SaveData->SelectedSingleUseBuffStates[StatArrayIndex]));
		for (int32 CopyIndex = 0; CopyIndex < SelectedCount && SlotIndex < SelectedSingleUseBuffSlotCount; ++CopyIndex)
		{
			DefaultPreset.SlotBuffs[SlotIndex++] = StatType;
		}

		if (SlotIndex >= SelectedSingleUseBuffSlotCount)
		{
			break;
		}
	}

	SaveData->TemporaryBuffPresets.Reset();
	SaveData->TemporaryBuffPresets.Add(DefaultPreset);
	SaveData->ActiveTemporaryBuffPresetIndex = 0;
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v7 temp-buff loadout to v8 named preset system."));
}

void UT66BuffSubsystem::MigrateV8ToV9PrimaryAccuracy()
{
	if (!SaveData) return;

	EnsureFillStepStatesSize(SaveData->WedgeTiersAccuracy);
	SaveData->RandomBonusAccuracy = FMath::Max(0, SaveData->RandomBonusAccuracy);
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v8 saves to v9 primary Accuracy progression."));
}

void UT66BuffSubsystem::MigrateV9ToV10SingleLoadoutSlots()
{
	if (!SaveData)
	{
		return;
	}

	EnsurePendingSingleUseStatesSize(SaveData->PendingSingleUseBuffStates);
	EnsureSelectedSingleUseStatesSize(SaveData->SelectedSingleUseBuffStates);
	SanitizeSelectedSingleUseStates(SaveData->SelectedSingleUseBuffStates, SaveData->PendingSingleUseBuffStates);

	SaveData->SelectedSingleUseBuffSlots.Init(ET66SecondaryStatType::None, SelectedSingleUseBuffSlotCount);
	int32 SlotIndex = 0;

	if (SaveData->TemporaryBuffPresets.Num() > 0)
	{
		const int32 LegacyPresetIndex = FMath::Clamp(
			SaveData->ActiveTemporaryBuffPresetIndex,
			0,
			SaveData->TemporaryBuffPresets.Num() - 1);
		TArray<ET66SecondaryStatType> LegacySlots = SaveData->TemporaryBuffPresets[LegacyPresetIndex].SlotBuffs;
		EnsureSelectedSingleUseBuffSlotsSize(LegacySlots);
		for (ET66SecondaryStatType SlotStat : LegacySlots)
		{
			if (SlotIndex >= SelectedSingleUseBuffSlotCount)
			{
				break;
			}

			SaveData->SelectedSingleUseBuffSlots[SlotIndex++] = T66IsLiveSecondaryStatType(SlotStat)
				? SlotStat
				: ET66SecondaryStatType::None;
		}
	}
	else
	{
		for (ET66SecondaryStatType StatType : GetAllSingleUseBuffTypes())
		{
			const int32 StatArrayIndex = GetSingleUseBuffIndex(StatType);
			if (StatArrayIndex == INDEX_NONE || !SaveData->SelectedSingleUseBuffStates.IsValidIndex(StatArrayIndex))
			{
				continue;
			}

			const int32 SelectedCount = FMath::Max(0, static_cast<int32>(SaveData->SelectedSingleUseBuffStates[StatArrayIndex]));
			for (int32 CopyIndex = 0; CopyIndex < SelectedCount && SlotIndex < SelectedSingleUseBuffSlotCount; ++CopyIndex)
			{
				SaveData->SelectedSingleUseBuffSlots[SlotIndex++] = StatType;
			}

			if (SlotIndex >= SelectedSingleUseBuffSlotCount)
			{
				break;
			}
		}
	}

	EnsureSelectedSingleUseBuffSlotsSize(SaveData->SelectedSingleUseBuffSlots);
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v9 temp-buff presets to v10 single 5-slot loadout."));
}

void UT66BuffSubsystem::Save()
{
	if (SaveData)
	{
		UGameplayStatics::AsyncSaveGameToSlot(SaveData, BuffSaveSlotName, BuffSaveUserIndex);
	}
}

int32 UT66BuffSubsystem::GetChadCouponBalance() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			return Achievements->GetChadCouponBalance();
		}
	}

	return 0;
}

int32 UT66BuffSubsystem::GetPowerCrystalBalance() const
{
	return GetChadCouponBalance();
}

void UT66BuffSubsystem::AddChadCoupons(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achievements->AddChadCoupons(Amount);
		}
	}
}

void UT66BuffSubsystem::AddPowerCrystals(int32 Amount)
{
	AddChadCoupons(Amount);
}

bool UT66BuffSubsystem::SpendChadCoupons(int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			return Achievements->SpendChadCoupons(Amount);
		}
	}

	return false;
}

bool UT66BuffSubsystem::SpendPowerCrystals(int32 Amount)
{
	return SpendChadCoupons(Amount);
}

void UT66BuffSubsystem::EnsureFillStepStatesSize(TArray<uint8>& Arr)
{
	if (Arr.Num() < MaxFillStepsPerStat)
	{
		Arr.SetNumZeroed(MaxFillStepsPerStat);
	}
	else if (Arr.Num() > MaxFillStepsPerStat)
	{
		Arr.SetNum(MaxFillStepsPerStat);
	}
}

TArray<uint8>* UT66BuffSubsystem::GetPendingSingleUseStates()
{
	return SaveData ? &SaveData->PendingSingleUseBuffStates : nullptr;
}

const TArray<uint8>* UT66BuffSubsystem::GetPendingSingleUseStates() const
{
	return SaveData ? &SaveData->PendingSingleUseBuffStates : nullptr;
}

void UT66BuffSubsystem::EnsurePendingSingleUseStatesSize(TArray<uint8>& Arr) const
{
	if (Arr.Num() < SingleUseBuffCount)
	{
		Arr.SetNumZeroed(SingleUseBuffCount);
	}
	else if (Arr.Num() > SingleUseBuffCount)
	{
		Arr.SetNum(SingleUseBuffCount);
	}
}

TArray<uint8>* UT66BuffSubsystem::GetSelectedSingleUseStates()
{
	return SaveData ? &SaveData->SelectedSingleUseBuffStates : nullptr;
}

const TArray<uint8>* UT66BuffSubsystem::GetSelectedSingleUseStates() const
{
	return SaveData ? &SaveData->SelectedSingleUseBuffStates : nullptr;
}

void UT66BuffSubsystem::EnsureSelectedSingleUseStatesSize(TArray<uint8>& Arr) const
{
	if (Arr.Num() < SingleUseBuffCount)
	{
		Arr.SetNumZeroed(SingleUseBuffCount);
	}
	else if (Arr.Num() > SingleUseBuffCount)
	{
		Arr.SetNum(SingleUseBuffCount);
	}
}

void UT66BuffSubsystem::SanitizeSelectedSingleUseStates(TArray<uint8>& SelectedStates, const TArray<uint8>& OwnedStates) const
{
	EnsureSelectedSingleUseStatesSize(SelectedStates);

	TArray<uint8> NormalizedOwned = OwnedStates;
	EnsurePendingSingleUseStatesSize(NormalizedOwned);

	int32 RemainingSelections = MaxSelectedSingleUseBuffs;
	for (ET66SecondaryStatType StatType : GetAllSingleUseBuffTypes())
	{
		const int32 StatIndex = GetSingleUseBuffIndex(StatType);
		if (StatIndex == INDEX_NONE || !SelectedStates.IsValidIndex(StatIndex) || !NormalizedOwned.IsValidIndex(StatIndex))
		{
			continue;
		}

		if (NormalizedOwned[StatIndex] == 0)
		{
			SelectedStates[StatIndex] = 0;
			continue;
		}

		const int32 SanitizedSelected = FMath::Clamp<int32>(SelectedStates[StatIndex], 0, FMath::Min<int32>(NormalizedOwned[StatIndex], RemainingSelections));
		SelectedStates[StatIndex] = static_cast<uint8>(SanitizedSelected);
		RemainingSelections -= SanitizedSelected;
	}
}

void UT66BuffSubsystem::EnsureSelectedSingleUseBuffSlotsSize(TArray<ET66SecondaryStatType>& Slots) const
{
	if (Slots.Num() < SelectedSingleUseBuffSlotCount)
	{
		const int32 OriginalNum = Slots.Num();
		Slots.SetNum(SelectedSingleUseBuffSlotCount);
		for (int32 Index = OriginalNum; Index < SelectedSingleUseBuffSlotCount; ++Index)
		{
			Slots[Index] = ET66SecondaryStatType::None;
		}
	}
	else if (Slots.Num() > SelectedSingleUseBuffSlotCount)
	{
		Slots.SetNum(SelectedSingleUseBuffSlotCount);
	}

	for (ET66SecondaryStatType& SlotStat : Slots)
	{
		if (SlotStat != ET66SecondaryStatType::None && !T66IsLiveSecondaryStatType(SlotStat))
		{
			SlotStat = ET66SecondaryStatType::None;
		}
	}
}

	void UT66BuffSubsystem::BuildSelectedSingleUseStateSnapshot(const TArray<uint8>& OwnedStates, TArray<uint8>& OutSelectedStates) const
	{
		OutSelectedStates.Init(0, SingleUseBuffCount);
		if (!SaveData)
		{
			return;
		}

		TArray<uint8> NormalizedOwned = OwnedStates;
		EnsurePendingSingleUseStatesSize(NormalizedOwned);

		TArray<ET66SecondaryStatType> Slots = SaveData->SelectedSingleUseBuffSlots;
		const_cast<UT66BuffSubsystem*>(this)->EnsureSelectedSingleUseBuffSlotsSize(Slots);

		int32 TotalSelected = 0;
		for (ET66SecondaryStatType SlotStat : Slots)
		{
			if (!T66IsLiveSecondaryStatType(SlotStat) || TotalSelected >= MaxSelectedSingleUseBuffs)
			{
				continue;
			}

			const int32 StatIndex = GetSingleUseBuffIndex(SlotStat);
			if (StatIndex == INDEX_NONE || !NormalizedOwned.IsValidIndex(StatIndex) || !OutSelectedStates.IsValidIndex(StatIndex))
			{
				continue;
			}

			if (static_cast<int32>(NormalizedOwned[StatIndex]) > static_cast<int32>(OutSelectedStates[StatIndex]))
			{
				OutSelectedStates[StatIndex] = static_cast<uint8>(static_cast<int32>(OutSelectedStates[StatIndex]) + 1);
				++TotalSelected;
			}
		}
	}

	bool UT66BuffSubsystem::EnsureSelectedSingleUseBuffLoadoutValid()
	{
		if (!SaveData)
		{
			return false;
		}

		const TArray<ET66SecondaryStatType> OriginalSlots = SaveData->SelectedSingleUseBuffSlots;
		EnsureSelectedSingleUseBuffSlotsSize(SaveData->SelectedSingleUseBuffSlots);
		const bool bSlotsChanged = SaveData->SelectedSingleUseBuffSlots != OriginalSlots;

		const int32 SanitizedEditSlotIndex = FMath::Clamp(ActiveSelectedSingleUseBuffEditSlotIndex, 0, SelectedSingleUseBuffSlotCount - 1);
		if (ActiveSelectedSingleUseBuffEditSlotIndex != SanitizedEditSlotIndex)
		{
			ActiveSelectedSingleUseBuffEditSlotIndex = SanitizedEditSlotIndex;
		}

		return bSlotsChanged;
	}

	bool UT66BuffSubsystem::RebuildSelectedSingleUseStatesFromLoadout()
	{
		TArray<uint8>* OwnedStates = GetPendingSingleUseStates();
		TArray<uint8>* SelectedStates = GetSelectedSingleUseStates();
		if (!SaveData || !OwnedStates || !SelectedStates)
		{
			return false;
		}

		EnsurePendingSingleUseStatesSize(*OwnedStates);
		EnsureSelectedSingleUseStatesSize(*SelectedStates);

		TArray<uint8> NewSelectedStates;
		BuildSelectedSingleUseStateSnapshot(*OwnedStates, NewSelectedStates);
		if (*SelectedStates == NewSelectedStates)
		{
			return false;
		}

		*SelectedStates = MoveTemp(NewSelectedStates);
		return true;
	}

int32 UT66BuffSubsystem::GetStatIndex(ET66HeroStatType StatType) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:      return 0;
	case ET66HeroStatType::AttackSpeed: return 1;
	case ET66HeroStatType::AttackScale: return 2;
	case ET66HeroStatType::Accuracy:    return 3;
	case ET66HeroStatType::Armor:       return 4;
	case ET66HeroStatType::Evasion:     return 5;
	case ET66HeroStatType::Luck:        return 6;
	default:                            return INDEX_NONE;
	}
}

int32 UT66BuffSubsystem::GetSingleUseBuffIndex(ET66SecondaryStatType StatType) const
{
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(GSingleUseBuffStats); ++Index)
	{
		if (GSingleUseBuffStats[Index] == StatType)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void UT66BuffSubsystem::AddBonusForStat(FT66HeroStatBonuses& Bonuses, ET66HeroStatType StatType, int32 Amount) const
{
	switch (StatType)
	{
	case ET66HeroStatType::Damage:      Bonuses.Damage += Amount; break;
	case ET66HeroStatType::AttackSpeed: Bonuses.AttackSpeed += Amount; break;
	case ET66HeroStatType::AttackScale: Bonuses.AttackScale += Amount; break;
	case ET66HeroStatType::Accuracy:    Bonuses.Accuracy += Amount; break;
	case ET66HeroStatType::Armor:       Bonuses.Armor += Amount; break;
	case ET66HeroStatType::Evasion:     Bonuses.Evasion += Amount; break;
	case ET66HeroStatType::Luck:        Bonuses.Luck += Amount; break;
	default:                            break;
	}
}

TArray<uint8>* UT66BuffSubsystem::GetFillStepStatesForStat(ET66HeroStatType StatType)
{
	if (!SaveData) return nullptr;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      return &SaveData->WedgeTiersDamage;
		case ET66HeroStatType::AttackSpeed: return &SaveData->WedgeTiersAttackSpeed;
		case ET66HeroStatType::AttackScale: return &SaveData->WedgeTiersAttackScale;
		case ET66HeroStatType::Accuracy:    return &SaveData->WedgeTiersAccuracy;
		case ET66HeroStatType::Armor:      return &SaveData->WedgeTiersArmor;
		case ET66HeroStatType::Evasion:    return &SaveData->WedgeTiersEvasion;
		case ET66HeroStatType::Luck:       return &SaveData->WedgeTiersLuck;
		default: return nullptr;
	}
}

const TArray<uint8>* UT66BuffSubsystem::GetFillStepStatesForStat(ET66HeroStatType StatType) const
{
	return const_cast<UT66BuffSubsystem*>(this)->GetFillStepStatesForStat(StatType);
}

int32 UT66BuffSubsystem::GetFillStepState(ET66HeroStatType StatType, int32 StepIndex) const
{
	const TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
	if (!Arr || StepIndex < 0 || StepIndex >= MaxFillStepsPerStat) return 0;
	if (StepIndex >= Arr->Num()) return 0;
	return (*Arr)[StepIndex] > 0 ? 1 : 0;
}

int32 UT66BuffSubsystem::GetUnlockedFillStepCount(ET66HeroStatType StatType) const
{
	const TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
	if (!Arr) return 0;

	int32 Count = 0;
	for (int32 i = 0; i < Arr->Num() && i < MaxFillStepsPerStat; ++i)
	{
		if ((*Arr)[i] > 0)
		{
			++Count;
		}
	}
	return Count;
}

int32 UT66BuffSubsystem::GetTotalStatBonus(ET66HeroStatType StatType) const
{
	return GetUnlockedFillStepCount(StatType) + GetRandomBonusForStat(StatType);
}

int32 UT66BuffSubsystem::GetCostForNextFillStepUnlock(ET66HeroStatType StatType) const
{
	return IsStatMaxed(StatType) ? 0 : PermanentBuffUnlockCostCC;
}

bool UT66BuffSubsystem::UnlockNextFillStep(ET66HeroStatType StatType)
{
	TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
	if (!SaveData || !Arr) return false;
	EnsureFillStepStatesSize(*Arr);

	const int32 Cost = GetCostForNextFillStepUnlock(StatType);
	if (Cost <= 0)
	{
		return false;
	}

	for (int32 i = 0; i < MaxFillStepsPerStat; ++i)
	{
		if ((*Arr)[i] == static_cast<uint8>(ET66BuffFillStepState::Locked))
		{
			if (!SpendChadCoupons(Cost))
			{
				return false;
			}
			(*Arr)[i] = static_cast<uint8>(ET66BuffFillStepState::Unlocked);
			Save();
			return true;
		}
	}

	return false;
}

bool UT66BuffSubsystem::UnlockRandomStat()
{
	if (!SaveData) return false;
	static const TArray<ET66HeroStatType> AllStats = {
		ET66HeroStatType::Damage,
		ET66HeroStatType::AttackSpeed,
		ET66HeroStatType::AttackScale,
		ET66HeroStatType::Accuracy,
		ET66HeroStatType::Armor,
		ET66HeroStatType::Evasion,
		ET66HeroStatType::Luck
	};
	TArray<ET66HeroStatType> Shuffled = AllStats;
	for (int32 i = Shuffled.Num() - 1; i > 0; --i)
	{
		Swap(Shuffled[i], Shuffled[FMath::RandRange(0, i)]);
	}
	for (ET66HeroStatType Stat : Shuffled)
	{
		TArray<uint8>* Arr = GetFillStepStatesForStat(Stat);
		if (!Arr) continue;
		EnsureFillStepStatesSize(*Arr);
		for (int32 i = 0; i < MaxFillStepsPerStat; ++i)
		{
			if ((*Arr)[i] == static_cast<uint8>(ET66BuffFillStepState::Locked))
			{
				if (!SpendChadCoupons(PermanentBuffUnlockCostCC))
				{
					return false;
				}
				(*Arr)[i] = static_cast<uint8>(ET66BuffFillStepState::Unlocked);
				Save();
				return true;
			}
		}
	}
	return false;
}

bool UT66BuffSubsystem::IsStatMaxed(ET66HeroStatType StatType) const
{
	const TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
	if (!Arr) return true;
	for (int32 i = 0; i < MaxFillStepsPerStat; ++i)
	{
		if (i >= Arr->Num() || (*Arr)[i] == static_cast<uint8>(ET66BuffFillStepState::Locked))
		{
			return false;
		}
	}
	return true;
}

FT66HeroStatBonuses UT66BuffSubsystem::GetPermanentBuffStatBonuses() const
{
	FT66HeroStatBonuses Out;
	if (!SaveData) return Out;
	Out.Damage = GetTotalStatBonus(ET66HeroStatType::Damage);
	Out.AttackSpeed = GetTotalStatBonus(ET66HeroStatType::AttackSpeed);
	Out.AttackScale = GetTotalStatBonus(ET66HeroStatType::AttackScale);
	Out.Accuracy = GetTotalStatBonus(ET66HeroStatType::Accuracy);
	Out.Armor = GetTotalStatBonus(ET66HeroStatType::Armor);
	Out.Evasion = GetTotalStatBonus(ET66HeroStatType::Evasion);
	Out.Luck = GetTotalStatBonus(ET66HeroStatType::Luck);
	return Out;
}

FT66HeroStatBonuses UT66BuffSubsystem::GetPowerupStatBonuses() const
{
	return GetPermanentBuffStatBonuses();
}

bool UT66BuffSubsystem::HasPendingSingleUseBuff(ET66SecondaryStatType StatType) const
{
	return GetOwnedSingleUseBuffCount(StatType) > 0;
}

int32 UT66BuffSubsystem::GetOwnedSingleUseBuffCount(ET66SecondaryStatType StatType) const
{
	const TArray<uint8>* PendingStates = GetPendingSingleUseStates();
	if (!PendingStates)
	{
		return 0;
	}

	const int32 StatIndex = GetSingleUseBuffIndex(StatType);
	if (StatIndex == INDEX_NONE)
	{
		return 0;
	}

	TArray<uint8> NormalizedStates = *PendingStates;
	EnsurePendingSingleUseStatesSize(NormalizedStates);
	return NormalizedStates.IsValidIndex(StatIndex) ? FMath::Max(0, static_cast<int32>(NormalizedStates[StatIndex])) : 0;
}

bool UT66BuffSubsystem::IsSingleUseBuffSelected(ET66SecondaryStatType StatType) const
{
	return GetSelectedSingleUseBuffCountForStat(StatType) > 0;
}

int32 UT66BuffSubsystem::GetSelectedSingleUseBuffCountForStat(ET66SecondaryStatType StatType) const
{
	const TArray<uint8>* OwnedStates = GetPendingSingleUseStates();
	if (!OwnedStates)
	{
		return 0;
	}

	const int32 StatIndex = GetSingleUseBuffIndex(StatType);
	if (StatIndex == INDEX_NONE)
	{
		return 0;
	}

	TArray<uint8> NormalizedSelected;
	BuildSelectedSingleUseStateSnapshot(*OwnedStates, NormalizedSelected);
	return NormalizedSelected.IsValidIndex(StatIndex) ? FMath::Max(0, static_cast<int32>(NormalizedSelected[StatIndex])) : 0;
}

int32 UT66BuffSubsystem::GetSelectedSingleUseBuffCount() const
{
	const TArray<uint8>* OwnedStates = GetPendingSingleUseStates();
	if (!OwnedStates)
	{
		return 0;
	}

	TArray<uint8> NormalizedSelected;
	BuildSelectedSingleUseStateSnapshot(*OwnedStates, NormalizedSelected);

	int32 Count = 0;
	for (uint8 State : NormalizedSelected)
	{
		Count += static_cast<int32>(State);
	}
	return Count;
}

TArray<ET66SecondaryStatType> UT66BuffSubsystem::GetOwnedSingleUseBuffs() const
{
	TArray<ET66SecondaryStatType> OwnedBuffs;
	for (ET66SecondaryStatType StatType : GetAllSingleUseBuffTypes())
	{
		if (HasPendingSingleUseBuff(StatType))
		{
			OwnedBuffs.Add(StatType);
		}
	}
	return OwnedBuffs;
}

TArray<ET66SecondaryStatType> UT66BuffSubsystem::GetSelectedSingleUseBuffs() const
{
	TArray<ET66SecondaryStatType> SelectedBuffs;
	for (ET66SecondaryStatType StatType : GetAllSingleUseBuffTypes())
	{
		const int32 SelectedCount = GetSelectedSingleUseBuffCountForStat(StatType);
		for (int32 CopyIndex = 0; CopyIndex < SelectedCount; ++CopyIndex)
		{
			SelectedBuffs.Add(StatType);
		}
	}
	return SelectedBuffs;
}

TArray<ET66SecondaryStatType> UT66BuffSubsystem::GetSelectedSingleUseBuffSlots() const
{
	TArray<ET66SecondaryStatType> Slots = SaveData ? SaveData->SelectedSingleUseBuffSlots : TArray<ET66SecondaryStatType>{};
	const_cast<UT66BuffSubsystem*>(this)->EnsureSelectedSingleUseBuffSlotsSize(Slots);
	return Slots;
}

ET66SecondaryStatType UT66BuffSubsystem::GetSelectedSingleUseBuffSlot(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SelectedSingleUseBuffSlotCount || !SaveData)
	{
		return ET66SecondaryStatType::None;
	}

	TArray<ET66SecondaryStatType> Slots = SaveData->SelectedSingleUseBuffSlots;
	const_cast<UT66BuffSubsystem*>(this)->EnsureSelectedSingleUseBuffSlotsSize(Slots);
	return Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : ET66SecondaryStatType::None;
}

bool UT66BuffSubsystem::SetSelectedSingleUseBuffSlot(int32 SlotIndex, ET66SecondaryStatType StatType)
{
	if (!SaveData || SlotIndex < 0 || SlotIndex >= SelectedSingleUseBuffSlotCount)
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	const ET66SecondaryStatType SanitizedStatType = (StatType == ET66SecondaryStatType::None || T66IsLiveSecondaryStatType(StatType))
		? StatType
		: ET66SecondaryStatType::None;
	if (SaveData->SelectedSingleUseBuffSlots[SlotIndex] == SanitizedStatType)
	{
		return false;
	}

	SaveData->SelectedSingleUseBuffSlots[SlotIndex] = SanitizedStatType;
	ActiveSelectedSingleUseBuffEditSlotIndex = SlotIndex;
	RebuildSelectedSingleUseStatesFromLoadout();
	Save();
	return true;
}

bool UT66BuffSubsystem::ClearSelectedSingleUseBuffSlot(int32 SlotIndex)
{
	return SetSelectedSingleUseBuffSlot(SlotIndex, ET66SecondaryStatType::None);
}

bool UT66BuffSubsystem::IsSelectedSingleUseBuffSlotOwned(int32 SlotIndex) const
{
	const ET66SecondaryStatType SlotStat = GetSelectedSingleUseBuffSlot(SlotIndex);
	if (!T66IsLiveSecondaryStatType(SlotStat))
	{
		return true;
	}

	int32 RequiredCopies = 0;
	for (int32 Index = 0; Index <= SlotIndex && Index < SelectedSingleUseBuffSlotCount; ++Index)
	{
		if (GetSelectedSingleUseBuffSlot(Index) == SlotStat)
		{
			++RequiredCopies;
		}
	}

	return GetOwnedSingleUseBuffCount(SlotStat) >= RequiredCopies;
}

int32 UT66BuffSubsystem::GetSelectedSingleUseBuffSlotAssignedCountForStat(ET66SecondaryStatType StatType) const
{
	if (!T66IsLiveSecondaryStatType(StatType))
	{
		return 0;
	}

	int32 Count = 0;
	for (ET66SecondaryStatType SlotStat : GetSelectedSingleUseBuffSlots())
	{
		if (SlotStat == StatType)
		{
			++Count;
		}
	}
	return Count;
}

bool UT66BuffSubsystem::PurchaseSelectedSingleUseBuffSlot(int32 SlotIndex)
{
	const ET66SecondaryStatType SlotStat = GetSelectedSingleUseBuffSlot(SlotIndex);
	return T66IsLiveSecondaryStatType(SlotStat) ? PurchaseSingleUseBuff(SlotStat) : false;
}

void UT66BuffSubsystem::SetSelectedSingleUseBuffEditSlotIndex(int32 SlotIndex)
{
	ActiveSelectedSingleUseBuffEditSlotIndex = FMath::Clamp(SlotIndex, 0, SelectedSingleUseBuffSlotCount - 1);
}

int32 UT66BuffSubsystem::GetSelectedSingleUseBuffEditSlotIndex() const
{
	return FMath::Clamp(ActiveSelectedSingleUseBuffEditSlotIndex, 0, SelectedSingleUseBuffSlotCount - 1);
}

bool UT66BuffSubsystem::SetSingleUseBuffSelected(ET66SecondaryStatType StatType, bool bSelected)
{
	if (bSelected)
	{
		return AddSelectedSingleUseBuff(StatType);
	}

	if (!SaveData || !T66IsLiveSecondaryStatType(StatType))
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	bool bChanged = false;
	for (ET66SecondaryStatType& SlotStat : SaveData->SelectedSingleUseBuffSlots)
	{
		if (SlotStat == StatType)
		{
			SlotStat = ET66SecondaryStatType::None;
			bChanged = true;
		}
	}

	if (!bChanged)
	{
		return false;
	}

	RebuildSelectedSingleUseStatesFromLoadout();
	Save();
	return true;
}

bool UT66BuffSubsystem::AddSelectedSingleUseBuff(ET66SecondaryStatType StatType)
{
	if (!SaveData || !T66IsLiveSecondaryStatType(StatType))
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	for (int32 SlotIndex = 0; SlotIndex < SaveData->SelectedSingleUseBuffSlots.Num(); ++SlotIndex)
	{
		if (SaveData->SelectedSingleUseBuffSlots[SlotIndex] == ET66SecondaryStatType::None)
		{
			SaveData->SelectedSingleUseBuffSlots[SlotIndex] = StatType;
			ActiveSelectedSingleUseBuffEditSlotIndex = SlotIndex;
			RebuildSelectedSingleUseStatesFromLoadout();
			Save();
			return true;
		}
	}

	return false;
}

bool UT66BuffSubsystem::RemoveSelectedSingleUseBuff(ET66SecondaryStatType StatType)
{
	if (!SaveData || !T66IsLiveSecondaryStatType(StatType))
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	for (int32 SlotIndex = SaveData->SelectedSingleUseBuffSlots.Num() - 1; SlotIndex >= 0; --SlotIndex)
	{
		if (SaveData->SelectedSingleUseBuffSlots[SlotIndex] == StatType)
		{
			SaveData->SelectedSingleUseBuffSlots[SlotIndex] = ET66SecondaryStatType::None;
			ActiveSelectedSingleUseBuffEditSlotIndex = SlotIndex;
			RebuildSelectedSingleUseStatesFromLoadout();
			Save();
			return true;
		}
	}

	return false;
}

bool UT66BuffSubsystem::PurchaseSingleUseBuff(ET66SecondaryStatType StatType)
{
	TArray<uint8>* PendingStates = GetPendingSingleUseStates();
	TArray<uint8>* SelectedStates = GetSelectedSingleUseStates();
	if (!SaveData || !PendingStates || !SelectedStates)
	{
		return false;
	}

	if (!T66IsLiveSecondaryStatType(StatType))
	{
		return false;
	}

	EnsurePendingSingleUseStatesSize(*PendingStates);
	EnsureSelectedSingleUseStatesSize(*SelectedStates);
	SanitizeSelectedSingleUseStates(*SelectedStates, *PendingStates);
	const int32 StatIndex = GetSingleUseBuffIndex(StatType);
	if (StatIndex == INDEX_NONE || !PendingStates->IsValidIndex(StatIndex))
	{
		return false;
	}

	if (!SpendChadCoupons(SingleUseBuffCostCC))
	{
		return false;
	}

	(*PendingStates)[StatIndex] = static_cast<uint8>(FMath::Min<int32>(255, static_cast<int32>((*PendingStates)[StatIndex]) + 1));
	RebuildSelectedSingleUseStatesFromLoadout();
	Save();
	return true;
}

TMap<ET66SecondaryStatType, float> UT66BuffSubsystem::GetPendingSingleUseBuffMultipliers() const
{
	TMap<ET66SecondaryStatType, float> Bonuses;
	const TArray<uint8>* OwnedStates = GetPendingSingleUseStates();
	const TArray<uint8>* SelectedStates = GetSelectedSingleUseStates();
	if (!OwnedStates || !SelectedStates)
	{
		return Bonuses;
	}

	TArray<uint8> NormalizedSelected;
	BuildSelectedSingleUseStateSnapshot(*OwnedStates, NormalizedSelected);
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(GSingleUseBuffStats); ++Index)
	{
		if (NormalizedSelected.IsValidIndex(Index) && NormalizedSelected[Index] > 0)
		{
			Bonuses.Add(GSingleUseBuffStats[Index], FMath::Pow(SingleUseSecondaryBuffMultiplier, static_cast<float>(NormalizedSelected[Index])));
		}
	}

	return Bonuses;
}

TMap<ET66SecondaryStatType, float> UT66BuffSubsystem::ConsumePendingSingleUseBuffMultipliers()
{
	TMap<ET66SecondaryStatType, float> Bonuses;
	TArray<uint8>* OwnedStates = GetPendingSingleUseStates();
	TArray<uint8>* SelectedStates = GetSelectedSingleUseStates();
	if (!SaveData || !OwnedStates || !SelectedStates)
	{
		return Bonuses;
	}

	EnsurePendingSingleUseStatesSize(*OwnedStates);
	EnsureSelectedSingleUseStatesSize(*SelectedStates);
	TArray<uint8> SelectedSnapshot;
	BuildSelectedSingleUseStateSnapshot(*OwnedStates, SelectedSnapshot);
	bool bConsumedAny = false;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(GSingleUseBuffStats); ++Index)
	{
		const int32 SelectedCount = SelectedSnapshot.IsValidIndex(Index) ? static_cast<int32>(SelectedSnapshot[Index]) : 0;
		const int32 OwnedCount = OwnedStates->IsValidIndex(Index) ? static_cast<int32>((*OwnedStates)[Index]) : 0;
		const int32 ConsumedCount = FMath::Min(SelectedCount, OwnedCount);
		if (ConsumedCount > 0)
		{
			Bonuses.Add(GSingleUseBuffStats[Index], FMath::Pow(SingleUseSecondaryBuffMultiplier, static_cast<float>(ConsumedCount)));
			(*OwnedStates)[Index] = static_cast<uint8>(OwnedCount - ConsumedCount);
			bConsumedAny = true;
		}
	}

	if (bConsumedAny)
	{
		RebuildSelectedSingleUseStatesFromLoadout();
		Save();
	}

	return Bonuses;
}

int32 UT66BuffSubsystem::GetRandomBonusForStat(ET66HeroStatType StatType) const
{
	if (!SaveData) return 0;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      return SaveData->RandomBonusDamage;
		case ET66HeroStatType::AttackSpeed: return SaveData->RandomBonusAttackSpeed;
		case ET66HeroStatType::AttackScale: return SaveData->RandomBonusAttackScale;
		case ET66HeroStatType::Accuracy:    return SaveData->RandomBonusAccuracy;
		case ET66HeroStatType::Armor:      return SaveData->RandomBonusArmor;
		case ET66HeroStatType::Evasion:    return SaveData->RandomBonusEvasion;
		case ET66HeroStatType::Luck:       return SaveData->RandomBonusLuck;
		default: return 0;
	}
}
