// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/T66StageProgressionTuningConfig.h"
#include "T66StageProgressionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStageProgressionChanged);

USTRUCT(BlueprintType)
struct T66_API FT66StageProgressionSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	ET66Difficulty SelectedDifficulty = ET66Difficulty::Easy;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	int32 GlobalStage = 1;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	int32 DifficultyStartStage = 1;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	int32 DifficultyEndStage = 5;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	int32 LocalStageIndex = 1;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	int32 LocalStageCount = 5;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float DifficultyScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float FinaleScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float EnemyStatScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float InitialPopulationScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float RuntimeTrickleCountScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float RuntimeTrickleIntervalScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float TrapDamageScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	float TrapSpeedScalar = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "StageProgression")
	FVector4 ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.0f);
};

UCLASS()
class T66_API UT66StageProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const FT66StageProgressionSnapshot& GetCurrentSnapshot() const { return CachedSnapshot; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StageProgression")
	FT66StageProgressionSnapshot GetCurrentSnapshotCopy() const { return CachedSnapshot; }

	UFUNCTION(BlueprintCallable, Category = "StageProgression")
	void RefreshSnapshot(bool bBroadcastIfChanged = true);

	UPROPERTY(BlueprintAssignable, Category = "StageProgression")
	FOnStageProgressionChanged StageProgressionChanged;

private:
	UFUNCTION()
	void HandleStageChanged();

	UFUNCTION()
	void HandleDifficultyChanged();

	FT66StageProgressionSnapshot BuildSnapshot() const;

	UT66StageProgressionTuningConfig CachedTuning;
	FT66StageProgressionSnapshot CachedSnapshot;
};
