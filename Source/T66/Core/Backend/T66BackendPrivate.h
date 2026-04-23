// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Core/T66BackendSubsystem.h"
#include "Core/Backend/T66BackendRunSerializer.h"
#include "Core/Backend/T66BackendRunSummaryParser.h"
#include "Core/Backend/T66BackendDailyClimbJson.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66LeaderboardPacingUtils.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66SteamHelper.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/DateTime.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66Backend, Log, All);

extern TAutoConsoleVariable<float> CVarT66PartyInvitePollMinIntervalSeconds;
extern TAutoConsoleVariable<float> CVarT66PartyInvitePollTickerIntervalSeconds;

inline void T66SetJsonStringIfNotEmpty(const TSharedPtr<FJsonObject>& Json, const FString& FieldName, const FString& Value)
{
	if (Json.IsValid() && !Value.IsEmpty())
	{
		Json->SetStringField(FieldName, Value);
	}
}

inline bool T66ArePartyInviteArraysEqual(const TArray<FT66PartyInviteEntry>& A, const TArray<FT66PartyInviteEntry>& B)
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

inline TArray<FT66PartyInviteEntry> T66ParsePendingPartyInvites(const TSharedPtr<FJsonObject>& Json)
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

inline bool T66SerializeJsonObjectToString(const TSharedPtr<FJsonObject>& Json, FString& OutJsonString)
{
	return T66BackendRunSerializer::SerializeJsonObjectToString(Json, OutJsonString);
}

inline bool T66DeserializeJsonObjectString(const FString& JsonString, TSharedPtr<FJsonObject>& OutJson)
{
	return T66BackendRunSerializer::DeserializeJsonObjectString(JsonString, OutJson);
}
