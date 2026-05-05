// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/T66DailyClimbTypes.h"
#include "Data/T66DataTypes.h"
#include "Containers/Ticker.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "T66BackendSubsystem.generated.h"

class UT66LeaderboardRunSummarySaveGame;
class FJsonObject;

struct FT66MultiplayerDiagnosticContext
{
	FString EventName;
	FString Severity = TEXT("info");
	FString Message;
	FString InviteId;
	FString HostSteamId;
	FString TargetSteamId;
	FString LobbyId;
	FString ExpectedLobbyId;
	FString FoundLobbyId;
	FString SourceAppId;
	FString StatusText;
	FString MapName;
	TMap<FString, FString> ExtraFields;
};

// ── Delegates ────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnSubmitRunResponse,
	bool, bSuccess,
	int32, ScoreRankAlltime,
	int32, ScoreRankWeekly,
	bool, bNewPersonalBest);

DECLARE_MULTICAST_DELEGATE_EightParams(
	FOnT66SubmitRunDataReady,
	const FString&,
	bool,
	int32,
	int32,
	int32,
	int32,
	bool,
	bool);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMyRankResponse,
	bool, bSuccess,
	int32, Rank,
	int32, TotalEntries);

DECLARE_MULTICAST_DELEGATE_FourParams(
	FOnT66MyRankDataReady,
	const FString&,
	bool,
	int32,
	int32);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnT66DailyClimbChallengeReady,
	const FString& /*RequestTag*/);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnT66MinigameDailyChallengeReady,
	const FString& /*RequestKey*/);

DECLARE_MULTICAST_DELEGATE_FiveParams(
	FOnT66DailyClimbSubmitDataReady,
	const FString& /*RequestKey*/,
	bool /*bSuccess*/,
	const FString& /*Status*/,
	int32 /*DailyRank*/,
	int32 /*CouponsAwarded*/);

DECLARE_MULTICAST_DELEGATE_FourParams(
	FOnT66MinigameSubmitDataReady,
	const FString& /*RequestKey*/,
	bool /*bSuccess*/,
	const FString& /*Status*/,
	int32 /*Rank*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAccountStatusResponse,
	bool, bSuccess,
	const FString&, Restriction);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnBackendActionResponse,
	bool, bSuccess,
	const FString&, Message);

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnT66BackendActionDataReady,
	bool /*bSuccess*/,
	const FString& /*Message*/);

DECLARE_MULTICAST_DELEGATE_FiveParams(
	FOnT66ClientLaunchPolicyResponse,
	bool /*bSuccess*/,
	bool /*bUpdateRequired*/,
	int32 /*RequiredBuildId*/,
	int32 /*LatestBuildId*/,
	const FString& /*Message*/);

USTRUCT(BlueprintType)
struct T66_API FT66PartyInviteEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString InviteId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString HostSteamId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString HostDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString HostAvatarUrl;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString HostLobbyId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString HostAppId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString TargetSteamId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString CreatedAtIso;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString ExpiresAtIso;
};

USTRUCT(BlueprintType)
struct T66_API FT66MinigameDailyChallengeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString ChallengeId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString GameId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString Difficulty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString ChallengeDateUtc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString StartsAtIso;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString EndsAtIso;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	int32 RunSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minigames")
	FString LeaderboardKey;

	bool IsValid() const { return !ChallengeId.IsEmpty() && !GameId.IsEmpty(); }
};

DECLARE_MULTICAST_DELEGATE(FOnT66PendingPartyInvitesChanged);
DECLARE_MULTICAST_DELEGATE_FourParams(
	FOnT66PartyInviteActionComplete,
	bool /*bSuccess*/,
	const FString& /*Action*/,
	const FString& /*InviteId*/,
	const FString& /*Message*/);

/**
 * Handles all HTTP communication with the T66 backend (Vercel).
 *
 * Reads BackendBaseUrl from DefaultGame.ini [T66.Online].
 * Uses a hex-encoded Steam session ticket for authentication.
 */
UCLASS()
class T66_API UT66BackendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── Configuration ────────────────────────────────────────

	/** Returns true if a backend URL is configured and reachable. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	bool IsBackendConfigured() const { return !BackendBaseUrl.IsEmpty(); }

	/** The backend base URL (e.g. https://t66-backend.vercel.app). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	FString GetBackendBaseUrl() const { return BackendBaseUrl; }

	// ── Steam Ticket ─────────────────────────────────────────

	/**
	 * Set the hex-encoded Steam Web API ticket.
	 * Call this once at game start after SteamAPI_Init + GetAuthTicketForWebApi.
	 * Until Steam is integrated, call SetSteamTicketHex("dev_placeholder") for testing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Backend")
	void SetSteamTicketHex(const FString& TicketHex);

	/** True if a session ticket has been set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	bool HasSteamTicket() const { return !CachedSteamTicketHex.IsEmpty(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend|Party")
	const TArray<FT66PartyInviteEntry>& GetPendingPartyInvites() const { return PendingPartyInvites; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend|Party")
	bool HasPendingPartyInvites() const { return PendingPartyInvites.Num() > 0; }

	UFUNCTION(BlueprintCallable, Category = "Backend|Party")
	bool SendPartyInvite(const FString& TargetSteamId, const FString& TargetDisplayName = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Backend|Party")
	void PollPendingPartyInvites(bool bForce = false);

	UFUNCTION(BlueprintCallable, Category = "Backend|Party")
	bool RespondToPartyInvite(const FString& InviteId, bool bAccept);

	void SubmitMultiplayerDiagnostic(const FT66MultiplayerDiagnosticContext& Diagnostic);

	FOnT66PendingPartyInvitesChanged& OnPendingPartyInvitesChanged() { return PendingPartyInvitesChanged; }
	FOnT66PartyInviteActionComplete& OnPartyInviteActionComplete() { return PartyInviteActionComplete; }

	// ── API: Submit Run ──────────────────────────────────────

	/**
	 * Submit the current run to the backend.
	 * If Snapshot is provided, full run summary data (stats, inventory, idols, damage) is included.
	 * Fires OnSubmitRunComplete when done.
	 */
	void SubmitRunToBackend(
		const FString& DisplayName,
		int32 Score,
		int32 TimeMs,
		ET66Difficulty Difficulty,
		ET66PartySize PartySize,
		int32 StageReached,
		const FString& HeroId,
		const FString& CompanionId,
		UT66LeaderboardRunSummarySaveGame* Snapshot = nullptr,
		const FString& RequestKey = FString());

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnSubmitRunResponse OnSubmitRunComplete;

	FOnT66SubmitRunDataReady OnSubmitRunDataReady;

	const FString& GetLastSubmitRunStatus() const { return LastSubmitRunStatus; }
	const FString& GetLastSubmitRunReason() const { return LastSubmitRunReason; }

	// ── API: My Rank ─────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void FetchMyRank(
		const FString& Type,
		const FString& Time,
		const FString& Party,
		const FString& Difficulty);

	void FetchMyRankFiltered(
		const FString& Type,
		const FString& Time,
		const FString& Party,
		const FString& Difficulty,
		const FString& Filter,
		const FString& FilterContext = FString());

	static FString MakeMyRankCacheKey(
		const FString& Type,
		const FString& Time,
		const FString& Party,
		const FString& Difficulty,
		const FString& Filter = TEXT("global"),
		const FString& FilterContext = FString());

	bool HasCachedMyRank(const FString& Key) const;
	bool GetCachedMyRank(const FString& Key, bool& bOutSuccess, int32& OutRank, int32& OutTotalEntries) const;

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnMyRankResponse OnMyRankComplete;

	FOnT66MyRankDataReady OnMyRankDataReady;

	// ── API: Daily Climb ─────────────────────────────────────

	void FetchCurrentDailyClimb();
	void StartDailyClimbAttempt();
	void FetchDailyLeaderboard(const FString& Filter = TEXT("global"));
	void SubmitDailyClimbRun(
		const FString& DisplayName,
		UT66LeaderboardRunSummarySaveGame* Snapshot,
		const FString& ChallengeId,
		const FString& AttemptId,
		const FString& RequestKey = FString());

	bool HasCachedDailyClimbChallenge() const { return CachedDailyClimbChallenge.IsValid(); }
	const FT66DailyClimbChallengeData& GetCachedDailyClimbChallenge() const { return CachedDailyClimbChallenge; }
	const FString& GetLastDailyClimbStatus() const { return LastDailyClimbStatus; }
	const FString& GetLastDailyClimbMessage() const { return LastDailyClimbMessage; }

	FOnT66DailyClimbChallengeReady OnDailyClimbChallengeReady;
	FOnT66DailyClimbSubmitDataReady OnDailyClimbSubmitDataReady;

	// ── API: Minigame Leaderboards ───────────────────────────

	void FetchCurrentMinigameDailyChallenge(
		const FString& GameId,
		const FString& Difficulty);

	void FetchMinigameLeaderboard(
		const FString& GameId,
		const FString& Scope,
		const FString& Difficulty,
		const FString& Filter = TEXT("global"));

	void SubmitMinigameScore(
		const FString& DisplayName,
		const FString& GameId,
		const FString& Scope,
		const FString& Difficulty,
		int32 Score,
		const FString& ChallengeId = FString(),
		int32 RunSeed = 0,
		const FString& RequestKey = FString());

	static FString MakeMinigameDailyChallengeCacheKey(
		const FString& GameId,
		const FString& Difficulty);

	bool GetCachedMinigameDailyChallenge(
		const FString& Key,
		FT66MinigameDailyChallengeData& OutChallenge) const;

	FOnT66MinigameDailyChallengeReady OnMinigameDailyChallengeReady;
	FOnT66MinigameSubmitDataReady OnMinigameSubmitDataReady;

	// ── API: Account Status ──────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void FetchAccountStatus();

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnAccountStatusResponse OnAccountStatusComplete;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	FString GetLastAccountStatusReason() const { return LastAccountStatusReason; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	FString GetLastAccountAppealStatus() const { return LastAccountAppealStatus; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	FString GetLastAccountRunSummaryId() const { return LastAccountRunSummaryId; }

	// ── API: Moderation / Bug Report ─────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void SubmitRunReport(const FString& TargetEntryId, const FString& Reason, FString EvidenceUrl = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void SubmitAppeal(const FString& Message, FString EvidenceUrl = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void UpdateProofOfRun(const FString& EntryId, const FString& ProofUrl);

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void SubmitBugReport(
		const FString& Message,
		int32 Stage,
		const FString& Difficulty,
		const FString& Party,
		const FString& HeroId);

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnBackendActionResponse OnRunReportComplete;

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnBackendActionResponse OnAppealSubmitComplete;

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnBackendActionResponse OnProofOfRunComplete;

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnBackendActionResponse OnBugReportComplete;

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnBackendActionResponse OnStreamerRequestComplete;

	FOnT66BackendActionDataReady OnStreamerRequestDataReady;

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void SubmitStreamerRequest(const FString& CreatorLink, const FString& SteamId);

	// ── API: Health Check ────────────────────────────────────

	/** Quick connectivity check. Logs result. */
	UFUNCTION(BlueprintCallable, Category = "Backend")
	void PingHealth();

	// ── API: Launch Policy ────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void FetchClientLaunchPolicy(int32 LocalSteamBuildId, const FString& SteamAppId, const FString& SteamBetaName = TEXT(""));

	FOnT66ClientLaunchPolicyResponse& OnClientLaunchPolicyResponse() { return ClientLaunchPolicyResponse; }

	// ── API: Fetch Leaderboard ───────────────────────────────

	/**
	 * Fetch Top 10 entries for a leaderboard key.
	 * Parses JSON into FLeaderboardEntry and caches per key.
	 * Fires OnLeaderboardDataReady when done.
	 */
	void FetchLeaderboard(
		const FString& Type, const FString& Time, const FString& Party,
		const FString& Difficulty, const FString& Filter = TEXT("global"));

	/** Check if cached entries exist for a leaderboard key. */
	bool HasCachedLeaderboard(const FString& Key) const;

	/** Get cached entries for a leaderboard key (empty if not cached). */
	TArray<FLeaderboardEntry> GetCachedLeaderboard(const FString& Key) const;

	/** Total entries count from the last fetch for this key. */
	int32 GetCachedTotalEntries(const FString& Key) const;

	/** Non-dynamic delegate fired when leaderboard data is fetched. Param = leaderboard key. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLeaderboardDataReady, const FString&);
	FOnLeaderboardDataReady OnLeaderboardDataReady;

	// ── API: Fetch Run Summary ──────────────────────────────

	/**
	 * Fetch a run summary for a leaderboard entry from the backend.
	 * Parses the JSON into a UT66LeaderboardRunSummarySaveGame snapshot.
	 * Fires OnRunSummaryReady when done.
	 */
	void FetchRunSummary(const FString& EntryId);

	/** Check if a run summary is cached for the given entry ID. */
	bool HasCachedRunSummary(const FString& EntryId) const;

	/** Get the cached run summary snapshot (nullptr if not cached). Caller does NOT own the pointer. */
	UT66LeaderboardRunSummarySaveGame* GetCachedRunSummary(const FString& EntryId) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnRunSummaryReady, const FString& /*EntryId*/);
	FOnRunSummaryReady OnRunSummaryReady;

private:
	FString BackendBaseUrl;
	FString CachedSteamTicketHex;
	FString LastAccountStatusReason;
	FString LastAccountAppealStatus;
	FString LastAccountRunSummaryId;
	FString LastClientLaunchPolicyMessage;
	int32 LastRequiredSteamBuildId = 0;
	int32 LastLatestSteamBuildId = 0;
	bool bLastClientLaunchPolicyRequiresUpdate = false;
	TArray<FT66PartyInviteEntry> PendingPartyInvites;
	FOnT66PendingPartyInvitesChanged PendingPartyInvitesChanged;
	FOnT66PartyInviteActionComplete PartyInviteActionComplete;
	FOnT66ClientLaunchPolicyResponse ClientLaunchPolicyResponse;

	struct FCachedLeaderboard
	{
		TArray<FLeaderboardEntry> Entries;
		int32 TotalEntries = 0;
	};

	struct FCachedMyRank
	{
		bool bSuccess = false;
		int32 Rank = 0;
		int32 TotalEntries = 0;
	};

	struct FPendingCoopSubmit
	{
		FString DisplayName;
		int32 Score = 0;
		int32 TimeMs = 0;
		ET66Difficulty Difficulty = ET66Difficulty::Easy;
		ET66PartySize PartySize = ET66PartySize::Solo;
		int32 StageReached = 1;
		FString HeroId;
		FString CompanionId;
		FString HostRunJson;
		FString RequestKey;
		int32 RetryCount = 0;
	};

	TMap<FString, FCachedLeaderboard> LeaderboardCache;
	TSet<FString> PendingLeaderboardFetches;
	TMap<FString, FCachedMyRank> MyRankCache;
	TSet<FString> PendingMyRankFetches;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UT66LeaderboardRunSummarySaveGame>> RunSummaryCache;
	FString LastSubmitRunStatus;
	FString LastSubmitRunReason;
	FT66DailyClimbChallengeData CachedDailyClimbChallenge;
	FString LastDailyClimbStatus;
	FString LastDailyClimbMessage;
	TMap<FString, FT66MinigameDailyChallengeData> MinigameDailyChallengeCache;
	FString LastMinigameDailyChallengeStatus;
	FString LastMinigameDailyChallengeMessage;
	FString LastMinigameSubmitStatus;
	FString LastMinigameSubmitMessage;
	TSet<FString> PendingRunSummaryFetches;
	FTSTicker::FDelegateHandle PartyInvitePollTickerHandle;
	FTSTicker::FDelegateHandle PendingCoopSubmitTickerHandle;
	TWeakPtr<IHttpRequest, ESPMode::ThreadSafe> PendingPartyInvitePollRequest;
	double LastPartyInvitePollTimeSeconds = 0.0;
	bool bPartyInvitePollInFlight = false;
	bool bPartyInvitePollRequestedWhileInFlight = false;
	TMap<FString, FPendingCoopSubmit> PendingCoopSubmitRequests;

	void SeedDevelopmentDummyLeaderboardsIfNeeded();
	bool TryPopulateDevelopmentDummyLeaderboard(const FString& Key);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateRequest(const FString& Verb, const FString& Endpoint) const;
	void SetAuthHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request) const;
	bool HandlePartyInvitePollTicker(float DeltaTime);
	bool HandlePendingCoopSubmitTicker(float DeltaTime);
	void SetPendingPartyInvites(TArray<FT66PartyInviteEntry>&& NewInvites);
	void QueuePendingCoopSubmit(
		const FString& DisplayName,
		int32 Score,
		int32 TimeMs,
		ET66Difficulty Difficulty,
		ET66PartySize PartySize,
		int32 StageReached,
		const FString& HeroId,
		const FString& CompanionId,
		const FString& HostRunJson,
		const FString& RequestKey);
	bool TrySubmitRunToBackendNow(
		const FString& DisplayName,
		int32 Score,
		int32 TimeMs,
		ET66Difficulty Difficulty,
		ET66PartySize PartySize,
		int32 StageReached,
		const FString& HeroId,
		const FString& CompanionId,
		const FString& HostRunJson,
		const FString& RequestKey);
	void SendSubmitRunRequest(const TSharedPtr<FJsonObject>& Root, const FString& RequestKey);

	static FString DifficultyToApiString(ET66Difficulty Diff);
	static FString PartySizeToApiString(ET66PartySize Party);

	// Response handlers
	void OnSubmitRunResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RequestKey);
	void OnMyRankResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RankKey);
	void OnAccountStatusResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnRunReportResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnAppealResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnProofOfRunResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnBugReportResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnStreamerRequestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnHealthResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnClientLaunchPolicyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, int32 LocalSteamBuildId);
	void OnSendPartyInviteResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnPendingPartyInvitesResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnRespondPartyInviteResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString InviteId, FString Action);
	void OnClientDiagnosticsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString EventName, FString InviteId);
	void OnLeaderboardResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString LeaderboardKey);
	void OnRunSummaryResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString EntryId);
	void OnDailyClimbStatusResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RequestTag);
	void OnDailyClimbSubmitResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RequestKey);
	void OnMinigameDailyChallengeResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RequestKey);
	void OnMinigameSubmitResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RequestKey, FString GameId, FString Scope, FString Difficulty);

	static ET66Difficulty ApiStringToDifficulty(const FString& S);
	static ET66PartySize ApiStringToPartySize(const FString& S);
	static FString ExtractResponseMessage(const TSharedPtr<FJsonObject>& Json, const FString& FallbackMessage);
	TSharedPtr<FJsonObject> BuildMultiplayerDiagnosticJson(const FT66MultiplayerDiagnosticContext& Diagnostic) const;
	void SaveMultiplayerDiagnosticLocally(const TSharedPtr<FJsonObject>& DiagnosticJson, const FString& EventName) const;

	/** Parse a run-summary JSON object into a UT66LeaderboardRunSummarySaveGame. */
	static UT66LeaderboardRunSummarySaveGame* ParseRunSummaryFromJson(const TSharedPtr<FJsonObject>& Json, UObject* Outer);
};
