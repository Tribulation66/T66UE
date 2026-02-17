// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66BackendSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/ConfigCacheIni.h"

void UT66BackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Read backend URL from DefaultGame.ini [T66.Online] BackendBaseUrl
	GConfig->GetString(TEXT("T66.Online"), TEXT("BackendBaseUrl"), BackendBaseUrl, GGameIni);

	if (BackendBaseUrl.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: BackendBaseUrl not configured in [T66.Online]. Online features disabled."));
	}
	else
	{
		// Strip trailing slash
		while (BackendBaseUrl.EndsWith(TEXT("/")))
		{
			BackendBaseUrl.LeftChopInline(1);
		}
		UE_LOG(LogTemp, Log, TEXT("Backend: configured URL = %s"), *BackendBaseUrl);
	}
}

void UT66BackendSubsystem::SetSteamTicketHex(const FString& TicketHex)
{
	CachedSteamTicketHex = TicketHex;
	UE_LOG(LogTemp, Log, TEXT("Backend: Steam ticket set (%d hex chars)"), TicketHex.Len());
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UT66BackendSubsystem::CreateRequest(const FString& Verb, const FString& Endpoint) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FString::Printf(TEXT("%s%s"), *BackendBaseUrl, *Endpoint));
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	return Request;
}

void UT66BackendSubsystem::SetAuthHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Request) const
{
	if (!CachedSteamTicketHex.IsEmpty())
	{
		Request->SetHeader(TEXT("X-Steam-Ticket"), CachedSteamTicketHex);
	}
}

FString UT66BackendSubsystem::DifficultyToApiString(ET66Difficulty Diff)
{
	switch (Diff)
	{
	case ET66Difficulty::Easy: return TEXT("easy");
	case ET66Difficulty::Medium: return TEXT("medium");
	case ET66Difficulty::Hard: return TEXT("hard");
	case ET66Difficulty::VeryHard: return TEXT("veryhard");
	case ET66Difficulty::Impossible: return TEXT("impossible");
	case ET66Difficulty::Perdition: return TEXT("perdition");
	case ET66Difficulty::Final: return TEXT("final");
	default: return TEXT("easy");
	}
}

FString UT66BackendSubsystem::PartySizeToApiString(ET66PartySize Party)
{
	switch (Party)
	{
	case ET66PartySize::Solo: return TEXT("solo");
	case ET66PartySize::Duo: return TEXT("duo");
	case ET66PartySize::Trio: return TEXT("trio");
	default: return TEXT("solo");
	}
}

// ── Submit Run ───────────────────────────────────────────────

void UT66BackendSubsystem::SubmitRunToBackend(
	const FString& DisplayName,
	int32 Score,
	int32 TimeMs,
	ET66Difficulty Difficulty,
	ET66PartySize PartySize,
	int32 StageReached,
	const FString& HeroId,
	const FString& CompanionId)
{
	if (!IsBackendConfigured())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: cannot submit run — no backend URL configured."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		return;
	}

	if (!HasSteamTicket())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: cannot submit run — no Steam ticket set."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		return;
	}

	// Build JSON payload matching /api/submit-run contract
	TSharedPtr<FJsonObject> RunObj = MakeShared<FJsonObject>();
	RunObj->SetStringField(TEXT("hero_id"), HeroId);
	RunObj->SetStringField(TEXT("companion_id"), CompanionId);
	RunObj->SetStringField(TEXT("difficulty"), DifficultyToApiString(Difficulty));
	RunObj->SetStringField(TEXT("party_size"), PartySizeToApiString(PartySize));
	RunObj->SetNumberField(TEXT("stage_reached"), StageReached);
	RunObj->SetNumberField(TEXT("score"), Score);
	RunObj->SetNumberField(TEXT("time_ms"), TimeMs);

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("display_name"), DisplayName);
	Root->SetObjectField(TEXT("run"), RunObj);

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/submit-run"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnSubmitRunResponseReceived);
	Request->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("Backend: submitting run (Score=%d, Difficulty=%s, Party=%s)"),
		Score, *DifficultyToApiString(Difficulty), *PartySizeToApiString(PartySize));
}

void UT66BackendSubsystem::OnSubmitRunResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: submit-run failed (connection error)."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		return;
	}

	const int32 Code = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: submit-run failed to parse JSON. Code=%d"), Code);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		return;
	}

	const FString Status = Json->GetStringField(TEXT("status"));
	if (Status == TEXT("accepted"))
	{
		const int32 ScoreRankAlltime = static_cast<int32>(Json->GetNumberField(TEXT("score_rank_alltime")));
		const int32 ScoreRankWeekly = static_cast<int32>(Json->GetNumberField(TEXT("score_rank_weekly")));
		const bool bNewPB = Json->GetBoolField(TEXT("is_new_personal_best_score"));

		UE_LOG(LogTemp, Log, TEXT("Backend: submit-run accepted. Alltime rank=%d, Weekly rank=%d, NewPB=%s"),
			ScoreRankAlltime, ScoreRankWeekly, bNewPB ? TEXT("true") : TEXT("false"));

		OnSubmitRunComplete.Broadcast(true, ScoreRankAlltime, ScoreRankWeekly, bNewPB);
	}
	else if (Status == TEXT("flagged") || Status == TEXT("banned"))
	{
		const FString Reason = Json->GetStringField(TEXT("reason"));
		UE_LOG(LogTemp, Warning, TEXT("Backend: submit-run %s — %s"), *Status, *Reason);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: submit-run unexpected status=%s, code=%d"), *Status, Code);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
	}
}

// ── My Rank ──────────────────────────────────────────────────

void UT66BackendSubsystem::FetchMyRank(
	const FString& Type,
	const FString& Time,
	const FString& Party,
	const FString& Difficulty)
{
	if (!IsBackendConfigured() || !HasSteamTicket())
	{
		OnMyRankComplete.Broadcast(false, 0, 0);
		return;
	}

	const FString Endpoint = FString::Printf(
		TEXT("/api/my-rank?type=%s&time=%s&party=%s&difficulty=%s"),
		*Type, *Time, *Party, *Difficulty);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	SetAuthHeaders(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnMyRankResponseReceived);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::OnMyRankResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		OnMyRankComplete.Broadcast(false, 0, 0);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		OnMyRankComplete.Broadcast(false, 0, 0);
		return;
	}

	int32 Rank = 0;
	if (Json->HasTypedField<EJson::Number>(TEXT("rank")))
	{
		Rank = static_cast<int32>(Json->GetNumberField(TEXT("rank")));
	}
	const int32 Total = static_cast<int32>(Json->GetNumberField(TEXT("total_entries")));

	UE_LOG(LogTemp, Log, TEXT("Backend: my-rank = %d / %d"), Rank, Total);
	OnMyRankComplete.Broadcast(true, Rank, Total);
}

// ── Account Status ───────────────────────────────────────────

void UT66BackendSubsystem::FetchAccountStatus()
{
	if (!IsBackendConfigured() || !HasSteamTicket())
	{
		OnAccountStatusComplete.Broadcast(false, TEXT(""));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), TEXT("/api/account-status"));
	SetAuthHeaders(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnAccountStatusResponseReceived);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::OnAccountStatusResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		OnAccountStatusComplete.Broadcast(false, TEXT(""));
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		OnAccountStatusComplete.Broadcast(false, TEXT(""));
		return;
	}

	const FString Restriction = Json->GetStringField(TEXT("restriction"));
	UE_LOG(LogTemp, Log, TEXT("Backend: account-status = %s"), *Restriction);
	OnAccountStatusComplete.Broadcast(true, Restriction);
}

// ── Health Check ─────────────────────────────────────────────

void UT66BackendSubsystem::PingHealth()
{
	if (!IsBackendConfigured())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: health ping skipped — no URL configured."));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), TEXT("/api/health"));
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnHealthResponseReceived);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::OnHealthResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: health ping FAILED (connection error)."));
		return;
	}

	const int32 Code = Response->GetResponseCode();
	UE_LOG(LogTemp, Log, TEXT("Backend: health ping response code=%d body=%s"),
		Code, *Response->GetContentAsString().Left(200));
}

// ── Fetch Leaderboard ────────────────────────────────────────

void UT66BackendSubsystem::FetchLeaderboard(
	const FString& Type, const FString& Time, const FString& Party,
	const FString& Difficulty, const FString& Filter)
{
	if (!IsBackendConfigured())
	{
		return;
	}

	// Build the cache key (same format as backend: type_time_party_difficulty)
	const FString Key = FString::Printf(TEXT("%s_%s_%s_%s_%s"), *Type, *Time, *Party, *Difficulty, *Filter);

	// Don't fire duplicate requests for the same key
	if (PendingLeaderboardFetches.Contains(Key))
	{
		return;
	}
	PendingLeaderboardFetches.Add(Key);

	const FString Endpoint = FString::Printf(
		TEXT("/api/leaderboard?type=%s&time=%s&party=%s&difficulty=%s&filter=%s"),
		*Type, *Time, *Party, *Difficulty, *Filter);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	// Leaderboard endpoint is public, no auth needed
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnLeaderboardResponseReceived, Key);
	Request->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("Backend: fetching leaderboard key=%s"), *Key);
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
	if (S == TEXT("perdition")) return ET66Difficulty::Perdition;
	if (S == TEXT("final")) return ET66Difficulty::Final;
	return ET66Difficulty::Easy;
}

ET66PartySize UT66BackendSubsystem::ApiStringToPartySize(const FString& S)
{
	if (S == TEXT("solo")) return ET66PartySize::Solo;
	if (S == TEXT("duo")) return ET66PartySize::Duo;
	if (S == TEXT("trio")) return ET66PartySize::Trio;
	return ET66PartySize::Solo;
}

void UT66BackendSubsystem::OnLeaderboardResponseReceived(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString LeaderboardKey)
{
	PendingLeaderboardFetches.Remove(LeaderboardKey);

	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: leaderboard fetch failed for key=%s (code=%d)"),
			*LeaderboardKey, Response.IsValid() ? Response->GetResponseCode() : 0);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: leaderboard JSON parse failed for key=%s"), *LeaderboardKey);
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* EntriesArray = nullptr;
	if (!Json->TryGetArrayField(TEXT("entries"), EntriesArray) || !EntriesArray)
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: leaderboard missing entries array for key=%s"), *LeaderboardKey);
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

		// For co-op, add party member names
		const TArray<TSharedPtr<FJsonValue>>* PartyMembers = nullptr;
		if (E->TryGetArrayField(TEXT("party_members"), PartyMembers) && PartyMembers)
		{
			Entry.PlayerNames.Reset();
			for (const TSharedPtr<FJsonValue>& MVal : *PartyMembers)
			{
				const TSharedPtr<FJsonObject>* MObj = nullptr;
				if (MVal.IsValid() && MVal->TryGetObject(MObj) && MObj && (*MObj).IsValid())
				{
					Entry.PlayerNames.Add((*MObj)->GetStringField(TEXT("display_name")));
				}
			}
			if (Entry.PlayerNames.Num() > 0)
			{
				Entry.PlayerName = Entry.PlayerNames[0];
			}
		}

		Entry.Score = static_cast<int64>(E->GetNumberField(TEXT("score")));
		Entry.StageReached = static_cast<int32>(E->GetNumberField(TEXT("stage_reached")));

		const FString HeroIdStr = E->GetStringField(TEXT("hero_id"));
		Entry.HeroID = HeroIdStr.IsEmpty() ? NAME_None : FName(*HeroIdStr);

		const FString PartySizeStr = E->GetStringField(TEXT("party_size"));
		Entry.PartySize = ApiStringToPartySize(PartySizeStr);

		Entry.bIsLocalPlayer = false;

		Cached.Entries.Add(Entry);
	}

	LeaderboardCache.Add(LeaderboardKey, MoveTemp(Cached));

	UE_LOG(LogTemp, Log, TEXT("Backend: leaderboard fetched key=%s entries=%d total=%d"),
		*LeaderboardKey, Cached.Entries.Num(), Cached.TotalEntries);

	// Notify listeners
	OnLeaderboardDataReady.Broadcast(LeaderboardKey);
}
