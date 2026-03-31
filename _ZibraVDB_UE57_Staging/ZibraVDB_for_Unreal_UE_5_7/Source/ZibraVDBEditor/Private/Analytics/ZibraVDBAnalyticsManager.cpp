// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBAnalyticsManager.h"

#include "Http.h"
#include "JsonObjectConverter.h"
#include "ZibraLicensingManager.h"
#include "ZibraVDBDiagnosticsInfo.h"

DECLARE_LOG_CATEGORY_CLASS(LogZibraVDBAnalytics, Log, All);

void FZibraVDBAnalyticsManager::SubmitEvent(const FAnalyticsEvent& event)
{
	UE_LOG(LogZibraVDBAnalytics, Verbose, TEXT("Received event \"%s\""), *event.EventType);

	FAnalyticsRequest Request;
	Request.CommonData = GetCommonData();
	Request.Events = {event};
	SendEvents(Request);
}

const FString FZibraVDBAnalyticsManager::AnalyticsURL = "https://analytics.zibra.ai/api/usageAnalytics";
const int32 FZibraVDBAnalyticsManager::SuccessCode = 201;

FAnalyticsCommonData FZibraVDBAnalyticsManager::GetCommonData()
{
	FZibraVDBDiagnosticsInfo DiagnosticsInfo = FZibraVDBDiagnosticsInfo::CollectInfo();

	FAnalyticsCommonData CommonData;
	CommonData.Product = "zibravdb";
	CommonData.HardwareID = FZibraLicensingManager::GetInstance().GetHardwareId();
	CommonData.LicenseKey = FZibraLicensingManager::GetInstance().GetLicenseKey();
	CommonData.OS = DiagnosticsInfo.OS;
	CommonData.OSVersion = DiagnosticsInfo.OSVersion;
	CommonData.GPU = DiagnosticsInfo.GPU;
	CommonData.CurrentRHI = DiagnosticsInfo.CurrentRHI;
	CommonData.VRAMMB = DiagnosticsInfo.VRAMMB;
	CommonData.PluginVersion = DiagnosticsInfo.Version;
	CommonData.Engine = "unreal";
	CommonData.EngineVersion = DiagnosticsInfo.UnrealVersion;
	return CommonData;
}

void FZibraVDBAnalyticsManager::SendEvents(const FAnalyticsRequest& Request)
{
	FString RequestJson;
	bool bWasSerializationSuccessful = FJsonObjectConverter::UStructToJsonObjectString(Request, RequestJson);
	if (!bWasSerializationSuccessful)
	{
		UE_LOG(LogZibraVDBAnalytics, Verbose, TEXT("Failed to serialize analytics request to JSON."));
		return;
	}

	UE_LOG(LogZibraVDBAnalytics, Verbose, TEXT("Sending %d events"), Request.Events.Num());
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(AnalyticsURL);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(RequestJson);
	HttpRequest->OnProcessRequestComplete().BindStatic(&FZibraVDBAnalyticsManager::GetResponse);
	HttpRequest->ProcessRequest();
}

void FZibraVDBAnalyticsManager::GetResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogZibraVDBAnalytics, Verbose, TEXT("Failed to send."));
		return;
	}

	if (Response->GetResponseCode() != SuccessCode)
	{
		UE_LOG(
			LogZibraVDBAnalytics, Verbose, TEXT("Failed to send. Response code: %d"), Response->GetResponseCode());
		return;
	}

	UE_LOG(LogZibraVDBAnalytics, Verbose, TEXT("Sent successfully"));
}
