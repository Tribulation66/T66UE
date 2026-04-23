// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendPrivate.h"

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
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
		return;
	}

	if (!HasSteamTicket())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: cannot submit run — no Steam ticket set."));
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
		return;
	}

	const TSharedPtr<FJsonObject> LocalRunObject = T66BackendRunSerializer::BuildRunJsonObject(
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
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
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
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
			return;
		}

		const bool bCachedForHost = SessionSubsystem->SubmitLocalPartyRunSummaryToHost(RequestKey, LocalRunJson);
		if (!bCachedForHost)
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: skipping co-op submit because the local run summary could not be handed to the host."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
			return;
		}

		if (!SessionSubsystem->IsLocalPlayerPartyHost())
		{
			UE_LOG(LogT66Backend, Log, TEXT("Backend: forwarded co-op run summary to host; non-host client will not submit directly."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
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
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
		return true;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("display_name"), DisplayName);
	if (!RequestKey.IsEmpty())
	{
		Root->SetStringField(TEXT("submission_id"), RequestKey);
	}

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
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
			return true;
		}

		TArray<FT66LobbyPlayerInfo> LobbyProfiles;
		SessionSubsystem->GetCurrentLobbyProfiles(LobbyProfiles);
		if (LobbyProfiles.Num() < 2)
		{
			UE_LOG(LogT66Backend, Warning, TEXT("Backend: co-op submit aborted because fewer than two lobby profiles are available."));
			OnSubmitRunComplete.Broadcast(false, 0, 0, false);
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
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
					OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
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
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
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

void UT66BackendSubsystem::FetchCurrentDailyClimb()
{
	if (!IsBackendConfigured())
	{
		LastDailyClimbStatus = TEXT("backend_unconfigured");
		LastDailyClimbMessage = TEXT("Backend URL is not configured.");
		OnDailyClimbChallengeReady.Broadcast(TEXT("current"));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("GET"), TEXT("/api/daily/current"));
	if (HasSteamTicket())
	{
		SetAuthHeaders(Request);
	}
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnDailyClimbStatusResponseReceived, FString(TEXT("current")));
	Request->ProcessRequest();
}

void UT66BackendSubsystem::StartDailyClimbAttempt()
{
	if (!IsBackendConfigured())
	{
		LastDailyClimbStatus = TEXT("backend_unconfigured");
		LastDailyClimbMessage = TEXT("Backend URL is not configured.");
		OnDailyClimbChallengeReady.Broadcast(TEXT("start"));
		return;
	}

	if (!HasSteamTicket())
	{
		LastDailyClimbStatus = TEXT("missing_auth");
		LastDailyClimbMessage = TEXT("Steam authentication is required for Daily Climb.");
		OnDailyClimbChallengeReady.Broadcast(TEXT("start"));
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/daily/start"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(TEXT("{}"));
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnDailyClimbStatusResponseReceived, FString(TEXT("start")));
	Request->ProcessRequest();
}

void UT66BackendSubsystem::SubmitDailyClimbRun(
	const FString& DisplayName,
	UT66LeaderboardRunSummarySaveGame* Snapshot,
	const FString& ChallengeId,
	const FString& AttemptId,
	const FString& RequestKey)
{
	if (!IsBackendConfigured() || !HasSteamTicket() || !Snapshot || ChallengeId.IsEmpty() || AttemptId.IsEmpty())
	{
		OnDailyClimbSubmitDataReady.Broadcast(RequestKey, false, TEXT("invalid_daily_submit"), 0, 0);
		return;
	}

	const FString HeroId = Snapshot->HeroID.IsNone() ? FString() : Snapshot->HeroID.ToString();
	const FString CompanionId = Snapshot->CompanionID.IsNone() ? FString() : Snapshot->CompanionID.ToString();
	const int32 TimeMs = FMath::Max(0, FMath::RoundToInt(Snapshot->RunDurationSeconds * 1000.f));
	const TSharedPtr<FJsonObject> RunObject = T66BackendRunSerializer::BuildRunJsonObject(
		HeroId,
		CompanionId,
		Snapshot->Difficulty,
		ET66PartySize::Solo,
		Snapshot->StageReached,
		Snapshot->Score,
		TimeMs,
		Snapshot);

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("display_name"), DisplayName);
	if (!RequestKey.IsEmpty())
	{
		Root->SetStringField(TEXT("submission_id"), RequestKey);
	}
	Root->SetStringField(TEXT("challenge_id"), ChallengeId);
	Root->SetStringField(TEXT("attempt_id"), AttemptId);
	Root->SetObjectField(TEXT("run"), RunObject);

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/daily/submit"));
	SetAuthHeaders(Request);
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnDailyClimbSubmitResponseReceived, RequestKey);
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
			OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
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
		LastSubmitRunStatus = TEXT("connection_error");
		LastSubmitRunReason = TEXT("connection_error");
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
		return;
	}

	const int32 Code = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run failed to parse JSON. Code=%d"), Code);
		LastSubmitRunStatus = TEXT("parse_error");
		LastSubmitRunReason = TEXT("parse_error");
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
		return;
	}

	const FString Status = Json->GetStringField(TEXT("status"));
	LastSubmitRunStatus = Status;
	LastSubmitRunReason = Json->HasField(TEXT("reason")) ? Json->GetStringField(TEXT("reason")) : FString();
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
		const int32 PrimaryRankAlltime = ScoreRankAlltime > 0 ? ScoreRankAlltime : SpeedRunRankAlltime;
		const int32 PrimaryRankWeekly = ScoreRankWeekly > 0 ? ScoreRankWeekly : SpeedRunRankWeekly;
		const bool bPrimaryNewPB = bNewScorePB || bNewSpeedRunPB;

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
		OnSubmitRunDataReady.Broadcast(
			RequestKey,
			true,
			ScoreRankAlltime,
			ScoreRankWeekly,
			SpeedRunRankAlltime,
			SpeedRunRankWeekly,
			bNewScorePB,
			bNewSpeedRunPB);
	}
	else if (Status == TEXT("unranked"))
	{
		const FString ReasonCode = Json->HasField(TEXT("reason_code")) ? Json->GetStringField(TEXT("reason_code")) : TEXT("unranked");
		const FString Reason = Json->HasField(TEXT("reason")) ? Json->GetStringField(TEXT("reason")) : TEXT("Run is not eligible for ranked submission.");
		LastSubmitRunReason = Reason;
		UE_LOG(LogT66Backend, Log, TEXT("Backend: submit-run unranked (%s) — %s"), *ReasonCode, *Reason);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
	}
	else if (Status == TEXT("flagged") || Status == TEXT("banned") || Status == TEXT("suspended"))
	{
		const FString Reason = Json->GetStringField(TEXT("reason"));
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run %s — %s"), *Status, *Reason);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
	}
	else
	{
		UE_LOG(LogT66Backend, Warning, TEXT("Backend: submit-run unexpected status=%s, code=%d"), *Status, Code);
		OnSubmitRunComplete.Broadcast(false, 0, 0, false);
		OnSubmitRunDataReady.Broadcast(RequestKey, false, 0, 0, 0, 0, false, false);
	}
}

void UT66BackendSubsystem::OnDailyClimbStatusResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully,
	FString RequestTag)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		LastDailyClimbStatus = TEXT("connection_error");
		LastDailyClimbMessage = TEXT("Daily Climb request failed.");
		OnDailyClimbChallengeReady.Broadcast(RequestTag);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		LastDailyClimbStatus = TEXT("parse_error");
		LastDailyClimbMessage = TEXT("Daily Climb response was invalid JSON.");
		OnDailyClimbChallengeReady.Broadcast(RequestTag);
		return;
	}

	if (!Json->TryGetStringField(TEXT("status"), LastDailyClimbStatus))
	{
		LastDailyClimbStatus = TEXT("ok");
	}
	LastDailyClimbMessage = ExtractResponseMessage(Json, LastDailyClimbStatus);

	FT66DailyClimbChallengeData ParsedChallenge;
	if (T66BackendDailyClimbJson::ParseChallengeData(Json, ParsedChallenge))
	{
		CachedDailyClimbChallenge = MoveTemp(ParsedChallenge);
	}

	OnDailyClimbChallengeReady.Broadcast(RequestTag);
}

void UT66BackendSubsystem::OnDailyClimbSubmitResponseReceived(
	FHttpRequestPtr Request,
	FHttpResponsePtr Response,
	bool bConnectedSuccessfully,
	FString RequestKey)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		LastDailyClimbStatus = TEXT("connection_error");
		LastDailyClimbMessage = TEXT("Daily Climb submit failed.");
		OnDailyClimbSubmitDataReady.Broadcast(RequestKey, false, LastDailyClimbStatus, 0, 0);
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		LastDailyClimbStatus = TEXT("parse_error");
		LastDailyClimbMessage = TEXT("Daily Climb submit returned invalid JSON.");
		OnDailyClimbSubmitDataReady.Broadcast(RequestKey, false, LastDailyClimbStatus, 0, 0);
		return;
	}

	if (!Json->TryGetStringField(TEXT("status"), LastDailyClimbStatus))
	{
		LastDailyClimbStatus = TEXT("unknown");
	}
	LastDailyClimbMessage = ExtractResponseMessage(Json, LastDailyClimbStatus);

	double DailyRankValue = 0.0;
	double CouponValue = 0.0;
	(void)Json->TryGetNumberField(TEXT("score_rank_daily"), DailyRankValue);
	(void)Json->TryGetNumberField(TEXT("coupons_awarded"), CouponValue);
	const int32 DailyRank = static_cast<int32>(DailyRankValue);
	const int32 CouponsAwarded = static_cast<int32>(CouponValue);

	if (CachedDailyClimbChallenge.IsValid()
		&& (LastDailyClimbStatus == TEXT("accepted")
			|| LastDailyClimbStatus == TEXT("unranked")
			|| LastDailyClimbStatus == TEXT("flagged")
			|| LastDailyClimbStatus == TEXT("banned")
			|| LastDailyClimbStatus == TEXT("suspended")))
	{
		CachedDailyClimbChallenge.AttemptState = TEXT("completed");
	}

	const bool bParsedResponse =
		LastDailyClimbStatus == TEXT("accepted")
		|| LastDailyClimbStatus == TEXT("unranked")
		|| LastDailyClimbStatus == TEXT("flagged")
		|| LastDailyClimbStatus == TEXT("banned")
		|| LastDailyClimbStatus == TEXT("suspended");
	OnDailyClimbSubmitDataReady.Broadcast(RequestKey, bParsedResponse, LastDailyClimbStatus, DailyRank, CouponsAwarded);
}

// ── My Rank ──────────────────────────────────────────────────

