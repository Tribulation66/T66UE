// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66BackendSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
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
	const FString& CompanionId,
	UT66LeaderboardRunSummarySaveGame* Snapshot)
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

	// Include full run summary data when available
	if (Snapshot)
	{
		RunObj->SetNumberField(TEXT("hero_level"), Snapshot->HeroLevel);

		// Primary stats
		TSharedPtr<FJsonObject> StatsObj = MakeShared<FJsonObject>();
		StatsObj->SetNumberField(TEXT("damage"), Snapshot->DamageStat);
		StatsObj->SetNumberField(TEXT("attack_speed"), Snapshot->AttackSpeedStat);
		StatsObj->SetNumberField(TEXT("attack_scale"), Snapshot->AttackScaleStat);
		StatsObj->SetNumberField(TEXT("armor"), Snapshot->ArmorStat);
		StatsObj->SetNumberField(TEXT("evasion"), Snapshot->EvasionStat);
		StatsObj->SetNumberField(TEXT("luck"), Snapshot->LuckStat);
		StatsObj->SetNumberField(TEXT("speed"), Snapshot->SpeedStat);
		RunObj->SetObjectField(TEXT("stats"), StatsObj);

		// Secondary stats
		TSharedPtr<FJsonObject> SecObj = MakeShared<FJsonObject>();
		for (const auto& Pair : Snapshot->SecondaryStatValues)
		{
			FString KeyName;
			switch (Pair.Key)
			{
			case ET66SecondaryStatType::AoeDamage: KeyName = TEXT("AoeDamage"); break;
			case ET66SecondaryStatType::BounceDamage: KeyName = TEXT("BounceDamage"); break;
			case ET66SecondaryStatType::PierceDamage: KeyName = TEXT("PierceDamage"); break;
			case ET66SecondaryStatType::DotDamage: KeyName = TEXT("DotDamage"); break;
			case ET66SecondaryStatType::AoeSpeed: KeyName = TEXT("AoeSpeed"); break;
			case ET66SecondaryStatType::BounceSpeed: KeyName = TEXT("BounceSpeed"); break;
			case ET66SecondaryStatType::PierceSpeed: KeyName = TEXT("PierceSpeed"); break;
			case ET66SecondaryStatType::DotSpeed: KeyName = TEXT("DotSpeed"); break;
			case ET66SecondaryStatType::AoeScale: KeyName = TEXT("AoeScale"); break;
			case ET66SecondaryStatType::BounceScale: KeyName = TEXT("BounceScale"); break;
			case ET66SecondaryStatType::PierceScale: KeyName = TEXT("PierceScale"); break;
			case ET66SecondaryStatType::DotScale: KeyName = TEXT("DotScale"); break;
			case ET66SecondaryStatType::CritDamage: KeyName = TEXT("CritDamage"); break;
			case ET66SecondaryStatType::CritChance: KeyName = TEXT("CritChance"); break;
			case ET66SecondaryStatType::CloseRangeDamage: KeyName = TEXT("CloseRangeDamage"); break;
			case ET66SecondaryStatType::LongRangeDamage: KeyName = TEXT("LongRangeDamage"); break;
			case ET66SecondaryStatType::AttackRange: KeyName = TEXT("AttackRange"); break;
			case ET66SecondaryStatType::Taunt: KeyName = TEXT("Taunt"); break;
			case ET66SecondaryStatType::ReflectDamage: KeyName = TEXT("ReflectDamage"); break;
			case ET66SecondaryStatType::HpRegen: KeyName = TEXT("HpRegen"); break;
			case ET66SecondaryStatType::Crush: KeyName = TEXT("Crush"); break;
			case ET66SecondaryStatType::Invisibility: KeyName = TEXT("Invisibility"); break;
			case ET66SecondaryStatType::CounterAttack: KeyName = TEXT("CounterAttack"); break;
			case ET66SecondaryStatType::LifeSteal: KeyName = TEXT("LifeSteal"); break;
			case ET66SecondaryStatType::Assassinate: KeyName = TEXT("Assassinate"); break;
			case ET66SecondaryStatType::SpinWheel: KeyName = TEXT("SpinWheel"); break;
			case ET66SecondaryStatType::Goblin: KeyName = TEXT("Goblin"); break;
			case ET66SecondaryStatType::Leprechaun: KeyName = TEXT("Leprechaun"); break;
			case ET66SecondaryStatType::TreasureChest: KeyName = TEXT("TreasureChest"); break;
			case ET66SecondaryStatType::Fountain: KeyName = TEXT("Fountain"); break;
			case ET66SecondaryStatType::Cheating: KeyName = TEXT("Cheating"); break;
			case ET66SecondaryStatType::Stealing: KeyName = TEXT("Stealing"); break;
			case ET66SecondaryStatType::MovementSpeed: KeyName = TEXT("MovementSpeed"); break;
			default: continue;
			}
			SecObj->SetNumberField(KeyName, Pair.Value);
		}
		RunObj->SetObjectField(TEXT("secondary_stats"), SecObj);

		// Ratings
		if (Snapshot->LuckRating0To100 >= 0) RunObj->SetNumberField(TEXT("luck_rating"), Snapshot->LuckRating0To100);
		if (Snapshot->LuckRatingQuantity0To100 >= 0) RunObj->SetNumberField(TEXT("luck_quantity"), Snapshot->LuckRatingQuantity0To100);
		if (Snapshot->LuckRatingQuality0To100 >= 0) RunObj->SetNumberField(TEXT("luck_quality"), Snapshot->LuckRatingQuality0To100);
		if (Snapshot->SkillRating0To100 >= 0) RunObj->SetNumberField(TEXT("skill_rating"), Snapshot->SkillRating0To100);

		// Equipped idols
		TArray<TSharedPtr<FJsonValue>> IdolArr;
		for (const FName& Idol : Snapshot->EquippedIdols)
		{
			IdolArr.Add(MakeShared<FJsonValueString>(Idol.ToString()));
		}
		RunObj->SetArrayField(TEXT("equipped_idols"), IdolArr);

		// Inventory
		TArray<TSharedPtr<FJsonValue>> InvArr;
		for (const FName& Item : Snapshot->Inventory)
		{
			InvArr.Add(MakeShared<FJsonValueString>(Item.ToString()));
		}
		RunObj->SetArrayField(TEXT("inventory"), InvArr);

		// Event log
		TArray<TSharedPtr<FJsonValue>> LogArr;
		for (const FString& Msg : Snapshot->EventLog)
		{
			LogArr.Add(MakeShared<FJsonValueString>(Msg));
		}
		RunObj->SetArrayField(TEXT("event_log"), LogArr);

		// Damage by source
		TSharedPtr<FJsonObject> DmgObj = MakeShared<FJsonObject>();
		for (const auto& Pair : Snapshot->DamageBySource)
		{
			DmgObj->SetNumberField(Pair.Key.ToString(), Pair.Value);
		}
		RunObj->SetObjectField(TEXT("damage_by_source"), DmgObj);
	}

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

	// Build the cache key (same format as backend: type_time_party_difficulty_filter)
	const FString Key = FString::Printf(TEXT("%s_%s_%s_%s_%s"), *Type, *Time, *Party, *Difficulty, *Filter);

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
			UE_LOG(LogTemp, Warning, TEXT("Backend: cannot fetch friends leaderboard — no Steam ticket."));
			PendingLeaderboardFetches.Remove(Key);
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

		UE_LOG(LogTemp, Log, TEXT("Backend: fetching friends leaderboard key=%s (friends=%d)"),
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

		UE_LOG(LogTemp, Log, TEXT("Backend: fetching leaderboard key=%s"), *Key);
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

		Entry.EntryId = E->GetStringField(TEXT("entry_id"));
		Entry.bHasRunSummary = E->HasField(TEXT("has_run_summary")) && E->GetBoolField(TEXT("has_run_summary"));

		Cached.Entries.Add(Entry);
	}

	const int32 NumEntries = Cached.Entries.Num();
	const int32 TotalEntries = Cached.TotalEntries;
	LeaderboardCache.Add(LeaderboardKey, MoveTemp(Cached));

	UE_LOG(LogTemp, Log, TEXT("Backend: leaderboard fetched key=%s entries=%d total=%d"),
		*LeaderboardKey, NumEntries, TotalEntries);

	// Notify listeners
	OnLeaderboardDataReady.Broadcast(LeaderboardKey);
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

	UE_LOG(LogTemp, Log, TEXT("Backend: fetching run summary for entry=%s"), *EntryId);
}

bool UT66BackendSubsystem::HasCachedRunSummary(const FString& EntryId) const
{
	return RunSummaryCache.Contains(EntryId);
}

UT66LeaderboardRunSummarySaveGame* UT66BackendSubsystem::GetCachedRunSummary(const FString& EntryId) const
{
	const TObjectPtr<UT66LeaderboardRunSummarySaveGame>* Found = RunSummaryCache.Find(EntryId);
	return Found ? Found->Get() : nullptr;
}

void UT66BackendSubsystem::OnRunSummaryResponseReceived(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString EntryId)
{
	PendingRunSummaryFetches.Remove(EntryId);

	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: run-summary fetch failed for entry=%s (code=%d)"),
			*EntryId, Response.IsValid() ? Response->GetResponseCode() : 0);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Backend: run-summary JSON parse failed for entry=%s"), *EntryId);
		return;
	}

	UT66LeaderboardRunSummarySaveGame* Snapshot = ParseRunSummaryFromJson(Json, this);
	if (Snapshot)
	{
		RunSummaryCache.Add(EntryId, Snapshot);
		UE_LOG(LogTemp, Log, TEXT("Backend: run summary cached for entry=%s (hero=%s, score=%d)"),
			*EntryId, *Snapshot->HeroID.ToString(), Snapshot->Score);
		OnRunSummaryReady.Broadcast(EntryId);
	}
}

UT66LeaderboardRunSummarySaveGame* UT66BackendSubsystem::ParseRunSummaryFromJson(
	const TSharedPtr<FJsonObject>& Json, UObject* Outer)
{
	if (!Json.IsValid())
	{
		return nullptr;
	}

	UT66LeaderboardRunSummarySaveGame* S = NewObject<UT66LeaderboardRunSummarySaveGame>(Outer);

	S->SchemaVersion = Json->HasField(TEXT("schema_version")) ? static_cast<int32>(Json->GetNumberField(TEXT("schema_version"))) : 6;
	S->StageReached = Json->HasField(TEXT("stage_reached")) ? static_cast<int32>(Json->GetNumberField(TEXT("stage_reached"))) : 1;
	S->Score = Json->HasField(TEXT("score")) ? static_cast<int32>(Json->GetNumberField(TEXT("score"))) : 0;

	const FString HeroIdStr = Json->GetStringField(TEXT("hero_id"));
	S->HeroID = HeroIdStr.IsEmpty() ? NAME_None : FName(*HeroIdStr);

	const FString CompanionIdStr = Json->GetStringField(TEXT("companion_id"));
	S->CompanionID = CompanionIdStr.IsEmpty() ? NAME_None : FName(*CompanionIdStr);

	S->HeroLevel = Json->HasField(TEXT("hero_level")) ? static_cast<int32>(Json->GetNumberField(TEXT("hero_level"))) : 1;

	S->DisplayName = Json->GetStringField(TEXT("display_name"));

	// Stats
	const TSharedPtr<FJsonObject>* StatsObj = nullptr;
	if (Json->TryGetObjectField(TEXT("stats"), StatsObj) && StatsObj && (*StatsObj).IsValid())
	{
		const TSharedPtr<FJsonObject>& St = *StatsObj;
		S->DamageStat = St->HasField(TEXT("damage")) ? static_cast<int32>(St->GetNumberField(TEXT("damage"))) : 1;
		S->AttackSpeedStat = St->HasField(TEXT("attack_speed")) ? static_cast<int32>(St->GetNumberField(TEXT("attack_speed"))) : 1;
		S->AttackScaleStat = St->HasField(TEXT("attack_scale")) ? static_cast<int32>(St->GetNumberField(TEXT("attack_scale"))) : 1;
		S->ArmorStat = St->HasField(TEXT("armor")) ? static_cast<int32>(St->GetNumberField(TEXT("armor"))) : 1;
		S->EvasionStat = St->HasField(TEXT("evasion")) ? static_cast<int32>(St->GetNumberField(TEXT("evasion"))) : 1;
		S->LuckStat = St->HasField(TEXT("luck")) ? static_cast<int32>(St->GetNumberField(TEXT("luck"))) : 1;
		S->SpeedStat = St->HasField(TEXT("speed")) ? static_cast<int32>(St->GetNumberField(TEXT("speed"))) : 1;
	}

	// Secondary stats
	const TSharedPtr<FJsonObject>* SecObj = nullptr;
	if (Json->TryGetObjectField(TEXT("secondary_stats"), SecObj) && SecObj && (*SecObj).IsValid())
	{
		for (const auto& Pair : (*SecObj)->Values)
		{
			double Val = 0.0;
			if (Pair.Value.IsValid() && Pair.Value->TryGetNumber(Val))
			{
				// Map JSON key name to ET66SecondaryStatType
				const FString& Key = Pair.Key;
				ET66SecondaryStatType StatType;
				bool bFound = true;

				if (Key == TEXT("CritChance")) StatType = ET66SecondaryStatType::CritChance;
				else if (Key == TEXT("CritDamage")) StatType = ET66SecondaryStatType::CritDamage;
				else if (Key == TEXT("Crush")) StatType = ET66SecondaryStatType::Crush;
				else if (Key == TEXT("Invisibility")) StatType = ET66SecondaryStatType::Invisibility;
				else if (Key == TEXT("LifeSteal")) StatType = ET66SecondaryStatType::LifeSteal;
				else if (Key == TEXT("Assassinate")) StatType = ET66SecondaryStatType::Assassinate;
				else if (Key == TEXT("Cheating")) StatType = ET66SecondaryStatType::Cheating;
				else if (Key == TEXT("Stealing")) StatType = ET66SecondaryStatType::Stealing;
				else if (Key == TEXT("AoeDamage")) StatType = ET66SecondaryStatType::AoeDamage;
				else if (Key == TEXT("BounceDamage")) StatType = ET66SecondaryStatType::BounceDamage;
				else if (Key == TEXT("PierceDamage")) StatType = ET66SecondaryStatType::PierceDamage;
				else if (Key == TEXT("DotDamage")) StatType = ET66SecondaryStatType::DotDamage;
				else if (Key == TEXT("CloseRangeDamage")) StatType = ET66SecondaryStatType::CloseRangeDamage;
				else if (Key == TEXT("LongRangeDamage")) StatType = ET66SecondaryStatType::LongRangeDamage;
				else if (Key == TEXT("AoeSpeed")) StatType = ET66SecondaryStatType::AoeSpeed;
				else if (Key == TEXT("BounceSpeed")) StatType = ET66SecondaryStatType::BounceSpeed;
				else if (Key == TEXT("PierceSpeed")) StatType = ET66SecondaryStatType::PierceSpeed;
				else if (Key == TEXT("DotSpeed")) StatType = ET66SecondaryStatType::DotSpeed;
				else if (Key == TEXT("AoeScale")) StatType = ET66SecondaryStatType::AoeScale;
				else if (Key == TEXT("BounceScale")) StatType = ET66SecondaryStatType::BounceScale;
				else if (Key == TEXT("PierceScale")) StatType = ET66SecondaryStatType::PierceScale;
				else if (Key == TEXT("DotScale")) StatType = ET66SecondaryStatType::DotScale;
				else if (Key == TEXT("AttackRange")) StatType = ET66SecondaryStatType::AttackRange;
				else if (Key == TEXT("Taunt")) StatType = ET66SecondaryStatType::Taunt;
				else if (Key == TEXT("ReflectDamage")) StatType = ET66SecondaryStatType::ReflectDamage;
				else if (Key == TEXT("CounterAttack")) StatType = ET66SecondaryStatType::CounterAttack;
				else if (Key == TEXT("HpRegen")) StatType = ET66SecondaryStatType::HpRegen;
				else if (Key == TEXT("SpinWheel")) StatType = ET66SecondaryStatType::SpinWheel;
				else if (Key == TEXT("Goblin")) StatType = ET66SecondaryStatType::Goblin;
				else if (Key == TEXT("Leprechaun")) StatType = ET66SecondaryStatType::Leprechaun;
				else if (Key == TEXT("TreasureChest")) StatType = ET66SecondaryStatType::TreasureChest;
				else if (Key == TEXT("Fountain")) StatType = ET66SecondaryStatType::Fountain;
				else if (Key == TEXT("MovementSpeed")) StatType = ET66SecondaryStatType::MovementSpeed;
				else { bFound = false; }

				if (bFound)
				{
					S->SecondaryStatValues.Add(StatType, static_cast<float>(Val));
				}
			}
		}
	}

	// Ratings
	S->LuckRating0To100 = Json->HasField(TEXT("luck_rating")) ? static_cast<int32>(Json->GetNumberField(TEXT("luck_rating"))) : -1;
	S->LuckRatingQuantity0To100 = Json->HasField(TEXT("luck_quantity")) ? static_cast<int32>(Json->GetNumberField(TEXT("luck_quantity"))) : -1;
	S->LuckRatingQuality0To100 = Json->HasField(TEXT("luck_quality")) ? static_cast<int32>(Json->GetNumberField(TEXT("luck_quality"))) : -1;
	S->SkillRating0To100 = Json->HasField(TEXT("skill_rating")) ? static_cast<int32>(Json->GetNumberField(TEXT("skill_rating"))) : -1;

	// Equipped idols
	const TArray<TSharedPtr<FJsonValue>>* IdolsArr = nullptr;
	if (Json->TryGetArrayField(TEXT("equipped_idols"), IdolsArr) && IdolsArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *IdolsArr)
		{
			FString IdolStr;
			if (V.IsValid() && V->TryGetString(IdolStr))
			{
				S->EquippedIdols.Add(FName(*IdolStr));
			}
		}
	}

	// Inventory
	const TArray<TSharedPtr<FJsonValue>>* InvArr = nullptr;
	if (Json->TryGetArrayField(TEXT("inventory"), InvArr) && InvArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *InvArr)
		{
			FString ItemStr;
			if (V.IsValid() && V->TryGetString(ItemStr))
			{
				S->Inventory.Add(FName(*ItemStr));
			}
		}
	}

	// Event log
	const TArray<TSharedPtr<FJsonValue>>* LogArr = nullptr;
	if (Json->TryGetArrayField(TEXT("event_log"), LogArr) && LogArr)
	{
		for (const TSharedPtr<FJsonValue>& V : *LogArr)
		{
			FString LogStr;
			if (V.IsValid() && V->TryGetString(LogStr))
			{
				S->EventLog.Add(LogStr);
			}
		}
	}

	// Damage by source
	const TSharedPtr<FJsonObject>* DmgObj = nullptr;
	if (Json->TryGetObjectField(TEXT("damage_by_source"), DmgObj) && DmgObj && (*DmgObj).IsValid())
	{
		for (const auto& Pair : (*DmgObj)->Values)
		{
			double Val = 0.0;
			if (Pair.Value.IsValid() && Pair.Value->TryGetNumber(Val))
			{
				S->DamageBySource.Add(FName(*Pair.Key), static_cast<int32>(Val));
			}
		}
	}

	// Proof of run
	S->ProofOfRunUrl = Json->GetStringField(TEXT("proof_of_run_url"));

	return S;
}
