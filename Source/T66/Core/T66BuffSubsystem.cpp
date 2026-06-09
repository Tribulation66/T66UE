// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66BuffSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BuffSaveGame.h"
#include "Core/T66ReleaseVariantSubsystem.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Buff, Log, All);

const FString UT66BuffSubsystem::BuffSaveSlotName(TEXT("T66_PowerUp"));

namespace
{
	static const ET66StatType GSingleUseBuffStats[28] = {
		ET66StatType::AoeDamage,
		ET66StatType::BounceDamage,
		ET66StatType::PierceDamage,
		ET66StatType::DotDamage,
		ET66StatType::AoeSpeed,
		ET66StatType::BounceSpeed,
		ET66StatType::PierceSpeed,
		ET66StatType::DotSpeed,
		ET66StatType::AoeScale,
		ET66StatType::BounceScale,
		ET66StatType::PierceScale,
		ET66StatType::DotScale,
		ET66StatType::CritChance,
		ET66StatType::HeadshotChance,
		ET66StatType::AttackRange,
		ET66StatType::Execute,
		ET66StatType::DamageReduction,
		ET66StatType::ReflectDamage,
		ET66StatType::Taunt,
		ET66StatType::Crush,
		ET66StatType::EvasionChance,
		ET66StatType::CounterAttack,
		ET66StatType::Invisibility,
		ET66StatType::Assassinate,
		ET66StatType::InteractableLuck,
		ET66StatType::StealingLuck,
		ET66StatType::GamblingLuck,
		ET66StatType::ProcLuck
	};

	static FT66RelicDefinition T66MakeBaseStatRelic(const TCHAR* RelicID, const FText& DisplayName, const ET66HeroStatType StatType)
	{
		FT66RelicDefinition Def;
		Def.RelicID = FName(RelicID);
		Def.DisplayName = DisplayName;
		Def.BaseStatType = StatType;
		Def.BonusStatPoints = UT66BuffSubsystem::RelicPermanentBonusStatPoints;
		Def.CostCC = UT66BuffSubsystem::RelicUnlockCostCC;
		return Def;
	}

	static FT66RelicDefinition T66MakeStatRelic(const TCHAR* RelicID, const FText& DisplayName, const ET66StatType StatType)
	{
		FT66RelicDefinition Def;
		Def.RelicID = FName(RelicID);
		Def.DisplayName = DisplayName;
		Def.StatType = StatType;
		Def.bUsesStat = true;
		Def.BonusStatPoints = UT66BuffSubsystem::RelicPermanentBonusStatPoints;
		Def.CostCC = UT66BuffSubsystem::RelicUnlockCostCC;
		return Def;
	}

	static FT66RelicDefinition T66MakeSolomonsRingRelic()
	{
		FT66RelicDefinition Def;
		Def.RelicID = FName(TEXT("Relic_SolomonsRing"));
		Def.DisplayName = NSLOCTEXT("T66.Relics", "SolomonsRing", "Solomon's Ring");
		Def.bIsSolomonsRing = true;
		Def.BonusStatPoints = 0;
		Def.CostCC = UT66BuffSubsystem::RelicUnlockCostCC;
		return Def;
	}

	static ET66ItemRarity T66RelicTierValueToRarity(const int32 TierValue)
	{
		switch (FMath::Clamp(TierValue, 1, UT66BuffSubsystem::MaxRelicRarityTier))
		{
		case 2:
			return ET66ItemRarity::Red;
		case 3:
			return ET66ItemRarity::Yellow;
		case 4:
			return ET66ItemRarity::White;
		default:
			return ET66ItemRarity::Black;
		}
	}
}

const TArray<FT66RelicDefinition>& UT66BuffSubsystem::GetAllRelicDefinitions()
{
	static const TArray<FT66RelicDefinition> Relics = {
		T66MakeBaseStatRelic(TEXT("Relic_BloodCrown"), NSLOCTEXT("T66.Relics", "BloodCrown", "Blood Crown"), ET66HeroStatType::Damage),
		T66MakeBaseStatRelic(TEXT("Relic_ClockworkFang"), NSLOCTEXT("T66.Relics", "ClockworkFang", "Clockwork Fang"), ET66HeroStatType::AttackSpeed),
		T66MakeBaseStatRelic(TEXT("Relic_GiantsThumb"), NSLOCTEXT("T66.Relics", "GiantsThumb", "Giant's Thumb"), ET66HeroStatType::AttackScale),
		T66MakeBaseStatRelic(TEXT("Relic_HawkEyeLens"), NSLOCTEXT("T66.Relics", "HawkEyeLens", "Hawk Eye Lens"), ET66HeroStatType::Accuracy),
		T66MakeBaseStatRelic(TEXT("Relic_IronHalo"), NSLOCTEXT("T66.Relics", "IronHalo", "Iron Halo"), ET66HeroStatType::Armor),
		T66MakeBaseStatRelic(TEXT("Relic_MirageCloak"), NSLOCTEXT("T66.Relics", "MirageCloak", "Mirage Cloak"), ET66HeroStatType::Evasion),
		T66MakeBaseStatRelic(TEXT("Relic_CloverIdol"), NSLOCTEXT("T66.Relics", "CloverIdol", "Clover Idol"), ET66HeroStatType::Luck),
		T66MakeBaseStatRelic(TEXT("Relic_WindAnklet"), NSLOCTEXT("T66.Relics", "WindAnklet", "Wind Anklet"), ET66HeroStatType::Speed),
		T66MakeStatRelic(TEXT("Relic_EmberBrand"), NSLOCTEXT("T66.Relics", "EmberBrand", "Ember Brand"), ET66StatType::FirePower),
		T66MakeStatRelic(TEXT("Relic_FrostShard"), NSLOCTEXT("T66.Relics", "FrostShard", "Frost Shard"), ET66StatType::IcePower),
		T66MakeStatRelic(TEXT("Relic_StormCoil"), NSLOCTEXT("T66.Relics", "StormCoil", "Storm Coil"), ET66StatType::ElectricityPower),
		T66MakeStatRelic(TEXT("Relic_RootseedCharm"), NSLOCTEXT("T66.Relics", "RootseedCharm", "Rootseed Charm"), ET66StatType::NaturePower),
		T66MakeStatRelic(TEXT("Relic_GalePinion"), NSLOCTEXT("T66.Relics", "GalePinion", "Gale Pinion"), ET66StatType::WindPower),
		T66MakeSolomonsRingRelic()
	};
	return Relics;
}

const TArray<ET66StatType>& UT66BuffSubsystem::GetAllSingleUseBuffTypes()
{
	static const TArray<ET66StatType> BuffTypes = []()
	{
		TArray<ET66StatType> Out;
		Out.Reserve(UE_ARRAY_COUNT(GSingleUseBuffStats));
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(GSingleUseBuffStats); ++Index)
		{
			if (T66IsLiveStatType(GSingleUseBuffStats[Index]))
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
	Collection.InitializeDependency(UT66ShutdownSubsystem::StaticClass());
	Super::Initialize(Collection);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			ShutdownParticipantHandle = Shutdown->RegisterParticipant(
				this,
				FName(TEXT("Buff.ProgressionSave")),
				ET66ShutdownPhase::DurableState,
				20,
				1.0,
				true,
				FT66ShutdownParticipantDelegate::CreateUObject(this, &UT66BuffSubsystem::HandleShutdown));
		}
	}
	LoadOrCreateSave();
}

void UT66BuffSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			Shutdown->UnregisterParticipant(ShutdownParticipantHandle);
		}
	}
	ShutdownParticipantHandle.Reset();
	FlushPendingDurableState(TEXT("Deinitialize"));
	Super::Deinitialize();
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
		MigrateV5ToV6StatSingleUseBuffs();
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

	if (SaveData->SaveVersion < 11)
	{
		MigrateV10ToV11PrimarySpeed();
		SaveData->SaveVersion = 11;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 12)
	{
		MigrateV11ToV12Relics();
		SaveData->SaveVersion = 12;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 13)
	{
		MigrateV12ToV13RelicTiers();
		SaveData->SaveVersion = 13;
		bNeedsSave = true;
	}

	EnsurePendingSingleUseStatesSize(SaveData->PendingSingleUseBuffStates);
	EnsureSelectedSingleUseStatesSize(SaveData->SelectedSingleUseBuffStates);
	SanitizeSelectedSingleUseStates(SaveData->SelectedSingleUseBuffStates, SaveData->PendingSingleUseBuffStates);
	bNeedsSave |= EnsureSelectedSingleUseBuffLoadoutValid();
	bNeedsSave |= RebuildSelectedSingleUseStatesFromLoadout();
	bNeedsSave |= EnsureRelicOwnershipValid();

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

void UT66BuffSubsystem::MigrateV5ToV6StatSingleUseBuffs()
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
	for (ET66StatType StatType : GetAllSingleUseBuffTypes())
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
	for (ET66StatType StatType : GetAllSingleUseBuffTypes())
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

	SaveData->SelectedSingleUseBuffSlots.Init(ET66StatType::None, SelectedSingleUseBuffSlotCount);
	int32 SlotIndex = 0;

	if (SaveData->TemporaryBuffPresets.Num() > 0)
	{
		const int32 LegacyPresetIndex = FMath::Clamp(
			SaveData->ActiveTemporaryBuffPresetIndex,
			0,
			SaveData->TemporaryBuffPresets.Num() - 1);
		TArray<ET66StatType> LegacySlots = SaveData->TemporaryBuffPresets[LegacyPresetIndex].SlotBuffs;
		EnsureSelectedSingleUseBuffSlotsSize(LegacySlots);
		for (ET66StatType SlotStat : LegacySlots)
		{
			if (SlotIndex >= SelectedSingleUseBuffSlotCount)
			{
				break;
			}

			SaveData->SelectedSingleUseBuffSlots[SlotIndex++] = T66IsLiveStatType(SlotStat)
				? SlotStat
				: ET66StatType::None;
		}
	}
	else
	{
		for (ET66StatType StatType : GetAllSingleUseBuffTypes())
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
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v9 temp-buff presets to v10 single temp-buff loadout."));
}

void UT66BuffSubsystem::MigrateV10ToV11PrimarySpeed()
{
	if (!SaveData)
	{
		return;
	}

	EnsureFillStepStatesSize(SaveData->WedgeTiersSpeed);
	SaveData->RandomBonusSpeed = FMath::Max(0, SaveData->RandomBonusSpeed);
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v10 saves to v11 primary Speed progression."));
}

void UT66BuffSubsystem::MigrateV11ToV12Relics()
{
	if (!SaveData)
	{
		return;
	}

	auto GetLegacyUnlockedFillStepCount = [this](const ET66HeroStatType StatType) -> int32
	{
		const TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
		if (!Arr)
		{
			return 0;
		}

		int32 Count = 0;
		for (int32 i = 0; i < Arr->Num() && i < MaxFillStepsPerStat; ++i)
		{
			if ((*Arr)[i] > 0)
			{
				++Count;
			}
		}
		return Count;
	};

	auto PromoteLegacyBaseStat = [this, GetLegacyUnlockedFillStepCount](const ET66HeroStatType StatType)
	{
		if (GetLegacyUnlockedFillStepCount(StatType) > 0 || GetRandomBonusForStat(StatType) > 0)
		{
			const FName RelicID = GetRelicIDForBaseStat(StatType);
			if (!RelicID.IsNone())
			{
				SaveData->OwnedRelicIDs.AddUnique(RelicID);
			}
		}
	};

	PromoteLegacyBaseStat(ET66HeroStatType::Damage);
	PromoteLegacyBaseStat(ET66HeroStatType::AttackSpeed);
	PromoteLegacyBaseStat(ET66HeroStatType::AttackScale);
	PromoteLegacyBaseStat(ET66HeroStatType::Accuracy);
	PromoteLegacyBaseStat(ET66HeroStatType::Armor);
	PromoteLegacyBaseStat(ET66HeroStatType::Evasion);
	PromoteLegacyBaseStat(ET66HeroStatType::Luck);
	PromoteLegacyBaseStat(ET66HeroStatType::Speed);
	EnsureRelicOwnershipValid();
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v11 permanent fill steps to v12 flat Relics."));
}

void UT66BuffSubsystem::MigrateV12ToV13RelicTiers()
{
	if (!SaveData)
	{
		return;
	}

	for (const FName RelicID : SaveData->OwnedRelicIDs)
	{
		const FT66RelicDefinition* Def = FindRelicDefinition(RelicID);
		if (Def && !Def->bIsSolomonsRing)
		{
			SaveData->RelicTierValues.FindOrAdd(RelicID) = 1;
		}
	}

	EnsureRelicOwnershipValid();
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v12 flat Relics to v13 per-rarity Relic tiers."));
}

void UT66BuffSubsystem::Save()
{
	if (SaveData)
	{
		const int64 SaveSequence = ++BuffSaveAsyncSequence;
		PendingBuffSaveSequence = SaveSequence;
		bBuffSaveFlushNeeded = true;

		TWeakObjectPtr<UT66BuffSubsystem> WeakThis(this);
		UGameplayStatics::AsyncSaveGameToSlot(SaveData, BuffSaveSlotName, BuffSaveUserIndex,
			FAsyncSaveGameToSlotDelegate::CreateLambda([WeakThis, SaveSequence](const FString& /*InSlotName*/, const int32 /*UserIndex*/, bool bSuccess)
			{
				if (UT66BuffSubsystem* BuffSubsystem = WeakThis.Get())
				{
					if (bSuccess && BuffSubsystem->PendingBuffSaveSequence == SaveSequence)
					{
						BuffSubsystem->bBuffSaveFlushNeeded = false;
					}
				}

				if (!bSuccess)
				{
					UE_LOG(LogT66Buff, Warning, TEXT("[Buffs] Async save failed."));
				}
			}));
	}
}

bool UT66BuffSubsystem::HandleShutdown(const FT66ShutdownContext& /*Context*/)
{
	return FlushPendingDurableState(TEXT("ShutdownSystem"));
}

bool UT66BuffSubsystem::FlushPendingDurableState(const TCHAR* Reason)
{
	const bool bHadPending = bBuffSaveFlushNeeded;
	bool bOk = true;

	if (bHadPending)
	{
		const bool bSaved = SaveData && UGameplayStatics::SaveGameToSlot(SaveData, BuffSaveSlotName, BuffSaveUserIndex);
		UE_LOG(LogT66Buff, Log, TEXT("[Shutdown] Durable flush buff save Saved=%d Reason=%s"),
			bSaved ? 1 : 0,
			Reason ? Reason : TEXT("Unknown"));
		if (bSaved)
		{
			bBuffSaveFlushNeeded = false;
		}
		bOk &= bSaved;
	}

	UE_LOG(LogT66Buff, Log, TEXT("[Shutdown] Durable flush complete BuffSavePending=%d Success=%d Reason=%s"),
		bHadPending ? 1 : 0,
		bOk ? 1 : 0,
		Reason ? Reason : TEXT("Unknown"));
	return bOk;
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
	for (ET66StatType StatType : GetAllSingleUseBuffTypes())
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

void UT66BuffSubsystem::EnsureSelectedSingleUseBuffSlotsSize(TArray<ET66StatType>& Slots) const
{
	if (Slots.Num() < SelectedSingleUseBuffSlotCount)
	{
		const int32 OriginalNum = Slots.Num();
		Slots.SetNum(SelectedSingleUseBuffSlotCount);
		for (int32 Index = OriginalNum; Index < SelectedSingleUseBuffSlotCount; ++Index)
		{
			Slots[Index] = ET66StatType::None;
		}
	}
	else if (Slots.Num() > SelectedSingleUseBuffSlotCount)
	{
		Slots.SetNum(SelectedSingleUseBuffSlotCount);
	}

	for (ET66StatType& SlotStat : Slots)
	{
		if (SlotStat != ET66StatType::None && !T66IsLiveStatType(SlotStat))
		{
			SlotStat = ET66StatType::None;
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

		TArray<ET66StatType> Slots = SaveData->SelectedSingleUseBuffSlots;
		const_cast<UT66BuffSubsystem*>(this)->EnsureSelectedSingleUseBuffSlotsSize(Slots);

		int32 TotalSelected = 0;
		for (ET66StatType SlotStat : Slots)
		{
			if (!T66IsLiveStatType(SlotStat) || TotalSelected >= MaxSelectedSingleUseBuffs)
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

		const TArray<ET66StatType> OriginalSlots = SaveData->SelectedSingleUseBuffSlots;
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
	case ET66HeroStatType::Speed:       return 7;
	default:                            return INDEX_NONE;
	}
}

FName UT66BuffSubsystem::GetRelicIDForBaseStat(const ET66HeroStatType StatType) const
{
	for (const FT66RelicDefinition& Def : GetAllRelicDefinitions())
	{
		if (!Def.bUsesStat && !Def.bIsSolomonsRing && Def.BaseStatType == StatType)
		{
			return Def.RelicID;
		}
	}
	return NAME_None;
}

const FT66RelicDefinition* UT66BuffSubsystem::FindRelicDefinition(const FName RelicID) const
{
	if (RelicID.IsNone())
	{
		return nullptr;
	}

	for (const FT66RelicDefinition& Def : GetAllRelicDefinitions())
	{
		if (Def.RelicID == RelicID)
		{
			return &Def;
		}
	}
	return nullptr;
}

bool UT66BuffSubsystem::EnsureRelicOwnershipValid()
{
	if (!SaveData)
	{
		return false;
	}

	const TArray<FName> Original = SaveData->OwnedRelicIDs;
	const TMap<FName, uint8> OriginalTiers = SaveData->RelicTierValues;
	TArray<FName> Sanitized;
	for (const FName RelicID : Original)
	{
		if (!RelicID.IsNone() && FindRelicDefinition(RelicID))
		{
			Sanitized.AddUnique(RelicID);
		}
	}

	TMap<FName, uint8> SanitizedTiers;
	for (const TPair<FName, uint8>& Pair : OriginalTiers)
	{
		const FT66RelicDefinition* Def = FindRelicDefinition(Pair.Key);
		if (Def && !Def->bIsSolomonsRing && Pair.Value > 0)
		{
			SanitizedTiers.Add(Pair.Key, static_cast<uint8>(FMath::Clamp(static_cast<int32>(Pair.Value), 1, MaxRelicRarityTier)));
			Sanitized.AddUnique(Pair.Key);
		}
	}

	for (const FName RelicID : Sanitized)
	{
		const FT66RelicDefinition* Def = FindRelicDefinition(RelicID);
		if (Def && !Def->bIsSolomonsRing && !SanitizedTiers.Contains(RelicID))
		{
			SanitizedTiers.Add(RelicID, 1);
		}
	}

	bool bRelicTierValuesChanged = OriginalTiers.Num() != SanitizedTiers.Num();
	if (!bRelicTierValuesChanged)
	{
		for (const TPair<FName, uint8>& Pair : OriginalTiers)
		{
			const uint8* NewValue = SanitizedTiers.Find(Pair.Key);
			if (!NewValue || *NewValue != Pair.Value)
			{
				bRelicTierValuesChanged = true;
				break;
			}
		}
	}

	SaveData->OwnedRelicIDs = MoveTemp(Sanitized);
	SaveData->RelicTierValues = MoveTemp(SanitizedTiers);
	return SaveData->OwnedRelicIDs != Original || bRelicTierValuesChanged;
}

int32 UT66BuffSubsystem::GetSingleUseBuffIndex(ET66StatType StatType) const
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
	case ET66HeroStatType::Speed:       Bonuses.Speed += Amount; break;
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
		case ET66HeroStatType::Speed:      return &SaveData->WedgeTiersSpeed;
		default: return nullptr;
	}
}

const TArray<uint8>* UT66BuffSubsystem::GetFillStepStatesForStat(ET66HeroStatType StatType) const
{
	return const_cast<UT66BuffSubsystem*>(this)->GetFillStepStatesForStat(StatType);
}

int32 UT66BuffSubsystem::GetFillStepState(ET66HeroStatType StatType, int32 StepIndex) const
{
	if (StepIndex < 0 || StepIndex >= MaxFillStepsPerStat)
	{
		return 0;
	}

	const FName RelicID = GetRelicIDForBaseStat(StatType);
	return (!RelicID.IsNone() && StepIndex < GetRelicTierValue(RelicID)) ? 1 : 0;
}

int32 UT66BuffSubsystem::GetUnlockedFillStepCount(ET66HeroStatType StatType) const
{
	const FName RelicID = GetRelicIDForBaseStat(StatType);
	return RelicID.IsNone() ? 0 : GetRelicTierValue(RelicID);
}

int32 UT66BuffSubsystem::GetTotalStatBonus(ET66HeroStatType StatType) const
{
	return GetRelicBaseStatBonus(StatType);
}

int32 UT66BuffSubsystem::GetCostForNextFillStepUnlock(ET66HeroStatType StatType) const
{
	return IsStatMaxed(StatType) ? 0 : RelicUnlockCostCC;
}

bool UT66BuffSubsystem::UnlockNextFillStep(ET66HeroStatType StatType)
{
	const FName RelicID = GetRelicIDForBaseStat(StatType);
	return !RelicID.IsNone() && PurchaseRelic(RelicID);
}

bool UT66BuffSubsystem::UnlockRandomStat()
{
	if (!SaveData)
	{
		return false;
	}

	TArray<FName> Candidates;
	for (const FT66RelicDefinition& Def : GetAllRelicDefinitions())
	{
		if (!Def.bUsesStat && !Def.bIsSolomonsRing && !IsRelicMaxTier(Def.RelicID))
		{
			Candidates.Add(Def.RelicID);
		}
	}

	if (Candidates.Num() <= 0)
	{
		return false;
	}

	return PurchaseRelic(Candidates[FMath::RandRange(0, Candidates.Num() - 1)]);
}

bool UT66BuffSubsystem::IsStatMaxed(ET66HeroStatType StatType) const
{
	const FName RelicID = GetRelicIDForBaseStat(StatType);
	return RelicID.IsNone() || IsRelicMaxTier(RelicID);
}

bool UT66BuffSubsystem::IsDemoDiplomaUpgradeLimitReached(ET66HeroStatType StatType) const
{
	static_cast<void>(StatType);
	return false;
}

FT66HeroStatBonuses UT66BuffSubsystem::GetPermanentBuffStatBonuses() const
{
	FT66HeroStatBonuses Bonuses;
	Bonuses.Damage = GetTotalStatBonus(ET66HeroStatType::Damage);
	Bonuses.AttackSpeed = GetTotalStatBonus(ET66HeroStatType::AttackSpeed);
	Bonuses.AttackScale = GetTotalStatBonus(ET66HeroStatType::AttackScale);
	Bonuses.Accuracy = GetTotalStatBonus(ET66HeroStatType::Accuracy);
	Bonuses.Armor = GetTotalStatBonus(ET66HeroStatType::Armor);
	Bonuses.Evasion = GetTotalStatBonus(ET66HeroStatType::Evasion);
	Bonuses.Luck = GetTotalStatBonus(ET66HeroStatType::Luck);
	Bonuses.Speed = GetTotalStatBonus(ET66HeroStatType::Speed);
	return Bonuses;
}

FT66HeroStatBonuses UT66BuffSubsystem::GetPowerupStatBonuses() const
{
	return GetPermanentBuffStatBonuses();
}

bool UT66BuffSubsystem::IsRelicOwned(const FName RelicID) const
{
	return SaveData && !RelicID.IsNone() && (SaveData->OwnedRelicIDs.Contains(RelicID) || GetRelicTierValue(RelicID) > 0);
}

bool UT66BuffSubsystem::PurchaseRelic(const FName RelicID)
{
	if (!SaveData || RelicID.IsNone())
	{
		return false;
	}

	const FT66RelicDefinition* Def = FindRelicDefinition(RelicID);
	if (!Def)
	{
		return false;
	}

	const int32 CurrentTier = GetRelicTierValue(RelicID);
	if (Def->bIsSolomonsRing)
	{
		if (IsRelicOwned(RelicID))
		{
			return false;
		}
	}
	else if (CurrentTier >= MaxRelicRarityTier)
	{
		return false;
	}

	if (!SpendChadCoupons(FMath::Max(0, Def->CostCC)))
	{
		return false;
	}

	SaveData->OwnedRelicIDs.AddUnique(RelicID);
	if (!Def->bIsSolomonsRing)
	{
		SaveData->RelicTierValues.FindOrAdd(RelicID) = static_cast<uint8>(FMath::Clamp(CurrentTier + 1, 1, MaxRelicRarityTier));
	}
	EnsureRelicOwnershipValid();
	Save();
	return true;
}

int32 UT66BuffSubsystem::GetRelicCost(const FName RelicID) const
{
	const FT66RelicDefinition* Def = FindRelicDefinition(RelicID);
	if (!Def || IsRelicMaxTier(RelicID))
	{
		return 0;
	}
	return Def ? FMath::Max(0, Def->CostCC) : 0;
}

int32 UT66BuffSubsystem::GetRelicTierValue(const FName RelicID) const
{
	if (!SaveData || RelicID.IsNone())
	{
		return 0;
	}

	const FT66RelicDefinition* Def = FindRelicDefinition(RelicID);
	if (!Def)
	{
		return 0;
	}

	if (Def->bIsSolomonsRing)
	{
		return SaveData->OwnedRelicIDs.Contains(RelicID) ? 1 : 0;
	}

	if (const uint8* Tier = SaveData->RelicTierValues.Find(RelicID))
	{
		return FMath::Clamp(static_cast<int32>(*Tier), 0, MaxRelicRarityTier);
	}

	return SaveData->OwnedRelicIDs.Contains(RelicID) ? 1 : 0;
}

ET66ItemRarity UT66BuffSubsystem::GetRelicRarity(const FName RelicID) const
{
	return T66RelicTierValueToRarity(FMath::Max(1, GetRelicTierValue(RelicID)));
}

bool UT66BuffSubsystem::IsRelicMaxTier(const FName RelicID) const
{
	const FT66RelicDefinition* Def = FindRelicDefinition(RelicID);
	if (!Def)
	{
		return true;
	}

	if (Def->bIsSolomonsRing)
	{
		return IsRelicOwned(RelicID);
	}

	return GetRelicTierValue(RelicID) >= MaxRelicRarityTier;
}

bool UT66BuffSubsystem::HasSolomonsRing() const
{
	return IsRelicOwned(FName(TEXT("Relic_SolomonsRing")));
}

int32 UT66BuffSubsystem::GetRelicBaseStatBonus(const ET66HeroStatType StatType) const
{
	int32 Total = 0;
	for (const FT66RelicDefinition& Def : GetAllRelicDefinitions())
	{
		if (!Def.bUsesStat && !Def.bIsSolomonsRing && Def.BaseStatType == StatType && IsRelicOwned(Def.RelicID))
		{
			Total += FMath::Max(0, Def.BonusStatPoints) * GetRelicTierValue(Def.RelicID);
		}
	}
	return Total;
}

int32 UT66BuffSubsystem::GetRelicStatBonus(const ET66StatType StatType) const
{
	int32 Total = 0;
	for (const FT66RelicDefinition& Def : GetAllRelicDefinitions())
	{
		if (Def.bUsesStat && Def.StatType == StatType && IsRelicOwned(Def.RelicID))
		{
			Total += FMath::Max(0, Def.BonusStatPoints) * GetRelicTierValue(Def.RelicID);
		}
	}
	return Total;
}

bool UT66BuffSubsystem::HasPendingSingleUseBuff(ET66StatType StatType) const
{
	return GetOwnedSingleUseBuffCount(StatType) > 0;
}

int32 UT66BuffSubsystem::GetOwnedSingleUseBuffCount(ET66StatType StatType) const
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

bool UT66BuffSubsystem::IsSingleUseBuffSelected(ET66StatType StatType) const
{
	return GetSelectedSingleUseBuffCountForStat(StatType) > 0;
}

int32 UT66BuffSubsystem::GetSelectedSingleUseBuffCountForStat(ET66StatType StatType) const
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

TArray<ET66StatType> UT66BuffSubsystem::GetOwnedSingleUseBuffs() const
{
	TArray<ET66StatType> OwnedBuffs;
	for (ET66StatType StatType : GetAllSingleUseBuffTypes())
	{
		if (HasPendingSingleUseBuff(StatType))
		{
			OwnedBuffs.Add(StatType);
		}
	}
	return OwnedBuffs;
}

TArray<ET66StatType> UT66BuffSubsystem::GetSelectedSingleUseBuffs() const
{
	TArray<ET66StatType> SelectedBuffs;
	for (ET66StatType StatType : GetAllSingleUseBuffTypes())
	{
		const int32 SelectedCount = GetSelectedSingleUseBuffCountForStat(StatType);
		for (int32 CopyIndex = 0; CopyIndex < SelectedCount; ++CopyIndex)
		{
			SelectedBuffs.Add(StatType);
		}
	}
	return SelectedBuffs;
}

TArray<ET66StatType> UT66BuffSubsystem::GetSelectedSingleUseBuffSlots() const
{
	TArray<ET66StatType> Slots = SaveData ? SaveData->SelectedSingleUseBuffSlots : TArray<ET66StatType>{};
	const_cast<UT66BuffSubsystem*>(this)->EnsureSelectedSingleUseBuffSlotsSize(Slots);
	return Slots;
}

ET66StatType UT66BuffSubsystem::GetSelectedSingleUseBuffSlot(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SelectedSingleUseBuffSlotCount || !SaveData)
	{
		return ET66StatType::None;
	}

	TArray<ET66StatType> Slots = SaveData->SelectedSingleUseBuffSlots;
	const_cast<UT66BuffSubsystem*>(this)->EnsureSelectedSingleUseBuffSlotsSize(Slots);
	return Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : ET66StatType::None;
}

bool UT66BuffSubsystem::SetSelectedSingleUseBuffSlot(int32 SlotIndex, ET66StatType StatType)
{
	if (!SaveData || SlotIndex < 0 || SlotIndex >= SelectedSingleUseBuffSlotCount)
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	const ET66StatType SanitizedStatType = (StatType == ET66StatType::None || T66IsLiveStatType(StatType))
		? StatType
		: ET66StatType::None;
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
	return SetSelectedSingleUseBuffSlot(SlotIndex, ET66StatType::None);
}

bool UT66BuffSubsystem::IsSelectedSingleUseBuffSlotOwned(int32 SlotIndex) const
{
	const ET66StatType SlotStat = GetSelectedSingleUseBuffSlot(SlotIndex);
	if (!T66IsLiveStatType(SlotStat))
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

int32 UT66BuffSubsystem::GetSelectedSingleUseBuffSlotAssignedCountForStat(ET66StatType StatType) const
{
	if (!T66IsLiveStatType(StatType))
	{
		return 0;
	}

	int32 Count = 0;
	for (ET66StatType SlotStat : GetSelectedSingleUseBuffSlots())
	{
		if (SlotStat == StatType)
		{
			++Count;
		}
	}
	return Count;
}

bool UT66BuffSubsystem::AreSingleUseBuffPurchasesAllowed() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66ReleaseVariantSubsystem* ReleaseVariant = GI->GetSubsystem<UT66ReleaseVariantSubsystem>())
		{
			return ReleaseVariant->AreDrugPurchasesAllowed();
		}
	}

	return false;
}

bool UT66BuffSubsystem::PurchaseSelectedSingleUseBuffSlot(int32 SlotIndex)
{
	const ET66StatType SlotStat = GetSelectedSingleUseBuffSlot(SlotIndex);
	return T66IsLiveStatType(SlotStat) ? PurchaseSingleUseBuff(SlotStat) : false;
}

void UT66BuffSubsystem::SetSelectedSingleUseBuffEditSlotIndex(int32 SlotIndex)
{
	ActiveSelectedSingleUseBuffEditSlotIndex = FMath::Clamp(SlotIndex, 0, SelectedSingleUseBuffSlotCount - 1);
}

int32 UT66BuffSubsystem::GetSelectedSingleUseBuffEditSlotIndex() const
{
	return FMath::Clamp(ActiveSelectedSingleUseBuffEditSlotIndex, 0, SelectedSingleUseBuffSlotCount - 1);
}

void UT66BuffSubsystem::BeginHeroSelectionSingleUseBuffEdit(int32 SlotIndex)
{
	SetSelectedSingleUseBuffEditSlotIndex(SlotIndex);
	bHeroSelectionSingleUseBuffEditActive = true;
}

void UT66BuffSubsystem::EndHeroSelectionSingleUseBuffEdit()
{
	bHeroSelectionSingleUseBuffEditActive = false;
}

bool UT66BuffSubsystem::SetSingleUseBuffSelected(ET66StatType StatType, bool bSelected)
{
	if (bSelected)
	{
		return AddSelectedSingleUseBuff(StatType);
	}

	if (!SaveData || !T66IsLiveStatType(StatType))
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	bool bChanged = false;
	for (ET66StatType& SlotStat : SaveData->SelectedSingleUseBuffSlots)
	{
		if (SlotStat == StatType)
		{
			SlotStat = ET66StatType::None;
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

bool UT66BuffSubsystem::AddSelectedSingleUseBuff(ET66StatType StatType)
{
	if (!SaveData || !T66IsLiveStatType(StatType))
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	for (int32 SlotIndex = 0; SlotIndex < SaveData->SelectedSingleUseBuffSlots.Num(); ++SlotIndex)
	{
		if (SaveData->SelectedSingleUseBuffSlots[SlotIndex] == ET66StatType::None)
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

bool UT66BuffSubsystem::RemoveSelectedSingleUseBuff(ET66StatType StatType)
{
	if (!SaveData || !T66IsLiveStatType(StatType))
	{
		return false;
	}

	EnsureSelectedSingleUseBuffLoadoutValid();
	for (int32 SlotIndex = SaveData->SelectedSingleUseBuffSlots.Num() - 1; SlotIndex >= 0; --SlotIndex)
	{
		if (SaveData->SelectedSingleUseBuffSlots[SlotIndex] == StatType)
		{
			SaveData->SelectedSingleUseBuffSlots[SlotIndex] = ET66StatType::None;
			ActiveSelectedSingleUseBuffEditSlotIndex = SlotIndex;
			RebuildSelectedSingleUseStatesFromLoadout();
			Save();
			return true;
		}
	}

	return false;
}

bool UT66BuffSubsystem::PurchaseSingleUseBuff(ET66StatType StatType)
{
	if (!AreSingleUseBuffPurchasesAllowed())
	{
		return false;
	}

	TArray<uint8>* PendingStates = GetPendingSingleUseStates();
	TArray<uint8>* SelectedStates = GetSelectedSingleUseStates();
	if (!SaveData || !PendingStates || !SelectedStates)
	{
		return false;
	}

	if (!T66IsLiveStatType(StatType))
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

TMap<ET66StatType, float> UT66BuffSubsystem::GetPendingSingleUseBuffMultipliers() const
{
	TMap<ET66StatType, float> Bonuses;
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
			Bonuses.Add(GSingleUseBuffStats[Index], FMath::Pow(SingleUseStatBuffMultiplier, static_cast<float>(NormalizedSelected[Index])));
		}
	}

	return Bonuses;
}

TMap<ET66StatType, float> UT66BuffSubsystem::ConsumePendingSingleUseBuffMultipliers()
{
	TMap<ET66StatType, float> Bonuses;
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
			Bonuses.Add(GSingleUseBuffStats[Index], FMath::Pow(SingleUseStatBuffMultiplier, static_cast<float>(ConsumedCount)));
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

#if !UE_BUILD_SHIPPING
void UT66BuffSubsystem::DebugSetDiplomaUnlockedSteps(const ET66HeroStatType StatType, const int32 Count)
{
	LoadOrCreateSave();
	if (!SaveData)
	{
		return;
	}

	const FName RelicID = GetRelicIDForBaseStat(StatType);
	if (RelicID.IsNone())
	{
		return;
	}

	if (Count > 0)
	{
		SaveData->OwnedRelicIDs.AddUnique(RelicID);
		SaveData->RelicTierValues.FindOrAdd(RelicID) = static_cast<uint8>(FMath::Clamp(Count, 1, MaxRelicRarityTier));
	}
	else
	{
		SaveData->OwnedRelicIDs.Remove(RelicID);
		SaveData->RelicTierValues.Remove(RelicID);
	}
	EnsureRelicOwnershipValid();
	Save();
}

void UT66BuffSubsystem::DebugGrantSingleUseBuff(const ET66StatType StatType, const int32 Count, const bool bSelectForNextRun)
{
	LoadOrCreateSave();
	if (!SaveData || !T66IsLiveStatType(StatType))
	{
		return;
	}

	const int32 StatIndex = GetSingleUseBuffIndex(StatType);
	if (StatIndex == INDEX_NONE)
	{
		return;
	}

	EnsurePendingSingleUseStatesSize(SaveData->PendingSingleUseBuffStates);
	SaveData->PendingSingleUseBuffStates[StatIndex] = static_cast<uint8>(FMath::Clamp(Count, 0, 255));

	if (bSelectForNextRun)
	{
		SaveData->SelectedSingleUseBuffSlots.Init(ET66StatType::None, MaxSelectedSingleUseBuffs);
		const int32 SelectedCount = FMath::Clamp(Count, 0, MaxSelectedSingleUseBuffs);
		for (int32 SlotIndex = 0; SlotIndex < SelectedCount; ++SlotIndex)
		{
			SaveData->SelectedSingleUseBuffSlots[SlotIndex] = StatType;
		}
		RebuildSelectedSingleUseStatesFromLoadout();
	}
}
#endif

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
		case ET66HeroStatType::Speed:      return SaveData->RandomBonusSpeed;
		default: return 0;
	}
}
