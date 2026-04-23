// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendPrivate.h"

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
	FString HostSteamId;
	FString TargetSteamId;
	FString HostLobbyId;
	FString HostAppId;
	if (Json.IsValid())
	{
		const TSharedPtr<FJsonObject>* InviteObject = nullptr;
		if (Json->TryGetObjectField(TEXT("invite"), InviteObject) && InviteObject && InviteObject->IsValid())
		{
			(*InviteObject)->TryGetStringField(TEXT("host_steam_id"), HostSteamId);
			(*InviteObject)->TryGetStringField(TEXT("target_steam_id"), TargetSteamId);
			(*InviteObject)->TryGetStringField(TEXT("host_lobby_id"), HostLobbyId);
			(*InviteObject)->TryGetStringField(TEXT("host_app_id"), HostAppId);
		}
	}

	if (!InviteId.IsEmpty())
	{
		for (FT66PartyInviteEntry& PendingInvite : PendingPartyInvites)
		{
			if (PendingInvite.InviteId != InviteId)
			{
				continue;
			}

			if (!HostSteamId.IsEmpty())
			{
				PendingInvite.HostSteamId = HostSteamId;
			}

			if (!HostLobbyId.IsEmpty())
			{
				PendingInvite.HostLobbyId = HostLobbyId;
			}

			if (!HostAppId.IsEmpty())
			{
				PendingInvite.HostAppId = HostAppId;
			}

			if (!TargetSteamId.IsEmpty())
			{
				PendingInvite.TargetSteamId = TargetSteamId;
			}

			break;
		}
	}

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
	Diagnostic.HostSteamId = HostSteamId;
	Diagnostic.TargetSteamId = TargetSteamId;
	Diagnostic.LobbyId = HostLobbyId;
	Diagnostic.SourceAppId = HostAppId;
	Diagnostic.ExtraFields.Add(TEXT("http_code"), FString::FromInt(Response.IsValid() ? Response->GetResponseCode() : 0));
	Diagnostic.ExtraFields.Add(TEXT("has_host_steam_id"), HostSteamId.IsEmpty() ? TEXT("false") : TEXT("true"));
	Diagnostic.ExtraFields.Add(TEXT("has_host_lobby_id"), HostLobbyId.IsEmpty() ? TEXT("false") : TEXT("true"));
	Diagnostic.ExtraFields.Add(TEXT("has_host_app_id"), HostAppId.IsEmpty() ? TEXT("false") : TEXT("true"));
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
		if (!Action.Equals(TEXT("accept"), ESearchCase::IgnoreCase))
		{
			PollPendingPartyInvites(true);
		}
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

