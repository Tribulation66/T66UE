// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66StageProgressionTuningConfig.h"

#include "Misc/ConfigCacheIni.h"

namespace
{
	static constexpr const TCHAR* T66StageProgressionConfigSection = TEXT("/Script/T66.T66StageProgressionTuningConfig");

	static FString GetT66StageProgressionConfigFilename()
	{
		FString ConfigFilename;
		FConfigCacheIni::LoadGlobalIniFile(ConfigFilename, TEXT("T66StageProgression"));
		return ConfigFilename;
	}

	template <typename StructType>
	static void LoadStageProgressionStructValue(const FString& ConfigFilename, const TCHAR* Key, StructType& Value)
	{
		if (!GConfig)
		{
			return;
		}

		FString RawValue;
		if (!GConfig->GetString(T66StageProgressionConfigSection, Key, RawValue, ConfigFilename) || RawValue.IsEmpty())
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

	static FT66StageProgressionStageTuning MakeStageTuning(
		const float EnemyStatScalar,
		const float InitialPopulationScalar,
		const float RuntimeTrickleCountScalar,
		const float RuntimeTrickleIntervalScalar,
		const float TrapDamageScalar,
		const float TrapSpeedScalar,
		const FVector4& ColorSaturation)
	{
		FT66StageProgressionStageTuning Tuning;
		Tuning.EnemyStatScalar = EnemyStatScalar;
		Tuning.InitialPopulationScalar = InitialPopulationScalar;
		Tuning.RuntimeTrickleCountScalar = RuntimeTrickleCountScalar;
		Tuning.RuntimeTrickleIntervalScalar = RuntimeTrickleIntervalScalar;
		Tuning.TrapDamageScalar = TrapDamageScalar;
		Tuning.TrapSpeedScalar = TrapSpeedScalar;
		Tuning.ColorSaturation = ColorSaturation;
		return Tuning;
	}
}

UT66StageProgressionTuningConfig::UT66StageProgressionTuningConfig()
	: Stage1(MakeStageTuning(1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, FVector4(0.95f, 0.95f, 0.95f, 1.0f)))
	, Stage2(MakeStageTuning(1.08f, 1.08f, 1.12f, 0.95f, 1.05f, 1.08f, FVector4(1.00f, 0.90f, 0.88f, 1.0f)))
	, Stage3(MakeStageTuning(1.16f, 1.18f, 1.25f, 0.90f, 1.12f, 1.16f, FVector4(1.05f, 0.86f, 0.82f, 1.0f)))
	, Stage4(MakeStageTuning(1.25f, 1.30f, 1.40f, 0.84f, 1.20f, 1.25f, FVector4(1.10f, 0.81f, 0.76f, 1.0f)))
	, Stage5(MakeStageTuning(1.35f, 1.45f, 1.60f, 0.78f, 1.30f, 1.35f, FVector4(1.14f, 0.76f, 0.70f, 1.0f)))
{
}

void UT66StageProgressionTuningConfig::LoadFromConfig()
{
	const FString ConfigFilename = GetT66StageProgressionConfigFilename();
	LoadStageProgressionStructValue(ConfigFilename, TEXT("Stage1"), Stage1);
	LoadStageProgressionStructValue(ConfigFilename, TEXT("Stage2"), Stage2);
	LoadStageProgressionStructValue(ConfigFilename, TEXT("Stage3"), Stage3);
	LoadStageProgressionStructValue(ConfigFilename, TEXT("Stage4"), Stage4);
	LoadStageProgressionStructValue(ConfigFilename, TEXT("Stage5"), Stage5);
}

const FT66StageProgressionStageTuning& UT66StageProgressionTuningConfig::GetStageTuning(const int32 LocalStageIndex) const
{
	switch (FMath::Clamp(LocalStageIndex, 1, 5))
	{
	case 1: return Stage1;
	case 2: return Stage2;
	case 3: return Stage3;
	case 4: return Stage4;
	default: return Stage5;
	}
}
