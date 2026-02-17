// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SteamHelper.h"
#include "Core/T66BackendSubsystem.h"
#include "Misc/ConfigCacheIni.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

static void T66_SetDevTicket(const TArray<FString>& Args, UWorld* World)
{
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend) return;

	FString TicketVal = (Args.Num() > 0) ? Args[0] : TEXT("dev_player_1");
	Backend->SetSteamTicketHex(TicketVal);
	UE_LOG(LogTemp, Log, TEXT("SetTicket: manually set backend ticket to '%s'"), *TicketVal);
}

static FAutoConsoleCommandWithWorldAndArgs T66SetTicketCommand(
	TEXT("setticket"),
	TEXT("Set the backend auth ticket manually (e.g. setticket dev_player_1). For local dev without Steam."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_SetDevTicket)
);

void UT66SteamHelper::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Try real Steam first
	if (SteamAPI_Init())
	{
		ISteamUser* SUser = SteamUser();
		if (SUser)
		{
			const CSteamID LocalId = SUser->GetSteamID();
			LocalSteamIdStr = FString::Printf(TEXT("%llu"), LocalId.ConvertToUint64());

			ISteamFriends* Friends = SteamFriends();
			if (Friends)
			{
				LocalDisplayName = FString(UTF8_TO_TCHAR(Friends->GetPersonaName()));
			}

			UE_LOG(LogTemp, Log, TEXT("SteamHelper: local SteamID=%s Name=%s"), *LocalSteamIdStr, *LocalDisplayName);

			CollectFriendsList();
			ObtainTicket();
			return;
		}
	}

	// Steam not available — check for dev ticket in DefaultGame.ini [T66.Online] DevTicket
	FString DevTicket;
	GConfig->GetString(TEXT("T66.Online"), TEXT("DevTicket"), DevTicket, GGameIni);

	if (!DevTicket.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("SteamHelper: Steam not available. Using dev ticket '%s' from config."), *DevTicket);

		TicketHex = DevTicket;
		LocalSteamIdStr = TEXT("76561100000000001");
		LocalDisplayName = TEXT("DevPlayer");
		bSteamReady = true;

		// Dev-mode friend list: add a test friend so the Friends leaderboard tab has data.
		FString DevFriends;
		GConfig->GetString(TEXT("T66.Online"), TEXT("DevFriendIds"), DevFriends, GGameIni);
		if (!DevFriends.IsEmpty())
		{
			TArray<FString> Parts;
			DevFriends.ParseIntoArray(Parts, TEXT(","));
			for (const FString& Part : Parts)
			{
				FString Trimmed = Part.TrimStartAndEnd();
				if (!Trimmed.IsEmpty())
				{
					FriendSteamIds.Add(Trimmed);
				}
			}
			UE_LOG(LogTemp, Log, TEXT("SteamHelper: dev friends loaded: %d IDs"), FriendSteamIds.Num());
		}

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
			{
				Backend->SetSteamTicketHex(TicketHex);
			}
		}

		OnSteamTicketReady.Broadcast(true, TicketHex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SteamHelper: Steam not available and no DevTicket configured. Online features disabled."));
	}
}

void UT66SteamHelper::Deinitialize()
{
	// Cancel the ticket if we have one
	if (TicketHandle != 0)
	{
		ISteamUser* User = SteamUser();
		if (User)
		{
			User->CancelAuthTicket(TicketHandle);
		}
		TicketHandle = 0;
	}

	Super::Deinitialize();
}

void UT66SteamHelper::RequestNewTicket()
{
	// Cancel old ticket
	if (TicketHandle != 0)
	{
		ISteamUser* User = SteamUser();
		if (User)
		{
			User->CancelAuthTicket(TicketHandle);
		}
		TicketHandle = 0;
	}

	TicketHex.Reset();
	bSteamReady = false;
	ObtainTicket();
}

void UT66SteamHelper::ObtainTicket()
{
	ISteamUser* User = SteamUser();
	if (!User)
	{
		UE_LOG(LogTemp, Warning, TEXT("SteamHelper: cannot obtain ticket — ISteamUser null."));
		OnSteamTicketReady.Broadcast(false, FString());
		return;
	}

	uint8 TicketBuffer[1024];
	uint32 TicketLen = 0;

	// GetAuthSessionTicket returns a handle; the ticket data is copied into TicketBuffer.
	TicketHandle = User->GetAuthSessionTicket(TicketBuffer, sizeof(TicketBuffer), &TicketLen, nullptr);

	if (TicketHandle == 0 || TicketLen == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SteamHelper: GetAuthSessionTicket failed."));
		OnSteamTicketReady.Broadcast(false, FString());
		return;
	}

	// Hex-encode the ticket bytes
	TicketHex.Reset();
	TicketHex.Reserve(TicketLen * 2);
	for (uint32 i = 0; i < TicketLen; ++i)
	{
		TicketHex += FString::Printf(TEXT("%02x"), TicketBuffer[i]);
	}

	bSteamReady = true;
	UE_LOG(LogTemp, Log, TEXT("SteamHelper: ticket obtained (%d bytes, %d hex chars)."), TicketLen, TicketHex.Len());

	// Wire into BackendSubsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->SetSteamTicketHex(TicketHex);
		}
	}

	OnSteamTicketReady.Broadcast(true, TicketHex);
}

void UT66SteamHelper::CollectFriendsList()
{
	FriendSteamIds.Reset();

	ISteamFriends* Friends = SteamFriends();
	if (!Friends) return;

	const int32 Count = Friends->GetFriendCount(k_EFriendFlagImmediate);
	FriendSteamIds.Reserve(Count);

	for (int32 i = 0; i < Count; ++i)
	{
		const CSteamID FriendId = Friends->GetFriendByIndex(i, k_EFriendFlagImmediate);
		FriendSteamIds.Add(FString::Printf(TEXT("%llu"), FriendId.ConvertToUint64()));
	}

	UE_LOG(LogTemp, Log, TEXT("SteamHelper: collected %d friends."), FriendSteamIds.Num());
}
