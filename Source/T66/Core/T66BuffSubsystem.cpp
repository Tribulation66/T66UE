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
		ET66StatType::DotDamage,
		ET66StatType::SummonDamage,
		ET66StatType::AoeSpeed,
		ET66StatType::BounceSpeed,
		ET66StatType::DotSpeed,
		ET66StatType::SummonSpeed,
		ET66StatType::AoeScale,
		ET66StatType::BounceScale,
		ET66StatType::DotScale,
		ET66StatType::SummonScale,
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

	static FT66SurgeryDefinition T66MakeBaseStatSurgery(const TCHAR* SurgeryID, const FText& DisplayName, const ET66HeroStatType StatType)
	{
		FT66SurgeryDefinition Def;
		Def.SurgeryID = FName(SurgeryID);
		Def.DisplayName = DisplayName;
		Def.BaseStatType = StatType;
		Def.BonusStatPoints = UT66BuffSubsystem::SurgeryPermanentBonusStatPoints;
		Def.CostCC = UT66BuffSubsystem::SurgeryUnlockCostCC;
		return Def;
	}

	static FT66SurgeryDefinition T66MakeStatSurgery(const TCHAR* SurgeryID, const FText& DisplayName, const ET66StatType StatType)
	{
		FT66SurgeryDefinition Def;
		Def.SurgeryID = FName(SurgeryID);
		Def.DisplayName = DisplayName;
		Def.StatType = StatType;
		Def.bUsesStat = true;
		Def.BonusStatPoints = UT66BuffSubsystem::SurgeryPermanentBonusStatPoints;
		Def.CostCC = UT66BuffSubsystem::SurgeryUnlockCostCC;
		return Def;
	}

	static ET66ItemRarity T66SurgeryTierValueToRarity(const int32 TierValue)
	{
		switch (FMath::Clamp(TierValue, 1, UT66BuffSubsystem::MaxSurgeryRarityTier))
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

const TArray<FT66SurgeryDefinition>& UT66BuffSubsystem::GetAllSurgeryDefinitions()
{
	static const TArray<FT66SurgeryDefinition> Surgeries = {
		T66MakeBaseStatSurgery(TEXT("Surgery_MuscleImplant"), NSLOCTEXT("T66.Surgeries", "MuscleImplant", "Muscle Implant Surgery"), ET66HeroStatType::Damage),
		T66MakeBaseStatSurgery(TEXT("Surgery_NerveReflex"), NSLOCTEXT("T66.Surgeries", "NerveReflex", "Nerve Reflex Surgery"), ET66HeroStatType::AttackSpeed),
		T66MakeBaseStatSurgery(TEXT("Surgery_Jawline"), NSLOCTEXT("T66.Surgeries", "Jawline", "Jawline Surgery"), ET66HeroStatType::AttackScale),
		T66MakeBaseStatSurgery(TEXT("Surgery_Lasik"), NSLOCTEXT("T66.Surgeries", "Lasik", "Lasik Surgery"), ET66HeroStatType::Accuracy),
		T66MakeBaseStatSurgery(TEXT("Surgery_DermalArmor"), NSLOCTEXT("T66.Surgeries", "DermalArmor", "Dermal Armor Surgery"), ET66HeroStatType::Armor),
		T66MakeBaseStatSurgery(TEXT("Surgery_TendonEvasion"), NSLOCTEXT("T66.Surgeries", "TendonEvasion", "Tendon Evasion Surgery"), ET66HeroStatType::Evasion),
		T66MakeBaseStatSurgery(TEXT("Surgery_LuckyGene"), NSLOCTEXT("T66.Surgeries", "LuckyGene", "Lucky Gene Surgery"), ET66HeroStatType::Luck),
		T66MakeBaseStatSurgery(TEXT("Surgery_NasalAirway"), NSLOCTEXT("T66.Surgeries", "NasalAirway", "Nasal Airway Surgery"), ET66HeroStatType::Speed),
		T66MakeStatSurgery(TEXT("Surgery_HeatGland"), NSLOCTEXT("T66.Surgeries", "HeatGland", "Heat Gland Surgery"), ET66StatType::FirePower),
		T66MakeStatSurgery(TEXT("Surgery_CryoLung"), NSLOCTEXT("T66.Surgeries", "CryoLung", "Cryo Lung Surgery"), ET66StatType::IcePower),
		T66MakeStatSurgery(TEXT("Surgery_ElectroNeural"), NSLOCTEXT("T66.Surgeries", "ElectroNeural", "Electro-Neural Surgery"), ET66StatType::ElectricityPower),
		T66MakeStatSurgery(TEXT("Surgery_ChlorophyllSkinGraft"), NSLOCTEXT("T66.Surgeries", "ChlorophyllSkinGraft", "Chlorophyll Skin Graft"), ET66StatType::NaturePower),
		T66MakeStatSurgery(TEXT("Surgery_Windpipe"), NSLOCTEXT("T66.Surgeries", "Windpipe", "Windpipe Surgery"), ET66StatType::WindPower),
	};
	return Surgeries;
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
		MigrateV11ToV12Surgeries();
		SaveData->SaveVersion = 12;
		bNeedsSave = true;
	}

	if (SaveData->SaveVersion < 13)
	{
		MigrateV12ToV13SurgeryTiers();
		SaveData->SaveVersion = 13;
		bNeedsSave = true;
	}

	EnsurePendingSingleUseStatesSize(SaveData->PendingSingleUseBuffStates);
	EnsureSelectedSingleUseStatesSize(SaveData->SelectedSingleUseBuffStates);
	SanitizeSelectedSingleUseStates(SaveData->SelectedSingleUseBuffStates, SaveData->PendingSingleUseBuffStates);
	bNeedsSave |= EnsureSelectedSingleUseBuffLoadoutValid();
	bNeedsSave |= RebuildSelectedSingleUseStatesFromLoadout();
	bNeedsSave |= EnsureSurgeryOwnershipValid();

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

void UT66BuffSubsystem::MigrateV11ToV12Surgeries()
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
			const FName SurgeryID = GetSurgeryIDForBaseStat(StatType);
			if (!SurgeryID.IsNone())
			{
				SaveData->OwnedSurgeryIDs.AddUnique(SurgeryID);
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
	EnsureSurgeryOwnershipValid();
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v11 permanent fill steps to v12 flat Surgeries."));
}

void UT66BuffSubsystem::MigrateV12ToV13SurgeryTiers()
{
	if (!SaveData)
	{
		return;
	}

	for (const FName SurgeryID : SaveData->OwnedSurgeryIDs)
	{
		const FT66SurgeryDefinition* Def = FindSurgeryDefinition(SurgeryID);
		if (Def)
		{
			SaveData->SurgeryTierValues.FindOrAdd(SurgeryID) = 1;
		}
	}

	EnsureSurgeryOwnershipValid();
	UE_LOG(LogT66Buff, Log, TEXT("[Buffs] Migrated v12 flat Surgeries to v13 per-rarity Surgery tiers."));
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

FName UT66BuffSubsystem::GetSurgeryIDForBaseStat(const ET66HeroStatType StatType) const
{
	for (const FT66SurgeryDefinition& Def : GetAllSurgeryDefinitions())
	{
		if (!Def.bUsesStat && Def.BaseStatType == StatType)
		{
			return Def.SurgeryID;
		}
	}
	return NAME_None;
}

const FT66SurgeryDefinition* UT66BuffSubsystem::FindSurgeryDefinition(const FName SurgeryID) const
{
	if (SurgeryID.IsNone())
	{
		return nullptr;
	}

	for (const FT66SurgeryDefinition& Def : GetAllSurgeryDefinitions())
	{
		if (Def.SurgeryID == SurgeryID)
		{
			return &Def;
		}
	}
	return nullptr;
}

bool UT66BuffSubsystem::EnsureSurgeryOwnershipValid()
{
	if (!SaveData)
	{
		return false;
	}

	const TArray<FName> Original = SaveData->OwnedSurgeryIDs;
	const TMap<FName, uint8> OriginalTiers = SaveData->SurgeryTierValues;
	TArray<FName> Sanitized;
	for (const FName SurgeryID : Original)
	{
		if (!SurgeryID.IsNone() && FindSurgeryDefinition(SurgeryID))
		{
			Sanitized.AddUnique(SurgeryID);
		}
	}

	TMap<FName, uint8> SanitizedTiers;
	for (const TPair<FName, uint8>& Pair : OriginalTiers)
	{
		const FT66SurgeryDefinition* Def = FindSurgeryDefinition(Pair.Key);
		if (Def && Pair.Value > 0)
		{
			SanitizedTiers.Add(Pair.Key, static_cast<uint8>(FMath::Clamp(static_cast<int32>(Pair.Value), 1, MaxSurgeryRarityTier)));
			Sanitized.AddUnique(Pair.Key);
		}
	}

	for (const FName SurgeryID : Sanitized)
	{
		const FT66SurgeryDefinition* Def = FindSurgeryDefinition(SurgeryID);
		if (Def && !SanitizedTiers.Contains(SurgeryID))
		{
			SanitizedTiers.Add(SurgeryID, 1);
		}
	}

	bool bSurgeryTierValuesChanged = OriginalTiers.Num() != SanitizedTiers.Num();
	if (!bSurgeryTierValuesChanged)
	{
		for (const TPair<FName, uint8>& Pair : OriginalTiers)
		{
			const uint8* NewValue = SanitizedTiers.Find(Pair.Key);
			if (!NewValue || *NewValue != Pair.Value)
			{
				bSurgeryTierValuesChanged = true;
				break;
			}
		}
	}

	SaveData->OwnedSurgeryIDs = MoveTemp(Sanitized);
	SaveData->SurgeryTierValues = MoveTemp(SanitizedTiers);
	return SaveData->OwnedSurgeryIDs != Original || bSurgeryTierValuesChanged;
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

	const FName SurgeryID = GetSurgeryIDForBaseStat(StatType);
	return (!SurgeryID.IsNone() && StepIndex < GetSurgeryTierValue(SurgeryID)) ? 1 : 0;
}

int32 UT66BuffSubsystem::GetUnlockedFillStepCount(ET66HeroStatType StatType) const
{
	const FName SurgeryID = GetSurgeryIDForBaseStat(StatType);
	return SurgeryID.IsNone() ? 0 : GetSurgeryTierValue(SurgeryID);
}

int32 UT66BuffSubsystem::GetTotalStatBonus(ET66HeroStatType StatType) const
{
	return GetSurgeryBaseStatBonus(StatType);
}

int32 UT66BuffSubsystem::GetCostForNextFillStepUnlock(ET66HeroStatType StatType) const
{
	return IsStatMaxed(StatType) ? 0 : SurgeryUnlockCostCC;
}

bool UT66BuffSubsystem::UnlockNextFillStep(ET66HeroStatType StatType)
{
	const FName SurgeryID = GetSurgeryIDForBaseStat(StatType);
	return !SurgeryID.IsNone() && PurchaseSurgery(SurgeryID);
}

bool UT66BuffSubsystem::UnlockRandomStat()
{
	if (!SaveData)
	{
		return false;
	}

	TArray<FName> Candidates;
	for (const FT66SurgeryDefinition& Def : GetAllSurgeryDefinitions())
	{
		if (!Def.bUsesStat && !IsSurgeryMaxTier(Def.SurgeryID))
		{
			Candidates.Add(Def.SurgeryID);
		}
	}

	if (Candidates.Num() <= 0)
	{
		return false;
	}

	return PurchaseSurgery(Candidates[FMath::RandRange(0, Candidates.Num() - 1)]);
}

bool UT66BuffSubsystem::IsStatMaxed(ET66HeroStatType StatType) const
{
	const FName SurgeryID = GetSurgeryIDForBaseStat(StatType);
	return SurgeryID.IsNone() || IsSurgeryMaxTier(SurgeryID);
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

bool UT66BuffSubsystem::IsSurgeryOwned(const FName SurgeryID) const
{
	return SaveData && !SurgeryID.IsNone() && (SaveData->OwnedSurgeryIDs.Contains(SurgeryID) || GetSurgeryTierValue(SurgeryID) > 0);
}

bool UT66BuffSubsystem::PurchaseSurgery(const FName SurgeryID)
{
	if (!SaveData || SurgeryID.IsNone())
	{
		return false;
	}

	const FT66SurgeryDefinition* Def = FindSurgeryDefinition(SurgeryID);
	if (!Def)
	{
		return false;
	}

	const int32 CurrentTier = GetSurgeryTierValue(SurgeryID);
	if (CurrentTier >= MaxSurgeryRarityTier)
	{
		return false;
	}

	if (!SpendChadCoupons(FMath::Max(0, Def->CostCC)))
	{
		return false;
	}

	SaveData->OwnedSurgeryIDs.AddUnique(SurgeryID);
	SaveData->SurgeryTierValues.FindOrAdd(SurgeryID) = static_cast<uint8>(FMath::Clamp(CurrentTier + 1, 1, MaxSurgeryRarityTier));
	EnsureSurgeryOwnershipValid();
	Save();
	return true;
}

bool UT66BuffSubsystem::RefundSurgery(const FName SurgeryID)
{
	if (!SaveData || SurgeryID.IsNone())
	{
		return false;
	}

	const FT66SurgeryDefinition* Def = FindSurgeryDefinition(SurgeryID);
	if (!Def)
	{
		return false;
	}

	const int32 CurrentTier = GetSurgeryTierValue(SurgeryID);
	if (CurrentTier <= 0)
	{
		return false;
	}

	const int32 RefundAmount = FMath::Max(0, Def->CostCC) * CurrentTier;
	SaveData->OwnedSurgeryIDs.Remove(SurgeryID);
	SaveData->SurgeryTierValues.Remove(SurgeryID);
	EnsureSurgeryOwnershipValid();
	AddChadCoupons(RefundAmount);
	Save();
	return RefundAmount > 0;
}

int32 UT66BuffSubsystem::GetSurgeryCost(const FName SurgeryID) const
{
	const FT66SurgeryDefinition* Def = FindSurgeryDefinition(SurgeryID);
	if (!Def || IsSurgeryMaxTier(SurgeryID))
	{
		return 0;
	}
	return Def ? FMath::Max(0, Def->CostCC) : 0;
}

int32 UT66BuffSubsystem::GetSurgeryTierValue(const FName SurgeryID) const
{
	if (!SaveData || SurgeryID.IsNone())
	{
		return 0;
	}

	const FT66SurgeryDefinition* Def = FindSurgeryDefinition(SurgeryID);
	if (!Def)
	{
		return 0;
	}

	if (const uint8* Tier = SaveData->SurgeryTierValues.Find(SurgeryID))
	{
		return FMath::Clamp(static_cast<int32>(*Tier), 0, MaxSurgeryRarityTier);
	}

	return SaveData->OwnedSurgeryIDs.Contains(SurgeryID) ? 1 : 0;
}

ET66ItemRarity UT66BuffSubsystem::GetSurgeryRarity(const FName SurgeryID) const
{
	return T66SurgeryTierValueToRarity(FMath::Max(1, GetSurgeryTierValue(SurgeryID)));
}

bool UT66BuffSubsystem::IsSurgeryMaxTier(const FName SurgeryID) const
{
	const FT66SurgeryDefinition* Def = FindSurgeryDefinition(SurgeryID);
	if (!Def)
	{
		return true;
	}

	return GetSurgeryTierValue(SurgeryID) >= MaxSurgeryRarityTier;
}

int32 UT66BuffSubsystem::GetSurgeryBaseStatBonus(const ET66HeroStatType StatType) const
{
	int32 Total = 0;
	for (const FT66SurgeryDefinition& Def : GetAllSurgeryDefinitions())
	{
		if (!Def.bUsesStat && Def.BaseStatType == StatType && IsSurgeryOwned(Def.SurgeryID))
		{
			Total += FMath::Max(0, Def.BonusStatPoints) * GetSurgeryTierValue(Def.SurgeryID);
		}
	}
	return Total;
}

int32 UT66BuffSubsystem::GetSurgeryStatBonus(const ET66StatType StatType) const
{
	int32 Total = 0;
	for (const FT66SurgeryDefinition& Def : GetAllSurgeryDefinitions())
	{
		if (Def.bUsesStat && Def.StatType == StatType && IsSurgeryOwned(Def.SurgeryID))
		{
			Total += FMath::Max(0, Def.BonusStatPoints) * GetSurgeryTierValue(Def.SurgeryID);
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

bool UT66BuffSubsystem::RefundSingleUseBuff(const ET66StatType StatType)
{
	if (!SaveData || !T66IsLiveStatType(StatType))
	{
		return false;
	}

	TArray<uint8>* PendingStates = GetPendingSingleUseStates();
	if (!PendingStates)
	{
		return false;
	}

	EnsurePendingSingleUseStatesSize(*PendingStates);
	const int32 StatIndex = GetSingleUseBuffIndex(StatType);
	if (StatIndex == INDEX_NONE || !PendingStates->IsValidIndex(StatIndex))
	{
		return false;
	}

	const int32 OwnedCount = FMath::Max(0, static_cast<int32>((*PendingStates)[StatIndex]));
	if (OwnedCount <= 0)
	{
		return false;
	}

	(*PendingStates)[StatIndex] = 0;
	EnsureSelectedSingleUseBuffLoadoutValid();
	for (ET66StatType& SlotStat : SaveData->SelectedSingleUseBuffSlots)
	{
		if (SlotStat == StatType)
		{
			SlotStat = ET66StatType::None;
		}
	}

	RebuildSelectedSingleUseStatesFromLoadout();
	AddChadCoupons(OwnedCount * FMath::Max(0, SingleUseBuffCostCC));
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

	const FName SurgeryID = GetSurgeryIDForBaseStat(StatType);
	if (SurgeryID.IsNone())
	{
		return;
	}

	if (Count > 0)
	{
		SaveData->OwnedSurgeryIDs.AddUnique(SurgeryID);
		SaveData->SurgeryTierValues.FindOrAdd(SurgeryID) = static_cast<uint8>(FMath::Clamp(Count, 1, MaxSurgeryRarityTier));
	}
	else
	{
		SaveData->OwnedSurgeryIDs.Remove(SurgeryID);
		SaveData->SurgeryTierValues.Remove(SurgeryID);
	}
	EnsureSurgeryOwnershipValid();
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
