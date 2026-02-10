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
	if (!SaveData)
	{
		SaveData = NewObject<UT66PowerUpSaveGame>(this);
		UE_LOG(LogTemp, Log, TEXT("[PowerUp] LoadOrCreateSave: Created fresh save (no file found)."));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[PowerUp] LoadOrCreateSave: Loaded existing save, balance=%d"), SaveData->PowerCrystalBalance);
	}
	SaveData->SaveVersion = FMath::Max(SaveData->SaveVersion, 1);
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

bool UT66PowerUpSubsystem::UnlockPowerupSlice(ET66HeroStatType StatType)
{
	if (!SaveData) return false;
	const int32 Current = GetSliceCountForStat(StatType);
	if (Current >= MaxSlicesPerStat || SaveData->PowerCrystalBalance < CrystalsPerSlice) return false;
	SaveData->PowerCrystalBalance -= CrystalsPerSlice;
	SetSliceCountForStat(StatType, Current + 1);
	Save();
	return true;
}

bool UT66PowerUpSubsystem::UnlockRandomStat()
{
	if (!SaveData || SaveData->PowerCrystalBalance < CrystalsPerSlice) return false;
	static const TArray<ET66HeroStatType> AllStats = {
		ET66HeroStatType::Damage,
		ET66HeroStatType::AttackSpeed,
		ET66HeroStatType::AttackSize,
		ET66HeroStatType::Armor,
		ET66HeroStatType::Evasion,
		ET66HeroStatType::Luck
	};
	SaveData->PowerCrystalBalance -= CrystalsPerSlice;
	const ET66HeroStatType Picked = AllStats[FMath::RandRange(0, AllStats.Num() - 1)];
	AddRandomBonusToStat(Picked);
	Save();
	return true;
}

int32 UT66PowerUpSubsystem::GetPowerupSlicesUnlocked(ET66HeroStatType StatType) const
{
	return SaveData ? FMath::Clamp(GetSliceCountForStat(StatType), 0, MaxSlicesPerStat) : 0;
}

FT66HeroStatBonuses UT66PowerUpSubsystem::GetPowerupStatBonuses() const
{
	FT66HeroStatBonuses Out;
	if (!SaveData) return Out;
	Out.Damage = GetSliceCountForStat(ET66HeroStatType::Damage) + GetRandomBonusForStat(ET66HeroStatType::Damage);
	Out.AttackSpeed = GetSliceCountForStat(ET66HeroStatType::AttackSpeed) + GetRandomBonusForStat(ET66HeroStatType::AttackSpeed);
	Out.AttackSize = GetSliceCountForStat(ET66HeroStatType::AttackSize) + GetRandomBonusForStat(ET66HeroStatType::AttackSize);
	Out.Armor = GetSliceCountForStat(ET66HeroStatType::Armor) + GetRandomBonusForStat(ET66HeroStatType::Armor);
	Out.Evasion = GetSliceCountForStat(ET66HeroStatType::Evasion) + GetRandomBonusForStat(ET66HeroStatType::Evasion);
	Out.Luck = GetSliceCountForStat(ET66HeroStatType::Luck) + GetRandomBonusForStat(ET66HeroStatType::Luck);
	return Out;
}

int32 UT66PowerUpSubsystem::GetSliceCountForStat(ET66HeroStatType StatType) const
{
	if (!SaveData) return 0;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      return SaveData->PowerupSlicesDamage;
		case ET66HeroStatType::AttackSpeed: return SaveData->PowerupSlicesAttackSpeed;
		case ET66HeroStatType::AttackSize: return SaveData->PowerupSlicesAttackSize;
		case ET66HeroStatType::Armor:      return SaveData->PowerupSlicesArmor;
		case ET66HeroStatType::Evasion:    return SaveData->PowerupSlicesEvasion;
		case ET66HeroStatType::Luck:       return SaveData->PowerupSlicesLuck;
		default: return 0;
	}
}

void UT66PowerUpSubsystem::SetSliceCountForStat(ET66HeroStatType StatType, int32 Count)
{
	if (!SaveData) return;
	Count = FMath::Clamp(Count, 0, MaxSlicesPerStat);
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      SaveData->PowerupSlicesDamage = Count; break;
		case ET66HeroStatType::AttackSpeed: SaveData->PowerupSlicesAttackSpeed = Count; break;
		case ET66HeroStatType::AttackSize: SaveData->PowerupSlicesAttackSize = Count; break;
		case ET66HeroStatType::Armor:      SaveData->PowerupSlicesArmor = Count; break;
		case ET66HeroStatType::Evasion:    SaveData->PowerupSlicesEvasion = Count; break;
		case ET66HeroStatType::Luck:       SaveData->PowerupSlicesLuck = Count; break;
		default: break;
	}
}

int32 UT66PowerUpSubsystem::GetRandomBonusForStat(ET66HeroStatType StatType) const
{
	if (!SaveData) return 0;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      return SaveData->RandomBonusDamage;
		case ET66HeroStatType::AttackSpeed: return SaveData->RandomBonusAttackSpeed;
		case ET66HeroStatType::AttackSize: return SaveData->RandomBonusAttackSize;
		case ET66HeroStatType::Armor:      return SaveData->RandomBonusArmor;
		case ET66HeroStatType::Evasion:    return SaveData->RandomBonusEvasion;
		case ET66HeroStatType::Luck:       return SaveData->RandomBonusLuck;
		default: return 0;
	}
}

void UT66PowerUpSubsystem::AddRandomBonusToStat(ET66HeroStatType StatType)
{
	if (!SaveData) return;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      SaveData->RandomBonusDamage++; break;
		case ET66HeroStatType::AttackSpeed: SaveData->RandomBonusAttackSpeed++; break;
		case ET66HeroStatType::AttackSize: SaveData->RandomBonusAttackSize++; break;
		case ET66HeroStatType::Armor:      SaveData->RandomBonusArmor++; break;
		case ET66HeroStatType::Evasion:    SaveData->RandomBonusEvasion++; break;
		case ET66HeroStatType::Luck:       SaveData->RandomBonusLuck++; break;
		default: break;
	}
}
