// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66LeaderboardSubsystem.generated.h"

struct FLeaderboardEntry;
class UT66LocalLeaderboardSaveGame;
class UDataTable;

/**
 * Foundation for leaderboard submission.
 * For now this provides a fully local (offline) "Top 10 + You" experience, with placeholder global targets.
 * Practice Mode blocks submission. "Anonymous" affects displayed name for the local entry.
 */
UCLASS()
class T66_API UT66LeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Submit a run bounty (local best). Returns false if blocked (e.g., Practice Mode). */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	bool SubmitRunBounty(int32 Bounty);

	/**
	 * Submit a completed stage time (local best), keyed by current difficulty/party size.
	 * Stage is expected to be 1..10 for the menu leaderboard, but we store any stage for future use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	bool SubmitStageSpeedRunTime(int32 Stage, float Seconds);

	/** Build menu entries for Bounty: Top 10 + local entry ("You") at rank 11 unless in Top 10. */
	TArray<FLeaderboardEntry> BuildBountyEntries(ET66Difficulty Difficulty, ET66PartySize PartySize) const;

	/** Build menu entries for Speed Run: per-stage Top 10 + local entry ("You") at rank 11 unless in Top 10. */
	TArray<FLeaderboardEntry> BuildSpeedRunEntries(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const;

	/** Returns the 10th-place target time for this stage (used by HUD "time to beat"). */
	bool GetSpeedRunTarget10Seconds(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage, float& OutSeconds) const;

private:
	static constexpr const TCHAR* LocalSaveSlotName = TEXT("T66_LocalLeaderboard");
	static constexpr const TCHAR* BountyTargetsDTPath = TEXT("/Game/Data/DT_Leaderboard_BountyTargets.DT_Leaderboard_BountyTargets");
	static constexpr const TCHAR* SpeedRunTargetsDTPath = TEXT("/Game/Data/DT_Leaderboard_SpeedrunTargets.DT_Leaderboard_SpeedrunTargets");

	UPROPERTY(Transient)
	TObjectPtr<UT66LocalLeaderboardSaveGame> LocalSave;

	// Targets (placeholder "global" tuning) loaded from CSV under Content/Data if present.
	// Bounty target is keyed by Difficulty+PartySize. Speedrun target is keyed by Difficulty+Stage (party applies as a multiplier).
	TMap<uint64, int64> BountyTarget10ByKey;
	TMap<uint64, float> SpeedRunTarget10ByKey_DiffStage;

	void LoadOrCreateLocalSave();
	void SaveLocalSave() const;

	bool LoadTargetsFromDataTablesIfPresent();
	void LoadTargetsFromCsv();
	void LoadBountyTargetsFromCsv(const FString& AbsPath);
	void LoadSpeedRunTargetsFromCsv(const FString& AbsPath);

	static uint64 MakeBountyKey(ET66Difficulty Difficulty, ET66PartySize PartySize);
	static uint64 MakeSpeedRunKey_DiffStage(ET66Difficulty Difficulty, int32 Stage);

	int64 GetBountyTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize) const;
	float GetSpeedRunTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const;

	bool GetSettingsPracticeAndAnon(bool& bOutPractice, bool& bOutAnon) const;
};

