// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SmartLootTuningConfig.h"

#include "Misc/ConfigCacheIni.h"

namespace
{
	static constexpr const TCHAR* T66SmartLootConfigSection = TEXT("/Script/T66.T66SmartLootTuningConfig");

	static FString GetT66SmartLootConfigFilename()
	{
		FString ConfigFilename;
		FConfigCacheIni::LoadGlobalIniFile(ConfigFilename, TEXT("T66SmartLoot"));
		return ConfigFilename;
	}

	static void LoadSmartLootBoolValue(const FString& ConfigFilename, const TCHAR* Key, bool& Value)
	{
		if (GConfig)
		{
			GConfig->GetBool(T66SmartLootConfigSection, Key, Value, ConfigFilename);
		}
	}

	static void LoadSmartLootFloatValue(const FString& ConfigFilename, const TCHAR* Key, float& Value)
	{
		if (GConfig)
		{
			GConfig->GetFloat(T66SmartLootConfigSection, Key, Value, ConfigFilename);
		}
	}
}

void UT66SmartLootTuningConfig::LoadFromConfig()
{
	const FString ConfigFilename = GetT66SmartLootConfigFilename();

	LoadSmartLootBoolValue(ConfigFilename, TEXT("bEnableSmartLoot"), bEnableSmartLoot);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("BaseCandidateWeight"), BaseCandidateWeight);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("InventoryBaseStatWeight"), InventoryBaseStatWeight);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("InventoryStatWeight"), InventoryStatWeight);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("InventoryAttackCategoryWeight"), InventoryAttackCategoryWeight);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("IdolElementWeight"), IdolElementWeight);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("IdolAttackCategoryWeight"), IdolAttackCategoryWeight);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("MaxCandidateWeight"), MaxCandidateWeight);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("ShopRerollSeenDecayFactor"), ShopRerollSeenDecayFactor);
	LoadSmartLootFloatValue(ConfigFilename, TEXT("ShopRerollSeenWeightFloor"), ShopRerollSeenWeightFloor);

	BaseCandidateWeight = FMath::Max(0.01f, BaseCandidateWeight);
	InventoryBaseStatWeight = FMath::Max(0.f, InventoryBaseStatWeight);
	InventoryStatWeight = FMath::Max(0.f, InventoryStatWeight);
	InventoryAttackCategoryWeight = FMath::Max(0.f, InventoryAttackCategoryWeight);
	IdolElementWeight = FMath::Max(0.f, IdolElementWeight);
	IdolAttackCategoryWeight = FMath::Max(0.f, IdolAttackCategoryWeight);
	MaxCandidateWeight = FMath::Max(BaseCandidateWeight, MaxCandidateWeight);
	ShopRerollSeenDecayFactor = FMath::Max(0.f, ShopRerollSeenDecayFactor);
	ShopRerollSeenWeightFloor = FMath::Clamp(ShopRerollSeenWeightFloor, 0.01f, 1.0f);
}
