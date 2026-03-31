// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66PowerUpSaveGame.h"
#include "Kismet/GameplayStatics.h"

const FString UT66PowerUpSubsystem::PowerUpSaveSlotName(TEXT("T66_PowerUp"));

void UT66PowerUpSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreateSave();
}

void UT66PowerUpSubsystem::LoadOrCreateSave()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(PowerUpSaveSlotName, PowerUpSaveUserIndex);
	SaveData = Cast<UT66PowerUpSaveGame>(Loaded);
	bool bNeedsSave = false;
	if (!SaveData)
	{
		SaveData = NewObject<UT66PowerUpSaveGame>(this);
		UE_LOG(LogTemp, Log, TEXT("[PowerUp] LoadOrCreateSave: Created fresh save (no file found)."));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[PowerUp] LoadOrCreateSave: Loaded existing save, balance=%d"), SaveData->PowerCrystalBalance);
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

	if (bNeedsSave)
	{
		Save();
	}
}

void UT66PowerUpSubsystem::MigrateV1ToV2WedgeTiers()
{
	if (!SaveData) return;
	auto Migrate = [](TArray<uint8>& Tiers, int32 LegacyCount)
	{
		Tiers.SetNumZeroed(UT66PowerUpSubsystem::LegacyV2SlotsPerStat);
		for (int32 i = 0; i < LegacyCount && i < UT66PowerUpSubsystem::LegacyV2SlotsPerStat; ++i)
		{
			Tiers[i] = static_cast<uint8>(ET66PowerUpFillStepState::Unlocked);
		}
	};
	Migrate(SaveData->WedgeTiersDamage, SaveData->PowerupSlicesDamage);
	Migrate(SaveData->WedgeTiersAttackSpeed, SaveData->PowerupSlicesAttackSpeed);
	Migrate(SaveData->WedgeTiersAttackScale, SaveData->PowerupSlicesAttackScale);
	Migrate(SaveData->WedgeTiersArmor, SaveData->PowerupSlicesArmor);
	Migrate(SaveData->WedgeTiersEvasion, SaveData->PowerupSlicesEvasion);
	Migrate(SaveData->WedgeTiersLuck, SaveData->PowerupSlicesLuck);
	UE_LOG(LogTemp, Log, TEXT("[PowerUp] Migrated v1 slice counts to v2 wedge tiers."));
}

void UT66PowerUpSubsystem::MigrateV2ToV3BodyParts()
{
	if (!SaveData) return;

	auto CountLegacyUnlocked = [](const TArray<uint8>& States) -> int32
	{
		int32 Count = 0;
		for (int32 i = 0; i < States.Num() && i < UT66PowerUpSubsystem::LegacyV2SlotsPerStat; ++i)
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
			States[i] = static_cast<uint8>(ET66PowerUpFillStepState::Unlocked);
		}

		RandomBonus = FMath::Max(0, RandomBonus) + OverflowBonus;
	};

	Convert(SaveData->WedgeTiersDamage, SaveData->RandomBonusDamage);
	Convert(SaveData->WedgeTiersAttackSpeed, SaveData->RandomBonusAttackSpeed);
	Convert(SaveData->WedgeTiersAttackScale, SaveData->RandomBonusAttackScale);
	Convert(SaveData->WedgeTiersArmor, SaveData->RandomBonusArmor);
	Convert(SaveData->WedgeTiersEvasion, SaveData->RandomBonusEvasion);
	Convert(SaveData->WedgeTiersLuck, SaveData->RandomBonusLuck);

	UE_LOG(LogTemp, Log, TEXT("[PowerUp] Migrated v2 wedge tiers to v3 six-part statues."));
}

void UT66PowerUpSubsystem::MigrateV3ToV4FillSteps()
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
		const int32 VisibleUnlocked = FMath::Clamp(TotalBonus, 0, UT66PowerUpSubsystem::MaxFillStepsPerStat);
		const int32 OverflowBonus = FMath::Max(0, TotalBonus - UT66PowerUpSubsystem::MaxFillStepsPerStat);

		States.SetNumZeroed(UT66PowerUpSubsystem::MaxFillStepsPerStat);
		for (int32 i = 0; i < VisibleUnlocked; ++i)
		{
			States[i] = static_cast<uint8>(ET66PowerUpFillStepState::Unlocked);
		}

		RandomBonus = OverflowBonus;
	};

	Convert(SaveData->WedgeTiersDamage, SaveData->RandomBonusDamage);
	Convert(SaveData->WedgeTiersAttackSpeed, SaveData->RandomBonusAttackSpeed);
	Convert(SaveData->WedgeTiersAttackScale, SaveData->RandomBonusAttackScale);
	Convert(SaveData->WedgeTiersArmor, SaveData->RandomBonusArmor);
	Convert(SaveData->WedgeTiersEvasion, SaveData->RandomBonusEvasion);
	Convert(SaveData->WedgeTiersLuck, SaveData->RandomBonusLuck);

	UE_LOG(LogTemp, Log, TEXT("[PowerUp] Migrated v3 six-part statues to v4 ten-step fill progression."));
}

void UT66PowerUpSubsystem::Save()
{
	if (SaveData)
	{
		UGameplayStatics::SaveGameToSlot(SaveData, PowerUpSaveSlotName, PowerUpSaveUserIndex);
	}
}

int32 UT66PowerUpSubsystem::GetPowerCrystalBalance() const
{
	return SaveData ? FMath::Max(0, SaveData->PowerCrystalBalance) : 0;
}

void UT66PowerUpSubsystem::AddPowerCrystals(int32 Amount)
{
	if (!SaveData || Amount <= 0) return;
	SaveData->PowerCrystalBalance = FMath::Clamp(SaveData->PowerCrystalBalance + Amount, 0, 2000000000);
	Save();
}

bool UT66PowerUpSubsystem::SpendPowerCrystals(int32 Amount)
{
	if (!SaveData || Amount <= 0 || SaveData->PowerCrystalBalance < Amount)
	{
		return false;
	}

	SaveData->PowerCrystalBalance -= Amount;
	Save();
	return true;
}

void UT66PowerUpSubsystem::EnsureFillStepStatesSize(TArray<uint8>& Arr)
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

TArray<uint8>* UT66PowerUpSubsystem::GetFillStepStatesForStat(ET66HeroStatType StatType)
{
	if (!SaveData) return nullptr;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      return &SaveData->WedgeTiersDamage;
		case ET66HeroStatType::AttackSpeed: return &SaveData->WedgeTiersAttackSpeed;
		case ET66HeroStatType::AttackScale: return &SaveData->WedgeTiersAttackScale;
		case ET66HeroStatType::Armor:      return &SaveData->WedgeTiersArmor;
		case ET66HeroStatType::Evasion:    return &SaveData->WedgeTiersEvasion;
		case ET66HeroStatType::Luck:       return &SaveData->WedgeTiersLuck;
		default: return nullptr;
	}
}

const TArray<uint8>* UT66PowerUpSubsystem::GetFillStepStatesForStat(ET66HeroStatType StatType) const
{
	return const_cast<UT66PowerUpSubsystem*>(this)->GetFillStepStatesForStat(StatType);
}

int32 UT66PowerUpSubsystem::GetFillStepState(ET66HeroStatType StatType, int32 StepIndex) const
{
	const TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
	if (!Arr || StepIndex < 0 || StepIndex >= MaxFillStepsPerStat) return 0;
	if (StepIndex >= Arr->Num()) return 0;
	return (*Arr)[StepIndex] > 0 ? 1 : 0;
}

int32 UT66PowerUpSubsystem::GetUnlockedFillStepCount(ET66HeroStatType StatType) const
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

int32 UT66PowerUpSubsystem::GetTotalStatBonus(ET66HeroStatType StatType) const
{
	return GetUnlockedFillStepCount(StatType) + GetRandomBonusForStat(StatType);
}

int32 UT66PowerUpSubsystem::GetCostForNextFillStepUnlock(ET66HeroStatType StatType) const
{
	return IsStatMaxed(StatType) ? 0 : 1;
}

bool UT66PowerUpSubsystem::UnlockNextFillStep(ET66HeroStatType StatType)
{
	TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
	if (!SaveData || !Arr) return false;
	EnsureFillStepStatesSize(*Arr);

	const int32 Cost = GetCostForNextFillStepUnlock(StatType);
	if (Cost <= 0 || SaveData->PowerCrystalBalance < Cost)
	{
		return false;
	}

	for (int32 i = 0; i < MaxFillStepsPerStat; ++i)
	{
		if ((*Arr)[i] == static_cast<uint8>(ET66PowerUpFillStepState::Locked))
		{
			SaveData->PowerCrystalBalance -= Cost;
			(*Arr)[i] = static_cast<uint8>(ET66PowerUpFillStepState::Unlocked);
			Save();
			return true;
		}
	}

	return false;
}

bool UT66PowerUpSubsystem::UnlockRandomStat()
{
	if (!SaveData || SaveData->PowerCrystalBalance < 1) return false;
	static const TArray<ET66HeroStatType> AllStats = {
		ET66HeroStatType::Damage,
		ET66HeroStatType::AttackSpeed,
		ET66HeroStatType::AttackScale,
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
			if ((*Arr)[i] == static_cast<uint8>(ET66PowerUpFillStepState::Locked))
			{
				SaveData->PowerCrystalBalance -= 1;
				(*Arr)[i] = static_cast<uint8>(ET66PowerUpFillStepState::Unlocked);
				Save();
				return true;
			}
		}
	}
	return false;
}

bool UT66PowerUpSubsystem::IsStatMaxed(ET66HeroStatType StatType) const
{
	const TArray<uint8>* Arr = GetFillStepStatesForStat(StatType);
	if (!Arr) return true;
	for (int32 i = 0; i < MaxFillStepsPerStat; ++i)
	{
		if (i >= Arr->Num() || (*Arr)[i] == static_cast<uint8>(ET66PowerUpFillStepState::Locked))
		{
			return false;
		}
	}
	return true;
}

FT66HeroStatBonuses UT66PowerUpSubsystem::GetPowerupStatBonuses() const
{
	FT66HeroStatBonuses Out;
	if (!SaveData) return Out;
	Out.Damage = GetTotalStatBonus(ET66HeroStatType::Damage);
	Out.AttackSpeed = GetTotalStatBonus(ET66HeroStatType::AttackSpeed);
	Out.AttackScale = GetTotalStatBonus(ET66HeroStatType::AttackScale);
	Out.Armor = GetTotalStatBonus(ET66HeroStatType::Armor);
	Out.Evasion = GetTotalStatBonus(ET66HeroStatType::Evasion);
	Out.Luck = GetTotalStatBonus(ET66HeroStatType::Luck);
	return Out;
}

int32 UT66PowerUpSubsystem::GetRandomBonusForStat(ET66HeroStatType StatType) const
{
	if (!SaveData) return 0;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      return SaveData->RandomBonusDamage;
		case ET66HeroStatType::AttackSpeed: return SaveData->RandomBonusAttackSpeed;
		case ET66HeroStatType::AttackScale: return SaveData->RandomBonusAttackScale;
		case ET66HeroStatType::Armor:      return SaveData->RandomBonusArmor;
		case ET66HeroStatType::Evasion:    return SaveData->RandomBonusEvasion;
		case ET66HeroStatType::Luck:       return SaveData->RandomBonusLuck;
		default: return 0;
	}
}
