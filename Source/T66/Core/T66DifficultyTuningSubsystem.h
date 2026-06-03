// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Engine/DataTable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66DifficultyTuningSubsystem.generated.h"

struct FStreamableHandle;

USTRUCT(BlueprintType)
struct T66_API FT66DifficultyTuningRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Difficulty|RunShape", meta = (ClampMin = "1", ClampMax = "20"))
	int32 StartStage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Difficulty|RunShape", meta = (ClampMin = "1", ClampMax = "20"))
	int32 EndStage = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Difficulty|RunShape")
	bool bUsesFinalSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Difficulty|HUD", meta = (ClampMin = "1"))
	int32 SkullColorBandSize = 4;
};

class T66_API FT66DifficultyTuningTable
{
public:
	FT66DifficultyTuningRow Easy;
	FT66DifficultyTuningRow Medium;
	FT66DifficultyTuningRow Hard;
	FT66DifficultyTuningRow VeryHard;
	FT66DifficultyTuningRow Impossible;

	FT66DifficultyTuningTable();

	bool LoadFromDataTable(const UDataTable* DataTable);
	const FT66DifficultyTuningRow& Get(ET66Difficulty Difficulty) const;
};

UCLASS()
class T66_API UT66DifficultyTuningSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
	int32 GetDifficultyStartStage(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
	int32 GetDifficultyEndStage(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
	int32 GetDifficultyLocalStage(ET66Difficulty Difficulty, int32 AbsoluteStage) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
	ET66ItemRarity GetLocalStageIdolRarity(int32 LocalStageNumber) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
	ET66WeaponRarity GetLocalStageWeaponRarity(int32 LocalStageNumber) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
	bool DoesDifficultyUseFinalSequence(ET66Difficulty Difficulty) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Difficulty")
	int32 GetDifficultySkullColorBandSize(ET66Difficulty Difficulty) const;

	const FT66DifficultyTuningRow& GetDifficultyTuning(ET66Difficulty Difficulty) const;

private:
	void QueueTuningDataTableLoad();
	void HandleTuningDataTableLoaded();
	bool IsTuningReady(const TCHAR* Caller) const;

	FT66DifficultyTuningTable CachedTuning;
	TSharedPtr<FStreamableHandle> TuningDataTableLoadHandle;
	bool bTuningLoaded = false;
	mutable bool bWarnedTuningUnavailable = false;
};
