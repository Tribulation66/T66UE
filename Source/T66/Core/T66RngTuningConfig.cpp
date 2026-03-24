// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RngTuningConfig.h"

#include "Misc/ConfigCacheIni.h"

namespace
{
	static constexpr const TCHAR* T66RngConfigSection = TEXT("/Script/T66.T66RngTuningConfig");

	static FString GetT66RngConfigFilename()
	{
		FString ConfigFilename;
		FConfigCacheIni::LoadGlobalIniFile(ConfigFilename, TEXT("T66Rng"));
		return ConfigFilename;
	}

	static void LoadFloatValue(const FString& ConfigFilename, const TCHAR* Key, float& Value)
	{
		if (GConfig)
		{
			GConfig->GetFloat(T66RngConfigSection, Key, Value, ConfigFilename);
		}
	}

	static void LoadIntValue(const FString& ConfigFilename, const TCHAR* Key, int32& Value)
	{
		if (GConfig)
		{
			GConfig->GetInt(T66RngConfigSection, Key, Value, ConfigFilename);
		}
	}

	template <typename StructType>
	static void LoadStructValue(const FString& ConfigFilename, const TCHAR* Key, StructType& Value)
	{
		if (!GConfig)
		{
			return;
		}

		FString RawValue;
		if (!GConfig->GetString(T66RngConfigSection, Key, RawValue, ConfigFilename) || RawValue.IsEmpty())
		{
			return;
		}

		UScriptStruct* StructTypeInfo = StructType::StaticStruct();
		if (!StructTypeInfo)
		{
			return;
		}

		StructTypeInfo->ImportText(*RawValue, &Value, nullptr, PPF_None, GLog, StructTypeInfo->GetName());
	}
}

void UT66RngTuningConfig::LoadFromConfig()
{
	const FString ConfigFilename = GetT66RngConfigFilename();

	LoadFloatValue(ConfigFilename, TEXT("LuckPerPoint"), LuckPerPoint);
	LoadFloatValue(ConfigFilename, TEXT("LuckMax01"), LuckMax01);
	LoadFloatValue(ConfigFilename, TEXT("RarityBiasStrength"), RarityBiasStrength);
	LoadFloatValue(ConfigFilename, TEXT("RangeHighBiasStrength"), RangeHighBiasStrength);
	LoadFloatValue(ConfigFilename, TEXT("BernoulliBiasStrength"), BernoulliBiasStrength);

	LoadFloatValue(ConfigFilename, TEXT("GoblinWaveChanceBase"), GoblinWaveChanceBase);
	LoadStructValue(ConfigFilename, TEXT("GoblinCountPerWave"), GoblinCountPerWave);
	LoadStructValue(ConfigFilename, TEXT("SpecialEnemyRarityBase"), SpecialEnemyRarityBase);

	LoadStructValue(ConfigFilename, TEXT("TreesPerStage"), TreesPerStage);
}
