// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "HttpFwd.h"
#include "JsonObjectWrapper.h"

#include "ZibraVDBAnalyticsManager.generated.h"

USTRUCT()
struct FAnalyticsCommonData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Product;

	UPROPERTY()
	FString HardwareID;

	UPROPERTY()
	FString LicenseKey;

	UPROPERTY()
	FString OS;

	UPROPERTY()
	FString OSVersion;

	UPROPERTY()
	FString GPU;

	UPROPERTY()
	FString CurrentRHI;

	UPROPERTY()
	FString VRAMMB;

	UPROPERTY()
	FString PluginVersion;

	UPROPERTY()
	FString Engine;

	UPROPERTY()
	FString EngineVersion;
};

USTRUCT()
struct FAnalyticsEvent
{
	GENERATED_BODY()

	UPROPERTY()
	FString EventType;

	UPROPERTY()
	FJsonObjectWrapper Data;
};

USTRUCT()
struct FAnalyticsRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FAnalyticsCommonData CommonData;

	UPROPERTY()
	TArray<FAnalyticsEvent> Events;

	UPROPERTY()
	FString APIVersion = "2024.07.05";
};

class ZIBRAVDBEDITOR_API FZibraVDBAnalyticsManager
{
public:
	static void SubmitEvent(const FAnalyticsEvent& event);

private:
	static const FString AnalyticsURL;
	static const int32 SuccessCode;

	static FAnalyticsCommonData GetCommonData();
	static void SendEvents(const FAnalyticsRequest& Request);
	static void GetResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
