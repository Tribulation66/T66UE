// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SteamHelper.h"
#include "Core/T66BackendSubsystem.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/Texture2D.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY_STATIC(LogT66Steam, Log, All);

class FT66SteamJoinRequestBridge
{
public:
	explicit FT66SteamJoinRequestBridge(UT66SteamHelper& InOwner)
		: Owner(InOwner)
	{
		LobbyJoinRequestedCallback.Register(this, &FT66SteamJoinRequestBridge::OnLobbyJoinRequested);
		RichPresenceJoinRequestedCallback.Register(this, &FT66SteamJoinRequestBridge::OnRichPresenceJoinRequested);
		WebApiTicketResponseCallback.Register(this, &FT66SteamJoinRequestBridge::OnWebApiTicketResponse);
	}

	~FT66SteamJoinRequestBridge()
	{
		Unregister();
	}

	void Unregister()
	{
		LobbyJoinRequestedCallback.Unregister();
		RichPresenceJoinRequestedCallback.Unregister();
		WebApiTicketResponseCallback.Unregister();
	}

private:
	UT66SteamHelper& Owner;

	STEAM_CALLBACK_MANUAL(FT66SteamJoinRequestBridge, OnLobbyJoinRequested, GameLobbyJoinRequested_t, LobbyJoinRequestedCallback);
	STEAM_CALLBACK_MANUAL(FT66SteamJoinRequestBridge, OnRichPresenceJoinRequested, GameRichPresenceJoinRequested_t, RichPresenceJoinRequestedCallback);
	STEAM_CALLBACK_MANUAL(FT66SteamJoinRequestBridge, OnWebApiTicketResponse, GetTicketForWebApiResponse_t, WebApiTicketResponseCallback);
};

namespace
{

	static int32 T66ReadConfiguredSteamAppId()
	{
		int32 ConfiguredAppId = 0;
		GConfig->GetInt(TEXT("OnlineSubsystemSteam"), TEXT("SteamAppId"), ConfiguredAppId, GEngineIni);
		return ConfiguredAppId;
	}

	static FString T66PresenceFromPersonaState(EPersonaState PersonaState)
	{
		switch (PersonaState)
		{
		case k_EPersonaStateOnline:
			return TEXT("Online");
		case k_EPersonaStateBusy:
			return TEXT("Busy");
		case k_EPersonaStateAway:
			return TEXT("Away");
		case k_EPersonaStateSnooze:
			return TEXT("Snooze");
		case k_EPersonaStateLookingToTrade:
			return TEXT("Looking to trade");
		case k_EPersonaStateLookingToPlay:
			return TEXT("Looking to play");
		case k_EPersonaStateOffline:
		default:
			return TEXT("Offline");
		}
	}

	static FString T66ResolveJoinSteamId(const char* ConnectStringUtf8, const CSteamID& FallbackFriendId)
	{
		const FString ConnectString = ConnectStringUtf8 ? FString(UTF8_TO_TCHAR(ConnectStringUtf8)) : FString();
		if (ConnectString.StartsWith(TEXT("t66party:"), ESearchCase::IgnoreCase))
		{
			return ConnectString.RightChop(9);
		}

		if (!ConnectString.IsEmpty())
		{
			return ConnectString;
		}

		return FString::Printf(TEXT("%llu"), FallbackFriendId.ConvertToUint64());
	}

	static int32 T66ResolveAvatarImageHandle(ISteamFriends* Friends, const CSteamID& SteamId)
	{
		if (!Friends)
		{
			return 0;
		}

		int32 ImageHandle = Friends->GetLargeFriendAvatar(SteamId);
		if (ImageHandle <= 0)
		{
			ImageHandle = Friends->GetMediumFriendAvatar(SteamId);
		}
		if (ImageHandle <= 0)
		{
			ImageHandle = Friends->GetSmallFriendAvatar(SteamId);
		}
		if (ImageHandle <= 0)
		{
			Friends->RequestUserInformation(SteamId, false);
			ImageHandle = Friends->GetLargeFriendAvatar(SteamId);
			if (ImageHandle <= 0)
			{
				ImageHandle = Friends->GetMediumFriendAvatar(SteamId);
			}
			if (ImageHandle <= 0)
			{
				ImageHandle = Friends->GetSmallFriendAvatar(SteamId);
			}
		}
		return ImageHandle;
	}

	static bool T66ParseSteamIdString(const FString& SteamIdString, CSteamID& OutSteamId)
	{
		if (SteamIdString.IsEmpty())
		{
			return false;
		}

		uint64 NumericSteamId = 0;
		if (!LexTryParseString(NumericSteamId, *SteamIdString) || NumericSteamId == 0)
		{
			return false;
		}

		OutSteamId = CSteamID(NumericSteamId);
		return OutSteamId.IsValid();
	}
}

void FT66SteamJoinRequestBridge::OnLobbyJoinRequested(GameLobbyJoinRequested_t* Param)
{
	if (!Param)
	{
		return;
	}

	Owner.HandleSteamLobbyJoinRequested(
		FString::Printf(TEXT("%llu"), Param->m_steamIDFriend.ConvertToUint64()),
		FString::Printf(TEXT("%llu"), Param->m_steamIDLobby.ConvertToUint64()));
}

void FT66SteamJoinRequestBridge::OnRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* Param)
{
	if (!Param)
	{
		return;
	}

	Owner.HandleSteamJoinRequested(T66ResolveJoinSteamId(Param->m_rgchConnect, Param->m_steamIDFriend));
}

void FT66SteamJoinRequestBridge::OnWebApiTicketResponse(GetTicketForWebApiResponse_t* Param)
{
	if (!Param)
	{
		return;
	}

	Owner.HandleWebApiTicketReady(
		static_cast<uint32>(Param->m_hAuthTicket),
		Param->m_eResult == k_EResultOK,
		Param->m_rgubTicket,
		Param->m_cubTicket);
}

static void T66_SetDevTicket(const TArray<FString>& Args, UWorld* World)
{
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend) return;

	FString TicketVal = (Args.Num() > 0) ? Args[0] : TEXT("dev_player_1");
	Backend->SetSteamTicketHex(TicketVal);
	UE_LOG(LogT66Steam, Log, TEXT("SetTicket: manually set backend ticket to '%s'"), *TicketVal);
}

static FAutoConsoleCommandWithWorldAndArgs T66SetTicketCommand(
	TEXT("setticket"),
	TEXT("Set the backend auth ticket manually (e.g. setticket dev_player_1). For local dev without Steam."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_SetDevTicket)
);

FString UT66SteamHelper::GetActiveSteamAppId() const
{
	if (ISteamUtils* SteamUtilsApi = SteamUtils())
	{
		return FString::Printf(TEXT("%u"), SteamUtilsApi->GetAppID());
	}

	const int32 ConfiguredAppId = T66ReadConfiguredSteamAppId();
	return ConfiguredAppId > 0 ? FString::FromInt(ConfiguredAppId) : FString();
}

int32 UT66SteamHelper::GetActiveSteamBuildId() const
{
	if (ISteamApps* Apps = SteamApps())
	{
		return Apps->GetAppBuildId();
	}

	return 0;
}

FString UT66SteamHelper::GetCurrentSteamBetaName() const
{
	ISteamApps* Apps = SteamApps();
	if (!Apps)
	{
		return FString();
	}

	char BetaName[256] = {};
	if (!Apps->GetCurrentBetaName(BetaName, UE_ARRAY_COUNT(BetaName)))
	{
		return FString();
	}

	return FString(UTF8_TO_TCHAR(BetaName));
}

void UT66SteamHelper::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LocalAvatarTexture = nullptr;
	FriendAvatarTextures.Reset();

	// Try real Steam first
	if (SteamAPI_Init())
	{
		if (!SteamJoinRequestBridge)
		{
			SteamJoinRequestBridge = new FT66SteamJoinRequestBridge(*this);
		}
		RefreshLocalSteamIdentity();
		CollectFriendsList();
		ObtainTicket();
		return;
	}

	// Steam not available — in non-shipping builds only, check for dev ticket in
	// DefaultGame.ini [T66.Online] DevTicket so local backend work can continue.
	UE_LOG(
		LogT66Steam,
		Warning,
		TEXT("SteamHelper: SteamAPI_Init failed for Steam App ID %d. For local standalone Development runs, launch with a matching steam_appid.txt beside the executable or through the Steam-distributed build for the real app."),
		T66ReadConfiguredSteamAppId());

#if !UE_BUILD_SHIPPING
	FString DevTicket;
	GConfig->GetString(TEXT("T66.Online"), TEXT("DevTicket"), DevTicket, GGameIni);

	if (!DevTicket.IsEmpty())
	{
		UE_LOG(LogT66Steam, Log, TEXT("SteamHelper: Steam not available. Using dev ticket '%s' from config."), *DevTicket);

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
					FT66SteamFriendInfo& FriendInfo = FriendInfos.AddDefaulted_GetRef();
					FriendInfo.SteamId = Trimmed;
					FriendInfo.DisplayName = FString::Printf(TEXT("Friend %d"), FriendInfos.Num());
					FriendInfo.PresenceText = TEXT("Online");
					FriendInfo.bOnline = true;
				}
			}
			UE_LOG(LogT66Steam, Log, TEXT("SteamHelper: dev friends loaded: %d IDs"), FriendSteamIds.Num());
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
		UE_LOG(LogT66Steam, Warning, TEXT("SteamHelper: Steam not available and no DevTicket configured. Online features disabled."));
	}
#else
	UE_LOG(LogT66Steam, Warning, TEXT("SteamHelper: Steam not available in shipping build. Online features disabled."));
#endif
}

void UT66SteamHelper::Deinitialize()
{
	if (SteamJoinRequestBridge)
	{
		SteamJoinRequestBridge->Unregister();
		delete SteamJoinRequestBridge;
		SteamJoinRequestBridge = nullptr;
	}

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

void UT66SteamHelper::RefreshSteamPresence()
{
	if (!bSteamReady && TicketHex.IsEmpty())
	{
		return;
	}

	RefreshLocalSteamIdentity();
	CollectFriendsList();
}

UTexture2D* UT66SteamHelper::GetAvatarTextureForSteamId(const FString& SteamId) const
{
	if (SteamId.IsEmpty())
	{
		return nullptr;
	}

	if (SteamId == LocalSteamIdStr)
	{
		return LocalAvatarTexture;
	}

	const TObjectPtr<UTexture2D>* FoundTexture = FriendAvatarTextures.Find(SteamId);
	return FoundTexture ? FoundTexture->Get() : nullptr;
}

bool UT66SteamHelper::IsFriendPlayingActiveApp(const FString& SteamId) const
{
	FString FriendAppId;
	return TryGetFriendCurrentGame(SteamId, &FriendAppId, nullptr) && FriendAppId == GetActiveSteamAppId();
}

bool UT66SteamHelper::TryGetFriendCurrentGame(const FString& SteamId, FString* OutAppId, FString* OutLobbyId) const
{
	if (OutAppId)
	{
		OutAppId->Reset();
	}
	if (OutLobbyId)
	{
		OutLobbyId->Reset();
	}

	ISteamFriends* Friends = SteamFriends();
	if (!Friends)
	{
		return false;
	}

	CSteamID FriendSteamId;
	if (!T66ParseSteamIdString(SteamId, FriendSteamId))
	{
		return false;
	}

	FriendGameInfo_t FriendGameInfo;
	if (!Friends->GetFriendGamePlayed(FriendSteamId, &FriendGameInfo))
	{
		return false;
	}

	if (OutAppId)
	{
		*OutAppId = FString::Printf(TEXT("%u"), FriendGameInfo.m_gameID.AppID());
	}
	if (OutLobbyId && FriendGameInfo.m_steamIDLobby.IsValid())
	{
		*OutLobbyId = FString::Printf(TEXT("%llu"), FriendGameInfo.m_steamIDLobby.ConvertToUint64());
	}

	return true;
}

void UT66SteamHelper::RefreshLocalSteamIdentity()
{
	ISteamUser* SUser = SteamUser();
	if (!SUser)
	{
		return;
	}

	const CSteamID LocalId = SUser->GetSteamID();
	LocalSteamIdStr = FString::Printf(TEXT("%llu"), LocalId.ConvertToUint64());

	ISteamFriends* Friends = SteamFriends();
	if (!Friends)
	{
		return;
	}

	LocalDisplayName = FString(UTF8_TO_TCHAR(Friends->GetPersonaName()));
	CacheAvatarForSteamId(LocalSteamIdStr, T66ResolveAvatarImageHandle(Friends, LocalId));
	LocalAvatarTexture = FriendAvatarTextures.FindRef(LocalSteamIdStr);
	UE_LOG(LogT66Steam, Verbose, TEXT("SteamHelper: refreshed local Steam identity SteamID=%s Name=%s"), *LocalSteamIdStr, *LocalDisplayName);
}

void UT66SteamHelper::ObtainTicket()
{
	ISteamUser* User = SteamUser();
	if (!User)
	{
		UE_LOG(LogT66Steam, Warning, TEXT("SteamHelper: cannot obtain ticket — ISteamUser null."));
		OnSteamTicketReady.Broadcast(false, FString());
		return;
	}

	TicketHandle = static_cast<uint32>(User->GetAuthTicketForWebApi("t66-backend"));
	if (TicketHandle == 0)
	{
		UE_LOG(LogT66Steam, Warning, TEXT("SteamHelper: GetAuthTicketForWebApi failed to start."));
		OnSteamTicketReady.Broadcast(false, FString());
		return;
	}

	UE_LOG(LogT66Steam, Log, TEXT("SteamHelper: requested Web API ticket handle %u."), TicketHandle);
}

void UT66SteamHelper::CollectFriendsList()
{
	FriendSteamIds.Reset();
	FriendInfos.Reset();

	ISteamFriends* Friends = SteamFriends();
	if (!Friends) return;

	const int32 Count = Friends->GetFriendCount(k_EFriendFlagImmediate);
	FriendSteamIds.Reserve(Count);

	for (int32 i = 0; i < Count; ++i)
	{
		const CSteamID FriendId = Friends->GetFriendByIndex(i, k_EFriendFlagImmediate);
		const FString FriendIdString = FString::Printf(TEXT("%llu"), FriendId.ConvertToUint64());
		FriendSteamIds.Add(FriendIdString);
		CacheAvatarForSteamId(FriendIdString, T66ResolveAvatarImageHandle(Friends, FriendId));

		FT66SteamFriendInfo& FriendInfo = FriendInfos.AddDefaulted_GetRef();
		FriendInfo.SteamId = FriendIdString;
		FriendInfo.DisplayName = FString(UTF8_TO_TCHAR(Friends->GetFriendPersonaName(FriendId)));
		FriendInfo.PresenceText = T66PresenceFromPersonaState(Friends->GetFriendPersonaState(FriendId));
		FriendInfo.bOnline = Friends->GetFriendPersonaState(FriendId) != k_EPersonaStateOffline;

		FriendGameInfo_t FriendGameInfo;
		if (Friends->GetFriendGamePlayed(FriendId, &FriendGameInfo))
		{
			FriendInfo.CurrentGameAppId = FString::Printf(TEXT("%u"), FriendGameInfo.m_gameID.AppID());
			if (FriendGameInfo.m_steamIDLobby.IsValid())
			{
				FriendInfo.CurrentLobbyId = FString::Printf(TEXT("%llu"), FriendGameInfo.m_steamIDLobby.ConvertToUint64());
			}
			FriendInfo.bPlayingThisGame = FriendInfo.CurrentGameAppId == GetActiveSteamAppId();
		}
	}

	UE_LOG(LogT66Steam, Verbose, TEXT("SteamHelper: collected %d friends."), FriendSteamIds.Num());
}

void UT66SteamHelper::CacheAvatarForSteamId(const FString& SteamId, int32 ImageHandle)
{
	if (SteamId.IsEmpty() || ImageHandle <= 0)
	{
		return;
	}

	if (FriendAvatarTextures.Contains(SteamId))
	{
		return;
	}

	if (UTexture2D* AvatarTexture = CreateTextureFromSteamImage(ImageHandle))
	{
		FriendAvatarTextures.Add(SteamId, AvatarTexture);
		++AvatarRevision;
	}
}

UTexture2D* UT66SteamHelper::CreateTextureFromSteamImage(int32 ImageHandle) const
{
	if (ImageHandle <= 0)
	{
		return nullptr;
	}

	ISteamUtils* SteamUtilsApi = SteamUtils();
	if (!SteamUtilsApi)
	{
		return nullptr;
	}

	uint32 Width = 0;
	uint32 Height = 0;
	if (!SteamUtilsApi->GetImageSize(ImageHandle, &Width, &Height) || Width == 0 || Height == 0)
	{
		return nullptr;
	}

	TArray<uint8> RawRgba;
	RawRgba.SetNumUninitialized(static_cast<int32>(Width * Height * 4));
	if (!SteamUtilsApi->GetImageRGBA(ImageHandle, RawRgba.GetData(), RawRgba.Num()))
	{
		return nullptr;
	}

	TArray<uint8> RawBgra;
	RawBgra.SetNumUninitialized(RawRgba.Num());
	for (int32 ByteIndex = 0; ByteIndex < RawRgba.Num(); ByteIndex += 4)
	{
		RawBgra[ByteIndex + 0] = RawRgba[ByteIndex + 2];
		RawBgra[ByteIndex + 1] = RawRgba[ByteIndex + 1];
		RawBgra[ByteIndex + 2] = RawRgba[ByteIndex + 0];
		RawBgra[ByteIndex + 3] = RawRgba[ByteIndex + 3];
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(static_cast<int32>(Width), static_cast<int32>(Height), PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->SRGB = true;
	Texture->Filter = TextureFilter::TF_Trilinear;
	Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	Texture->CompressionSettings = TC_EditorIcon;
	Texture->NeverStream = true;

	void* MipData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, RawBgra.GetData(), RawBgra.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

void UT66SteamHelper::HandleSteamJoinRequested(const FString& FriendSteamId)
{
	if (FriendSteamId.IsEmpty())
	{
		return;
	}

	UE_LOG(LogT66Steam, Log, TEXT("SteamHelper: received Steam join request from friend %s."), *FriendSteamId);
	SteamJoinRequested.Broadcast(FriendSteamId);
}

void UT66SteamHelper::HandleSteamLobbyJoinRequested(const FString& FriendSteamId, const FString& LobbySteamId)
{
	if (FriendSteamId.IsEmpty() || LobbySteamId.IsEmpty())
	{
		return;
	}

	UE_LOG(LogT66Steam, Log, TEXT("SteamHelper: received Steam lobby join request from friend %s for lobby %s."), *FriendSteamId, *LobbySteamId);
	SteamLobbyJoinRequested.Broadcast(FriendSteamId, LobbySteamId);
}

void UT66SteamHelper::HandleWebApiTicketReady(uint32 InTicketHandle, bool bSuccess, const uint8* TicketBytes, int32 TicketByteCount)
{
	if (InTicketHandle != TicketHandle)
	{
		UE_LOG(LogT66Steam, Verbose, TEXT("SteamHelper: ignoring stale Web API ticket response for handle %u."), InTicketHandle);
		return;
	}

	if (!bSuccess || !TicketBytes || TicketByteCount <= 0)
	{
		UE_LOG(LogT66Steam, Warning, TEXT("SteamHelper: Web API ticket request failed for handle %u."), InTicketHandle);
		TicketHex.Reset();
		bSteamReady = false;
		OnSteamTicketReady.Broadcast(false, FString());
		return;
	}

	TicketHex.Reset();
	TicketHex.Reserve(TicketByteCount * 2);
	for (int32 ByteIndex = 0; ByteIndex < TicketByteCount; ++ByteIndex)
	{
		TicketHex += FString::Printf(TEXT("%02x"), TicketBytes[ByteIndex]);
	}

	bSteamReady = true;
	UE_LOG(LogT66Steam, Log, TEXT("SteamHelper: Web API ticket obtained (%d bytes, %d hex chars)."), TicketByteCount, TicketHex.Len());

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->SetSteamTicketHex(TicketHex);
		}
	}

	OnSteamTicketReady.Broadcast(true, TicketHex);
}
