// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66TDFrontendStateSubsystem.generated.h"

UCLASS()
class T66TD_API UT66TDFrontendStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ResetRunSetup();
	void BeginNewRun();
	void BeginDailyRun(FName DifficultyID, const FString& ChallengeId, int32 Seed);
	void SelectDifficulty(FName DifficultyID);
	void SelectMap(FName MapID);
	void SelectStage(FName StageID);
	void ResetBattleRewardGrant();
	bool TryMarkBattleRewardGranted();

	FName GetSelectedDifficultyID() const { return SelectedDifficultyID; }
	FName GetSelectedMapID() const { return SelectedMapID; }
	FName GetSelectedStageID() const { return SelectedStageID; }
	bool IsDailyRun() const { return bDailyRun; }
	const FString& GetDailyChallengeId() const { return DailyChallengeId; }
	int32 GetDailySeed() const { return DailySeed; }
	bool HasSelectedDifficulty() const { return SelectedDifficultyID != NAME_None; }
	bool HasSelectedMap() const { return SelectedMapID != NAME_None; }
	bool HasSelectedStage() const { return SelectedStageID != NAME_None; }

private:
	UPROPERTY()
	FName SelectedDifficultyID = NAME_None;

	UPROPERTY()
	FName SelectedMapID = NAME_None;

	UPROPERTY()
	FName SelectedStageID = NAME_None;

	UPROPERTY()
	bool bBattleRewardGranted = false;

	UPROPERTY()
	bool bDailyRun = false;

	UPROPERTY()
	FString DailyChallengeId;

	UPROPERTY()
	int32 DailySeed = 0;
};
