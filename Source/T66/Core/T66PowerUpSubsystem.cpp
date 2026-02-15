// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PowerUpSubsystem.h"
#include "Core/T66PowerUpSaveGame.h"
#include "Kismet/GameplayStatics.h"

const FString UT66PowerUpSubsystem::PowerUpSaveSlotName(TEXT("T66_PowerUp"));

int32 UT66PowerUpSubsystem::GetCostForTierStep(uint8 FromTier)
{
	switch (FromTier)
	{
		case 0: return 1;   // Empty -> Black
		case 1: return 3;   // Black -> Red
		case 2: return 5;   // Red -> Yellow
		case 3: return 10; // Yellow -> White
		default: return 0; // White or invalid
	}
}

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
	if (SaveData->SaveVersion < 2)
	{
		MigrateV1ToV2WedgeTiers();
		SaveData->SaveVersion = 2;
	}
}

void UT66PowerUpSubsystem::MigrateV1ToV2WedgeTiers()
{
	if (!SaveData) return;
	auto Migrate = [this](TArray<uint8>& Tiers, int32 LegacyCount)
	{
		Tiers.SetNumZeroed(MaxSlicesPerStat);
		for (int32 i = 0; i < LegacyCount && i < MaxSlicesPerStat; ++i)
		{
			Tiers[i] = static_cast<uint8>(ET66PowerUpWedgeTier::Black);
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

void UT66PowerUpSubsystem::EnsureWedgeTiersSize(TArray<uint8>& Arr)
{
	if (Arr.Num() < MaxSlicesPerStat)
	{
		Arr.SetNumZeroed(MaxSlicesPerStat);
	}
	else if (Arr.Num() > MaxSlicesPerStat)
	{
		Arr.SetNum(MaxSlicesPerStat);
	}
}

TArray<uint8>* UT66PowerUpSubsystem::GetWedgeTiersForStat(ET66HeroStatType StatType)
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

const TArray<uint8>* UT66PowerUpSubsystem::GetWedgeTiersForStat(ET66HeroStatType StatType) const
{
	return const_cast<UT66PowerUpSubsystem*>(this)->GetWedgeTiersForStat(StatType);
}

int32 UT66PowerUpSubsystem::GetWedgeTier(ET66HeroStatType StatType, int32 WedgeIndex) const
{
	const TArray<uint8>* Arr = GetWedgeTiersForStat(StatType);
	if (!Arr || WedgeIndex < 0 || WedgeIndex >= MaxSlicesPerStat) return 0;
	if (WedgeIndex >= Arr->Num()) return 0;
	return FMath::Clamp(static_cast<int32>((*Arr)[WedgeIndex]), 0, 4);
}

int32 UT66PowerUpSubsystem::GetCostForNextUpgrade(ET66HeroStatType StatType) const
{
	const TArray<uint8>* Arr = GetWedgeTiersForStat(StatType);
	if (!Arr) return 0;
	// Fill by tier: all Black first, then all Red, then Yellow, then White. Cost = cost for the current min tier.
	uint8 MinTier = 4;
	for (int32 i = 0; i < MaxSlicesPerStat; ++i)
	{
		const uint8 T = (i < Arr->Num()) ? (*Arr)[i] : 0;
		if (T < MinTier) MinTier = T;
	}
	return (MinTier < 4) ? GetCostForTierStep(MinTier) : 0;
}

bool UT66PowerUpSubsystem::UnlockNextWedgeUpgrade(ET66HeroStatType StatType)
{
	TArray<uint8>* Arr = GetWedgeTiersForStat(StatType);
	if (!SaveData || !Arr) return false;
	EnsureWedgeTiersSize(*Arr);
	// Find minimum tier across all wedges; upgrade the first wedge at that tier.
	uint8 MinTier = 4;
	for (int32 i = 0; i < MaxSlicesPerStat; ++i)
	{
		const uint8 T = (*Arr)[i];
		if (T < MinTier) MinTier = T;
	}
	if (MinTier >= 4) return false;
	const int32 Cost = GetCostForTierStep(MinTier);
	if (SaveData->PowerCrystalBalance < Cost) return false;
	for (int32 i = 0; i < MaxSlicesPerStat; ++i)
	{
		if ((*Arr)[i] == MinTier)
		{
			SaveData->PowerCrystalBalance -= Cost;
			(*Arr)[i]++;
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
		TArray<uint8>* Arr = GetWedgeTiersForStat(Stat);
		if (!Arr) continue;
		EnsureWedgeTiersSize(*Arr);
		for (int32 i = 0; i < MaxSlicesPerStat; ++i)
		{
			if ((*Arr)[i] == 0)
			{
				SaveData->PowerCrystalBalance -= 1;
				(*Arr)[i] = static_cast<uint8>(ET66PowerUpWedgeTier::Black);
				Save();
				return true;
			}
		}
	}
	return false;
}

bool UT66PowerUpSubsystem::IsStatMaxed(ET66HeroStatType StatType) const
{
	const TArray<uint8>* Arr = GetWedgeTiersForStat(StatType);
	if (!Arr) return true;
	for (int32 i = 0; i < MaxSlicesPerStat; ++i)
	{
		if (i >= Arr->Num() || (*Arr)[i] < 4) return false;
	}
	return true;
}

FT66HeroStatBonuses UT66PowerUpSubsystem::GetPowerupStatBonuses() const
{
	FT66HeroStatBonuses Out;
	if (!SaveData) return Out;
	auto SumWedges = [this](ET66HeroStatType Stat) -> int32
	{
		const TArray<uint8>* Arr = GetWedgeTiersForStat(Stat);
		if (!Arr) return 0;
		int32 S = 0;
		for (int32 i = 0; i < Arr->Num() && i < MaxSlicesPerStat; ++i)
		{
			if ((*Arr)[i] >= 1) S++;
		}
		return S;
	};
	Out.Damage = SumWedges(ET66HeroStatType::Damage) + GetRandomBonusForStat(ET66HeroStatType::Damage);
	Out.AttackSpeed = SumWedges(ET66HeroStatType::AttackSpeed) + GetRandomBonusForStat(ET66HeroStatType::AttackSpeed);
	Out.AttackScale = SumWedges(ET66HeroStatType::AttackScale) + GetRandomBonusForStat(ET66HeroStatType::AttackScale);
	Out.Armor = SumWedges(ET66HeroStatType::Armor) + GetRandomBonusForStat(ET66HeroStatType::Armor);
	Out.Evasion = SumWedges(ET66HeroStatType::Evasion) + GetRandomBonusForStat(ET66HeroStatType::Evasion);
	Out.Luck = SumWedges(ET66HeroStatType::Luck) + GetRandomBonusForStat(ET66HeroStatType::Luck);
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

void UT66PowerUpSubsystem::AddRandomBonusToStat(ET66HeroStatType StatType)
{
	if (!SaveData) return;
	switch (StatType)
	{
		case ET66HeroStatType::Damage:      SaveData->RandomBonusDamage++; break;
		case ET66HeroStatType::AttackSpeed: SaveData->RandomBonusAttackSpeed++; break;
		case ET66HeroStatType::AttackScale: SaveData->RandomBonusAttackScale++; break;
		case ET66HeroStatType::Armor:      SaveData->RandomBonusArmor++; break;
		case ET66HeroStatType::Evasion:    SaveData->RandomBonusEvasion++; break;
		case ET66HeroStatType::Luck:       SaveData->RandomBonusLuck++; break;
		default: break;
	}
}
