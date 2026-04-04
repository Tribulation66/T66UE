// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SessionSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66SteamHelper.h"
#include "Gameplay/T66PlayerController.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY_STATIC(LogT66Session, Log, All);

const FName UT66SessionSubsystem::PartySessionName(TEXT("T66PartySession"));

void UT66SessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleCreateSessionComplete);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleDestroySessionComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleJoinSessionComplete);
	StartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleStartSessionComplete);
	SessionInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleSessionUserInviteAccepted);

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
		DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
		JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
		StartSessionCompleteHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
		SessionInviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegate);
	}
}

void UT66SessionSubsystem::Deinitialize()
{
	ClearSteamRichPresence();

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		if (CreateSessionCompleteHandle.IsValid())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
		}
		if (DestroySessionCompleteHandle.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
		}
		if (JoinSessionCompleteHandle.IsValid())
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
		}
		if (StartSessionCompleteHandle.IsValid())
		{
			SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteHandle);
		}
		if (SessionInviteAcceptedHandle.IsValid())
		{
			SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedHandle);
		}
	}

	Super::Deinitialize();
}

bool UT66SessionSubsystem::PrepareToHostFrontendLobby(ET66PartySize DesiredPartySize)
{
	PendingPartySize = DesiredPartySize;
	bLocalReadyState = false;

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (World->GetNetMode() == NM_Client)
	{
		BroadcastStateChanged(TEXT("Clients cannot start a new party lobby."));
		return false;
	}

	if (IsPartySessionActive())
	{
		if (UT66GameInstance* GI = GetT66GameInstance())
		{
			GI->PendingFrontendScreen = ET66ScreenType::Lobby;
		}
		BroadcastStateChanged(TEXT("Lobby already active."));
		return false;
	}

	bPendingCreateLobbySession = true;
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		GI->PendingFrontendScreen = ET66ScreenType::Lobby;
	}

	if (World->GetNetMode() == NM_Standalone)
	{
		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		UGameplayStatics::OpenLevel(this, FName(*MapName), true, TEXT("listen"));
		return true;
	}

	return false;
}

void UT66SessionSubsystem::HandleLobbyScreenActivated()
{
	if (bPendingCreateLobbySession && !IsPartySessionActive())
	{
		CreatePartySession();
	}

	SyncLocalLobbyProfile();
	UpdateSteamRichPresence();
}

bool UT66SessionSubsystem::SendInviteToFriend(const FString& FriendPlayerId)
{
	if (FriendPlayerId.IsEmpty() || !CanSendInvites())
	{
		BroadcastStateChanged(TEXT("Lobby is not ready to send invites."));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = GetIdentityInterface();
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!IdentityInterface.IsValid() || !SessionInterface.IsValid())
	{
		BroadcastStateChanged(TEXT("Steam session interfaces are not available."));
		return false;
	}

	FUniqueNetIdPtr FriendNetId = IdentityInterface->CreateUniquePlayerId(FriendPlayerId);
	if (!FriendNetId.IsValid())
	{
		BroadcastStateChanged(TEXT("Could not resolve Steam friend ID."));
		return false;
	}

	const bool bSent = SessionInterface->SendSessionInviteToFriend(0, PartySessionName, *FriendNetId);
	BroadcastStateChanged(bSent ? TEXT("Invite sent.") : TEXT("Steam invite send failed."));
	return bSent;
}

bool UT66SessionSubsystem::StartGameplayTravel()
{
	UWorld* World = GetWorld();
	UT66GameInstance* GI = GetT66GameInstance();
	if (!World || !GI)
	{
		return false;
	}

	if (World->GetNetMode() == NM_Client)
	{
		BroadcastStateChanged(TEXT("Only the host can start the run."));
		return false;
	}

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		SessionInterface->StartSession(PartySessionName);
	}

	UpdateSteamRichPresence();

	const FName LevelToOpen = UT66GameInstance::UseDemoMapForTribulation()
		? UT66GameInstance::GetDemoMapLevelNameForTribulation()
		: UT66GameInstance::GetGameplayLevelName();

	const FString TravelURL = FString::Printf(TEXT("%s?listen"), *LevelToOpen.ToString());
	World->ServerTravel(TravelURL);
	return true;
}

bool UT66SessionSubsystem::LeaveFrontendLobby(ET66ScreenType ReturnScreen)
{
	PendingFrontendReturnScreen = ReturnScreen;
	bPendingCreateLobbySession = false;
	bPendingJoinAfterDestroy = false;
	bLocalReadyState = false;

	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		GI->PendingFrontendScreen = ReturnScreen;
	}

	if (IsPartySessionActive())
	{
		bPendingTravelToStandaloneFrontendAfterDestroy = true;
		DestroyPartySession();
		return true;
	}

	UWorld* World = GetWorld();
	if (World && World->GetNetMode() != NM_Standalone)
	{
		TravelToStandaloneFrontendMap();
		return true;
	}

	ClearSteamRichPresence();
	return false;
}

bool UT66SessionSubsystem::SyncLocalLobbyProfile()
{
	AT66PlayerController* PlayerController = GetPrimaryPlayerController();
	if (!PlayerController)
	{
		return false;
	}

	PlayerController->PushLobbyProfileToServer(BuildLocalLobbyProfile());
	return true;
}

void UT66SessionSubsystem::SetLocalLobbyReady(bool bReady)
{
	bLocalReadyState = bReady;
	SyncLocalLobbyProfile();
}

bool UT66SessionSubsystem::IsPartySessionActive() const
{
	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		return SessionInterface->GetNamedSession(PartySessionName) != nullptr;
	}
	return false;
}

bool UT66SessionSubsystem::IsHostingPartySession() const
{
	UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client && IsPartySessionActive();
}

bool UT66SessionSubsystem::CanSendInvites() const
{
	return IsHostingPartySession() && !bJoinInProgress && !bDestroyInProgress;
}

int32 UT66SessionSubsystem::GetMaxPartyMembers() const
{
	switch (PendingPartySize)
	{
	case ET66PartySize::Quad: return 4;
	case ET66PartySize::Trio: return 3;
	case ET66PartySize::Duo: return 2;
	case ET66PartySize::Solo:
	default:
		return 1;
	}
}

IOnlineSessionPtr UT66SessionSubsystem::GetSessionInterface() const
{
	if (IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
	{
		return OnlineSubsystem->GetSessionInterface();
	}

	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		return OnlineSubsystem->GetSessionInterface();
	}

	return nullptr;
}

IOnlineIdentityPtr UT66SessionSubsystem::GetIdentityInterface() const
{
	if (IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
	{
		return OnlineSubsystem->GetIdentityInterface();
	}

	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		return OnlineSubsystem->GetIdentityInterface();
	}

	return nullptr;
}

AT66PlayerController* UT66SessionSubsystem::GetPrimaryPlayerController() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return Cast<AT66PlayerController>(UGameplayStatics::GetPlayerController(World, 0));
}

UT66GameInstance* UT66SessionSubsystem::GetT66GameInstance() const
{
	return Cast<UT66GameInstance>(GetGameInstance());
}

UT66SteamHelper* UT66SessionSubsystem::GetSteamHelper() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SteamHelper>() : nullptr;
}

bool UT66SessionSubsystem::CreatePartySession()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		BroadcastStateChanged(TEXT("Steam session interface is unavailable."));
		return false;
	}

	if (SessionInterface->GetNamedSession(PartySessionName))
	{
		bPendingCreateLobbySession = false;
		BroadcastStateChanged(TEXT("Lobby session already exists."));
		return true;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = GetMaxPartyMembers();
	SessionSettings.NumPrivateConnections = 0;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUseLobbiesVoiceChatIfAvailable = false;
	SessionSettings.Set(SETTING_MAPNAME, UWorld::RemovePIEPrefix(GetWorld()->GetMapName()), EOnlineDataAdvertisementType::ViaOnlineService);

	const bool bStarted = SessionInterface->CreateSession(0, PartySessionName, SessionSettings);
	if (!bStarted)
	{
		BroadcastStateChanged(TEXT("CreateSession failed to start."));
	}
	else
	{
		BroadcastStateChanged(TEXT("Creating Steam lobby..."));
	}

	return bStarted;
}

bool UT66SessionSubsystem::JoinPartySession(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		BroadcastStateChanged(TEXT("Steam session interface is unavailable."));
		return false;
	}

	bJoinInProgress = true;
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		GI->PendingFrontendScreen = ET66ScreenType::Lobby;
	}

	const bool bStarted = SessionInterface->JoinSession(0, PartySessionName, SearchResult);
	if (!bStarted)
	{
		bJoinInProgress = false;
		BroadcastStateChanged(TEXT("JoinSession failed to start."));
	}
	else
	{
		BroadcastStateChanged(TEXT("Joining party lobby..."));
	}

	return bStarted;
}

void UT66SessionSubsystem::DestroyPartySession()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		if (bPendingTravelToStandaloneFrontendAfterDestroy)
		{
			bPendingTravelToStandaloneFrontendAfterDestroy = false;
			TravelToStandaloneFrontendMap();
		}
		return;
	}

	if (!SessionInterface->GetNamedSession(PartySessionName))
	{
		if (bPendingTravelToStandaloneFrontendAfterDestroy)
		{
			bPendingTravelToStandaloneFrontendAfterDestroy = false;
			TravelToStandaloneFrontendMap();
		}
		return;
	}

	bDestroyInProgress = true;
	if (SessionInterface->DestroySession(PartySessionName))
	{
		BroadcastStateChanged(TEXT("Leaving party lobby..."));
		return;
	}

	bDestroyInProgress = false;
	BroadcastStateChanged(TEXT("DestroySession failed to start."));
}

void UT66SessionSubsystem::BroadcastStateChanged(const TCHAR* NewStatus)
{
	if (NewStatus)
	{
		LastStatusText = NewStatus;
		UE_LOG(LogT66Session, Log, TEXT("%s"), NewStatus);
	}

	SessionStateChanged.Broadcast();
}

void UT66SessionSubsystem::TravelToStandaloneFrontendMap()
{
	ClearSteamRichPresence();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
	UGameplayStatics::OpenLevel(this, FName(*MapName));
}

void UT66SessionSubsystem::UpdateSteamRichPresence()
{
	ISteamFriends* Friends = SteamFriends();
	if (!Friends)
	{
		return;
	}

	if (IsPartySessionActive())
	{
		Friends->SetRichPresence("steam_display", "#Status_InLobby");
		Friends->SetRichPresence("steam_player_group_size", TCHAR_TO_UTF8(*FString::FromInt(GetMaxPartyMembers())));
	}
	else
	{
		Friends->SetRichPresence("steam_display", "#Status_MainMenu");
		Friends->SetRichPresence("steam_player_group_size", "");
	}
}

void UT66SessionSubsystem::ClearSteamRichPresence()
{
	if (ISteamFriends* Friends = SteamFriends())
	{
		Friends->ClearRichPresence();
	}
}

FT66LobbyPlayerInfo UT66SessionSubsystem::BuildLocalLobbyProfile() const
{
	FT66LobbyPlayerInfo LobbyInfo;

	if (const UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		LobbyInfo.SteamId = SteamHelper->GetLocalSteamId();
		LobbyInfo.DisplayName = SteamHelper->GetLocalDisplayName();
	}

	if (const UT66GameInstance* GI = GetT66GameInstance())
	{
		LobbyInfo.SelectedHeroID = GI->SelectedHeroID;
		LobbyInfo.SelectedCompanionID = GI->SelectedCompanionID;
		LobbyInfo.SelectedHeroBodyType = GI->SelectedHeroBodyType;
		LobbyInfo.SelectedCompanionBodyType = GI->SelectedCompanionBodyType;
		LobbyInfo.SelectedHeroSkinID = GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : GI->SelectedHeroSkinID;
	}

	if (LobbyInfo.DisplayName.IsEmpty())
	{
		LobbyInfo.DisplayName = TEXT("You");
	}

	LobbyInfo.bLobbyReady = bLocalReadyState;
	LobbyInfo.bPartyHost = IsHostingPartySession() || bPendingCreateLobbySession;
	return LobbyInfo;
}

void UT66SessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionName != PartySessionName)
	{
		return;
	}

	bPendingCreateLobbySession = false;
	BroadcastStateChanged(bWasSuccessful ? TEXT("Steam lobby ready.") : TEXT("Steam lobby creation failed."));
	SyncLocalLobbyProfile();
	UpdateSteamRichPresence();
}

void UT66SessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionName != PartySessionName)
	{
		return;
	}

	bDestroyInProgress = false;

	if (bPendingJoinAfterDestroy)
	{
		bPendingJoinAfterDestroy = false;
		JoinPartySession(PendingJoinSearchResult);
		return;
	}

	if (bPendingTravelToStandaloneFrontendAfterDestroy)
	{
		bPendingTravelToStandaloneFrontendAfterDestroy = false;
		TravelToStandaloneFrontendMap();
		return;
	}

	BroadcastStateChanged(bWasSuccessful ? TEXT("Lobby closed.") : TEXT("Lobby close failed."));
}

void UT66SessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionName != PartySessionName)
	{
		return;
	}

	bJoinInProgress = false;

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	AT66PlayerController* PlayerController = GetPrimaryPlayerController();
	if (!SessionInterface.IsValid() || !PlayerController)
	{
		BroadcastStateChanged(TEXT("Joined session, but no connect target was available."));
		return;
	}

	FString ConnectString;
	if (Result == EOnJoinSessionCompleteResult::Success && SessionInterface->GetResolvedConnectString(PartySessionName, ConnectString))
	{
		BroadcastStateChanged(TEXT("Joined party lobby."));
		PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
		return;
	}

	BroadcastStateChanged(TEXT("JoinSession completed without a valid connection."));
}

void UT66SessionSubsystem::HandleStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionName == PartySessionName)
	{
		BroadcastStateChanged(bWasSuccessful ? TEXT("Lobby session started.") : TEXT("Lobby session start failed."));
	}
}

void UT66SessionSubsystem::HandleSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful)
	{
		BroadcastStateChanged(TEXT("Steam invite was accepted, but the session result was invalid."));
		return;
	}

	if (IsPartySessionActive())
	{
		PendingJoinSearchResult = InviteResult;
		bPendingJoinAfterDestroy = true;
		DestroyPartySession();
		return;
	}

	JoinPartySession(InviteResult);
}
