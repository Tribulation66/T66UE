// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBUpdateChecker.h"

#include "Http.h"
#include "JsonObjectConverter.h"
#include "ZibraVDBInfo.h"
#include "ZibraVDBNotifications.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

const FString FZibraVDBUpdateChecker::UpdateCheckURL =
	"https://generation.zibra.ai/api/pluginVersion?effect=zibravdb&engine=unreal&sku=pro";

void FZibraVDBUpdateChecker::CheckForUpdates(bool IsAutomatic)
{
	if (!IsAutomatic)
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("UpdateCheckRun",
			"Checking for updates..."));
	}

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	Request->SetVerb("GET");
	Request->SetURL(UpdateCheckURL);
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
	Request->OnProcessRequestComplete().BindStatic(&GetUpdateCheckResponse, IsAutomatic);
	Request->ProcessRequest();
}

void FZibraVDBUpdateChecker::GetUpdateCheckResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, bool IsAutomatic)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		if (!IsAutomatic)
		{
			FZibraVDBNotifications::AddNotification(LOCTEXT("UpdateCheckFailedNetworkError",
				"Update check failed. Request was unsuccessful. Please check your internet connection and firewall settings or try again later."));
		}
		return;
	}

	FString ResponseText = Response->GetContentAsString();
	FUpdateCheckResponse UpdateCheckResponse;
	bool bWasDeserializationSuccessful = FJsonObjectConverter::JsonObjectStringToUStruct(ResponseText, &UpdateCheckResponse);
	if (!bWasDeserializationSuccessful)
	{
		if (!IsAutomatic)
		{
			FZibraVDBNotifications::AddNotification(FText::Format(
				LOCTEXT("UpdateCheckFailedBadResponse",
					"Update check failed. Failed to parse response from the server. Please check your firewall settings or try again later. Server Response: {0}"),
				FText::FromString(ResponseText)));
		}
		return;
	}

	auto CurrentVersion = ParseVersion(FZibraVDBInfo::GetVersion());

	if (!CurrentVersion.IsSet())
	{
		if (!IsAutomatic)
		{
			FZibraVDBNotifications::AddNotification(
				FText::Format(LOCTEXT("UpdateCheckCantParseCurrentVersion",
								  "Can't check for updates for this version. Current version is {0}. Latest version is {1}."),
					FText::FromString(FZibraVDBInfo::GetVersion()), FText::FromString(UpdateCheckResponse.version)));
		}
		return;
	}

	auto LatestVersion = ParseVersion(UpdateCheckResponse.version);

	if (!LatestVersion.IsSet())
	{
		if (!IsAutomatic)
		{
			FZibraVDBNotifications::AddNotification(
				FText::Format(LOCTEXT("UpdateCheckCantParseLatestVersion",
								  "Error parsing server response. Current version is {0}. Latest version is {1}."),
					FText::FromString(FZibraVDBInfo::GetVersion()), FText::FromString(UpdateCheckResponse.version)));
		}
		return;
	}

	if (!IsLatest(*CurrentVersion, *LatestVersion))
	{
		FZibraVDBNotifications::AddNotification(
			FText::Format(LOCTEXT("UpdateCheckNewVersionIsAvailable",
							  "New version of ZibraVDB is available. Current version is {0}. Latest version is {1}."),
				FText::FromString(FZibraVDBInfo::GetVersion()), FText::FromString(UpdateCheckResponse.version)));
	}
	else
	{
		if (!IsAutomatic)
		{
			FZibraVDBNotifications::AddNotification(
				FText::Format(LOCTEXT("UpdateCheckUpToDate",
								  "Plugin is up to date. Current version is {0}."),
					FText::FromString(FZibraVDBInfo::GetVersion())));
		}
	}
}

TOptional<TArray<int32>> FZibraVDBUpdateChecker::ParseVersion(const FString& VersionString)
{
	TArray<int32> VersionArray;
	TArray<FString> VersionParts;
	VersionParts.Reserve(3);
	VersionString.ParseIntoArray(VersionParts, TEXT("."), true);

	if (VersionParts.Num() != 3)
	{
		return {};
	}

	VersionArray.Reserve(3);

	for (const auto& VersionPart : VersionParts)
	{
		uint32 VersionPartInt;
		if (!VersionPart.IsNumeric())
		{
			return {};
		}

		VersionPartInt = FCString::Atoi(*VersionPart);
		VersionArray.Add(VersionPartInt);
	}

	check(VersionArray.Num() == 3);

	return VersionArray;
}

bool FZibraVDBUpdateChecker::IsLatest(const TArray<int32>& CurrentVersion, const TArray<int32>& NewVersion)
{
	check(CurrentVersion.Num() == 3);
	check(NewVersion.Num() == 3);

	for (int32 i = 0; i < CurrentVersion.Num(); ++i)
	{
		if (CurrentVersion[i] < NewVersion[i])
		{
			return false;
		}
		else if (CurrentVersion[i] > NewVersion[i])
		{
			return true;
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
