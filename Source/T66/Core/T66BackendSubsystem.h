// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "T66BackendSubsystem.generated.h"

class UT66LeaderboardRunSummarySaveGame;

// ── Delegates ────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnSubmitRunResponse,
	bool, bSuccess,
	int32, ScoreRankAlltime,
	int32, ScoreRankWeekly,
	bool, bNewPersonalBest);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMyRankResponse,
	bool, bSuccess,
	int32, Rank,
	int32, TotalEntries);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAccountStatusResponse,
	bool, bSuccess,
	const FString&, Restriction);

/**
 * Handles all HTTP communication with the T66 backend (Vercel).
 *
 * Reads BackendBaseUrl from DefaultGame.ini [T66.Online].
 * Uses a hex-encoded Steam session ticket for authentication
 * (placeholder until Steamworks SDK is integrated).
 */
UCLASS()
class T66_API UT66BackendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ── Configuration ────────────────────────────────────────

	/** Returns true if a backend URL is configured and reachable. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	bool IsBackendConfigured() const { return !BackendBaseUrl.IsEmpty(); }

	/** The backend base URL (e.g. https://t66-backend.vercel.app). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	FString GetBackendBaseUrl() const { return BackendBaseUrl; }

	// ── Steam Ticket ─────────────────────────────────────────

	/**
	 * Set the hex-encoded Steam session ticket.
	 * Call this once at game start after SteamAPI_Init + GetAuthSessionTicket.
	 * Until Steam is integrated, call SetSteamTicketHex("dev_placeholder") for testing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Backend")
	void SetSteamTicketHex(const FString& TicketHex);

	/** True if a session ticket has been set. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Backend")
	bool HasSteamTicket() const { return !CachedSteamTicketHex.IsEmpty(); }

	// ── API: Submit Run ──────────────────────────────────────

	/**
	 * Submit the current run to the backend.
	 * Fires OnSubmitRunComplete when done.
	 */
	UFUNCTION(BlueprintCallable, Category = "Backend")
	void SubmitRunToBackend(
		const FString& DisplayName,
		int32 Score,
		int32 TimeMs,
		ET66Difficulty Difficulty,
		ET66PartySize PartySize,
		int32 StageReached,
		const FString& HeroId,
		const FString& CompanionId);

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnSubmitRunResponse OnSubmitRunComplete;

	// ── API: My Rank ─────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void FetchMyRank(
		const FString& Type,
		const FString& Time,
		const FString& Party,
		const FString& Difficulty);

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnMyRankResponse OnMyRankComplete;

	// ── API: Account Status ──────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Backend")
	void FetchAccountStatus();

	UPROPERTY(BlueprintAssignable, Category = "Backend")
	FOnAccountStatusResponse OnAccountStatusComplete;

	// ── API: Health Check ────────────────────────────────────

	/** Quick connectivity check. Logs result. */
	UFUNCTION(BlueprintCallable, Category = "Backend")
	void PingHealth();

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

private:
	FString BackendBaseUrl;
	FString CachedSteamTicketHex;

	struct FCachedLeaderboard
	{
		TArray<FLeaderboardEntry> Entries;
		int32 TotalEntries = 0;
	};
	TMap<FString, FCachedLeaderboard> LeaderboardCache;
	TSet<FString> PendingLeaderboardFetches;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateRequest(const FString& Verb, const FString& Endpoint) const;
	void SetAuthHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request) const;

	static FString DifficultyToApiString(ET66Difficulty Diff);
	static FString PartySizeToApiString(ET66PartySize Party);

	// Response handlers
	void OnSubmitRunResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnMyRankResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnAccountStatusResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnHealthResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnLeaderboardResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString LeaderboardKey);

	static ET66Difficulty ApiStringToDifficulty(const FString& S);
	static ET66PartySize ApiStringToPartySize(const FString& S);
};
