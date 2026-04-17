// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66BackendSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Backend, Log, All);

static TAutoConsoleVariable<float> CVarT66PartyInvitePollMinIntervalSeconds(
	TEXT("T66.Backend.PartyInvitePollMinIntervalSeconds"),
	0.15f,
	TEXT("Minimum interval between non-forced party invite polls."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66PartyInvitePollTickerIntervalSeconds(
	TEXT("T66.Backend.PartyInvitePollTickerIntervalSeconds"),
	0.15f,
	TEXT("Ticker cadence used to drive invite polling while the backend subsystem is alive."),
	ECVF_Default);

namespace
{
	struct FT66DummyLeaderboardIdentity
	{
		FString Type;
		FString Time;
		FString Party;
		FString Difficulty;
		FString Filter;
	};

	void T66SetJsonStringIfNotEmpty(const TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FString& Value)
	{
		if (Json.IsValid() && !Value.IsEmpty())
		{
			Json->SetStringField(FieldName, Value);
		}
	}

	bool T66ArePartyInviteArraysEqual(const TArray<FT66PartyInviteEntry>& A, const TArray<FT66PartyInviteEntry>& B)
	{
		if (A.Num() != B.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < A.Num(); ++Index)
		{
			if (A[Index].InviteId != B[Index].InviteId
				|| A[Index].HostSteamId != B[Index].HostSteamId
				|| A[Index].HostDisplayName != B[Index].HostDisplayName
				|| A[Index].HostAvatarUrl != B[Index].HostAvatarUrl
				|| A[Index].HostLobbyId != B[Index].HostLobbyId
				|| A[Index].HostAppId != B[Index].HostAppId
				|| A[Index].TargetSteamId != B[Index].TargetSteamId
				|| A[Index].CreatedAtIso != B[Index].CreatedAtIso
				|| A[Index].ExpiresAtIso != B[Index].ExpiresAtIso)
			{
				return false;
			}
		}

		return true;
	}

	TArray<FT66PartyInviteEntry> T66ParsePendingPartyInvites(const TSharedPtr<FJsonObject>& Json)
	{
		TArray<FT66PartyInviteEntry> Result;
		if (!Json.IsValid())
		{
			return Result;
		}

		const TArray<TSharedPtr<FJsonValue>>* InvitesArray = nullptr;
		if (!Json->TryGetArrayField(TEXT("invites"), InvitesArray) || !InvitesArray)
		{
			return Result;
		}

		for (const TSharedPtr<FJsonValue>& InviteValue : *InvitesArray)
		{
			const TSharedPtr<FJsonObject>* InviteObject = nullptr;
			if (!InviteValue.IsValid() || !InviteValue->TryGetObject(InviteObject) || !InviteObject || !InviteObject->IsValid())
			{
				continue;
			}

			FT66PartyInviteEntry& Invite = Result.AddDefaulted_GetRef();
			(*InviteObject)->TryGetStringField(TEXT("invite_id"), Invite.InviteId);
			(*InviteObject)->TryGetStringField(TEXT("host_steam_id"), Invite.HostSteamId);
			(*InviteObject)->TryGetStringField(TEXT("host_display_name"), Invite.HostDisplayName);
			(*InviteObject)->TryGetStringField(TEXT("host_avatar_url"), Invite.HostAvatarUrl);
			(*InviteObject)->TryGetStringField(TEXT("host_lobby_id"), Invite.HostLobbyId);
			(*InviteObject)->TryGetStringField(TEXT("host_app_id"), Invite.HostAppId);
			(*InviteObject)->TryGetStringField(TEXT("target_steam_id"), Invite.TargetSteamId);
			(*InviteObject)->TryGetStringField(TEXT("created_at"), Invite.CreatedAtIso);
			(*InviteObject)->TryGetStringField(TEXT("expires_at"), Invite.ExpiresAtIso);

			if (Invite.InviteId.IsEmpty() || Invite.HostSteamId.IsEmpty())
			{
				Result.Pop();
			}
		}

		return Result;
	}

	bool T66TryParseDummyLeaderboardKey(const FString& Key, FT66DummyLeaderboardIdentity& OutIdentity)
	{
		TArray<FString> Tokens;
		Key.ParseIntoArray(Tokens, TEXT("_"));
		if (Tokens.Num() != 5)
		{
			return false;
		}

		OutIdentity.Type = Tokens[0].ToLower();
		OutIdentity.Time = Tokens[1].ToLower();
		OutIdentity.Party = Tokens[2].ToLower();
		OutIdentity.Difficulty = Tokens[3].ToLower();
		OutIdentity.Filter = Tokens[4].ToLower();

		const bool bValidType = OutIdentity.Type == TEXT("score") || OutIdentity.Type == TEXT("speedrun");
		const bool bValidTime = OutIdentity.Time == TEXT("weekly") || OutIdentity.Time == TEXT("alltime");
		const bool bValidFilter = OutIdentity.Filter == TEXT("global") || OutIdentity.Filter == TEXT("friends") || OutIdentity.Filter == TEXT("streamers");

		return bValidType
			&& bValidTime
			&& bValidFilter
			&& OutIdentity.Party == TEXT("solo")
			&& OutIdentity.Difficulty == TEXT("easy");
	}

	bool T66ShouldUseDevelopmentDummyLeaderboards()
	{
#if UE_BUILD_SHIPPING
		return false;
#else
		static const bool bUseDummyLeaderboards = []()
		{
			bool bEnabledFromConfig = false;
			if (GConfig)
			{
				GConfig->GetBool(
					TEXT("T66.Online"),
					TEXT("bEnableDevelopmentDummyLeaderboards"),
					bEnabledFromConfig,
					GGameIni);
			}

			return GIsEditor
				|| bEnabledFromConfig
				|| FParse::Param(FCommandLine::Get(), TEXT("T66EnableDummyLeaderboards"));
		}();

		return bUseDummyLeaderboards;
#endif
	}

	FString T66GetDummyPlayerName(const FString& Filter, const int32 Index)
	{
		static const TArray<FString> GlobalNames = {
			TEXT("Ash Vale"), TEXT("Rune Kestrel"), TEXT("Iris Flint"), TEXT("Morrow Pike"), TEXT("Sable Crest")
		};
		static const TArray<FString> FriendNames = {
			TEXT("Niko Vale"), TEXT("Mina Torch"), TEXT("Oren Bloom"), TEXT("Pax Hollow"), TEXT("Rhea Vale")
		};
		static const TArray<FString> StreamerNames = {
			TEXT("Clip Monarch"), TEXT("Split Saint"), TEXT("Nova Route"), TEXT("Tempo Hex"), TEXT("Frame Witch")
		};

		const TArray<FString>* Pool = &GlobalNames;
		if (Filter == TEXT("friends"))
		{
			Pool = &FriendNames;
		}
		else if (Filter == TEXT("streamers"))
		{
			Pool = &StreamerNames;
		}

		return (*Pool)[Index % Pool->Num()];
	}

	int32 T66GetDummyFilterIndex(const FString& Filter)
	{
		if (Filter == TEXT("friends"))
		{
			return 1;
		}
		if (Filter == TEXT("streamers"))
		{
			return 2;
		}
		return 0;
	}

	bool T66SerializeJsonObjectToString(const TSharedPtr<FJsonObject>& Json, FString& OutJsonString)
	{
		if (!Json.IsValid())
		{
			return false;
		}

		OutJsonString.Reset();
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonString);
		return FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);
	}

	bool T66DeserializeJsonObjectString(const FString& JsonString, TSharedPtr<FJsonObject>& OutJson)
	{
		OutJson.Reset();
		if (JsonString.IsEmpty())
		{
			return false;
		}

		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		return FJsonSerializer::Deserialize(Reader, OutJson) && OutJson.IsValid();
	}

	FName T66GetDummyHeroId(const int32 Index)
	{
		static const FName HeroIds[] = {
			FName(TEXT("Hero_1")),
			FName(TEXT("Hero_2")),
			FName(TEXT("Hero_3")),
			FName(TEXT("Hero_4")),
			FName(TEXT("Hero_5"))
		};

		return HeroIds[Index % UE_ARRAY_COUNT(HeroIds)];
	}

	TObjectPtr<UT66LeaderboardRunSummarySaveGame> T66BuildDummyRunSummary(
		UObject* Outer,
		const FT66DummyLeaderboardIdentity& Identity,
		const FLeaderboardEntry& Entry,
		const FString& DisplayName)
	{
		UT66LeaderboardRunSummarySaveGame* Summary = NewObject<UT66LeaderboardRunSummarySaveGame>(Outer);
		Summary->SchemaVersion = 8;
		Summary->EntryId = Entry.EntryId;
		Summary->OwnerSteamId = FString::Printf(TEXT("dummy_%s_%02d"), *Identity.Filter, Entry.Rank);
		Summary->LeaderboardType = (Identity.Type == TEXT("speedrun")) ? ET66LeaderboardType::SpeedRun : ET66LeaderboardType::Score;
		Summary->Difficulty = ET66Difficulty::Easy;
		Summary->PartySize = ET66PartySize::Solo;
		Summary->SavedAtUtc = FDateTime::UtcNow();
		Summary->RunEndedAtUtc = Summary->SavedAtUtc;
		Summary->RunDurationSeconds = Entry.TimeSeconds;
		Summary->bWasFullClear = true;
		Summary->bWasSpeedRunMode = (Summary->LeaderboardType == ET66LeaderboardType::SpeedRun);
		Summary->StageReached = 5;
		Summary->Score = static_cast<int32>(Entry.Score);
		Summary->HeroID = Entry.HeroID;
		Summary->CompanionID = NAME_None;
		Summary->HeroLevel = 18 - Entry.Rank;
		Summary->DamageStat = 6;
		Summary->AttackSpeedStat = 5;
		Summary->AttackScaleStat = 4;
		Summary->AccuracyStat = 4;
		Summary->ArmorStat = 3;
		Summary->EvasionStat = 4;
		Summary->LuckStat = 5;
		Summary->SpeedStat = 6;
		Summary->LuckRating0To100 = 72 - Entry.Rank;
		Summary->LuckRatingQuantity0To100 = 68 - Entry.Rank;
		Summary->LuckRatingQuality0To100 = 74 - Entry.Rank;
		Summary->SkillRating0To100 = 80 - (Entry.Rank * 2);
		Summary->EquippedIdols = { FName(TEXT("Idol_Luck")), FName(TEXT("Idol_Speed")), FName(TEXT("Idol_Damage")) };
		Summary->Inventory = { FName(TEXT("Item_GoldTooth")), FName(TEXT("Item_TravelerBoots")), FName(TEXT("Item_LuckyCoin")) };
		Summary->DamageBySource.Add(FName(TEXT("PrimaryAttack")), FMath::Max(2500, Summary->Score / 2));
		Summary->DamageBySource.Add(FName(TEXT("Ultimate")), FMath::Max(600, Summary->Score / 6));
		Summary->DisplayName = DisplayName;

		const TArray<float> TimeFractions = { 0.18f, 0.38f, 0.59f, 0.79f, 1.0f };
		const TArray<float> ScoreFractions = { 0.12f, 0.29f, 0.49f, 0.71f, 1.0f };
		for (int32 StageIndex = 0; StageIndex < 5; ++StageIndex)
		{
			const int32 StageNumber = StageIndex + 1;
			const float ElapsedSeconds = FMath::RoundToFloat(Entry.TimeSeconds * TimeFractions[StageIndex] * 100.0f) / 100.0f;
			const int32 StageScore = FMath::RoundToInt(static_cast<float>(Entry.Score) * ScoreFractions[StageIndex]);

			FT66StagePacingPoint& Point = Summary->StagePacingPoints.AddDefaulted_GetRef();
			Point.Stage = StageNumber;
			Point.Score = StageScore;
			Point.ElapsedSeconds = ElapsedSeconds;

			Summary->EventLog.Add(FString::Printf(
				TEXT("Stage %d cleared with %d score at %.2fs"),
				StageNumber,
				StageScore,
				ElapsedSeconds));
			Summary->EventLog.Add(T66LeaderboardPacing::MakeStageMarker(StageNumber, StageScore, ElapsedSeconds));
		}

		Summary->EventLog.Add(FString::Printf(TEXT("%s finished the dummy Easy solo run."), *DisplayName));
		return Summary;
	}
}

void UT66BackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Read backend URL from DefaultGame.ini [T66.Online] BackendBaseUrl
	GConfig->GetString(TEXT("T66.Online"), TEXT("BackendBaseUrl"), BackendBaseUrl, GGameIni);

	if (BackendBaseUrl.IsEmpty())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: BackendBaseUrl not configured in [T66.Online]. Online features disabled."));
	}
	else
	{
		// Strip trailing slash
		while (BackendBaseUrl.EndsWith(TEXT("/")))
		{
			BackendBaseUrl.LeftChopInline(1);
		}
		UE_LOG(LogT66Backend, Log, TEXT("Backend: configured URL = %s"), *BackendBaseUrl);
	}

	SeedDevelopmentDummyLeaderboardsIfNeeded();

	PartyInvitePollTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UT66BackendSubsystem::HandlePartyInvitePollTicker),
		FMath::Max(0.05f, CVarT66PartyInvitePollTickerIntervalSeconds.GetValueOnGameThread()));
}

void UT66BackendSubsystem::Deinitialize()
{
	if (PartyInvitePollTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PartyInvitePollTickerHandle);
		PartyInvitePollTickerHandle.Reset();
	}

	if (PendingCoopSubmitTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PendingCoopSubmitTickerHandle);
		PendingCoopSubmitTickerHandle.Reset();
	}

	if (TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> PendingInviteRequest = PendingPartyInvitePollRequest.Pin())
	{
		PendingInviteRequest->OnProcessRequestComplete().Unbind();
		PendingInviteRequest->CancelRequest();
		PendingPartyInvitePollRequest.Reset();
	}

	bPartyInvitePollInFlight = false;
	bPartyInvitePollRequestedWhileInFlight = false;
	PendingPartyInvites.Reset();
	PendingCoopSubmitRequests.Reset();
	PendingPartyInvitesChanged.Clear();
	PartyInviteActionComplete.Clear();

	Super::Deinitialize();
}

void UT66BackendSubsystem::SetSteamTicketHex(const FString& TicketHex)
{
	CachedSteamTicketHex = TicketHex;
	UE_LOG(LogT66Backend, Log, TEXT("Backend: Steam ticket set (%d hex chars)"), TicketHex.Len());
}

bool UT66BackendSubsystem::SendPartyInvite(const FString& TargetSteamId, const FString& TargetDisplayName)
{
	if (!IsBackendConfigured() || !HasSteamTicket() || TargetSteamId.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("target_steam_id"), TargetSteamId);
	if (!TargetDisplayName.IsEmpty())
	{
		Root->SetStringField(TEXT("target_display_name"), TargetDisplayName);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			const FString HostLobbyId = SessionSubsystem->GetCurrentPartyLobbyId();
			if (!HostLobbyId.IsEmpty())
			{
				Root->SetStringField(TEXT("host_lobby_id"), HostLobbyId);
			}
			else
			{
				UE_LOG(LogT66Backend, Warning, TEXT("Backend: sending party invite without host lobby id for %s."), *TargetSteamId);
			}
		}

		if (UT66SteamHelper* SteamHelper = GI->GetSubsystem<UT66SteamHelper>())
		{
			const FString HostAppId = SteamHelper->GetActiveSteamAppId();
			if (!HostAppId.IsEmpty())
			{
				Root->SetStringField(TEXT("host_app_id"), HostAppId);
			}

			const FString HostDisplayName = SteamHelper->GetLocalDisplayName();
			if (!HostDisplayName.IsEmpty())
			{
				Root->SetStringField(TEXT("host_display_name"), HostDisplayName);
			}
		}
	}

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/party-invite/send"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnSendPartyInviteResponseReceived);
	Request->ProcessRequest();

	UE_LOG(LogT66Backend, Log, TEXT("Backend: sending party invite to %s"), *TargetSteamId);
	return true;
}

void UT66BackendSubsystem::PollPendingPartyInvites(bool bForce)
{
	FLagScopedScope LagScope(GetWorld(), bForce ? TEXT("MP-02 Backend::PollPendingPartyInvites[force]") : TEXT("MP-02 Backend::PollPendingPartyInvites"));

	if (!IsBackendConfigured() || !HasSteamTicket())
	{
		bPartyInvitePollRequestedWhileInFlight = false;
		SetPendingPartyInvites({});
		return;
	}

	if (bPartyInvitePollInFlight)
	{
		if (bForce)
		{
			bPartyInvitePollRequestedWhileInFlight = true;
		}
		return;
	}

	const double NowSeconds = FPlatformTime::Seconds();
	const double MinPollIntervalSeconds = static_cast<double>(FMath::Max(0.05f, CVarT66PartyInvitePollMinIntervalSeconds.GetValueOnGameThread()));
	if (!bForce && (NowSeconds - LastPartyInvitePollTimeSeconds) < MinPollIntervalSeconds)
	{
		return;
	}

	LastPartyInvitePollTimeSeconds = NowSeconds;
	bPartyInvitePollInFlight = true;
	bPartyInvitePollRequestedWhileInFlight = false;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), TEXT("/api/party-invite/pending"));
	PendingPartyInvitePollRequest = Request;
	SetAuthHeaders(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnPendingPartyInvitesResponseReceived);
	Request->ProcessRequest();
}

bool UT66BackendSubsystem::RespondToPartyInvite(const FString& InviteId, bool bAccept)
{
	FLagScopedScope LagScope(GetWorld(), bAccept ? TEXT("MP-03 Backend::RespondToPartyInvite[accept]") : TEXT("MP-03 Backend::RespondToPartyInvite[reject]"));

	if (!IsBackendConfigured() || !HasSteamTicket() || InviteId.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("invite_id"), InviteId);
	Root->SetStringField(TEXT("action"), bAccept ? TEXT("accept") : TEXT("reject"));

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	const FString Action = bAccept ? TEXT("accept") : TEXT("reject");
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/party-invite/respond"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnRespondPartyInviteResponseReceived, InviteId, Action);
	Request->ProcessRequest();

	UE_LOG(LogT66Backend, Log, TEXT("Backend: responding to party invite %s with %s"), *InviteId, *Action);
	return true;
}

void UT66BackendSubsystem::SeedDevelopmentDummyLeaderboardsIfNeeded()
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (!T66ShouldUseDevelopmentDummyLeaderboards())
	{
		return;
	}

	if (IsBackendConfigured())
	{
		return;
	}

	static const TCHAR* const KeysToSeed[] = {
		TEXT("score_weekly_solo_easy_global"),
		TEXT("score_alltime_solo_easy_global"),
		TEXT("score_weekly_solo_easy_friends"),
		TEXT("score_alltime_solo_easy_friends"),
		TEXT("score_weekly_solo_easy_streamers"),
		TEXT("score_alltime_solo_easy_streamers"),
		TEXT("speedrun_weekly_solo_easy_global"),
		TEXT("speedrun_alltime_solo_easy_global"),
		TEXT("speedrun_weekly_solo_easy_friends"),
		TEXT("speedrun_alltime_solo_easy_friends"),
		TEXT("speedrun_weekly_solo_easy_streamers"),
		TEXT("speedrun_alltime_solo_easy_streamers")
	};

	for (const TCHAR* Key : KeysToSeed)
	{
		TryPopulateDevelopmentDummyLeaderboard(Key);
	}
#endif
}

bool UT66BackendSubsystem::TryPopulateDevelopmentDummyLeaderboard(const FString& Key)
{
#if UE_BUILD_SHIPPING
	return false;
#else
	if (!T66ShouldUseDevelopmentDummyLeaderboards())
	{
		return false;
	}

	if (const FCachedLeaderboard* Existing = LeaderboardCache.Find(Key))
	{
		if (Existing->Entries.Num() > 0)
		{
			return true;
		}
	}

	FT66DummyLeaderboardIdentity Identity;
	if (!T66TryParseDummyLeaderboardKey(Key, Identity))
	{
		return false;
	}

	const bool bScoreBoard = Identity.Type == TEXT("score");
	const bool bAllTime = Identity.Time == TEXT("alltime");
	const int32 FilterIndex = T66GetDummyFilterIndex(Identity.Filter);
	const int32 BaseScore = bScoreBoard ? (bAllTime ? 9400 : 8200) : (bAllTime ? 7600 : 7000);
	const float BaseTime = bScoreBoard ? (bAllTime ? 244.0f : 258.0f) : (bAllTime ? 231.0f : 239.0f);

	FCachedLeaderboard Cached;
	Cached.TotalEntries = 5;

	for (int32 EntryIndex = 0; EntryIndex < 5; ++EntryIndex)
	{
		const int32 Rank = EntryIndex + 1;
		const int32 FilterScoreModifier = (FilterIndex == 0) ? 0 : (FilterIndex == 1 ? -220 : -360);
		const float FilterTimeModifier = (FilterIndex == 0) ? 0.f : (FilterIndex == 1 ? 6.5f : 11.0f);

		FLeaderboardEntry& Entry = Cached.Entries.AddDefaulted_GetRef();
		Entry.Rank = Rank;
		Entry.PlayerName = T66GetDummyPlayerName(Identity.Filter, EntryIndex);
		Entry.PlayerNames = { Entry.PlayerName };
		Entry.Score = FMath::Max<int64>(3500, BaseScore + FilterScoreModifier - (EntryIndex * 420));
		Entry.TimeSeconds = FMath::Max(60.0f, BaseTime + FilterTimeModifier + (EntryIndex * 8.75f));
		Entry.HeroID = T66GetDummyHeroId(EntryIndex + FilterIndex);
		Entry.PartySize = ET66PartySize::Solo;
		Entry.Difficulty = ET66Difficulty::Easy;
		Entry.StageReached = 5;
		Entry.bIsLocalPlayer = false;
		Entry.EntryId = FString::Printf(
			TEXT("dummy_%s_%s_%s_r%d"),
			*Identity.Type,
			*Identity.Time,
			*Identity.Filter,
			Rank);
		Entry.bHasRunSummary = true;
		Entry.AvatarUrl.Reset();

		RunSummaryCache.FindOrAdd(Entry.EntryId) = T66BuildDummyRunSummary(this, Identity, Entry, Entry.PlayerName);
	}

	LeaderboardCache.Add(Key, MoveTemp(Cached));
	UE_LOG(LogT66Backend, Log, TEXT("Backend: seeded dummy leaderboard for key=%s"), *Key);
	return true;
#endif
}

bool UT66BackendSubsystem::HandlePartyInvitePollTicker(float DeltaTime)
{
	PollPendingPartyInvites(false);
	return true;
}

void UT66BackendSubsystem::SetPendingPartyInvites(TArray<FT66PartyInviteEntry>&& NewInvites)
{
	if (T66ArePartyInviteArraysEqual(PendingPartyInvites, NewInvites))
	{
		return;
	}

	PendingPartyInvites = MoveTemp(NewInvites);
	PendingPartyInvitesChanged.Broadcast();
}

TSharedPtr<FJsonObject> UT66BackendSubsystem::BuildMultiplayerDiagnosticJson(const FT66MultiplayerDiagnosticContext& Diagnostic) const
{
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("category"), TEXT("multiplayer"));
	Root->SetStringField(TEXT("event"), Diagnostic.EventName.IsEmpty() ? TEXT("unknown") : Diagnostic.EventName);
	Root->SetStringField(TEXT("severity"), Diagnostic.Severity.IsEmpty() ? TEXT("info") : Diagnostic.Severity);
	Root->SetStringField(TEXT("timestamp_utc"), FDateTime::UtcNow().ToIso8601());

	FString BuildVersion = FApp::GetBuildVersion();
	if (BuildVersion.IsEmpty())
	{
		GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), BuildVersion, GGameIni);
	}

	const UWorld* World = GetWorld();
	const FString MapName = Diagnostic.MapName.IsEmpty() && World
		? UWorld::RemovePIEPrefix(World->GetMapName())
		: Diagnostic.MapName;

	UT66SteamHelper* SteamHelper = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SteamHelper>() : nullptr;
	const FString ActiveAppId = SteamHelper ? SteamHelper->GetActiveSteamAppId() : FString();
	const FString BetaName = SteamHelper ? SteamHelper->GetCurrentSteamBetaName() : FString();
	const FString LocalSteamId = SteamHelper ? SteamHelper->GetLocalSteamId() : FString();
	const FString LocalDisplayName = SteamHelper ? SteamHelper->GetLocalDisplayName() : FString();

	T66SetJsonStringIfNotEmpty(Root, TEXT("message"), Diagnostic.Message);
	T66SetJsonStringIfNotEmpty(Root, TEXT("invite_id"), Diagnostic.InviteId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("host_steam_id"), Diagnostic.HostSteamId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("target_steam_id"), Diagnostic.TargetSteamId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("lobby_id"), Diagnostic.LobbyId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("expected_lobby_id"), Diagnostic.ExpectedLobbyId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("found_lobby_id"), Diagnostic.FoundLobbyId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("source_app_id"), Diagnostic.SourceAppId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("active_app_id"), ActiveAppId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("steam_beta"), BetaName);
	T66SetJsonStringIfNotEmpty(Root, TEXT("local_steam_id"), LocalSteamId);
	T66SetJsonStringIfNotEmpty(Root, TEXT("local_display_name"), LocalDisplayName);
	T66SetJsonStringIfNotEmpty(Root, TEXT("status_text"), Diagnostic.StatusText);
	T66SetJsonStringIfNotEmpty(Root, TEXT("map_name"), MapName);
	T66SetJsonStringIfNotEmpty(Root, TEXT("build_version"), BuildVersion);
	T66SetJsonStringIfNotEmpty(Root, TEXT("backend_base_url"), BackendBaseUrl);
	T66SetJsonStringIfNotEmpty(Root, TEXT("project_name"), FString(FApp::GetProjectName()));
	T66SetJsonStringIfNotEmpty(Root, TEXT("os_version"), FPlatformMisc::GetOSVersion());
	T66SetJsonStringIfNotEmpty(Root, TEXT("engine_version"), FEngineVersion::Current().ToString());

	if (Diagnostic.ExtraFields.Num() > 0)
	{
		TSharedPtr<FJsonObject> Extra = MakeShared<FJsonObject>();
		for (const TPair<FString, FString>& Pair : Diagnostic.ExtraFields)
		{
			T66SetJsonStringIfNotEmpty(Extra, Pair.Key, Pair.Value);
		}

		if (Extra->Values.Num() > 0)
		{
			Root->SetObjectField(TEXT("extra"), Extra);
		}
	}

	return Root;
}

void UT66BackendSubsystem::SaveMultiplayerDiagnosticLocally(const TSharedPtr<FJsonObject>& DiagnosticJson, const FString& EventName) const
{
	if (!DiagnosticJson.IsValid())
	{
		return;
	}

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(DiagnosticJson.ToSharedRef(), Writer);

	const FString DiagnosticsDir = FPaths::ProjectSavedDir() / TEXT("Diagnostics") / TEXT("Multiplayer");
	IFileManager::Get().MakeDirectory(*DiagnosticsDir, true);

	const FString SafeEventName = FPaths::MakeValidFileName(EventName.IsEmpty() ? TEXT("unknown") : EventName);
	const FString Timestamp = FDateTime::UtcNow().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString FilePath = DiagnosticsDir / FString::Printf(TEXT("%s_%s.json"), *Timestamp, *SafeEventName);
	FFileHelper::SaveStringToFile(Payload, *FilePath);
}

void UT66BackendSubsystem::SubmitMultiplayerDiagnostic(const FT66MultiplayerDiagnosticContext& Diagnostic)
{
	const TSharedPtr<FJsonObject> Root = BuildMultiplayerDiagnosticJson(Diagnostic);
	SaveMultiplayerDiagnosticLocally(Root, Diagnostic.EventName);

	if (!IsBackendConfigured() || !HasSteamTicket())
	{
		UE_LOG(
			LogT66Backend,
			Log,
			TEXT("Backend: multiplayer diagnostic saved locally only. Event=%s BackendConfigured=%d HasTicket=%d"),
			*Diagnostic.EventName,
			IsBackendConfigured() ? 1 : 0,
			HasSteamTicket() ? 1 : 0);
		return;
	}

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/client-diagnostics"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(
		this,
		&UT66BackendSubsystem::OnClientDiagnosticsResponseReceived,
		Diagnostic.EventName,
		Diagnostic.InviteId);
	Request->ProcessRequest();
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

FString UT66BackendSubsystem::ExtractResponseMessage(const TSharedPtr<FJsonObject>& Json, const FString& FallbackMessage)
{
	if (!Json.IsValid())
	{
		return FallbackMessage;
	}

	FString Message;
	if (Json->TryGetStringField(TEXT("message"), Message) && !Message.IsEmpty())
	{
		return Message;
	}

	FString Status;
	if (Json->TryGetStringField(TEXT("status"), Status) && !Status.IsEmpty())
	{
		return Status;
	}

	return FallbackMessage;
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
	case ET66PartySize::Quad: return TEXT("quad");
	default: return TEXT("solo");
	}
}

namespace
{
	TSharedPtr<FJsonObject> T66BuildRunJsonObject(
		const FString& HeroId,
		const FString& CompanionId,
		ET66Difficulty Difficulty,
		ET66PartySize PartySize,
		int32 StageReached,
		int32 Score,
		int32 TimeMs,
		UT66LeaderboardRunSummarySaveGame* Snapshot)
	{
		auto DifficultyToApi = [](ET66Difficulty InDifficulty) -> FString
		{
			switch (InDifficulty)
			{
			case ET66Difficulty::Easy: return TEXT("easy");
			case ET66Difficulty::Medium: return TEXT("medium");
			case ET66Difficulty::Hard: return TEXT("hard");
			case ET66Difficulty::VeryHard: return TEXT("veryhard");
			case ET66Difficulty::Impossible: return TEXT("impossible");
			default: return TEXT("easy");
			}
		};

		auto PartyToApi = [](ET66PartySize InPartySize) -> FString
		{
			switch (InPartySize)
			{
			case ET66PartySize::Solo: return TEXT("solo");
			case ET66PartySize::Duo: return TEXT("duo");
			case ET66PartySize::Trio: return TEXT("trio");
			case ET66PartySize::Quad: return TEXT("quad");
			default: return TEXT("solo");
			}
		};

		TSharedPtr<FJsonObject> RunObj = MakeShared<FJsonObject>();
		RunObj->SetStringField(TEXT("hero_id"), HeroId);
		RunObj->SetStringField(TEXT("companion_id"), CompanionId);
		RunObj->SetStringField(TEXT("difficulty"), DifficultyToApi(Difficulty));
		RunObj->SetStringField(TEXT("party_size"), PartyToApi(PartySize));
		RunObj->SetNumberField(TEXT("stage_reached"), StageReached);
		RunObj->SetNumberField(TEXT("score"), Score);
		RunObj->SetNumberField(TEXT("time_ms"), TimeMs);

		if (!Snapshot)
		{
			return RunObj;
		}

		RunObj->SetNumberField(TEXT("hero_level"), Snapshot->HeroLevel);

		TSharedPtr<FJsonObject> StatsObj = MakeShared<FJsonObject>();
		StatsObj->SetNumberField(TEXT("damage"), Snapshot->DamageStat);
		StatsObj->SetNumberField(TEXT("attack_speed"), Snapshot->AttackSpeedStat);
		StatsObj->SetNumberField(TEXT("attack_scale"), Snapshot->AttackScaleStat);
		StatsObj->SetNumberField(TEXT("accuracy"), Snapshot->AccuracyStat);
		StatsObj->SetNumberField(TEXT("armor"), Snapshot->ArmorStat);
		StatsObj->SetNumberField(TEXT("evasion"), Snapshot->EvasionStat);
		StatsObj->SetNumberField(TEXT("luck"), Snapshot->LuckStat);
		StatsObj->SetNumberField(TEXT("speed"), Snapshot->SpeedStat);
		RunObj->SetObjectField(TEXT("stats"), StatsObj);

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
			case ET66SecondaryStatType::LootCrate: KeyName = TEXT("LootCrate"); break;
			case ET66SecondaryStatType::DamageReduction: KeyName = TEXT("DamageReduction"); break;
			case ET66SecondaryStatType::EvasionChance: KeyName = TEXT("EvasionChance"); break;
			case ET66SecondaryStatType::Alchemy: KeyName = TEXT("Alchemy"); break;
			case ET66SecondaryStatType::Accuracy: KeyName = TEXT("Accuracy"); break;
			default: continue;
			}
			SecObj->SetNumberField(KeyName, Pair.Value);
		}
		RunObj->SetObjectField(TEXT("secondary_stats"), SecObj);

		if (Snapshot->LuckRating0To100 >= 0) RunObj->SetNumberField(TEXT("luck_rating"), Snapshot->LuckRating0To100);
		if (Snapshot->LuckRatingQuantity0To100 >= 0) RunObj->SetNumberField(TEXT("luck_quantity"), Snapshot->LuckRatingQuantity0To100);
		if (Snapshot->LuckRatingQuality0To100 >= 0) RunObj->SetNumberField(TEXT("luck_quality"), Snapshot->LuckRatingQuality0To100);
		if (Snapshot->SkillRating0To100 >= 0) RunObj->SetNumberField(TEXT("skill_rating"), Snapshot->SkillRating0To100);

		TSharedPtr<FJsonObject> AntiCheatObj = MakeShared<FJsonObject>();
		AntiCheatObj->SetNumberField(TEXT("run_seed"), Snapshot->RunSeed);
		AntiCheatObj->SetNumberField(TEXT("luck_quantity_sample_count"), Snapshot->LuckQuantitySampleCount);
		AntiCheatObj->SetNumberField(TEXT("luck_quality_sample_count"), Snapshot->LuckQualitySampleCount);
		if (Snapshot->LuckQuantityAccumulators.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> QuantityCategoryArr;
			for (const FT66SavedLuckAccumulator& Accumulator : Snapshot->LuckQuantityAccumulators)
			{
				TSharedPtr<FJsonObject> CategoryObj = MakeShared<FJsonObject>();
				CategoryObj->SetStringField(TEXT("category"), Accumulator.Category.ToString());
				CategoryObj->SetNumberField(TEXT("sum_01"), Accumulator.Sum01);
				CategoryObj->SetNumberField(TEXT("count"), Accumulator.Count);
				CategoryObj->SetNumberField(TEXT("avg_01"), Accumulator.Count > 0 ? (Accumulator.Sum01 / static_cast<float>(Accumulator.Count)) : 0.5f);
				QuantityCategoryArr.Add(MakeShared<FJsonValueObject>(CategoryObj));
			}
			AntiCheatObj->SetArrayField(TEXT("luck_quantity_categories"), QuantityCategoryArr);
		}
		if (Snapshot->LuckQualityAccumulators.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> QualityCategoryArr;
			for (const FT66SavedLuckAccumulator& Accumulator : Snapshot->LuckQualityAccumulators)
			{
				TSharedPtr<FJsonObject> CategoryObj = MakeShared<FJsonObject>();
				CategoryObj->SetStringField(TEXT("category"), Accumulator.Category.ToString());
				CategoryObj->SetNumberField(TEXT("sum_01"), Accumulator.Sum01);
				CategoryObj->SetNumberField(TEXT("count"), Accumulator.Count);
				CategoryObj->SetNumberField(TEXT("avg_01"), Accumulator.Count > 0 ? (Accumulator.Sum01 / static_cast<float>(Accumulator.Count)) : 0.5f);
				QualityCategoryArr.Add(MakeShared<FJsonValueObject>(CategoryObj));
			}
			AntiCheatObj->SetArrayField(TEXT("luck_quality_categories"), QualityCategoryArr);
		}
		if (Snapshot->AntiCheatLuckEvents.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> LuckEventArr;
			for (const FT66AntiCheatLuckEvent& Event : Snapshot->AntiCheatLuckEvents)
			{
				TSharedPtr<FJsonObject> EventObj = MakeShared<FJsonObject>();
				const TCHAR* EventTypeString = TEXT("quantity_roll");
				switch (Event.EventType)
				{
				case ET66AntiCheatLuckEventType::QuantityBool:
					EventTypeString = TEXT("quantity_bool");
					break;
				case ET66AntiCheatLuckEventType::QualityRarity:
					EventTypeString = TEXT("quality_rarity");
					break;
				default:
					break;
				}

				EventObj->SetStringField(TEXT("event_type"), EventTypeString);
				EventObj->SetStringField(TEXT("category"), Event.Category.ToString());
				EventObj->SetNumberField(TEXT("time_seconds"), Event.TimeSeconds);
				EventObj->SetNumberField(TEXT("value_01"), Event.Value01);
				EventObj->SetNumberField(TEXT("raw_value"), Event.RawValue);
				EventObj->SetNumberField(TEXT("raw_min"), Event.RawMin);
				EventObj->SetNumberField(TEXT("raw_max"), Event.RawMax);
				if (Event.RunDrawIndex != INDEX_NONE)
				{
					EventObj->SetNumberField(TEXT("draw_index"), Event.RunDrawIndex);
					EventObj->SetNumberField(TEXT("pre_draw_seed"), Event.PreDrawSeed);
				}
				if (Event.ExpectedChance01 >= 0.f)
				{
					EventObj->SetNumberField(TEXT("expected_chance_01"), Event.ExpectedChance01);
				}
				if (Event.bHasRarityWeights)
				{
					EventObj->SetNumberField(TEXT("weight_black"), Event.RarityWeights.Black);
					EventObj->SetNumberField(TEXT("weight_red"), Event.RarityWeights.Red);
					EventObj->SetNumberField(TEXT("weight_yellow"), Event.RarityWeights.Yellow);
					EventObj->SetNumberField(TEXT("weight_white"), Event.RarityWeights.White);
				}
				if (Event.bHasFloatReplayRange)
				{
					EventObj->SetNumberField(TEXT("float_min"), Event.FloatReplayMin);
					EventObj->SetNumberField(TEXT("float_max"), Event.FloatReplayMax);
				}
				LuckEventArr.Add(MakeShared<FJsonValueObject>(EventObj));
			}
			AntiCheatObj->SetArrayField(TEXT("luck_events"), LuckEventArr);
		}
		AntiCheatObj->SetBoolField(TEXT("luck_events_truncated"), Snapshot->bAntiCheatLuckEventsTruncated);
		if (Snapshot->AntiCheatHitCheckEvents.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> HitCheckEventArr;
			for (const FT66AntiCheatHitCheckEvent& Event : Snapshot->AntiCheatHitCheckEvents)
			{
				TSharedPtr<FJsonObject> EventObj = MakeShared<FJsonObject>();
				EventObj->SetNumberField(TEXT("time_seconds"), Event.TimeSeconds);
				EventObj->SetNumberField(TEXT("evasion_chance_01"), Event.EvasionChance01);
				EventObj->SetBoolField(TEXT("dodged"), Event.bDodged);
				EventObj->SetBoolField(TEXT("damage_applied"), Event.bDamageApplied);
				HitCheckEventArr.Add(MakeShared<FJsonValueObject>(EventObj));
			}
			AntiCheatObj->SetArrayField(TEXT("hit_check_events"), HitCheckEventArr);
		}
		AntiCheatObj->SetBoolField(TEXT("hit_check_events_truncated"), Snapshot->bAntiCheatHitCheckEventsTruncated);
		AntiCheatObj->SetNumberField(TEXT("incoming_hit_checks"), Snapshot->IncomingHitChecks);
		AntiCheatObj->SetNumberField(TEXT("damage_taken_hit_count"), Snapshot->DamageTakenHitCount);
		AntiCheatObj->SetNumberField(TEXT("dodge_count"), Snapshot->DodgeCount);
		AntiCheatObj->SetNumberField(TEXT("max_consecutive_dodges"), Snapshot->MaxConsecutiveDodges);
		AntiCheatObj->SetNumberField(TEXT("total_evasion_chance"), Snapshot->TotalEvasionChance);
		if (Snapshot->AntiCheatEvasionBuckets.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> EvasionBucketArr;
			for (const FT66AntiCheatEvasionBucketSummary& Bucket : Snapshot->AntiCheatEvasionBuckets)
			{
				TSharedPtr<FJsonObject> BucketObj = MakeShared<FJsonObject>();
				BucketObj->SetNumberField(TEXT("bucket_index"), Bucket.BucketIndex);
				BucketObj->SetNumberField(TEXT("min_chance_01"), Bucket.MinChance01);
				BucketObj->SetNumberField(TEXT("max_chance_01"), Bucket.MaxChance01);
				BucketObj->SetNumberField(TEXT("hit_checks"), Bucket.HitChecks);
				BucketObj->SetNumberField(TEXT("dodges"), Bucket.Dodges);
				BucketObj->SetNumberField(TEXT("damage_applied"), Bucket.DamageApplied);
				BucketObj->SetNumberField(TEXT("expected_dodges"), Bucket.ExpectedDodges);
				EvasionBucketArr.Add(MakeShared<FJsonValueObject>(BucketObj));
			}
			AntiCheatObj->SetArrayField(TEXT("evasion_buckets"), EvasionBucketArr);
		}
		{
			TSharedPtr<FJsonObject> PressureObj = MakeShared<FJsonObject>();
			PressureObj->SetNumberField(TEXT("window_seconds"), Snapshot->AntiCheatPressureWindowSummary.WindowSeconds);
			PressureObj->SetNumberField(TEXT("active_windows"), Snapshot->AntiCheatPressureWindowSummary.ActiveWindows);
			PressureObj->SetNumberField(TEXT("pressured_windows_4_plus"), Snapshot->AntiCheatPressureWindowSummary.PressuredWindows4Plus);
			PressureObj->SetNumberField(TEXT("pressured_windows_8_plus"), Snapshot->AntiCheatPressureWindowSummary.PressuredWindows8Plus);
			PressureObj->SetNumberField(TEXT("zero_damage_windows_4_plus"), Snapshot->AntiCheatPressureWindowSummary.ZeroDamageWindows4Plus);
			PressureObj->SetNumberField(TEXT("zero_damage_windows_8_plus"), Snapshot->AntiCheatPressureWindowSummary.ZeroDamageWindows8Plus);
			PressureObj->SetNumberField(TEXT("near_perfect_windows_8_plus"), Snapshot->AntiCheatPressureWindowSummary.NearPerfectWindows8Plus);
			PressureObj->SetNumberField(TEXT("max_hit_checks_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxHitChecksInWindow);
			PressureObj->SetNumberField(TEXT("max_dodges_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxDodgesInWindow);
			PressureObj->SetNumberField(TEXT("max_damage_applied_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxDamageAppliedInWindow);
			PressureObj->SetNumberField(TEXT("max_expected_dodges_in_window"), Snapshot->AntiCheatPressureWindowSummary.MaxExpectedDodgesInWindow);
			PressureObj->SetNumberField(TEXT("total_hit_checks"), Snapshot->AntiCheatPressureWindowSummary.TotalHitChecks);
			PressureObj->SetNumberField(TEXT("total_dodges"), Snapshot->AntiCheatPressureWindowSummary.TotalDodges);
			PressureObj->SetNumberField(TEXT("total_damage_applied"), Snapshot->AntiCheatPressureWindowSummary.TotalDamageApplied);
			PressureObj->SetNumberField(TEXT("total_expected_dodges"), Snapshot->AntiCheatPressureWindowSummary.TotalExpectedDodges);
			AntiCheatObj->SetObjectField(TEXT("pressure_window_summary"), PressureObj);
		}
		if (Snapshot->AntiCheatGamblerSummaries.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> GamblerSummaryArr;
			for (const FT66AntiCheatGamblerGameSummary& Summary : Snapshot->AntiCheatGamblerSummaries)
			{
				const TCHAR* GameTypeString = TEXT("coin_flip");
				switch (Summary.GameType)
				{
				case ET66AntiCheatGamblerGameType::RockPaperScissors: GameTypeString = TEXT("rps"); break;
				case ET66AntiCheatGamblerGameType::BlackJack: GameTypeString = TEXT("blackjack"); break;
				case ET66AntiCheatGamblerGameType::Lottery: GameTypeString = TEXT("lottery"); break;
				case ET66AntiCheatGamblerGameType::Plinko: GameTypeString = TEXT("plinko"); break;
				case ET66AntiCheatGamblerGameType::BoxOpening: GameTypeString = TEXT("box_opening"); break;
				default: break;
				}

				TSharedPtr<FJsonObject> SummaryObj = MakeShared<FJsonObject>();
				SummaryObj->SetStringField(TEXT("game_type"), GameTypeString);
				SummaryObj->SetNumberField(TEXT("rounds"), Summary.Rounds);
				SummaryObj->SetNumberField(TEXT("wins"), Summary.Wins);
				SummaryObj->SetNumberField(TEXT("losses"), Summary.Losses);
				SummaryObj->SetNumberField(TEXT("draws"), Summary.Draws);
				SummaryObj->SetNumberField(TEXT("cheat_attempts"), Summary.CheatAttempts);
				SummaryObj->SetNumberField(TEXT("cheat_successes"), Summary.CheatSuccesses);
				SummaryObj->SetNumberField(TEXT("total_bet_gold"), Summary.TotalBetGold);
				SummaryObj->SetNumberField(TEXT("total_payout_gold"), Summary.TotalPayoutGold);
				GamblerSummaryArr.Add(MakeShared<FJsonValueObject>(SummaryObj));
			}
			AntiCheatObj->SetArrayField(TEXT("gambler_summary"), GamblerSummaryArr);
		}
		if (Snapshot->AntiCheatGamblerEvents.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> GamblerEventArr;
			for (const FT66AntiCheatGamblerEvent& Event : Snapshot->AntiCheatGamblerEvents)
			{
				const TCHAR* GameTypeString = TEXT("coin_flip");
				switch (Event.GameType)
				{
				case ET66AntiCheatGamblerGameType::RockPaperScissors: GameTypeString = TEXT("rps"); break;
				case ET66AntiCheatGamblerGameType::BlackJack: GameTypeString = TEXT("blackjack"); break;
				case ET66AntiCheatGamblerGameType::Lottery: GameTypeString = TEXT("lottery"); break;
				case ET66AntiCheatGamblerGameType::Plinko: GameTypeString = TEXT("plinko"); break;
				case ET66AntiCheatGamblerGameType::BoxOpening: GameTypeString = TEXT("box_opening"); break;
				default: break;
				}

				TSharedPtr<FJsonObject> EventObj = MakeShared<FJsonObject>();
				EventObj->SetStringField(TEXT("game_type"), GameTypeString);
				EventObj->SetNumberField(TEXT("time_seconds"), Event.TimeSeconds);
				EventObj->SetNumberField(TEXT("bet_gold"), Event.BetGold);
				EventObj->SetNumberField(TEXT("payout_gold"), Event.PayoutGold);
				EventObj->SetBoolField(TEXT("cheat_attempted"), Event.bCheatAttempted);
				EventObj->SetBoolField(TEXT("cheat_succeeded"), Event.bCheatSucceeded);
				EventObj->SetBoolField(TEXT("win"), Event.bWin);
				EventObj->SetBoolField(TEXT("draw"), Event.bDraw);
				if (Event.PlayerChoice != INDEX_NONE) EventObj->SetNumberField(TEXT("player_choice"), Event.PlayerChoice);
				if (Event.OpponentChoice != INDEX_NONE) EventObj->SetNumberField(TEXT("opponent_choice"), Event.OpponentChoice);
				EventObj->SetNumberField(TEXT("outcome_value"), Event.OutcomeValue);
				EventObj->SetNumberField(TEXT("outcome_value_secondary"), Event.OutcomeSecondaryValue);
				if (Event.SelectedMask != 0) EventObj->SetNumberField(TEXT("selected_mask"), Event.SelectedMask);
				if (Event.ResolvedMask != 0) EventObj->SetNumberField(TEXT("resolved_mask"), Event.ResolvedMask);
				EventObj->SetNumberField(TEXT("path_bits"), Event.PathBits);
				if (Event.ShuffleStartDrawIndex != INDEX_NONE)
				{
					EventObj->SetNumberField(TEXT("shuffle_pre_draw_seed"), Event.ShufflePreDrawSeed);
					EventObj->SetNumberField(TEXT("shuffle_start_draw_index"), Event.ShuffleStartDrawIndex);
				}
				if (Event.OutcomeDrawIndex != INDEX_NONE)
				{
					EventObj->SetNumberField(TEXT("outcome_pre_draw_seed"), Event.OutcomePreDrawSeed);
					EventObj->SetNumberField(TEXT("outcome_draw_index"), Event.OutcomeDrawIndex);
				}
				if (Event.OutcomeExpectedChance01 >= 0.f)
				{
					EventObj->SetNumberField(TEXT("outcome_expected_chance_01"), Event.OutcomeExpectedChance01);
				}
				if (!Event.ActionSequence.IsEmpty())
				{
					EventObj->SetStringField(TEXT("action_sequence"), Event.ActionSequence);
				}
				GamblerEventArr.Add(MakeShared<FJsonValueObject>(EventObj));
			}
			AntiCheatObj->SetArrayField(TEXT("gambler_events"), GamblerEventArr);
		}
		AntiCheatObj->SetBoolField(TEXT("gambler_events_truncated"), Snapshot->bAntiCheatGamblerEventsTruncated);
		RunObj->SetObjectField(TEXT("anti_cheat_context"), AntiCheatObj);

		TArray<TSharedPtr<FJsonValue>> IdolArr;
		for (const FName& Idol : Snapshot->EquippedIdols)
		{
			IdolArr.Add(MakeShared<FJsonValueString>(Idol.ToString()));
		}
		RunObj->SetArrayField(TEXT("equipped_idols"), IdolArr);

		TArray<TSharedPtr<FJsonValue>> InvArr;
		for (const FName& Item : Snapshot->Inventory)
		{
			InvArr.Add(MakeShared<FJsonValueString>(Item.ToString()));
		}
		RunObj->SetArrayField(TEXT("inventory"), InvArr);

		TArray<TSharedPtr<FJsonValue>> LogArr;
		for (const FString& Msg : Snapshot->EventLog)
		{
			LogArr.Add(MakeShared<FJsonValueString>(Msg));
		}
		RunObj->SetArrayField(TEXT("event_log"), LogArr);

		if (Snapshot->StagePacingPoints.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> StageSplitArr;
			for (const FT66StagePacingPoint& Point : Snapshot->StagePacingPoints)
			{
				const int32 SplitMs = FMath::RoundToInt(FMath::Max(0.f, Point.ElapsedSeconds) * 1000.f);
				StageSplitArr.Add(MakeShared<FJsonValueNumber>(SplitMs));
			}
			RunObj->SetArrayField(TEXT("stage_splits_ms"), StageSplitArr);
		}

		TSharedPtr<FJsonObject> DmgObj = MakeShared<FJsonObject>();
		for (const auto& Pair : Snapshot->DamageBySource)
		{
			DmgObj->SetNumberField(Pair.Key.ToString(), Pair.Value);
		}
		RunObj->SetObjectField(TEXT("damage_by_source"), DmgObj);

		return RunObj;
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
	UT66LeaderboardRunSummarySaveGame* Snapshot,
	const FString& RequestKey)
{
	if (!IsBackendConfigured())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: cannot submit run — no backend URL configured."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
		return;
	}

	if (!HasSteamTicket())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: cannot submit run — no Steam ticket set."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
		return;
	}

	const TSharedPtr<FJsonObject> LocalRunObject = T66BuildRunJsonObject(
		HeroId,
		CompanionId,
		Difficulty,
		PartySize,
		StageReached,
		Score,
		TimeMs,
		Snapshot);

	FString LocalRunJson;
	if (!T66SerializeJsonObjectToString(LocalRunObject, LocalRunJson))
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: cannot submit run — failed to serialize run summary payload."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;

	if (PartySize != ET66PartySize::Solo)
	{
		if (!SessionSubsystem || !SessionSubsystem->IsPartySessionActive())
		{
			UE_LOG(LogT66Backend, Log, TEXT("Backend: skipping co-op submit because no active party session was found."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
			return;
		}

		const bool bCachedForHost = SessionSubsystem->SubmitLocalPartyRunSummaryToHost(RequestKey, LocalRunJson);
		if (!bCachedForHost)
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: skipping co-op submit because the local run summary could not be handed to the host."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
			return;
		}

		if (!SessionSubsystem->IsLocalPlayerPartyHost())
		{
			UE_LOG(LogT66Backend, Log, TEXT("Backend: forwarded co-op run summary to host; non-host client will not submit directly."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
			return;
		}
	}

	(void)TrySubmitRunToBackendNow(
		DisplayName,
		Score,
		TimeMs,
		Difficulty,
		PartySize,
		StageReached,
		HeroId,
		CompanionId,
		LocalRunJson,
		RequestKey);
}

void UT66BackendSubsystem::QueuePendingCoopSubmit(
	const FString& DisplayName,
	int32 Score,
	int32 TimeMs,
	ET66Difficulty Difficulty,
	ET66PartySize PartySize,
	int32 StageReached,
	const FString& HeroId,
	const FString& CompanionId,
	const FString& HostRunJson,
	const FString& RequestKey)
{
	FPendingCoopSubmit& Pending = PendingCoopSubmitRequests.FindOrAdd(RequestKey);
	if (Pending.RequestKey.IsEmpty())
	{
		Pending.DisplayName = DisplayName;
		Pending.Score = Score;
		Pending.TimeMs = TimeMs;
		Pending.Difficulty = Difficulty;
		Pending.PartySize = PartySize;
		Pending.StageReached = StageReached;
		Pending.HeroId = HeroId;
		Pending.CompanionId = CompanionId;
		Pending.HostRunJson = HostRunJson;
		Pending.RequestKey = RequestKey;
		Pending.RetryCount = 0;
	}

	if (!PendingCoopSubmitTickerHandle.IsValid())
	{
		PendingCoopSubmitTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UT66BackendSubsystem::HandlePendingCoopSubmitTicker),
			0.25f);
	}
}

bool UT66BackendSubsystem::TrySubmitRunToBackendNow(
	const FString& DisplayName,
	int32 Score,
	int32 TimeMs,
	ET66Difficulty Difficulty,
	ET66PartySize PartySize,
	int32 StageReached,
	const FString& HeroId,
	const FString& CompanionId,
	const FString& HostRunJson,
	const FString& RequestKey)
{
	TSharedPtr<FJsonObject> HostRunObject;
	if (!T66DeserializeJsonObjectString(HostRunJson, HostRunObject) || !HostRunObject.IsValid())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run failed because the host run payload could not be parsed."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
		return true;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("display_name"), DisplayName);

	FString RootHeroId = HeroId;
	FString RootCompanionId = CompanionId;
	HostRunObject->TryGetStringField(TEXT("hero_id"), RootHeroId);
	HostRunObject->TryGetStringField(TEXT("companion_id"), RootCompanionId);

	if (PartySize != ET66PartySize::Solo)
	{
		UGameInstance* GI = GetGameInstance();
		UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
		if (!SessionSubsystem || !SessionSubsystem->IsPartySessionActive() || !SessionSubsystem->IsLocalPlayerPartyHost())
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: co-op submit aborted because the local player is no longer the active party host."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
			return true;
		}

		TArray<FT66LobbyPlayerInfo> LobbyProfiles;
		SessionSubsystem->GetCurrentLobbyProfiles(LobbyProfiles);
		if (LobbyProfiles.Num() < 2)
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: co-op submit aborted because fewer than two lobby profiles are available."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
			return true;
		}

		TArray<TSharedPtr<FJsonValue>> PartyMemberJsonValues;
		PartyMemberJsonValues.Reserve(LobbyProfiles.Num());
		for (int32 PartySlot = 0; PartySlot < LobbyProfiles.Num(); ++PartySlot)
		{
			const FT66LobbyPlayerInfo& LobbyInfo = LobbyProfiles[PartySlot];
			if (LobbyInfo.SteamId.IsEmpty())
			{
				continue;
			}

			TSharedPtr<FJsonObject> MemberRunObject;
			if (LobbyInfo.bPartyHost)
			{
				MemberRunObject = HostRunObject;
			}
			else
			{
				FString MemberRunJson;
				if (!SessionSubsystem->GetCachedPartyRunSummaryJson(RequestKey, LobbyInfo.SteamId, MemberRunJson))
				{
					QueuePendingCoopSubmit(DisplayName, Score, TimeMs, Difficulty, PartySize, StageReached, HeroId, CompanionId, HostRunJson, RequestKey);
					UE_LOG(LogT66Backend, Log, TEXT("Backend: waiting for co-op run summary from steam_id=%s before submitting request=%s"), *LobbyInfo.SteamId, *RequestKey);
					return false;
				}

				if (!T66DeserializeJsonObjectString(MemberRunJson, MemberRunObject) || !MemberRunObject.IsValid())
				{
					UE_LOG(LogT66Backend, Warning, TEXT("Backend: co-op submit aborted because a member run payload was invalid. steam_id=%s"), *LobbyInfo.SteamId);
					OnSubmitRunComplete.Broadcast(false, 0, 0, false);
					OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
					return true;
				}
			}

			FString MemberHeroId = LobbyInfo.SelectedHeroID.IsNone() ? HeroId : LobbyInfo.SelectedHeroID.ToString();
			FString MemberCompanionId = LobbyInfo.SelectedCompanionID.IsNone() ? CompanionId : LobbyInfo.SelectedCompanionID.ToString();
			MemberRunObject->TryGetStringField(TEXT("hero_id"), MemberHeroId);
			MemberRunObject->TryGetStringField(TEXT("companion_id"), MemberCompanionId);
			if (LobbyInfo.bPartyHost)
			{
				RootHeroId = MemberHeroId;
				RootCompanionId = MemberCompanionId;
			}

			TSharedPtr<FJsonObject> MemberObj = MakeShared<FJsonObject>();
			MemberObj->SetStringField(TEXT("steam_id"), LobbyInfo.SteamId);
			MemberObj->SetStringField(TEXT("display_name"), LobbyInfo.DisplayName.IsEmpty() ? DisplayName : LobbyInfo.DisplayName);
			MemberObj->SetNumberField(TEXT("party_slot"), PartySlot);
			MemberObj->SetStringField(TEXT("hero_id"), MemberHeroId);
			MemberObj->SetObjectField(TEXT("run_summary"), MemberRunObject);
			PartyMemberJsonValues.Add(MakeShared<FJsonValueObject>(MemberObj));
		}

		if (PartyMemberJsonValues.Num() < 2)
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: co-op submit aborted because the party payload is incomplete."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
			return true;
		}

		TSharedPtr<FJsonObject> CoopObj = MakeShared<FJsonObject>();
		CoopObj->SetArrayField(TEXT("party_members"), PartyMemberJsonValues);
		Root->SetObjectField(TEXT("co_op"), CoopObj);
	}

	Root->SetObjectField(TEXT("run"), HostRunObject);
	SendSubmitRunRequest(Root, RequestKey);

	UE_LOG(LogT66Backend, Log, TEXT("Backend: submitting run (Score=%d, Difficulty=%s, Party=%s)"),
		Score, *DifficultyToApiString(Difficulty), *PartySizeToApiString(PartySize));
	return true;
}

void UT66BackendSubsystem::SendSubmitRunRequest(const TSharedPtr<FJsonObject>& Root, const FString& RequestKey)
{
	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/submit-run"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnSubmitRunResponseReceived, RequestKey);
	Request->ProcessRequest();
}

bool UT66BackendSubsystem::HandlePendingCoopSubmitTicker(float DeltaTime)
{
	(void)DeltaTime;

	if (PendingCoopSubmitRequests.Num() <= 0)
	{
		PendingCoopSubmitTickerHandle.Reset();
		return false;
	}

	TArray<FString> RequestKeys;
	PendingCoopSubmitRequests.GetKeys(RequestKeys);
	for (const FString& RequestKey : RequestKeys)
	{
		FPendingCoopSubmit* Pending = PendingCoopSubmitRequests.Find(RequestKey);
		if (!Pending)
		{
			continue;
		}

		Pending->RetryCount++;
		if (TrySubmitRunToBackendNow(
			Pending->DisplayName,
			Pending->Score,
			Pending->TimeMs,
			Pending->Difficulty,
			Pending->PartySize,
			Pending->StageReached,
			Pending->HeroId,
			Pending->CompanionId,
			Pending->HostRunJson,
			Pending->RequestKey))
		{
			PendingCoopSubmitRequests.Remove(RequestKey);
			continue;
		}

		if (Pending->RetryCount >= 12)
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: co-op submit timed out waiting for member summaries. request=%s"), *RequestKey);
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
			PendingCoopSubmitRequests.Remove(RequestKey);
		}
	}

	if (PendingCoopSubmitRequests.Num() <= 0)
	{
		PendingCoopSubmitTickerHandle.Reset();
		return false;
	}

	return true;
}

void UT66BackendSubsystem::OnSubmitRunResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FString RequestKey)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run failed (connection error)."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
		return;
	}

	const int32 Code = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run failed to parse JSON. Code=%d"), Code);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
		return;
	}

	const FString Status = Json->GetStringField(TEXT("status"));
	if (Status == TEXT("accepted"))
	{
		double ScoreRankAlltimeValue = 0.0;
		double ScoreRankWeeklyValue = 0.0;
		double SpeedRunRankAlltimeValue = 0.0;
		double SpeedRunRankWeeklyValue = 0.0;
		bool bNewScorePB = false;
		bool bNewSpeedRunPB = false;
		(void)Json->TryGetNumberField(TEXT("score_rank_alltime"), ScoreRankAlltimeValue);
		(void)Json->TryGetNumberField(TEXT("score_rank_weekly"), ScoreRankWeeklyValue);
		(void)Json->TryGetNumberField(TEXT("speedrun_rank_alltime"), SpeedRunRankAlltimeValue);
		(void)Json->TryGetNumberField(TEXT("speedrun_rank_weekly"), SpeedRunRankWeeklyValue);
		(void)Json->TryGetBoolField(TEXT("is_new_personal_best_score"), bNewScorePB);
		(void)Json->TryGetBoolField(TEXT("is_new_personal_best_speedrun"), bNewSpeedRunPB);
		const int32 ScoreRankAlltime = static_cast<int32>(ScoreRankAlltimeValue);
		const int32 ScoreRankWeekly = static_cast<int32>(ScoreRankWeeklyValue);
		const int32 SpeedRunRankAlltime = static_cast<int32>(SpeedRunRankAlltimeValue);
		const int32 SpeedRunRankWeekly = static_cast<int32>(SpeedRunRankWeeklyValue);
		const bool bExpectCompletedRunRank = RequestKey.StartsWith(TEXT("best_rank_completed_"));
		const int32 PrimaryRankAlltime = bExpectCompletedRunRank ? SpeedRunRankAlltime : ScoreRankAlltime;
		const int32 PrimaryRankWeekly = bExpectCompletedRunRank ? SpeedRunRankWeekly : ScoreRankWeekly;
		const bool bPrimaryNewPB = bExpectCompletedRunRank ? bNewSpeedRunPB : bNewScorePB;

		UE_LOG(
			LogT66Backend,
			Log,
			TEXT("Backend: submit-run accepted. Score(All=%d Weekly=%d NewPB=%s) SpeedRun(All=%d Weekly=%d NewPB=%s) Selected(All=%d Weekly=%d NewPB=%s)"),
			ScoreRankAlltime,
			ScoreRankWeekly,
			bNewScorePB ? TEXT("true") : TEXT("false"),
			SpeedRunRankAlltime,
			SpeedRunRankWeekly,
			bNewSpeedRunPB ? TEXT("true") : TEXT("false"),
			PrimaryRankAlltime,
			PrimaryRankWeekly,
			bPrimaryNewPB ? TEXT("true") : TEXT("false"));

		OnSubmitRunComplete.Broadcast(true, PrimaryRankAlltime, PrimaryRankWeekly, bPrimaryNewPB);
		OnSubmitRunDataReady.Broadcast(RequestKey, true, PrimaryRankAlltime, PrimaryRankWeekly, bPrimaryNewPB);
	}
	else if (Status == TEXT("flagged") || Status == TEXT("banned") || Status == TEXT("suspended"))
	{
		const FString Reason = Json->GetStringField(TEXT("reason"));
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run %s — %s"), *Status, *Reason);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
	}
	else
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run unexpected status=%s, code=%d"), *Status, Code);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, false);
	}
}

// ── My Rank ──────────────────────────────────────────────────

void UT66BackendSubsystem::FetchMyRank(
	const FString& Type,
	const FString& Time,
	const FString& Party,
	const FString& Difficulty)
{
	const FString RankKey = MakeMyRankCacheKey(Type, Time, Party, Difficulty);
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

	const FString Endpoint = FString::Printf(
		TEXT("/api/my-rank?type=%s&time=%s&party=%s&difficulty=%s"),
		*Type, *Time, *Party, *Difficulty);

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
	const FString& Difficulty)
{
	return FString::Printf(TEXT("%s_%s_%s_%s"), *Type, *Time, *Party, *Difficulty);
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
		LastAccountStatusReason.Reset();
		LastAccountAppealStatus.Reset();
		LastAccountRunSummaryId.Reset();
		OnAccountStatusComplete.Broadcast(false, TEXT(""));
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		LastAccountStatusReason.Reset();
		LastAccountAppealStatus.Reset();
		LastAccountRunSummaryId.Reset();
		OnAccountStatusComplete.Broadcast(false, TEXT(""));
		return;
	}

	const FString Restriction = Json->GetStringField(TEXT("restriction"));
	Json->TryGetStringField(TEXT("reason"), LastAccountStatusReason);
	Json->TryGetStringField(TEXT("appeal_status"), LastAccountAppealStatus);
	Json->TryGetStringField(TEXT("run_summary_id"), LastAccountRunSummaryId);
	UE_LOG(LogT66Backend, Log, TEXT("Backend: account-status = %s"), *Restriction);
	OnAccountStatusComplete.Broadcast(true, Restriction);
}

void UT66BackendSubsystem::SubmitRunReport(const FString& TargetEntryId, const FString& Reason, FString EvidenceUrl)
{
	if (!IsBackendConfigured() || !HasSteamTicket() || TargetEntryId.IsEmpty())
	{
		OnRunReportComplete.Broadcast(false, TEXT("Run report unavailable."));
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("target_entry_id"), TargetEntryId);
	if (!Reason.TrimStartAndEnd().IsEmpty())
	{
		Root->SetStringField(TEXT("reason"), Reason.TrimStartAndEnd());
	}
	if (!EvidenceUrl.TrimStartAndEnd().IsEmpty())
	{
		Root->SetStringField(TEXT("evidence_url"), EvidenceUrl.TrimStartAndEnd());
	}

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/report-run"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnRunReportResponseReceived);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::SubmitAppeal(const FString& Message, FString EvidenceUrl)
{
	if (!IsBackendConfigured() || !HasSteamTicket())
	{
		OnAppealSubmitComplete.Broadcast(false, TEXT("Appeals unavailable."));
		return;
	}

	FString TrimmedMessage = Message;
	TrimmedMessage.TrimStartAndEndInline();
	if (TrimmedMessage.IsEmpty())
	{
		OnAppealSubmitComplete.Broadcast(false, TEXT("Appeal message is required."));
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("message"), TrimmedMessage);
	if (!EvidenceUrl.TrimStartAndEnd().IsEmpty())
	{
		Root->SetStringField(TEXT("evidence_url"), EvidenceUrl.TrimStartAndEnd());
	}

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/submit-appeal"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnAppealResponseReceived);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::UpdateProofOfRun(const FString& EntryId, const FString& ProofUrl)
{
	if (!IsBackendConfigured() || !HasSteamTicket() || EntryId.IsEmpty())
	{
		OnProofOfRunComplete.Broadcast(false, TEXT("Proof of run unavailable."));
		return;
	}

	FString TrimmedUrl = ProofUrl;
	TrimmedUrl.TrimStartAndEndInline();
	if (TrimmedUrl.IsEmpty())
	{
		OnProofOfRunComplete.Broadcast(false, TEXT("Proof URL is required."));
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("entry_id"), EntryId);
	Root->SetStringField(TEXT("proof_url"), TrimmedUrl);

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("PUT"), TEXT("/api/proof-of-run"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnProofOfRunResponseReceived);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::SubmitBugReport(
	const FString& Message,
	int32 Stage,
	const FString& Difficulty,
	const FString& Party,
	const FString& HeroId)
{
	if (!IsBackendConfigured() || !HasSteamTicket())
	{
		OnBugReportComplete.Broadcast(false, TEXT("Bug reports unavailable."));
		return;
	}

	FString TrimmedMessage = Message;
	TrimmedMessage.TrimStartAndEndInline();
	if (TrimmedMessage.IsEmpty())
	{
		OnBugReportComplete.Broadcast(false, TEXT("Bug report message is required."));
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("message"), TrimmedMessage);

	TSharedPtr<FJsonObject> RunContext = MakeShared<FJsonObject>();
	if (Stage > 0)
	{
		RunContext->SetNumberField(TEXT("stage"), Stage);
	}
	if (!Difficulty.TrimStartAndEnd().IsEmpty())
	{
		RunContext->SetStringField(TEXT("difficulty"), Difficulty.TrimStartAndEnd());
	}
	if (!Party.TrimStartAndEnd().IsEmpty())
	{
		RunContext->SetStringField(TEXT("party_size"), Party.TrimStartAndEnd());
	}
	if (!HeroId.TrimStartAndEnd().IsEmpty())
	{
		RunContext->SetStringField(TEXT("hero_id"), HeroId.TrimStartAndEnd());
	}
	if (RunContext->Values.Num() > 0)
	{
		Root->SetObjectField(TEXT("run_context"), RunContext);
	}

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/bug-report"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnBugReportResponseReceived);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::FetchClientLaunchPolicy(int32 LocalSteamBuildId, const FString& SteamAppId, const FString& SteamBetaName)
{
	if (!IsBackendConfigured())
	{
		LastClientLaunchPolicyMessage.Reset();
		LastRequiredSteamBuildId = 0;
		LastLatestSteamBuildId = 0;
		bLastClientLaunchPolicyRequiresUpdate = false;
		ClientLaunchPolicyResponse.Broadcast(false, false, 0, 0, FString());
		return;
	}

	const FString EncodedAppId = FGenericPlatformHttp::UrlEncode(SteamAppId.IsEmpty() ? FString(TEXT("0")) : SteamAppId);
	const FString EncodedBetaName = FGenericPlatformHttp::UrlEncode(SteamBetaName.IsEmpty() ? FString(TEXT("default")) : SteamBetaName);
	const FString Endpoint = FString::Printf(
		TEXT("/api/client-config?app_id=%s&beta_name=%s&local_build_id=%d"),
		*EncodedAppId,
		*EncodedBetaName,
		LocalSteamBuildId);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), Endpoint);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnClientLaunchPolicyResponseReceived, LocalSteamBuildId);
	Request->ProcessRequest();
}

void UT66BackendSubsystem::OnRunReportResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		OnRunReportComplete.Broadcast(false, TEXT("Run report failed."));
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	const bool bParsed = FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid();
	const int32 Code = Response->GetResponseCode();
	if (Code == 200 && bParsed && Json->GetStringField(TEXT("status")) == TEXT("submitted"))
	{
		OnRunReportComplete.Broadcast(true, TEXT("Run reported."));
		return;
	}

	OnRunReportComplete.Broadcast(false, ExtractResponseMessage(Json, TEXT("Run report failed.")));
}

void UT66BackendSubsystem::OnAppealResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		OnAppealSubmitComplete.Broadcast(false, TEXT("Appeal submission failed."));
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	const bool bParsed = FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid();
	const int32 Code = Response->GetResponseCode();
	if (Code == 200 && bParsed && Json->GetStringField(TEXT("status")) == TEXT("submitted"))
	{
		OnAppealSubmitComplete.Broadcast(true, TEXT("Appeal submitted."));
		return;
	}

	OnAppealSubmitComplete.Broadcast(false, ExtractResponseMessage(Json, TEXT("Appeal submission failed.")));
}

void UT66BackendSubsystem::OnProofOfRunResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		OnProofOfRunComplete.Broadcast(false, TEXT("Proof of run update failed."));
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	const bool bParsed = FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid();
	const int32 Code = Response->GetResponseCode();
	if (Code == 200 && bParsed && Json->GetStringField(TEXT("status")) == TEXT("updated"))
	{
		OnProofOfRunComplete.Broadcast(true, TEXT("Proof of run updated."));
		return;
	}

	OnProofOfRunComplete.Broadcast(false, ExtractResponseMessage(Json, TEXT("Proof of run update failed.")));
}

void UT66BackendSubsystem::OnBugReportResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		OnBugReportComplete.Broadcast(false, TEXT("Bug report failed."));
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	const bool bParsed = FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid();
	const int32 Code = Response->GetResponseCode();
	if (Code == 200 && bParsed && Json->GetStringField(TEXT("status")) == TEXT("submitted"))
	{
		OnBugReportComplete.Broadcast(true, TEXT("Bug report submitted."));
		return;
	}

	OnBugReportComplete.Broadcast(false, ExtractResponseMessage(Json, TEXT("Bug report failed.")));
}

void UT66BackendSubsystem::OnClientLaunchPolicyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, int32 LocalSteamBuildId)
{
	if (!bConnectedSuccessfully || !Response.IsValid() || Response->GetResponseCode() != 200)
	{
		LastClientLaunchPolicyMessage.Reset();
		LastRequiredSteamBuildId = 0;
		LastLatestSteamBuildId = 0;
		bLastClientLaunchPolicyRequiresUpdate = false;
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: client-config unavailable for local build %d."), LocalSteamBuildId);
		ClientLaunchPolicyResponse.Broadcast(false, false, 0, 0, FString());
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		LastClientLaunchPolicyMessage.Reset();
		LastRequiredSteamBuildId = 0;
		LastLatestSteamBuildId = 0;
		bLastClientLaunchPolicyRequiresUpdate = false;
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: failed to parse client-config response."));
		ClientLaunchPolicyResponse.Broadcast(false, false, 0, 0, FString());
		return;
	}

	bool bUpdateRequired = false;
	Json->TryGetBoolField(TEXT("update_required"), bUpdateRequired);

	if (Json->HasTypedField<EJson::Number>(TEXT("required_steam_build_id")))
	{
		LastRequiredSteamBuildId = static_cast<int32>(Json->GetNumberField(TEXT("required_steam_build_id")));
	}
	else
	{
		LastRequiredSteamBuildId = 0;
	}

	if (Json->HasTypedField<EJson::Number>(TEXT("latest_steam_build_id")))
	{
		LastLatestSteamBuildId = static_cast<int32>(Json->GetNumberField(TEXT("latest_steam_build_id")));
	}
	else
	{
		LastLatestSteamBuildId = LastRequiredSteamBuildId;
	}

	LastClientLaunchPolicyMessage = ExtractResponseMessage(Json, FString());
	bLastClientLaunchPolicyRequiresUpdate = bUpdateRequired;

	UE_LOG(
		LogT66Backend,
		Log,
		TEXT("Backend: client-config local=%d required=%d latest=%d update_required=%s"),
		LocalSteamBuildId,
		LastRequiredSteamBuildId,
		LastLatestSteamBuildId,
		bUpdateRequired ? TEXT("true") : TEXT("false"));

	ClientLaunchPolicyResponse.Broadcast(
		true,
		bUpdateRequired,
		LastRequiredSteamBuildId,
		LastLatestSteamBuildId,
		LastClientLaunchPolicyMessage);
}

// ── Health Check ─────────────────────────────────────────────

void UT66BackendSubsystem::PingHealth()
{
	if (!IsBackendConfigured())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: health ping skipped — no URL configured."));
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
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: health ping FAILED (connection error)."));
		return;
	}

	const int32 Code = Response->GetResponseCode();
	UE_LOG(LogT66Backend, Log, TEXT("Backend: health ping response code=%d body=%s"),
		Code, *Response->GetContentAsString().Left(200));
}

void UT66BackendSubsystem::OnSendPartyInviteResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> Json;
	if (bConnectedSuccessfully && Response.IsValid())
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		FJsonSerializer::Deserialize(Reader, Json);
	}

	const bool bSuccess = bConnectedSuccessfully && Response.IsValid() && Response->GetResponseCode() == 200;
	const FString Message = ExtractResponseMessage(Json, bSuccess ? TEXT("Party invite sent.") : TEXT("Party invite failed."));
	FString InviteId;
	FString HostSteamId;
	FString TargetSteamId;
	FString HostLobbyId;
	FString HostAppId;
	if (Json.IsValid())
	{
		const TSharedPtr<FJsonObject>* InviteObject = nullptr;
		if (Json->TryGetObjectField(TEXT("invite"), InviteObject) && InviteObject && InviteObject->IsValid())
		{
			(*InviteObject)->TryGetStringField(TEXT("invite_id"), InviteId);
			(*InviteObject)->TryGetStringField(TEXT("host_steam_id"), HostSteamId);
			(*InviteObject)->TryGetStringField(TEXT("target_steam_id"), TargetSteamId);
			(*InviteObject)->TryGetStringField(TEXT("host_lobby_id"), HostLobbyId);
			(*InviteObject)->TryGetStringField(TEXT("host_app_id"), HostAppId);
		}
	}

	if (bSuccess)
	{
		UE_LOG(LogT66Backend, Log, TEXT("Backend: party invite send result: %s invite=%s"), *Message, *InviteId);
	}
	else
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: party invite send result: %s invite=%s"), *Message, *InviteId);
	}

	FT66MultiplayerDiagnosticContext Diagnostic;
	Diagnostic.EventName = TEXT("backend_invite_send");
	Diagnostic.Severity = bSuccess ? TEXT("info") : TEXT("error");
	Diagnostic.Message = Message;
	Diagnostic.InviteId = InviteId;
	Diagnostic.HostSteamId = HostSteamId;
	Diagnostic.TargetSteamId = TargetSteamId;
	Diagnostic.LobbyId = HostLobbyId;
	Diagnostic.SourceAppId = HostAppId;
	SubmitMultiplayerDiagnostic(Diagnostic);

	PartyInviteActionComplete.Broadcast(bSuccess, TEXT("send"), InviteId, Message);

	if (bSuccess)
	{
		PollPendingPartyInvites(true);
	}
}

void UT66BackendSubsystem::OnPendingPartyInvitesResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	bPartyInvitePollInFlight = false;
	const bool bRepollAfterCompletion = bPartyInvitePollRequestedWhileInFlight;
	bPartyInvitePollRequestedWhileInFlight = false;
	if (TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> PendingInviteRequest = PendingPartyInvitePollRequest.Pin();
		PendingInviteRequest.Get() == Request.Get())
	{
		PendingPartyInvitePollRequest.Reset();
	}

	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		UE_LOG(LogT66Backend, Verbose, TEXT("Backend: pending party invite poll failed."));
		if (bRepollAfterCompletion)
		{
			PollPendingPartyInvites(true);
		}
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: failed to parse pending party invites JSON."));
		if (bRepollAfterCompletion)
		{
			PollPendingPartyInvites(true);
		}
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: pending party invite poll returned HTTP %d."), Response->GetResponseCode());
		if (bRepollAfterCompletion)
		{
			PollPendingPartyInvites(true);
		}
		return;
	}

	TArray<FT66PartyInviteEntry> ParsedInvites = T66ParsePendingPartyInvites(Json);
	if (ParsedInvites.Num() > 0 && !T66ArePartyInviteArraysEqual(PendingPartyInvites, ParsedInvites))
	{
		const FT66PartyInviteEntry& Invite = ParsedInvites[0];
		FT66MultiplayerDiagnosticContext Diagnostic;
		Diagnostic.EventName = TEXT("backend_invite_received");
		Diagnostic.Severity = TEXT("info");
		Diagnostic.Message = TEXT("Pending party invite received.");
		Diagnostic.InviteId = Invite.InviteId;
		Diagnostic.HostSteamId = Invite.HostSteamId;
		Diagnostic.TargetSteamId = Invite.TargetSteamId;
		Diagnostic.LobbyId = Invite.HostLobbyId;
		Diagnostic.SourceAppId = Invite.HostAppId;
		SubmitMultiplayerDiagnostic(Diagnostic);
	}

	SetPendingPartyInvites(MoveTemp(ParsedInvites));

	if (bRepollAfterCompletion)
	{
		PollPendingPartyInvites(true);
	}
}

void UT66BackendSubsystem::OnRespondPartyInviteResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully,
	FString InviteId,
	FString Action)
{
	TSharedPtr<FJsonObject> Json;
	if (bConnectedSuccessfully && Response.IsValid())
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		FJsonSerializer::Deserialize(Reader, Json);
	}

	const bool bSuccess = bConnectedSuccessfully && Response.IsValid() && Response->GetResponseCode() == 200;
	const FString Message = ExtractResponseMessage(
		Json,
		bSuccess
			? FString::Printf(TEXT("Party invite %sed."), *Action)
			: FString::Printf(TEXT("Party invite %s failed."), *Action));

	if (bSuccess)
	{
		UE_LOG(LogT66Backend, Log, TEXT("Backend: party invite %s result for %s: %s"), *Action, *InviteId, *Message);
	}
	else
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: party invite %s result for %s: %s"), *Action, *InviteId, *Message);
	}

	FT66MultiplayerDiagnosticContext Diagnostic;
	Diagnostic.EventName = FString::Printf(TEXT("backend_invite_%s"), *Action);
	Diagnostic.Severity = bSuccess ? TEXT("info") : TEXT("error");
	Diagnostic.Message = Message;
	Diagnostic.InviteId = InviteId;
	SubmitMultiplayerDiagnostic(Diagnostic);

	// Broadcast the action completion before mutating the pending invite list.
	// The invite modal owns the join kickoff and unsubscribes when it closes, so
	// clearing the invite first can close the modal before the join callback runs.
	PartyInviteActionComplete.Broadcast(bSuccess, Action, InviteId, Message);

	if (bSuccess)
	{
		TArray<FT66PartyInviteEntry> UpdatedInvites = PendingPartyInvites;
		UpdatedInvites.RemoveAll([&InviteId](const FT66PartyInviteEntry& Invite)
		{
			return Invite.InviteId == InviteId;
		});
		SetPendingPartyInvites(MoveTemp(UpdatedInvites));
		PollPendingPartyInvites(true);
	}
	else
	{
		PollPendingPartyInvites(true);
	}
}

void UT66BackendSubsystem::OnClientDiagnosticsResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully,
	FString EventName,
	FString InviteId)
{
	if (bConnectedSuccessfully && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		UE_LOG(LogT66Backend, Verbose, TEXT("Backend: multiplayer diagnostic submitted event=%s invite=%s"), *EventName, *InviteId);
		return;
	}

	UE_LOG(
		LogT66Backend,
		Warning,
		TEXT("Backend: multiplayer diagnostic submit failed event=%s invite=%s code=%d"),
		*EventName,
		*InviteId,
		Response.IsValid() ? Response->GetResponseCode() : 0);
}

// ── Fetch Leaderboard ────────────────────────────────────────

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

UT66LeaderboardRunSummarySaveGame* UT66BackendSubsystem::ParseRunSummaryFromJson(
	const TSharedPtr<FJsonObject>& Json, UObject* Outer)
{
	if (!Json.IsValid())
	{
		return nullptr;
	}

	UT66LeaderboardRunSummarySaveGame* S = NewObject<UT66LeaderboardRunSummarySaveGame>(Outer);

	S->SchemaVersion = Json->HasField(TEXT("schema_version")) ? static_cast<int32>(Json->GetNumberField(TEXT("schema_version"))) : 6;
	S->EntryId = Json->HasField(TEXT("entry_id")) ? Json->GetStringField(TEXT("entry_id")) : FString();
	S->OwnerSteamId = Json->HasField(TEXT("steam_id")) ? Json->GetStringField(TEXT("steam_id")) : FString();
	S->StageReached = Json->HasField(TEXT("stage_reached")) ? static_cast<int32>(Json->GetNumberField(TEXT("stage_reached"))) : 1;
	S->Score = Json->HasField(TEXT("score")) ? static_cast<int32>(Json->GetNumberField(TEXT("score"))) : 0;
	S->RunDurationSeconds = Json->HasField(TEXT("time_seconds")) ? static_cast<float>(Json->GetNumberField(TEXT("time_seconds"))) : 0.f;

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
		S->AccuracyStat = St->HasField(TEXT("accuracy")) ? static_cast<int32>(St->GetNumberField(TEXT("accuracy"))) : 1;
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
				else if (Key == TEXT("LootCrate")) StatType = ET66SecondaryStatType::LootCrate;
				else if (Key == TEXT("DamageReduction")) StatType = ET66SecondaryStatType::DamageReduction;
				else if (Key == TEXT("EvasionChance")) StatType = ET66SecondaryStatType::EvasionChance;
				else if (Key == TEXT("Alchemy")) StatType = ET66SecondaryStatType::Alchemy;
				else if (Key == TEXT("Accuracy")) StatType = ET66SecondaryStatType::Accuracy;
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

	T66LeaderboardPacing::ExtractStageMarkers(S->EventLog, S->StagePacingPoints);

	const TArray<TSharedPtr<FJsonValue>>* StageSplitsArr = nullptr;
	if (Json->TryGetArrayField(TEXT("stage_splits_ms"), StageSplitsArr) && StageSplitsArr)
	{
		const int32 StageCount = StageSplitsArr->Num();
		const int32 StageStart = FMath::Max(1, S->StageReached - StageCount + 1);
		for (int32 SplitIndex = 0; SplitIndex < StageCount; ++SplitIndex)
		{
			double SplitMsValue = 0.0;
			if (!(*StageSplitsArr)[SplitIndex].IsValid() || !(*StageSplitsArr)[SplitIndex]->TryGetNumber(SplitMsValue))
			{
				continue;
			}

			FT66StagePacingPoint Point;
			Point.Stage = StageStart + SplitIndex;
			Point.Score = 0;
			Point.ElapsedSeconds = FMath::Max(0.0, SplitMsValue) / 1000.0;

			const int32 ExistingIndex = S->StagePacingPoints.IndexOfByPredicate([Point](const FT66StagePacingPoint& ExistingPoint)
			{
				return ExistingPoint.Stage == Point.Stage;
			});

			if (ExistingIndex != INDEX_NONE)
			{
				S->StagePacingPoints[ExistingIndex].ElapsedSeconds = Point.ElapsedSeconds;
			}
			else
			{
				S->StagePacingPoints.Add(Point);
			}
		}

		S->StagePacingPoints.Sort([](const FT66StagePacingPoint& A, const FT66StagePacingPoint& B)
		{
			return A.Stage < B.Stage;
		});
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
	if (Json->HasField(TEXT("proof_of_run_url")))
	{
		S->ProofOfRunUrl = Json->GetStringField(TEXT("proof_of_run_url"));
	}
	S->bProofOfRunLocked =
		Json->HasField(TEXT("proof_locked"))
		? Json->GetBoolField(TEXT("proof_locked"))
		: !S->ProofOfRunUrl.IsEmpty();

	return S;
}
