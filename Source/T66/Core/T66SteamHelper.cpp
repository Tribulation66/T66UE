// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SteamHelper.h"
#include "Core/T66BackendSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

void UT66SteamHelper::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!SteamAPI_Init())
	{
		UE_LOG(LogTemp, Warning, TEXT("SteamHelper: SteamAPI not initialized. Online features unavailable."));
		return;
	}

	ISteamUser* SUser = SteamUser();
	if (!SUser)
	{
		UE_LOG(LogTemp, Warning, TEXT("SteamHelper: ISteamUser is null."));
		return;
	}

	// Cache local identity
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
