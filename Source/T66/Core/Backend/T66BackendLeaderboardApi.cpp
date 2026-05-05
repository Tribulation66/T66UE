// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendPrivate.h"

void UT66BackendSubsystem::FetchDailyLeaderboard(const FString& Filter)
{
	if (!IsBackendConfigured())
	{
		return;
	}

	const FString NormalizedFilter = Filter.IsEmpty() ? TEXT("global") : Filter.ToLower();
	const FString CacheKey = FString::Printf(TEXT("daily_%s"), *NormalizedFilter);
	if (PendingLeaderboardFetches.Contains(CacheKey))
	{
		return;
	}
	PendingLeaderboardFetches.Add(CacheKey);

	const FString Endpoint = FString::Printf(TEXT("/api/daily/leaderboard?filter=%s"), *NormalizedFilter);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnLeaderboardResponseReceived, CacheKey);
	Request->ProcessRequest();
}

FString UT66BackendSubsystem::MakeMinigameDailyChallengeCacheKey(
	const FString& GameId,
	const FString& Difficulty)
{
	return FString::Printf(TEXT("%s_%s"), *GameId.ToLower(), *Difficulty.ToLower());
}

void UT66BackendSubsystem::FetchCurrentMinigameDailyChallenge(
	const FString& GameId,
	const FString& Difficulty)
{
	const FString NormalizedGameId = GameId.ToLower();
	const FString NormalizedDifficulty = Difficulty.ToLower();
	const FString RequestKey = MakeMinigameDailyChallengeCacheKey(NormalizedGameId, NormalizedDifficulty);

	if (!IsBackendConfigured())
	{
		LastMinigameDailyChallengeStatus = TEXT("backend_unconfigured");
		LastMinigameDailyChallengeMessage = TEXT("Backend URL is not configured.");
		OnMinigameDailyChallengeReady.Broadcast(RequestKey);
		return;
	}

	const FString Endpoint = FString::Printf(
		TEXT("/api/minigames/challenge?game_id=%s&difficulty=%s"),
		*NormalizedGameId,
		*NormalizedDifficulty);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnMinigameDailyChallengeResponseReceived, RequestKey);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::FetchMinigameLeaderboard(
	const FString& GameId,
	const FString& Scope,
	const FString& Difficulty,
	const FString& Filter)
{
	const FString NormalizedGameId = GameId.ToLower();
	const FString NormalizedScope = Scope.IsEmpty() ? TEXT("daily") : Scope.ToLower();
	const FString NormalizedDifficulty = Difficulty.ToLower();
	const FString NormalizedFilter = Filter.IsEmpty() ? TEXT("global") : Filter.ToLower();
	const FString Key = FString::Printf(
		TEXT("minigame_%s_%s_%s_%s"),
		*NormalizedGameId,
		*NormalizedScope,
		*NormalizedDifficulty,
		*NormalizedFilter);

	if (!IsBackendConfigured())
	{
		return;
	}

	if (PendingLeaderboardFetches.Contains(Key))
	{
		return;
	}
	PendingLeaderboardFetches.Add(Key);

	const FString Endpoint = FString::Printf(
		TEXT("/api/minigames/leaderboard?game_id=%s&scope=%s&difficulty=%s&filter=%s"),
		*NormalizedGameId,
		*NormalizedScope,
		*NormalizedDifficulty,
		*NormalizedFilter);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnLeaderboardResponseReceived, Key);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::SubmitMinigameScore(
	const FString& DisplayName,
	const FString& GameId,
	const FString& Scope,
	const FString& Difficulty,
	const int32 Score,
	const FString& ChallengeId,
	const int32 RunSeed,
	const FString& RequestKey)
{
	const FString NormalizedGameId = GameId.ToLower();
	const FString NormalizedScope = Scope.IsEmpty() ? TEXT("alltime") : Scope.ToLower();
	const FString NormalizedDifficulty = Difficulty.ToLower();
	const FString SubmitKey = RequestKey.IsEmpty()
		? FString::Printf(TEXT("minigame_submit_%s_%s_%s"), *NormalizedGameId, *NormalizedScope, *NormalizedDifficulty)
		: RequestKey;

	if (!IsBackendConfigured() || !HasSteamTicket() || DisplayName.TrimStartAndEnd().IsEmpty() || NormalizedGameId.IsEmpty() || NormalizedDifficulty.IsEmpty() || Score < 0)
	{
		LastMinigameSubmitStatus = TEXT("invalid_minigame_submit");
		LastMinigameSubmitMessage = TEXT("Minigame score submission is not ready.");
		OnMinigameSubmitDataReady.Broadcast(SubmitKey, false, LastMinigameSubmitStatus, 0);
		return;
	}

	if (NormalizedScope == TEXT("daily") && ChallengeId.IsEmpty())
	{
		LastMinigameSubmitStatus = TEXT("missing_daily_challenge");
		LastMinigameSubmitMessage = TEXT("Daily minigame score is missing its challenge id.");
		OnMinigameSubmitDataReady.Broadcast(SubmitKey, false, LastMinigameSubmitStatus, 0);
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("display_name"), DisplayName.TrimStartAndEnd());
	Root->SetStringField(TEXT("game_id"), NormalizedGameId);
	Root->SetStringField(TEXT("scope"), NormalizedScope);
	Root->SetStringField(TEXT("difficulty"), NormalizedDifficulty);
	Root->SetNumberField(TEXT("score"), Score);
	if (!SubmitKey.IsEmpty())
	{
		Root->SetStringField(TEXT("submission_id"), SubmitKey);
	}
	if (NormalizedScope == TEXT("daily"))
	{
		Root->SetStringField(TEXT("challenge_id"), ChallengeId);
		Root->SetNumberField(TEXT("run_seed"), RunSeed);
	}

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/minigames/submit"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(
		this,
		&UT66BackendSubsystem::OnMinigameSubmitResponseReceived,
		SubmitKey,
		NormalizedGameId,
		NormalizedScope,
		NormalizedDifficulty);
	Request->ProcessRequest();
}

bool UT66BackendSubsystem::GetCachedMinigameDailyChallenge(
	const FString& Key,
	FT66MinigameDailyChallengeData& OutChallenge) const
{
	if (const FT66MinigameDailyChallengeData* Found = MinigameDailyChallengeCache.Find(Key))
	{
		OutChallenge = *Found;
		return Found->IsValid();
	}

	OutChallenge = FT66MinigameDailyChallengeData();
	return false;
}

void UT66BackendSubsystem::FetchMyRank(
	const FString& Type,
	const FString& Time,
	const FString& Party,
	const FString& Difficulty)
{
	FetchMyRankFiltered(Type, Time, Party, Difficulty, TEXT("global"));
}

void UT66BackendSubsystem::FetchMyRankFiltered(
	const FString& Type,
	const FString& Time,
	const FString& Party,
	const FString& Difficulty,
	const FString& Filter,
	const FString& FilterContext)
{
	const FString ResolvedFilter = Filter.IsEmpty() ? TEXT("global") : Filter;
	FString ResolvedFilterContext = FilterContext;
	if (ResolvedFilter.Equals(TEXT("friends"), ESearchCase::IgnoreCase) && ResolvedFilterContext.IsEmpty())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66SteamHelper* Steam = GI->GetSubsystem<UT66SteamHelper>())
			{
				ResolvedFilterContext = FString::Join(Steam->GetFriendSteamIds(), TEXT(","));
			}
		}
	}

	const FString RankKey = MakeMyRankCacheKey(Type, Time, Party, Difficulty, ResolvedFilter, ResolvedFilterContext);
	if (PendingMyRankFetches.Contains(RankKey))
	{
		return;
	}

	if (!IsBackendConfigured() || !HasSteamTicket())
	{
		FCachedMyRank Cached;
		MyRankCache.Add(RankKey, Cached);
		OnMyRankComplete.Broadcast(false, 0, 0);
		OnMyRankDataReady.Broadcast(RankKey, false, 0, 0);
		return;
	}

	FString Endpoint = FString::Printf(
		TEXT("/api/my-rank?type=%s&time=%s&party=%s&difficulty=%s&filter=%s"),
		*Type,
		*Time,
		*Party,
		*Difficulty,
		*ResolvedFilter);
	if (ResolvedFilter.Equals(TEXT("friends"), ESearchCase::IgnoreCase))
	{
		Endpoint += FString::Printf(TEXT("&friend_ids=%s"), *ResolvedFilterContext);
	}
	else if (ResolvedFilter.Equals(TEXT("hero"), ESearchCase::IgnoreCase) && !ResolvedFilterContext.IsEmpty())
	{
		Endpoint += FString::Printf(TEXT("&hero_id=%s"), *ResolvedFilterContext);
	}

	PendingMyRankFetches.Add(RankKey);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	SetAuthHeaders(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnMyRankResponseReceived, RankKey);
	Request->ProcessRequest();
}

FString UT66BackendSubsystem::MakeMyRankCacheKey(
	const FString& Type,
	const FString& Time,
	const FString& Party,
	const FString& Difficulty,
	const FString& Filter,
	const FString& FilterContext)
{
	const FString ResolvedFilter = Filter.IsEmpty() ? TEXT("global") : Filter;
	const uint32 FilterContextHash = FilterContext.IsEmpty() ? 0u : GetTypeHash(FilterContext);
	return FString::Printf(
		TEXT("%s_%s_%s_%s_%s_%u"),
		*Type,
		*Time,
		*Party,
		*Difficulty,
		*ResolvedFilter,
		FilterContextHash);
}

bool UT66BackendSubsystem::HasCachedMyRank(const FString& Key) const
{
	return MyRankCache.Contains(Key);
}

bool UT66BackendSubsystem::GetCachedMyRank(const FString& Key, bool& bOutSuccess, int32& OutRank, int32& OutTotalEntries) const
{
	if (const FCachedMyRank* Found = MyRankCache.Find(Key))
	{
		bOutSuccess = Found->bSuccess;
		OutRank = Found->Rank;
		OutTotalEntries = Found->TotalEntries;
		return true;
	}

	bOutSuccess = false;
	OutRank = 0;
	OutTotalEntries = 0;
	return false;
}

void UT66BackendSubsystem::OnMyRankResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RankKey)
{
	PendingMyRankFetches.Remove(RankKey);

	FCachedMyRank Cached;
	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		MyRankCache.Add(RankKey, Cached);
		OnMyRankComplete.Broadcast(false, 0, 0);
		OnMyRankDataReady.Broadcast(RankKey, false, 0, 0);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		MyRankCache.Add(RankKey, Cached);
		OnMyRankComplete.Broadcast(false, 0, 0);
		OnMyRankDataReady.Broadcast(RankKey, false, 0, 0);
		return;
	}

	int32 Rank = 0;
	if (Json->HasTypedField<EJson::Number>(TEXT("rank")))
	{
		Rank = static_cast<int32>(Json->GetNumberField(TEXT("rank")));
	}
	const int32 Total = static_cast<int32>(Json->GetNumberField(TEXT("total_entries")));

	Cached.bSuccess = true;
	Cached.Rank = Rank;
	Cached.TotalEntries = Total;
	MyRankCache.Add(RankKey, Cached);

	UE_LOG(LogT66Backend, Log, TEXT("Backend: my-rank = %d / %d"), Rank, Total);
	OnMyRankComplete.Broadcast(true, Rank, Total);
	OnMyRankDataReady.Broadcast(RankKey, true, Rank, Total);
}

// ── Account Status ───────────────────────────────────────────

void UT66BackendSubsystem::FetchLeaderboard(
	const FString& Type, const FString& Time, const FString& Party,
	const FString& Difficulty, const FString& Filter)
{
	const FString Key = FString::Printf(TEXT("%s_%s_%s_%s_%s"), *Type, *Time, *Party, *Difficulty, *Filter);

	if (!IsBackendConfigured())
	{
		if (TryPopulateDevelopmentDummyLeaderboard(Key))
		{
			OnLeaderboardDataReady.Broadcast(Key);
		}
		return;
	}

	// Don't fire duplicate requests for the same key
	if (PendingLeaderboardFetches.Contains(Key))
	{
		return;
	}
	PendingLeaderboardFetches.Add(Key);

	// Friends filter uses a separate authenticated endpoint with friend_ids
	if (Filter == TEXT("friends"))
	{
		if (!HasSteamTicket())
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: cannot fetch friends leaderboard — no Steam ticket."));
			PendingLeaderboardFetches.Remove(Key);
			if (TryPopulateDevelopmentDummyLeaderboard(Key))
			{
				OnLeaderboardDataReady.Broadcast(Key);
			}
			return;
		}

		// Collect friend IDs from SteamHelper
		FString FriendIds;
		if (UGameInstance* GI = Cast<UGameInstance>(GetOuter()))
		{
			if (UT66SteamHelper* Steam = GI->GetSubsystem<UT66SteamHelper>())
			{
				FriendIds = FString::Join(Steam->GetFriendSteamIds(), TEXT(","));
			}
		}

		// If no friend list from Steam, use the dev fallback (empty string still works — shows just self)
		const FString Endpoint = FString::Printf(
			TEXT("/api/leaderboard/friends?type=%s&time=%s&party=%s&difficulty=%s&friend_ids=%s"),
			*Type, *Time, *Party, *Difficulty, *FriendIds);

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
		SetAuthHeaders(Request);
		Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnLeaderboardResponseReceived, Key);
		Request->ProcessRequest();

		UE_LOG(LogT66Backend, Log, TEXT("Backend: fetching friends leaderboard key=%s (friends=%d)"),
			*Key, FriendIds.IsEmpty() ? 0 : FriendIds.Len());
	}
	else
	{
		// Global and Streamers use the public endpoint
		const FString Endpoint = FString::Printf(
			TEXT("/api/leaderboard?type=%s&time=%s&party=%s&difficulty=%s&filter=%s"),
			*Type, *Time, *Party, *Difficulty, *Filter);

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
		Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnLeaderboardResponseReceived, Key);
		Request->ProcessRequest();

		UE_LOG(LogT66Backend, Log, TEXT("Backend: fetching leaderboard key=%s"), *Key);
	}
}

bool UT66BackendSubsystem::HasCachedLeaderboard(const FString& Key) const
{
	return LeaderboardCache.Contains(Key);
}

TArray<FLeaderboardEntry> UT66BackendSubsystem::GetCachedLeaderboard(const FString& Key) const
{
	if (const FCachedLeaderboard* Found = LeaderboardCache.Find(Key))
	{
		return Found->Entries;
	}
	return {};
}

int32 UT66BackendSubsystem::GetCachedTotalEntries(const FString& Key) const
{
	if (const FCachedLeaderboard* Found = LeaderboardCache.Find(Key))
	{
		return Found->TotalEntries;
	}
	return 0;
}

ET66Difficulty UT66BackendSubsystem::ApiStringToDifficulty(const FString& S)
{
	if (S == TEXT("easy")) return ET66Difficulty::Easy;
	if (S == TEXT("medium")) return ET66Difficulty::Medium;
	if (S == TEXT("hard")) return ET66Difficulty::Hard;
	if (S == TEXT("veryhard")) return ET66Difficulty::VeryHard;
	if (S == TEXT("impossible")) return ET66Difficulty::Impossible;
	return ET66Difficulty::Easy;
}

ET66PartySize UT66BackendSubsystem::ApiStringToPartySize(const FString& S)
{
	if (S == TEXT("solo")) return ET66PartySize::Solo;
	if (S == TEXT("duo")) return ET66PartySize::Duo;
	if (S == TEXT("trio")) return ET66PartySize::Trio;
	if (S == TEXT("quad")) return ET66PartySize::Quad;
	return ET66PartySize::Solo;
}

void UT66BackendSubsystem::OnLeaderboardResponseReceived(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString LeaderboardKey)
{
	PendingLeaderboardFetches.Remove(LeaderboardKey);
	const bool bIsSpeedRunLeaderboard = LeaderboardKey.StartsWith(TEXT("speedrun_"));

	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: leaderboard fetch failed for key=%s (code=%d)"),
			*LeaderboardKey, Response.IsValid() ? Response->GetResponseCode() : 0);
		if (TryPopulateDevelopmentDummyLeaderboard(LeaderboardKey))
		{
			OnLeaderboardDataReady.Broadcast(LeaderboardKey);
		}
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: leaderboard JSON parse failed for key=%s"), *LeaderboardKey);
		if (TryPopulateDevelopmentDummyLeaderboard(LeaderboardKey))
		{
			OnLeaderboardDataReady.Broadcast(LeaderboardKey);
		}
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* EntriesArray = nullptr;
	if (!Json->TryGetArrayField(TEXT("entries"), EntriesArray) || !EntriesArray)
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: leaderboard missing entries array for key=%s"), *LeaderboardKey);
		if (TryPopulateDevelopmentDummyLeaderboard(LeaderboardKey))
		{
			OnLeaderboardDataReady.Broadcast(LeaderboardKey);
		}
		return;
	}

	FCachedLeaderboard Cached;
	Cached.TotalEntries = static_cast<int32>(Json->GetNumberField(TEXT("total_entries")));

	for (const TSharedPtr<FJsonValue>& Val : *EntriesArray)
	{
		const TSharedPtr<FJsonObject>* EntryObj = nullptr;
		if (!Val.IsValid() || !Val->TryGetObject(EntryObj) || !EntryObj || !(*EntryObj).IsValid())
		{
			continue;
		}

		const TSharedPtr<FJsonObject>& E = *EntryObj;

		FLeaderboardEntry Entry;
		Entry.Rank = static_cast<int32>(E->GetNumberField(TEXT("rank")));
		Entry.PlayerName = E->GetStringField(TEXT("display_name"));
		Entry.PlayerNames.Add(Entry.PlayerName);
		E->TryGetStringField(TEXT("steam_id"), Entry.SteamId);
		if (!Entry.SteamId.IsEmpty())
		{
			Entry.PlayerSteamIds.Add(Entry.SteamId);
		}

		// For co-op, add party member names
		const TArray<TSharedPtr<FJsonValue>>* PartyMembers = nullptr;
		if (E->TryGetArrayField(TEXT("party_members"), PartyMembers) && PartyMembers)
		{
			Entry.PlayerNames.Reset();
			Entry.PlayerSteamIds.Reset();
			for (const TSharedPtr<FJsonValue>& MVal : *PartyMembers)
			{
				const TSharedPtr<FJsonObject>* MObj = nullptr;
				if (MVal.IsValid() && MVal->TryGetObject(MObj) && MObj && (*MObj).IsValid())
				{
					FString MemberDisplayName;
					(*MObj)->TryGetStringField(TEXT("display_name"), MemberDisplayName);
					Entry.PlayerNames.Add(MemberDisplayName);

					FString MemberSteamId;
					(*MObj)->TryGetStringField(TEXT("steam_id"), MemberSteamId);
					Entry.PlayerSteamIds.Add(MemberSteamId);
				}
			}
			if (Entry.PlayerNames.Num() > 0)
			{
				Entry.PlayerName = Entry.PlayerNames[0];
			}
			if (Entry.PlayerSteamIds.Num() > 0 && !Entry.PlayerSteamIds[0].IsEmpty())
			{
				Entry.SteamId = Entry.PlayerSteamIds[0];
			}
		}

		Entry.Score = static_cast<int64>(E->GetNumberField(TEXT("score")));
		if (bIsSpeedRunLeaderboard)
		{
			Entry.TimeSeconds = static_cast<float>(Entry.Score) / 1000.0f;
		}
		Entry.StageReached = static_cast<int32>(E->GetNumberField(TEXT("stage_reached")));

		const FString HeroIdStr = E->GetStringField(TEXT("hero_id"));
		Entry.HeroID = HeroIdStr.IsEmpty() ? NAME_None : FName(*HeroIdStr);

		const FString PartySizeStr = E->GetStringField(TEXT("party_size"));
		Entry.PartySize = ApiStringToPartySize(PartySizeStr);

		Entry.bIsLocalPlayer = false;

		Entry.EntryId = E->GetStringField(TEXT("entry_id"));
		Entry.bHasRunSummary = E->HasField(TEXT("has_run_summary")) && E->GetBoolField(TEXT("has_run_summary"));

		FString AvUrl;
		if (E->TryGetStringField(TEXT("avatar_url"), AvUrl))
		{
			Entry.AvatarUrl = AvUrl;
		}

		Cached.Entries.Add(Entry);
	}

	if (Cached.Entries.Num() == 0 && TryPopulateDevelopmentDummyLeaderboard(LeaderboardKey))
	{
		OnLeaderboardDataReady.Broadcast(LeaderboardKey);
		return;
	}

	const int32 NumEntries = Cached.Entries.Num();
	const int32 TotalEntries = Cached.TotalEntries;
	LeaderboardCache.Add(LeaderboardKey, MoveTemp(Cached));

	UE_LOG(LogT66Backend, Log, TEXT("Backend: leaderboard fetched key=%s entries=%d total=%d"),
		*LeaderboardKey, NumEntries, TotalEntries);

	// Notify listeners
	OnLeaderboardDataReady.Broadcast(LeaderboardKey);
}

void UT66BackendSubsystem::OnMinigameDailyChallengeResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully,
	FString RequestKey)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		LastMinigameDailyChallengeStatus = TEXT("connection_error");
		LastMinigameDailyChallengeMessage = TEXT("Minigame daily challenge request failed.");
		OnMinigameDailyChallengeReady.Broadcast(RequestKey);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		LastMinigameDailyChallengeStatus = TEXT("parse_error");
		LastMinigameDailyChallengeMessage = TEXT("Minigame daily challenge response was invalid JSON.");
		OnMinigameDailyChallengeReady.Broadcast(RequestKey);
		return;
	}

	if (!Json->TryGetStringField(TEXT("status"), LastMinigameDailyChallengeStatus))
	{
		LastMinigameDailyChallengeStatus = TEXT("ok");
	}
	LastMinigameDailyChallengeMessage = ExtractResponseMessage(Json, LastMinigameDailyChallengeStatus);

	const TSharedPtr<FJsonObject>* ChallengeObj = nullptr;
	if (Json->TryGetObjectField(TEXT("challenge"), ChallengeObj) && ChallengeObj && (*ChallengeObj).IsValid())
	{
		FT66MinigameDailyChallengeData Parsed;
		(*ChallengeObj)->TryGetStringField(TEXT("challenge_id"), Parsed.ChallengeId);
		(*ChallengeObj)->TryGetStringField(TEXT("game_id"), Parsed.GameId);
		(*ChallengeObj)->TryGetStringField(TEXT("difficulty"), Parsed.Difficulty);
		(*ChallengeObj)->TryGetStringField(TEXT("challenge_date_utc"), Parsed.ChallengeDateUtc);
		(*ChallengeObj)->TryGetStringField(TEXT("starts_at"), Parsed.StartsAtIso);
		(*ChallengeObj)->TryGetStringField(TEXT("ends_at"), Parsed.EndsAtIso);
		(*ChallengeObj)->TryGetStringField(TEXT("title"), Parsed.Title);
		(*ChallengeObj)->TryGetStringField(TEXT("leaderboard_key"), Parsed.LeaderboardKey);

		double RunSeedValue = 0.0;
		if ((*ChallengeObj)->TryGetNumberField(TEXT("run_seed"), RunSeedValue))
		{
			Parsed.RunSeed = static_cast<int32>(RunSeedValue);
		}

		if (Parsed.IsValid())
		{
			MinigameDailyChallengeCache.Add(RequestKey, MoveTemp(Parsed));
		}
	}

	OnMinigameDailyChallengeReady.Broadcast(RequestKey);
}

void UT66BackendSubsystem::OnMinigameSubmitResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully,
	FString RequestKey,
	FString GameId,
	FString Scope,
	FString Difficulty)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		LastMinigameSubmitStatus = TEXT("connection_error");
		LastMinigameSubmitMessage = TEXT("Minigame score submit failed.");
		OnMinigameSubmitDataReady.Broadcast(RequestKey, false, LastMinigameSubmitStatus, 0);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		LastMinigameSubmitStatus = TEXT("parse_error");
		LastMinigameSubmitMessage = TEXT("Minigame score submit returned invalid JSON.");
		OnMinigameSubmitDataReady.Broadcast(RequestKey, false, LastMinigameSubmitStatus, 0);
		return;
	}

	if (!Json->TryGetStringField(TEXT("status"), LastMinigameSubmitStatus))
	{
		LastMinigameSubmitStatus = TEXT("unknown");
	}
	LastMinigameSubmitMessage = ExtractResponseMessage(Json, LastMinigameSubmitStatus);

	double RankValue = 0.0;
	(void)Json->TryGetNumberField(TEXT("rank"), RankValue);
	const int32 Rank = static_cast<int32>(RankValue);
	const bool bAccepted = LastMinigameSubmitStatus == TEXT("accepted");
	if (bAccepted)
	{
		FetchMinigameLeaderboard(GameId, Scope, Difficulty, TEXT("global"));
	}

	OnMinigameSubmitDataReady.Broadcast(RequestKey, bAccepted, LastMinigameSubmitStatus, Rank);
}

// ── Fetch Run Summary ────────────────────────────────────────

void UT66BackendSubsystem::FetchRunSummary(const FString& EntryId)
{
	if (!IsBackendConfigured() || EntryId.IsEmpty())
	{
		return;
	}

	if (PendingRunSummaryFetches.Contains(EntryId))
	{
		return;
	}
	PendingRunSummaryFetches.Add(EntryId);

	const FString Endpoint = FString::Printf(TEXT("/api/run-summary/%s"), *EntryId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnRunSummaryResponseReceived, EntryId);
	Request->ProcessRequest();

	UE_LOG(LogT66Backend, Log, TEXT("Backend: fetching run summary for entry=%s"), *EntryId);
}

bool UT66BackendSubsystem::HasCachedRunSummary(const FString& EntryId) const
{
	const TObjectPtr<UT66LeaderboardRunSummarySaveGame>* Found = RunSummaryCache.Find(EntryId);
	return Found && Found->Get() != nullptr;
}

UT66LeaderboardRunSummarySaveGame* UT66BackendSubsystem::GetCachedRunSummary(const FString& EntryId) const
{
	const TObjectPtr<UT66LeaderboardRunSummarySaveGame>* Found = RunSummaryCache.Find(EntryId);
	return (Found && Found->Get() != nullptr) ? Found->Get() : nullptr;
}

void UT66BackendSubsystem::OnRunSummaryResponseReceived(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString EntryId)
{
	PendingRunSummaryFetches.Remove(EntryId);

	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: run-summary fetch failed for entry=%s (code=%d)"),
			*EntryId, Response.IsValid() ? Response->GetResponseCode() : 0);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: run-summary JSON parse failed for entry=%s"), *EntryId);
		return;
	}

	UT66LeaderboardRunSummarySaveGame* Snapshot = ParseRunSummaryFromJson(Json, this);
	if (Snapshot)
	{
		RunSummaryCache.Add(EntryId, Snapshot);
		UE_LOG(LogT66Backend, Log, TEXT("Backend: run summary cached for entry=%s (hero=%s, score=%d)"),
			*EntryId, *Snapshot->HeroID.ToString(), Snapshot->Score);
		OnRunSummaryReady.Broadcast(EntryId);
	}
}

