// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66IdleDataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66IdleFrontendStateSubsystem.generated.h"

UCLASS()
class T66IDLE_API UT66IdleFrontendStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void ResetSessionState();
	void BeginNewSession();
	void BeginDailySession(FName DifficultyID, const FString& ChallengeId, int32 Seed);
	void SelectHero(FName HeroID);
	void SetProfileSnapshot(const FT66IdleProfileSnapshot& Snapshot);

	FName GetSelectedHeroID() const { return SelectedHeroID; }
	FName GetSelectedDifficultyID() const { return SelectedDifficultyID; }
	bool IsDailySession() const { return bDailySession; }
	const FString& GetDailyChallengeId() const { return DailyChallengeId; }
	int32 GetDailySeed() const { return DailySeed; }
	bool HasSelectedHero() const { return SelectedHeroID != NAME_None; }
	const FT66IdleProfileSnapshot& GetProfileSnapshot() const { return ProfileSnapshot; }

private:
	UPROPERTY()
	FName SelectedHeroID = NAME_None;

	UPROPERTY()
	FName SelectedDifficultyID = NAME_None;

	UPROPERTY()
	bool bDailySession = false;

	UPROPERTY()
	FString DailyChallengeId;

	UPROPERTY()
	int32 DailySeed = 0;

	UPROPERTY()
	FT66IdleProfileSnapshot ProfileSnapshot;
};
