// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66LegacyRuntimeDataAccess.h"
#include "Data/T66DataTypes.h"
#include "Core/T66LocalLeaderboardSaveGame.h"
#include "UI/T66UITypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66LeaderboardSubsystem.generated.h"

class UT66BackendSubsystem;

struct FLeaderboardEntry;
class UT66LocalLeaderboardSaveGame;
class UT66LeaderboardRunSummarySaveGame;
class UDataTable;

/**
 * Handles local leaderboard persistence plus online backend sync for the
 * frontend leaderboard, run summaries, and moderation surfaces.
 */
UCLASS()
class T66_API UT66LeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Submit a run score (local best). Returns false if blocked (e.g., Practice Mode). */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	bool SubmitRunScore(int32 Score, const FString& ExistingRunSummarySlotName);

	/**
	 * Legacy hook for stage-complete speedrun timing.
	 * Stage pacing now lives in run summaries, so this no longer writes any local leaderboard state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	bool SubmitStageSpeedRunTime(int32 Stage, float Seconds);
 
	/** Returns true if a saved run summary exists for the local best score entry. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	bool HasLocalBestScoreRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize) const;

	/** True if the most recent score submit set a new personal best (for the active difficulty/party). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	bool WasLastScoreNewPersonalBest() const { return bLastScoreWasNewBest; }

	/** Legacy stage-speedrun PB flag. Remains false while per-stage local PBs are disabled. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	bool WasLastSpeedRunNewPersonalBest() const { return bLastSpeedRunWasNewBest; }

	/** Stage number associated with the most recent legacy stage-speedrun submit attempt. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	int32 GetLastSpeedRunSubmittedStage() const { return LastSpeedRunSubmittedStage; }

	/** True if the most recent completed full-run time submit set a new personal best. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	bool WasLastCompletedRunTimeNewPersonalBest() const { return bLastCompletedRunTimeWasNewBest; }

	/**
	 * UI helper: request opening the local-best score run summary.
	 * `UT66RunSummaryScreen` will consume this request on activation.
	 */
	void RequestOpenLocalBestScoreRunSummary(ET66Difficulty Difficulty, ET66PartySize PartySize);

	/** UI helper: request opening a specific saved run summary slot. */
	bool RequestOpenRunSummarySlot(const FString& SlotName, ET66ScreenType ReturnModalAfterClose = ET66ScreenType::None);

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

	/** Save a unique run-summary snapshot for the current run and append it to the recent-runs list. */
	bool SaveFinishedRunSummarySnapshot(FString& OutSlotName);

	/** Submit a completed full-run time PB (lower is better). */
	bool SubmitCompletedRunTime(float Seconds, const FString& ExistingRunSummarySlotName);

	/** Submit a completed difficulty clear in a single backend request, updating score and speedrun boards together. */
	bool SubmitDifficultyClearRun(float Seconds, const FString& ExistingRunSummarySlotName);

	/** Completed runs, newest first, with no hard history cap. */
	TArray<FT66RecentRunRecord> GetRecentRuns() const;

	/** Best score record for the requested difficulty + party size. */
	bool GetLocalBestScoreRecord(ET66Difficulty Difficulty, ET66PartySize PartySize, FT66LocalScoreRecord& OutRecord) const;

	/** Best completed full-run time record for the requested difficulty + party size. */
	bool GetLocalBestCompletedRunTimeRecord(ET66Difficulty Difficulty, ET66PartySize PartySize, FT66LocalCompletedRunTimeRecord& OutRecord) const;

	// ===== Account Status (Suspension / Appeal) =====

	/** True when the Main Menu should show the Account Status button. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	bool ShouldShowAccountStatusButton() const;

	/** Refresh backend-backed account restriction data when Steam auth is available. */
	UFUNCTION(BlueprintCallable, Category = "AccountStatus")
	void RefreshAccountStatusFromBackend();

	/** Current account restriction record (backend-backed with a local cache). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	FT66AccountRestrictionRecord GetAccountRestrictionRecord() const;

	/** True when the account is allowed to place new scores and times on leaderboards. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	bool IsAccountEligibleForLeaderboard() const;

	/** True if there is a run summary slot associated with the restricted run. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	bool HasAccountRestrictionRunSummary() const;

	/** True if the player can submit an appeal right now. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AccountStatus")
	bool CanSubmitAccountAppeal() const;

	/** Submit an appeal message + evidence URL. */
	UFUNCTION(BlueprintCallable, Category = "AccountStatus")
	void SubmitAccountAppeal(const FString& Message, const FString& EvidenceUrl);

	/**
	 * UI helper: request opening the restricted run summary. When the Run Summary closes, it should return to Account Status.
	 * Returns false if there is no associated run summary.
	 */
	bool RequestOpenAccountRestrictionRunSummary();

	/** Set which modal viewer-mode Run Summary should return to when it closes. */
	void SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType ModalType);

	/** Consume the "return to modal after viewer-mode Run Summary" request. */
	ET66ScreenType ConsumePendingReturnModalAfterViewerRunSummary();

	/** Returns the 10th-place target time for this stage (used by HUD "time to beat"). */
	bool GetSpeedRunTarget10Seconds(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage, float& OutSeconds) const;

	/** Weekly rank of the current run if available, otherwise the locally cached weekly best score rank; 0 if none. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	int32 GetLocalScoreRank(ET66Difficulty Difficulty, ET66PartySize PartySize) const;

	/** Weekly rank of the current run if available, otherwise the locally cached weekly best speedrun rank; 0 if none is recorded. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Leaderboard")
	int32 GetLocalSpeedRunRank(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const;

	/** Debug helper: delete local leaderboard + local best run summary snapshots, then reload. */
	void DebugClearLocalLeaderboard();

private:
	enum class EBestRankRecordType : uint8
	{
		Score,
		CompletedRunTime,
		DifficultyClear,
	};

	struct FPendingBestRankSubmission
	{
		EBestRankRecordType Type = EBestRankRecordType::Score;
		ET66Difficulty Difficulty = ET66Difficulty::Easy;
		ET66PartySize PartySize = ET66PartySize::Solo;
		int64 Score = 0;
		float Seconds = 0.f;
		bool bSubmittedAnonymous = false;
		FString RunSummarySlotName;
		FDateTime AchievedAtUtc = FDateTime::MinValue();
	};

	static constexpr const TCHAR* LocalSaveSlotName = TEXT("T66_LocalLeaderboard");
	static constexpr const TCHAR* AccountRestrictionRunSummarySlotName = TEXT("T66_AccountRestrictionRunSummary");
	static constexpr int32 MinCompatibleLocalSaveSchemaVersion = 3;
	static constexpr const TCHAR* ScoreTargetsDTPath = TEXT("/Game/Data/Leaderboard_ScoreTargets.Leaderboard_ScoreTargets");
	static constexpr const TCHAR* SpeedRunTargetsDTPath = TEXT("/Game/Data/DT_Leaderboard_SpeedrunTargets.DT_Leaderboard_SpeedrunTargets");

	UPROPERTY(Transient)
	TObjectPtr<UT66LocalLeaderboardSaveGame> LocalSave;

	// Legacy runtime targets. Cleanup keeps the current CSV-first behavior visible and
	// explicit until the packaged-parity pass moves these to cooked ownership.
	TMap<uint64, int64> ScoreTarget10ByKey;
	TMap<uint64, float> SpeedRunTarget10ByKey_DiffStage;
	ET66RuntimeDataSource ScoreTargetRuntimeSource = ET66RuntimeDataSource::None;
	ET66RuntimeDataSource SpeedRunTargetRuntimeSource = ET66RuntimeDataSource::None;

	void LoadOrCreateLocalSave();
	void SaveLocalSave() const;

	static FString DifficultyKey(ET66Difficulty Difficulty);
	static FString PartySizeKey(ET66PartySize PartySize);
	static FString MakeLocalBestScoreRunSummarySlotName(ET66Difficulty Difficulty, ET66PartySize PartySize);
	static FString MakeRecentRunSummarySlotName(const FGuid& RunId);

	bool SaveLocalBestScoreRunSummarySnapshot(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Score, const FString& ExistingRunSummarySlotName = FString()) const;
	UT66LeaderboardRunSummarySaveGame* CreateCurrentRunSummarySnapshot(ET66LeaderboardType LeaderboardType, ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Score) const;
	bool SaveRunSummarySnapshotToSlot(UT66LeaderboardRunSummarySaveGame* Snapshot, const FString& SlotName) const;
	void AppendRecentRunRecord(const FString& RunSummarySlotName);
	bool IsRunSummarySlotStillReferenced(const FString& SlotName) const;
	bool DeleteRunSummarySlotIfUnreferenced(const FString& SlotName) const;
	const FT66LocalScoreRecord* FindLocalScoreRecord(ET66Difficulty Difficulty, ET66PartySize PartySize) const;
	FT66LocalScoreRecord* FindLocalScoreRecordMutable(ET66Difficulty Difficulty, ET66PartySize PartySize);
	const FT66LocalCompletedRunTimeRecord* FindLocalCompletedRunTimeRecord(ET66Difficulty Difficulty, ET66PartySize PartySize) const;
	FT66LocalCompletedRunTimeRecord* FindLocalCompletedRunTimeRecordMutable(ET66Difficulty Difficulty, ET66PartySize PartySize);

	bool LoadTargetsFromDataTablesIfPresent();

	static uint64 MakeScoreKey(ET66Difficulty Difficulty, ET66PartySize PartySize);
	static uint64 MakeSpeedRunKey_DiffStage(ET66Difficulty Difficulty, int32 Stage);

	int64 GetScoreTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize) const;
	float GetSpeedRunTarget10(ET66Difficulty Difficulty, ET66PartySize PartySize, int32 Stage) const;

	bool GetSettingsPracticeAndAnon(bool& bOutPractice, bool& bOutAnon) const;
	FString MakePendingBestRankRequestKey(EBestRankRecordType Type, ET66Difficulty Difficulty, ET66PartySize PartySize) const;
	void HandleBackendSubmitRunDataReady(
		const FString& RequestKey,
		bool bSuccess,
		int32 ScoreRankAlltime,
		int32 ScoreRankWeekly,
		int32 SpeedRunRankAlltime,
		int32 SpeedRunRankWeekly,
		bool bNewScorePersonalBest,
		bool bNewSpeedRunPersonalBest);

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
	TMap<FString, FPendingBestRankSubmission> PendingBestRankSubmissions;
	FString LastSubmittedScoreRequestKey;
	FString LastSubmittedCompletedRunTimeRequestKey;

	// ===== Last submit results (for Run Summary "New Personal Best" banners) =====
	bool bLastScoreWasNewBest = false;
	bool bLastSpeedRunWasNewBest = false;
	bool bLastCompletedRunTimeWasNewBest = false;
	int32 LastSpeedRunSubmittedStage = 0;
	int32 LastSubmittedScoreRankAllTime = 0;
	int32 LastSubmittedScoreRankWeekly = 0;
	int32 LastSubmittedCompletedRunRankAllTime = 0;
	int32 LastSubmittedCompletedRunRankWeekly = 0;

	UFUNCTION()
	void HandleBackendAccountStatusComplete(bool bSuccess, const FString& Restriction);

	UFUNCTION()
	void HandleBackendAppealSubmitComplete(bool bSuccess, const FString& Message);

	void HandleBackendRunSummaryReady(const FString& EntryId);

	bool bAccountStatusRefreshRequested = false;
	bool bAccountAppealSubmitInFlight = false;
	FString PendingAccountRestrictionRunSummaryId;
	FString PendingAppealMessage;
	FString PendingAppealEvidenceUrl;
};

