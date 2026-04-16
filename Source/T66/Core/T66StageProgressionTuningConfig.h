// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66StageProgressionTuningConfig.generated.h"

USTRUCT(BlueprintType)
struct T66_API FT66StageProgressionStageTuning
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StageProgression", meta = (ClampMin = "0.10"))
	float EnemyStatScalar = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StageProgression", meta = (ClampMin = "0.10"))
	float InitialPopulationScalar = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StageProgression", meta = (ClampMin = "0.10"))
	float RuntimeTrickleCountScalar = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StageProgression", meta = (ClampMin = "0.10"))
	float RuntimeTrickleIntervalScalar = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StageProgression", meta = (ClampMin = "0.10"))
	float TrapDamageScalar = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StageProgression", meta = (ClampMin = "0.10"))
	float TrapSpeedScalar = 1.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "StageProgression")
	FVector4 ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.0f);
};

class T66_API UT66StageProgressionTuningConfig
{
public:
	UT66StageProgressionTuningConfig();

	void LoadFromConfig();
	const FT66StageProgressionStageTuning& GetStageTuning(int32 LocalStageIndex) const;

	FT66StageProgressionStageTuning Stage1;
	FT66StageProgressionStageTuning Stage2;
	FT66StageProgressionStageTuning Stage3;
	FT66StageProgressionStageTuning Stage4;
	FT66StageProgressionStageTuning Stage5;
};
