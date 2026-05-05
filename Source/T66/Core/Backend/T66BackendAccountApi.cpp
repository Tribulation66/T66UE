// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendPrivate.h"

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

void UT66BackendSubsystem::SubmitStreamerRequest(const FString& CreatorLink, const FString& SteamId)
{
	if (!IsBackendConfigured())
	{
		OnStreamerRequestComplete.Broadcast(false, TEXT("Streamer requests unavailable."));
		OnStreamerRequestDataReady.Broadcast(false, TEXT("Streamer requests unavailable."));
		return;
	}

	FString TrimmedLink = CreatorLink;
	TrimmedLink.TrimStartAndEndInline();
	FString TrimmedSteamId = SteamId;
	TrimmedSteamId.TrimStartAndEndInline();

	if (TrimmedLink.IsEmpty() || TrimmedSteamId.IsEmpty())
	{
		OnStreamerRequestComplete.Broadcast(false, TEXT("Creator link and Steam ID are required."));
		OnStreamerRequestDataReady.Broadcast(false, TEXT("Creator link and Steam ID are required."));
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("creator_link"), TrimmedLink);
	Root->SetStringField(TEXT("steam_id"), TrimmedSteamId);

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(TEXT("POST"), TEXT("/api/streamer-request"));
	Request->SetContentAsString(Payload);
	Request->OnProcessRequestComplete().BindUObject(this, &UT66BackendSubsystem::OnStreamerRequestResponseReceived);
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

void UT66BackendSubsystem::OnStreamerRequestResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !Response.IsValid())
	{
		OnStreamerRequestComplete.Broadcast(false, TEXT("Streamer request failed."));
		OnStreamerRequestDataReady.Broadcast(false, TEXT("Streamer request failed."));
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	const bool bParsed = FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid();
	const int32 Code = Response->GetResponseCode();
	if ((Code == 200 || Code == 201) && bParsed && Json->GetStringField(TEXT("status")) == TEXT("submitted"))
	{
		OnStreamerRequestComplete.Broadcast(true, TEXT("Streamer request submitted."));
		OnStreamerRequestDataReady.Broadcast(true, TEXT("Streamer request submitted."));
		return;
	}

	const FString Message = ExtractResponseMessage(Json, TEXT("Streamer request failed."));
	OnStreamerRequestComplete.Broadcast(false, Message);
	OnStreamerRequestDataReady.Broadcast(false, Message);
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

