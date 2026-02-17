// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalLeaderboardSaveGame.h"
#include "UI/T66UITypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66LeaderboardSubsystem.generated.h"

struct FLeaderboardEntry;
class UT66LocalLeaderboardSaveGame;
class UT66LeaderboardRunSummarySaveGame;
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

	/** Submit a run score (local best). Returns false if blocked (e.g., Practice Mode). */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	bool SubmitRunScore(int32 Score);

	/**
	 * Submit a completed stage time (local best), keyed by current difficulty/party size.
	 * Stage is expected to be 1..10 for the menu leaderboard, but we store any stage for future use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	bool SubmitStageSpeedRunTime(int32 Stage, float Seconds);
 
	/** Returns true if a saved run summary exists for the local best score entry. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	bool HasLocalBestScoreRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize) const;

	/** True if the most recent score submit set a new personal best (for the active difficulty/party). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	bool WasLastScoreNewPersonalBest() const { return bLastScoreWasNewBest; }

	/** True if the most recent speed run submit set a new personal best time (for the active difficulty/party/stage). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	bool WasLastSpeedRunNewPersonalBest() const { return bLastSpeedRunWasNewBest; }

	/** Stage number associated with the most recent speed run submission. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	int32 GetLastSpeedRunSubmittedStage() const { return LastSpeedRunSubmittedStage; }

	/**
	 * UI helper: request opening the local-best score run summary.
	 * `UT66RunSummaryScreen` will consume this request on activation.
	 */
	void RequestOpenLocalBestScoreRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize);

	/** Consume the pending "open run summary" request (returns true if one existed). */
	bool ConsumePendingRunSummaryRequest(FString& OutSaveSlotName);

	/** Set an in-memory fake run summary (e.g. for non-local leaderboard rows). Run Summary screen consumes on activation. */
	void SetPendingFakeRunSummarySnapshot(UT66LeaderboardRunSummarySaveGame* Snapshot);
	/** Consume the pending fake snapshot (returns it and clears; caller / Run Summary owns it). */
	UT66LeaderboardRunSummarySaveGame* ConsumePendingFakeRunSummarySnapshot();

	/** Set 2 or 3 snapshots for the "Pick the Player" modal (duo/trio). Picker reads via GetPendingPickerSnapshots. */
	void SetPendingPickerSnapshots(TArray<UT66LeaderboardRunSummarySaveGame*> Snapshots);
	/** Snapshot array for the picker UI (do not modify). */
	const TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>>& GetPendingPickerSnapshots() const;
	/** Clear picker snapshots (call after user selects one and you've set PendingFakeSnapshot). */
	void ClearPendingPickerSnapshots();

	/** Create a fake run summary snapshot for a leaderboard entry (random hero, idols, items; full slots). Caller owns result. */
	UT66LeaderboardRunSummarySaveGame* CreateFakeRunSummarySnapshot(
		ET66LeaderboardFilter Filter, ET66LeaderboardType Type, ET66Difficulty Difficulty, ET66PartySize PartySize,
		int32 Rank, int32 SlotIndex, const FString& PlayerDisplayName, int64 Score, float TimeSeconds) const;

	// ===== Account Status (Suspension / Appeal) =====

	/** True when the Main Menu should show the Account Status button. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	bool ShouldShowAccountStatusButton() const;

	/** Current account restriction record (local placeholder until backend exists). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	FT66AccountRestrictionRecord GetAccountRestrictionRecord() const;

	/** True if there is a run summary slot associated with the restricted run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	bool HasAccountRestrictionRunSummary() const;

	/** True if the player can submit an appeal right now. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	bool CanSubmitAccountAppeal() const;

	/** Submit an appeal message + evidence URL (local placeholder). */
	UFUNCTION(BlueprintCallable, Category = "AccountStatus")
	void SubmitAccountAppeal(const FString& Message, const FString& EvidenceUrl);

	/**
	 * UI helper: request opening the restricted run summary. When the Run Summary closes, it should return to Account Status.
	 * Returns false if there is no associated run summary.
	 */
	bool RequestOpenAccountRestrictionRunSummary();

	/** Consume the "return to modal after viewer-mode Run Summary" request. */
	ET66ScreenType ConsumePendingReturnModalAfterViewerRunSummary();

	/** Build menu entries for Score: Top 10 + local entry ("You") at rank 11 unless in Top 10. */
	TArray<FLeaderboardEntry> BuildScoreEntries(ET66Difficulty Difficulty, ET66PartySize PartySize) const;

	/** Build menu entries for Speed Run: per-stage Top 10 + local entry ("You") at rank 11 unless in Top 10. */
	TArray<FLeaderboardEntry> BuildSpeedRunEntries(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const;

	/**
	 * Build leaderboard entries for the current filter (Global / Friends / Streamers).
	 * Global uses the existing built list (Score or SpeedRun). Friends/Streamers load from DT_Leaderboard_Friends / DT_Leaderboard_Streamers.
	 */
	TArray<FLeaderboardEntry> BuildEntriesForFilter(ET66LeaderboardFilter Filter, ET66LeaderboardType Type, ET66Difficulty Difficulty, ET66PartySize PartySize, int32 SpeedRunStage) const;

	/** Returns the 10th-place target time for this stage (used by HUD "time to beat"). */
	bool GetSpeedRunTarget10Seconds(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage, float& OutSeconds) const;

	/** Rank (1–11) of local player's best score for this difficulty/party; 0 if none. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	int32 GetLocalScoreRank(ET66Difficulty Difficulty, ET66PartySize PartySize) const;

	/** Rank (1–11) of local player's best speed run time for this stage; 0 if no time submitted. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	int32 GetLocalSpeedRunRank(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const;

	/** Debug helper: delete local leaderboard + local best run summary snapshots, then reload. */
	void DebugClearLocalLeaderboard();

private:
	static constexpr const TCHAR* LocalSaveSlotName = TEXT("T66_LocalLeaderboard");
	static constexpr const TCHAR* ScoreTargetsDTPath = TEXT("/Game/Data/DT_Leaderboard_ScoreTargets.DT_Leaderboard_ScoreTargets");
	static constexpr const TCHAR* SpeedRunTargetsDTPath = TEXT("/Game/Data/DT_Leaderboard_SpeedrunTargets.DT_Leaderboard_SpeedrunTargets");
	static constexpr const TCHAR* FriendsDTPath = TEXT("/Game/Data/DT_Leaderboard_Friends.DT_Leaderboard_Friends");
	static constexpr const TCHAR* StreamersDTPath = TEXT("/Game/Data/DT_Leaderboard_Streamers.DT_Leaderboard_Streamers");

	UPROPERTY(Transient)
	TObjectPtr<UT66LocalLeaderboardSaveGame> LocalSave;

	// Targets (placeholder "global" tuning) loaded from CSV under Content/Data if present.
	// Score target is keyed by Difficulty+PartySize. Speedrun target is keyed by Difficulty+Stage (party applies as a multiplier).
	TMap<uint64, int64> ScoreTarget10ByKey;
	TMap<uint64, float> SpeedRunTarget10ByKey_DiffStage;

	void LoadOrCreateLocalSave();
	void SaveLocalSave() const;

	static FString DifficultyKey(ET66Difficulty Difficulty);
	static FString PartySizeKey(ET66PartySize PartySize);
	static FString MakeLocalBestScoreRunSummarySlotName(ET66Difficulty Difficulty, ET66PartySize PartySize);

	bool SaveLocalBestScoreRunSummarySnapshot(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Score) const;

	bool LoadTargetsFromDataTablesIfPresent();
	void LoadTargetsFromCsv();
	void LoadScoreTargetsFromCsv(const FString& AbsPath);
	void LoadSpeedRunTargetsFromCsv(const FString& AbsPath);

	static uint64 MakeScoreKey(ET66Difficulty Difficulty, ET66PartySize PartySize);
	static uint64 MakeSpeedRunKey_DiffStage(ET66Difficulty Difficulty, int32 Stage);

	int64 GetScoreTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize) const;
	float GetSpeedRunTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const;

	bool GetSettingsPracticeAndAnon(bool& bOutPractice, bool& bOutAnon) const;

	// Transient UI "handshake": panel sets a requested slot name, RunSummaryScreen consumes it.
	FString PendingRunSummarySlotName;

	/** In-memory fake snapshot for non-local rows (consumed by Run Summary on open). */
	UPROPERTY(Transient)
	TObjectPtr<UT66LeaderboardRunSummarySaveGame> PendingFakeSnapshot;

	/** 2 or 3 snapshots for "Pick the Player" (duo/trio); cleared when user selects one. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UT66LeaderboardRunSummarySaveGame>> PendingPickerSnapshots;

	// Transient UI "handshake": a viewer-mode run summary can request reopening a modal afterwards (single-modal UI manager).
	ET66ScreenType PendingReturnModalAfterViewerRunSummary = ET66ScreenType::None;

	// ===== Last submit results (for Run Summary "New Personal Best" banners) =====
	bool bLastScoreWasNewBest = false;
	bool bLastSpeedRunWasNewBest = false;
	int32 LastSpeedRunSubmittedStage = 0;
};

