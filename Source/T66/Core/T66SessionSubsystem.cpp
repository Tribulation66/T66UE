// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SessionSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66RunIntegritySubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY_STATIC(LogT66Session, Log, All);

static TAutoConsoleVariable<float> CVarT66PendingFriendJoinRetryDelaySeconds(
	TEXT("T66.Session.PendingFriendJoinRetryDelaySeconds"),
	0.10f,
	TEXT("Delay before retrying a pending friend-session join lookup."),
	ECVF_Default);

const FName UT66SessionSubsystem::PartySessionName(TEXT("T66PartySession"));

namespace
{
	constexpr int32 T66MaxPendingFriendJoinLookupAttempts = 10;
	constexpr double T66InviteFeedbackLifetimeSeconds = 12.0;
	constexpr int32 T66DefaultSteamJoinPort = 7777;

	FString T66JoinSessionResultToString(EOnJoinSessionCompleteResult::Type Result)
	{
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::Success:
			return TEXT("Success");
		case EOnJoinSessionCompleteResult::SessionIsFull:
			return TEXT("SessionIsFull");
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			return TEXT("SessionDoesNotExist");
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			return TEXT("CouldNotRetrieveAddress");
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			return TEXT("AlreadyInSession");
		case EOnJoinSessionCompleteResult::UnknownError:
		default:
			return TEXT("UnknownError");
		}
	}

	bool T66TryParseSteamIdString(const FString& SteamIdString, uint64& OutSteamId)
	{
		OutSteamId = 0;
		return !SteamIdString.IsEmpty() && LexTryParseString(OutSteamId, *SteamIdString) && OutSteamId != 0;
	}

	FString T66ExtractSearchResultHostSteamId(const FOnlineSessionSearchResult& SearchResult)
	{
		if (SearchResult.Session.OwningUserId.IsValid())
		{
			const FString CandidateSteamId = SearchResult.Session.OwningUserId->ToString();
			uint64 ParsedSteamId = 0;
			if (T66TryParseSteamIdString(CandidateSteamId, ParsedSteamId))
			{
				return CandidateSteamId;
			}
		}

		return FString();
	}

	int32 T66ResolveSteamJoinPort(const UWorld* World)
	{
		return (World && World->URL.Port > 0) ? World->URL.Port : T66DefaultSteamJoinPort;
	}

	FString T66BuildSteamJoinConnectString(const FString& HostSteamId)
	{
		return HostSteamId.IsEmpty()
			? FString()
			: FString::Printf(TEXT("t66party:%s"), *HostSteamId);
	}

	FString T66BuildSteamDirectConnectString(const FString& HostSteamId, const int32 JoinPort)
	{
		return HostSteamId.IsEmpty()
			? FString()
			: FString::Printf(TEXT("steam.%s:%d"), *HostSteamId, JoinPort > 0 ? JoinPort : T66DefaultSteamJoinPort);
	}

	bool T66IsMiniFrontendScreen(const ET66ScreenType ScreenType)
	{
		switch (ScreenType)
		{
		case ET66ScreenType::MiniMainMenu:
		case ET66ScreenType::MiniSaveSlots:
		case ET66ScreenType::MiniCharacterSelect:
		case ET66ScreenType::MiniCompanionSelect:
		case ET66ScreenType::MiniDifficultySelect:
		case ET66ScreenType::MiniIdolSelect:
		case ET66ScreenType::MiniShop:
		case ET66ScreenType::MiniRunSummary:
			return true;
		default:
			return false;
		}
	}
}

void UT66SessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleCreateSessionComplete);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleDestroySessionComplete);
	FindFriendSessionCompleteDelegate = FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleFindFriendSessionComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleJoinSessionComplete);
	StartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleStartSessionComplete);
	SessionInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UT66SessionSubsystem::HandleSessionUserInviteAccepted);

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
		DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
		FindFriendSessionCompleteHandle = SessionInterface->AddOnFindFriendSessionCompleteDelegate_Handle(0, FindFriendSessionCompleteDelegate);
		JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
		StartSessionCompleteHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
		SessionInviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegate);
	}

	if (UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		SteamLobbyJoinRequestedHandle = SteamHelper->OnSteamLobbyJoinRequested().AddUObject(this, &UT66SessionSubsystem::HandleSteamLobbyJoinRequested);
		SteamJoinRequestedHandle = SteamHelper->OnSteamJoinRequested().AddUObject(this, &UT66SessionSubsystem::HandleSteamJoinRequested);
	}
}

void UT66SessionSubsystem::Deinitialize()
{
	ClearSteamRichPresence();
	ClearPendingFriendJoinRetry();

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
		if (FindFriendSessionCompleteHandle.IsValid())
		{
			SessionInterface->ClearOnFindFriendSessionCompleteDelegate_Handle(0, FindFriendSessionCompleteHandle);
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

	if (UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		SteamHelper->OnSteamLobbyJoinRequested().Remove(SteamLobbyJoinRequestedHandle);
		SteamLobbyJoinRequestedHandle.Reset();
		SteamHelper->OnSteamJoinRequested().Remove(SteamJoinRequestedHandle);
		SteamJoinRequestedHandle.Reset();
	}

	Super::Deinitialize();
}

bool UT66SessionSubsystem::PrepareToHostFrontendLobby(ET66PartySize DesiredPartySize)
{
	return EnsurePartySessionReady(DesiredPartySize, ET66ScreenType::MainMenu);
}

bool UT66SessionSubsystem::EnsurePartySessionReady(ET66PartySize DesiredPartySize, ET66ScreenType PartyHubScreen)
{
	PendingPartySize = DesiredPartySize;
	PartyHubScreenType = PartyHubScreen;
	LocalFrontendScreen = PartyHubScreen;
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
			GI->PendingFrontendScreen = PartyHubScreenType;
		}
		SyncLocalLobbyProfile();
		UpdateSteamRichPresence();
		BroadcastStateChanged(TEXT("Party session already active."));
		return true;
	}

	bPendingCreateLobbySession = true;
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		GI->PendingFrontendScreen = PartyHubScreenType;
	}

	if (World->GetNetMode() == NM_Standalone)
	{
		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		UGameplayStatics::OpenLevel(this, FName(*MapName), true, TEXT("listen"));
		return true;
	}

	return CreatePartySession();
}

void UT66SessionSubsystem::HandlePartyHubScreenActivated()
{
	UWorld* World = GetWorld();
	bGameplaySaveAndReturnInProgress = false;
	if (World && World->URL.HasOption(TEXT("closed")))
	{
		SyncLocalLobbyProfile();
		UpdateSteamRichPresence();
		return;
	}

	if (!bPendingCreateLobbySession
		&& !IsPartySessionActive()
		&& !bJoinInProgress
		&& !bDestroyInProgress
		&& World
		&& World->GetNetMode() == NM_Standalone)
	{
		if (const UT66SteamHelper* SteamHelper = GetSteamHelper(); SteamHelper && SteamHelper->IsSteamReady())
		{
			bPendingCreateLobbySession = true;
			if (UT66GameInstance* GI = GetT66GameInstance())
			{
				GI->PendingFrontendScreen = LocalFrontendScreen;
			}

			const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
			UE_LOG(LogT66Session, Log, TEXT("Auto-hosting frontend Steam lobby on %s."), *MapName);
			UGameplayStatics::OpenLevel(this, FName(*MapName), true, TEXT("listen"));
			return;
		}
	}

	if (bPendingCreateLobbySession && !IsPartySessionActive())
	{
		CreatePartySession();
	}
	else if (IsPartySessionActive())
	{
		UpdatePartySessionJoinability(true, TEXT("frontend_party_hub"));
	}

	SyncLocalLobbyProfile();
	UpdateSteamRichPresence();
}

void UT66SessionSubsystem::HandleLobbyScreenActivated()
{
	HandlePartyHubScreenActivated();
}

bool UT66SessionSubsystem::SendInviteToFriend(const FString& FriendPlayerId, const FString& FriendDisplayName)
{
	if (FriendPlayerId.IsEmpty())
	{
		UE_LOG(LogT66Session, Warning, TEXT("SendInviteToFriend rejected: empty friend id."));
		return false;
	}

	UE_LOG(LogT66Session, Log, TEXT("SendInviteToFriend requested for %s. ActiveSession=%d"), *FriendPlayerId, IsPartySessionActive() ? 1 : 0);

	if (!IsPartySessionActive())
	{
		PendingInviteFriendPlayerId = FriendPlayerId;
		PendingInviteFriendDisplayName = FriendDisplayName;
		ET66PartySize DesiredPartySize = ET66PartySize::Duo;
		if (const UT66GameInstance* GI = GetT66GameInstance())
		{
			DesiredPartySize = GI->SelectedPartySize;
		}
		if (DesiredPartySize == ET66PartySize::Solo)
		{
			DesiredPartySize = ET66PartySize::Duo;
		}

		const bool bStartedLobby = EnsurePartySessionReady(DesiredPartySize, ET66ScreenType::MainMenu);
		if (!bStartedLobby)
		{
			PendingInviteFriendPlayerId.Reset();
			PendingInviteFriendDisplayName.Reset();
		}
		return bStartedLobby;
	}

	return SendInviteToFriendInternal(FriendPlayerId, FriendDisplayName);
}

bool UT66SessionSubsystem::SendInviteToFriendInternal(const FString& FriendPlayerId, const FString& FriendDisplayName)
{
	PruneInviteFeedbackState();

	if (FriendPlayerId.IsEmpty() || !CanSendInvites())
	{
		UE_LOG(LogT66Session, Warning, TEXT("SendInviteToFriendInternal rejected. Friend=%s CanSendInvites=%d"), *FriendPlayerId, CanSendInvites() ? 1 : 0);
		BroadcastStateChanged(TEXT("Lobby is not ready to send invites."));
		return false;
	}

	bool bBackendInviteSent = false;
	bool bSteamInviteSent = false;
	bool bPreferInGameInvite = false;
	FString FriendCurrentAppId;
	FString FriendCurrentLobbyId;

	UT66GameInstance* GI = GetT66GameInstance();
	UT66SteamHelper* SteamHelper = GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
	if (SteamHelper)
	{
		SteamHelper->TryGetFriendCurrentGame(FriendPlayerId, &FriendCurrentAppId, &FriendCurrentLobbyId);
		bPreferInGameInvite = SteamHelper->IsFriendPlayingActiveApp(FriendPlayerId);
	}

	UE_LOG(LogT66Session, Log, TEXT("Invite route for %s prefers %s."), *FriendPlayerId, bPreferInGameInvite ? TEXT("in-game modal") : TEXT("Steam session invite"));
	TMap<FString, FString> InviteRouteFields;
	InviteRouteFields.Add(TEXT("friend_current_app_id"), FriendCurrentAppId);
	InviteRouteFields.Add(TEXT("friend_current_lobby_id"), FriendCurrentLobbyId);
	InviteRouteFields.Add(TEXT("invite_route"), bPreferInGameInvite ? TEXT("backend") : TEXT("steam_session"));
	SubmitSessionDiagnostic(TEXT("invite_route_decision"), TEXT("info"), FString::Printf(TEXT("Invite route selected for %s"), *FriendPlayerId), FString(), FString(), FString(), InviteRouteFields);

	UT66BackendSubsystem* BackendSubsystem = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (bPreferInGameInvite && BackendSubsystem)
	{
		bBackendInviteSent = BackendSubsystem->SendPartyInvite(FriendPlayerId, FriendDisplayName);
		if (bBackendInviteSent)
		{
			UE_LOG(LogT66Session, Log, TEXT("Backend invite sent for %s."), *FriendPlayerId);
		}
		else
		{
			UE_LOG(LogT66Session, Warning, TEXT("Backend invite failed for %s."), *FriendPlayerId);
		}
	}
	else
	{
		UE_LOG(LogT66Session, Verbose, TEXT("Skipping backend invite for %s because the friend is not confirmed to be in this app."), *FriendPlayerId);
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	IOnlineIdentityPtr IdentityInterface = GetIdentityInterface();
	if (SessionInterface.IsValid() && IdentityInterface.IsValid())
	{
		FUniqueNetIdPtr FriendNetId = IdentityInterface->CreateUniquePlayerId(FriendPlayerId);
		if (FriendNetId.IsValid())
		{
			bSteamInviteSent = SessionInterface->SendSessionInviteToFriend(0, PartySessionName, *FriendNetId);
			if (bSteamInviteSent)
			{
				UE_LOG(LogT66Session, Log, TEXT("Steam session invite sent for %s."), *FriendPlayerId);
				TMap<FString, FString> DiagnosticFields;
				DiagnosticFields.Add(TEXT("target_steam_id"), FriendPlayerId);
				SubmitSessionDiagnostic(TEXT("steam_session_invite_sent"), TEXT("info"), TEXT("Steam session invite sent."), FString(), FString(), FString(), DiagnosticFields);
			}
			else
			{
				UE_LOG(LogT66Session, Warning, TEXT("Steam session invite failed for %s."), *FriendPlayerId);
				TMap<FString, FString> DiagnosticFields;
				DiagnosticFields.Add(TEXT("target_steam_id"), FriendPlayerId);
				SubmitSessionDiagnostic(TEXT("steam_session_invite_failed"), TEXT("warning"), TEXT("Steam session invite failed."), FString(), FString(), FString(), DiagnosticFields);
			}
		}
		else
		{
			UE_LOG(LogT66Session, Warning, TEXT("Steam session invite failed: could not resolve friend id %s."), *FriendPlayerId);
			TMap<FString, FString> DiagnosticFields;
			DiagnosticFields.Add(TEXT("target_steam_id"), FriendPlayerId);
			SubmitSessionDiagnostic(TEXT("steam_session_invite_resolve_failed"), TEXT("error"), TEXT("Could not resolve friend Steam ID."), FString(), FString(), FString(), DiagnosticFields);
		}
	}
	else
	{
		UE_LOG(LogT66Session, Verbose, TEXT("Skipping Steam session invite for %s because the in-game invite route was selected."), *FriendPlayerId);
	}

	if (bBackendInviteSent && bSteamInviteSent)
	{
		InviteFeedbackExpiryByFriendId.Add(FriendPlayerId, FPlatformTime::Seconds() + T66InviteFeedbackLifetimeSeconds);
		BroadcastStateChanged(TEXT("Steam and in-game invites sent."));
		return true;
	}

	if (bBackendInviteSent)
	{
		InviteFeedbackExpiryByFriendId.Add(FriendPlayerId, FPlatformTime::Seconds() + T66InviteFeedbackLifetimeSeconds);
		BroadcastStateChanged(TEXT("In-game invite sent. Steam invite unavailable."));
		return true;
	}

	if (bSteamInviteSent)
	{
		InviteFeedbackExpiryByFriendId.Add(FriendPlayerId, FPlatformTime::Seconds() + T66InviteFeedbackLifetimeSeconds);
		BroadcastStateChanged(TEXT("Steam invite sent. In-game popup unavailable."));
		return true;
	}

	BroadcastStateChanged(TEXT("Party invite could not be sent."));
	return false;
}

void UT66SessionSubsystem::PrimePendingJoinContext(const FString& HostSteamId, const FString& LobbyId, const FString& InviteId, const FString& AppId)
{
	PendingJoinFriendPlayerId = HostSteamId;
	PendingExpectedJoinLobbyId = LobbyId;
	PendingJoinInviteId = InviteId;
	PendingJoinSourceAppId = AppId;
	PendingFoundLobbyId = LobbyId;
	PendingJoinFriendLookupAttempts = 0;
}

bool UT66SessionSubsystem::StartDirectJoinByHostSteamId(
	const FString& HostSteamId,
	const FString& LobbyId,
	const FString& AppId,
	const FString& InviteId,
	const TCHAR* JoinReason)
{
	FLagScopedScope LagScope(GetWorld(), TEXT("MP-03 Session::StartDirectJoinByHostSteamId"));

	if (HostSteamId.IsEmpty())
	{
		SubmitSessionDiagnostic(TEXT("invite_join_missing_host"), TEXT("error"), TEXT("Party invite is missing the host Steam ID."), InviteId, LobbyId);
		BroadcastStateChanged(TEXT("Party invite is missing the host Steam ID."));
		return false;
	}

	FString EffectiveLobbyId = LobbyId;
	FString EffectiveAppId = AppId;
	TMap<FString, FString> DiagnosticFields;
	DiagnosticFields.Add(TEXT("host_steam_id"), HostSteamId);
	DiagnosticFields.Add(TEXT("invite_app_id"), AppId);
	DiagnosticFields.Add(TEXT("has_lobby_id"), LobbyId.IsEmpty() ? TEXT("false") : TEXT("true"));
	DiagnosticFields.Add(TEXT("join_reason"), JoinReason ? FString(JoinReason) : TEXT("direct"));

	if (UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		FString PresenceAppId;
		FString PresenceLobbyId;
		if (SteamHelper->TryGetFriendCurrentGame(HostSteamId, &PresenceAppId, &PresenceLobbyId))
		{
			if (!PresenceAppId.IsEmpty())
			{
				DiagnosticFields.Add(TEXT("presence_app_id"), PresenceAppId);
				if (EffectiveAppId.IsEmpty())
				{
					EffectiveAppId = PresenceAppId;
				}
				else if (EffectiveAppId != PresenceAppId)
				{
					DiagnosticFields.Add(TEXT("stale_invite_app_id"), EffectiveAppId);
					DiagnosticFields.Add(TEXT("resolved_from_presence_app_id"), PresenceAppId);
					EffectiveAppId = PresenceAppId;
				}
			}

			if (!PresenceLobbyId.IsEmpty())
			{
				DiagnosticFields.Add(TEXT("presence_lobby_id"), PresenceLobbyId);
				if (EffectiveLobbyId.IsEmpty())
				{
					EffectiveLobbyId = PresenceLobbyId;
				}
				else if (EffectiveLobbyId != PresenceLobbyId)
				{
					DiagnosticFields.Add(TEXT("stale_invite_lobby_id"), EffectiveLobbyId);
					DiagnosticFields.Add(TEXT("resolved_from_presence_lobby_id"), PresenceLobbyId);
					EffectiveLobbyId = PresenceLobbyId;
				}
			}
		}

		const FString ActiveAppId = SteamHelper->GetActiveSteamAppId();
		if (!EffectiveAppId.IsEmpty() && !ActiveAppId.IsEmpty() && EffectiveAppId != ActiveAppId)
		{
			DiagnosticFields.Add(TEXT("active_app_id"), ActiveAppId);
			SubmitSessionDiagnostic(TEXT("invite_join_app_mismatch"), TEXT("error"), TEXT("Invite app ID does not match the active app ID."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
			ClearPendingJoinState();
			BroadcastStateChanged(TEXT("Party invite targets a different Steam app/build."));
			return false;
		}
	}

	if (!EffectiveLobbyId.IsEmpty())
	{
		DiagnosticFields.Add(TEXT("resolved_lobby_id"), EffectiveLobbyId);
	}

	if (!EffectiveAppId.IsEmpty())
	{
		DiagnosticFields.Add(TEXT("resolved_app_id"), EffectiveAppId);
	}

	PrimePendingJoinContext(HostSteamId, EffectiveLobbyId, InviteId, EffectiveAppId);
	ClearPendingFriendJoinRetry();

	if (bJoinInProgress)
	{
		DiagnosticFields.Add(TEXT("join_in_progress"), TEXT("true"));
		SubmitSessionDiagnostic(TEXT("invite_join_blocked_by_active_join"), TEXT("warning"), TEXT("Another join is already in progress."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
		ClearPendingJoinState();
		BroadcastStateChanged(TEXT("Another party join is already in progress."));
		return false;
	}

	const int32 JoinPort = T66ResolveSteamJoinPort(GetWorld());
	const FString DirectConnectString = T66BuildSteamDirectConnectString(HostSteamId, JoinPort);
	if (DirectConnectString.IsEmpty())
	{
		DiagnosticFields.Add(TEXT("join_port"), FString::FromInt(JoinPort));
		SubmitSessionDiagnostic(TEXT("invite_join_direct_bootstrap_failed"), TEXT("error"), TEXT("Could not build a Steam direct-connect target from the invite."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
		ClearPendingJoinState();
		BroadcastStateChanged(TEXT("Party invite could not be prepared for joining."));
		return false;
	}

	DiagnosticFields.Add(TEXT("join_port"), FString::FromInt(JoinPort));
	UE_LOG(
		LogT66Session,
		Log,
		TEXT("Starting direct party join. Host=%s Lobby=%s App=%s Port=%d Reason=%s"),
		*HostSteamId,
		*EffectiveLobbyId,
		*EffectiveAppId,
		JoinPort,
		JoinReason ? JoinReason : TEXT("direct"));

	if (IsPartySessionActive())
	{
		const FString CurrentLobbyId = GetCurrentPartyLobbyId();
		if (!CurrentLobbyId.IsEmpty() && !EffectiveLobbyId.IsEmpty() && CurrentLobbyId == EffectiveLobbyId)
		{
			SubmitSessionDiagnostic(TEXT("invite_join_already_in_target"), TEXT("info"), TEXT("Already joined to the invited party lobby."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
			ClearPendingJoinState();
			BroadcastStateChanged(TEXT("Already in party lobby."));
			return true;
		}

		if ((CurrentLobbyId.IsEmpty() || EffectiveLobbyId.IsEmpty()))
		{
			FT66LobbyPlayerInfo HostLobbyInfo;
			if (GetHostLobbyProfile(HostLobbyInfo)
				&& !HostLobbyInfo.SteamId.IsEmpty()
				&& HostLobbyInfo.SteamId == HostSteamId)
			{
				DiagnosticFields.Add(TEXT("already_in_target_by_host"), TEXT("true"));
				SubmitSessionDiagnostic(TEXT("invite_join_already_with_host"), TEXT("info"), TEXT("Already connected to the invited host party."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
				ClearPendingJoinState();
				BroadcastStateChanged(TEXT("Already in party lobby."));
				return true;
			}
		}
	}

	const FString PreviousPendingDirectJoinConnectString = PendingDirectJoinConnectString;
	PendingDirectJoinConnectString = DirectConnectString;
	SubmitSessionDiagnostic(TEXT("invite_join_direct_bootstrap_started"), TEXT("info"), TEXT("Built direct Steam travel target from host data."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);

	if (bDestroyInProgress)
	{
		if (!PreviousPendingDirectJoinConnectString.IsEmpty() && PreviousPendingDirectJoinConnectString == DirectConnectString)
		{
			DiagnosticFields.Add(TEXT("destroy_in_progress"), TEXT("true"));
			DiagnosticFields.Add(TEXT("join_already_queued"), TEXT("true"));
			SubmitSessionDiagnostic(TEXT("invite_join_waiting_for_destroy"), TEXT("info"), TEXT("Waiting for the current party session to finish destroying before travel."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
			BroadcastStateChanged(TEXT("Leaving current party lobby..."));
			return true;
		}

		DiagnosticFields.Add(TEXT("destroy_in_progress"), TEXT("true"));
		SubmitSessionDiagnostic(TEXT("invite_join_waiting_for_destroy"), TEXT("info"), TEXT("Waiting for the current party session to finish destroying before travel."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Leaving current party lobby..."));
		return true;
	}

	if (IsPartySessionActive())
	{
		SubmitSessionDiagnostic(TEXT("invite_join_destroy_before_direct_travel"), TEXT("info"), TEXT("Destroying the current party before direct travel."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
		DestroyPartySession();
		if (bDestroyInProgress)
		{
			return true;
		}

		SubmitSessionDiagnostic(TEXT("invite_join_destroy_start_failed"), TEXT("error"), TEXT("Could not begin destroying the current party before joining the target party."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
		ClearPendingJoinState();
		return false;
	}

	AT66PlayerController* PlayerController = GetPrimaryPlayerController();
	if (!PlayerController)
	{
		SubmitSessionDiagnostic(TEXT("invite_join_direct_start_failed"), TEXT("error"), TEXT("Could not resolve the local player controller for direct Steam travel."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
		ClearPendingJoinState();
		BroadcastStateChanged(TEXT("Could not start joining the invited Steam lobby."));
		return false;
	}

	DiagnosticFields.Add(TEXT("connect_string"), DirectConnectString);
	SubmitSessionDiagnostic(TEXT("invite_join_direct_travel_started"), TEXT("info"), TEXT("Travelling directly to the invited Steam host."), InviteId, EffectiveLobbyId, FString(), DiagnosticFields);
	BroadcastStateChanged(TEXT("Joining party lobby..."));
	PlayerController->ClientTravel(DirectConnectString, TRAVEL_Absolute);
	ClearPendingJoinState();
	return true;
}

bool UT66SessionSubsystem::JoinFriendPartySessionBySteamId(const FString& FriendPlayerId, const FString& InviteId, const FString& AppId)
{
	FString EffectiveAppId = AppId;

	if (UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		FString FriendCurrentAppId;
		FString FriendCurrentLobbyId;
		if (SteamHelper->TryGetFriendCurrentGame(FriendPlayerId, &FriendCurrentAppId, &FriendCurrentLobbyId))
		{
			if (EffectiveAppId.IsEmpty())
			{
				EffectiveAppId = FriendCurrentAppId;
			}

			if (!FriendCurrentLobbyId.IsEmpty())
			{
				return JoinPartySessionByLobbyId(FriendCurrentLobbyId, FriendPlayerId, EffectiveAppId, InviteId);
			}
		}
	}

	return StartDirectJoinByHostSteamId(FriendPlayerId, FString(), EffectiveAppId, InviteId, TEXT("friend_steam_id"));
}

bool UT66SessionSubsystem::JoinPartySessionByLobbyId(const FString& LobbyId, const FString& HostSteamId, const FString& AppId, const FString& InviteId)
{
	FLagScopedScope LagScope(GetWorld(), TEXT("MP-03 Session::JoinPartySessionByLobbyId"));

	if (LobbyId.IsEmpty())
	{
		SubmitSessionDiagnostic(TEXT("invite_join_missing_lobby"), TEXT("error"), TEXT("Party invite is missing the host lobby ID."), InviteId);
		BroadcastStateChanged(TEXT("Party invite is missing the host lobby ID."));
		return false;
	}

	return StartDirectJoinByHostSteamId(HostSteamId, LobbyId, AppId, InviteId, TEXT("invite_lobby"));
}

bool UT66SessionSubsystem::StartLoadedGameplayTravel(const UT66RunSaveGame* LoadedSave, int32 SaveSlotIndex)
{
	if (!LoadedSave || !IsPartySessionActive())
	{
		return false;
	}

	if (!IsHostingPartySession())
	{
		BroadcastStateChanged(TEXT("Only the host can load a party save."));
		return false;
	}

	ApplyLoadedRunToGameInstance(LoadedSave, SaveSlotIndex);
	ApplySavedPartyProfilesToCurrentSession(LoadedSave);
	return StartGameplayTravel();
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

	PreparePartySessionForWorldTravel(TEXT("gameplay_travel"));

	const FName LevelToOpen = UT66GameInstance::GetGameplayLevelName();
	const FString TravelURL = FString::Printf(TEXT("%s?listen"), *LevelToOpen.ToString());
	World->ServerTravel(TravelURL);
	return true;
}

bool UT66SessionSubsystem::PreparePartySessionForWorldTravel(const TCHAR* Context)
{
	if (!IsPartySessionActive())
	{
		return false;
	}

	ClearCachedPartyRunSummaries();

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		UpdatePartySessionJoinability(false, Context ? Context : TEXT("gameplay_travel"));
		SessionInterface->StartSession(PartySessionName);
	}

	UpdateSteamRichPresence();
	return true;
}

UT66RunSaveGame* UT66SessionSubsystem::BuildCurrentRunSaveSnapshot(UObject* Outer) const
{
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI)
	{
		return nullptr;
	}

	UT66RunSaveGame* SaveObj = NewObject<UT66RunSaveGame>(Outer ? Outer : const_cast<UT66SessionSubsystem*>(this));
	if (!SaveObj)
	{
		return nullptr;
	}

	SaveObj->HeroID = GI->SelectedHeroID;
	SaveObj->HeroBodyType = GI->SelectedHeroBodyType;
	SaveObj->CompanionID = GI->SelectedCompanionID;
	SaveObj->Difficulty = GI->SelectedDifficulty;
	SaveObj->PartySize = GI->SelectedPartySize;
	SaveObj->MapName = GetWorld() ? UWorld::RemovePIEPrefix(GetWorld()->GetMapName()) : FString();
	SaveObj->LastPlayedUtc = FDateTime::UtcNow().ToIso8601();
	SaveObj->RunSeed = GI->RunSeed;
	SaveObj->bIsDailyClimbRun = GI->IsDailyClimbRunActive();
	SaveObj->DailyClimbChallenge = SaveObj->bIsDailyClimbRun ? GI->ActiveDailyClimbChallenge : FT66DailyClimbChallengeData{};
	SaveObj->MainMapLayoutVariant = GI->CurrentMainMapLayoutVariant;
	SaveObj->bRunIneligibleForLeaderboard = GI->bRunIneligibleForLeaderboard;
	if (UT66RunIntegritySubsystem* Integrity = GI->GetSubsystem<UT66RunIntegritySubsystem>())
	{
		Integrity->CopyCurrentContextTo(SaveObj->IntegrityContext);
	}

	if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
	{
		SaveObj->StageReached = RunState->GetCurrentStage();
		RunState->ExportSavedRunSnapshot(SaveObj->RunSnapshot);
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			SaveObj->EquippedIdols = IdolManager->GetEquippedIdols();
			SaveObj->EquippedIdolTiers = IdolManager->GetEquippedIdolTierValues();
		}
		else
		{
			SaveObj->EquippedIdols = RunState->GetEquippedIdols();
			SaveObj->EquippedIdolTiers = RunState->GetEquippedIdolTierValues();
		}
	}

	if (AT66PlayerController* PlayerController = GetPrimaryPlayerController())
	{
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			SaveObj->PlayerTransform = Pawn->GetActorTransform();
		}
	}

	const UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>();
	SaveObj->OwnerPlayerId = !GI->CurrentRunOwnerPlayerId.IsEmpty()
		? GI->CurrentRunOwnerPlayerId
		: (PartySubsystem ? PartySubsystem->GetLocalPlayerId() : TEXT("local_player"));
	SaveObj->OwnerDisplayName = !GI->CurrentRunOwnerDisplayName.IsEmpty()
		? GI->CurrentRunOwnerDisplayName
		: (PartySubsystem ? PartySubsystem->GetLocalDisplayName() : TEXT("You"));
	SaveObj->PartyMemberIds = GI->CurrentRunPartyMemberIds.Num() > 0
		? GI->CurrentRunPartyMemberIds
		: (PartySubsystem ? PartySubsystem->GetCurrentPartyMemberIds() : TArray<FString>());
	SaveObj->PartyMemberDisplayNames = GI->CurrentRunPartyMemberDisplayNames.Num() > 0
		? GI->CurrentRunPartyMemberDisplayNames
		: (PartySubsystem ? PartySubsystem->GetCurrentPartyMemberDisplayNames() : TArray<FString>());
	if (SaveObj->PartyMemberIds.Num() == 0 && !SaveObj->OwnerPlayerId.IsEmpty())
	{
		SaveObj->PartyMemberIds.Add(SaveObj->OwnerPlayerId);
	}
	if (SaveObj->PartyMemberDisplayNames.Num() == 0 && !SaveObj->OwnerDisplayName.IsEmpty())
	{
		SaveObj->PartyMemberDisplayNames.Add(SaveObj->OwnerDisplayName);
	}

	SaveObj->SavedPartyPlayers.Reset();
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PlayerState : GameState->PlayerArray)
			{
				const AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState);
				if (!SessionPlayerState)
				{
					continue;
				}

				const FT66LobbyPlayerInfo& LobbyInfo = SessionPlayerState->GetLobbyInfo();
				if (LobbyInfo.SteamId.IsEmpty())
				{
					continue;
				}

				FT66SavedPartyPlayerState& SavedPlayer = SaveObj->SavedPartyPlayers.AddDefaulted_GetRef();
				SavedPlayer.PlayerId = LobbyInfo.SteamId;
				SavedPlayer.DisplayName = LobbyInfo.DisplayName;
				SavedPlayer.HeroID = LobbyInfo.SelectedHeroID;
				SavedPlayer.HeroBodyType = LobbyInfo.SelectedHeroBodyType;
				SavedPlayer.HeroSkinID = LobbyInfo.SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : LobbyInfo.SelectedHeroSkinID;
				SavedPlayer.CompanionID = LobbyInfo.SelectedCompanionID;
				SavedPlayer.CompanionBodyType = LobbyInfo.SelectedCompanionBodyType;
				SavedPlayer.bIsPartyHost = LobbyInfo.bPartyHost;
			}
		}
	}

	if (SaveObj->SavedPartyPlayers.Num() == 0)
	{
		FT66SavedPartyPlayerState& LocalSavedPlayer = SaveObj->SavedPartyPlayers.AddDefaulted_GetRef();
		LocalSavedPlayer.PlayerId = SaveObj->OwnerPlayerId;
		LocalSavedPlayer.DisplayName = SaveObj->OwnerDisplayName;
		LocalSavedPlayer.HeroID = SaveObj->HeroID;
		LocalSavedPlayer.HeroBodyType = SaveObj->HeroBodyType;
		LocalSavedPlayer.HeroSkinID = GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : GI->SelectedHeroSkinID;
		LocalSavedPlayer.CompanionID = SaveObj->CompanionID;
		LocalSavedPlayer.PlayerTransform = SaveObj->PlayerTransform;
		LocalSavedPlayer.bIsPartyHost = true;
	}
	else
	{
		for (FT66SavedPartyPlayerState& SavedPlayer : SaveObj->SavedPartyPlayers)
		{
			if (SavedPlayer.PlayerId == SaveObj->OwnerPlayerId)
			{
				SavedPlayer.PlayerTransform = SaveObj->PlayerTransform;
				SavedPlayer.bIsPartyHost = true;
				break;
			}
		}
	}

	SaveObj->PartyMemberIds.Reset();
	SaveObj->PartyMemberDisplayNames.Reset();
	for (const FT66SavedPartyPlayerState& SavedPlayer : SaveObj->SavedPartyPlayers)
	{
		if (!SavedPlayer.PlayerId.IsEmpty())
		{
			SaveObj->PartyMemberIds.AddUnique(SavedPlayer.PlayerId);
		}
		if (!SavedPlayer.DisplayName.IsEmpty())
		{
			SaveObj->PartyMemberDisplayNames.AddUnique(SavedPlayer.DisplayName);
		}
	}

	switch (SaveObj->SavedPartyPlayers.Num())
	{
	case 4:
		SaveObj->PartySize = ET66PartySize::Quad;
		break;
	case 3:
		SaveObj->PartySize = ET66PartySize::Trio;
		break;
	case 2:
		SaveObj->PartySize = ET66PartySize::Duo;
		break;
	case 1:
	default:
		SaveObj->PartySize = ET66PartySize::Solo;
		break;
	}

	return SaveObj;
}

bool UT66SessionSubsystem::SaveCurrentRunAndReturnToFrontend()
{
	if (bGameplaySaveAndReturnInProgress)
	{
		return true;
	}

	UWorld* World = GetWorld();
	UT66GameInstance* GI = GetT66GameInstance();
	if (!World || !GI || World->GetNetMode() == NM_Client)
	{
		return false;
	}

	UT66SaveSubsystem* SaveSub = GI->GetSubsystem<UT66SaveSubsystem>();
	if (!SaveSub)
	{
		return false;
	}

	int32 SlotIndex = GI->CurrentSaveSlotIndex;
	if (SlotIndex < 0 || SlotIndex >= UT66SaveSubsystem::MaxSlots)
	{
		SlotIndex = SaveSub->FindFirstEmptySlot();
		if (SlotIndex == INDEX_NONE)
		{
			SlotIndex = SaveSub->FindOldestOccupiedSlot();
			if (SlotIndex == INDEX_NONE)
			{
				return false;
			}
		}
		GI->CurrentSaveSlotIndex = SlotIndex;
	}

	UT66RunSaveGame* SaveObj = BuildCurrentRunSaveSnapshot(this);
	if (!SaveObj || !SaveSub->SaveToSlot(SlotIndex, SaveObj))
	{
		return false;
	}

	bGameplaySaveAndReturnInProgress = true;
	bPendingCreateLobbySession = false;
	bPendingJoinAfterDestroy = false;
	bPendingTravelToStandaloneFrontendAfterDestroy = false;
	bJoinInProgress = false;
	bLocalReadyState = false;
	ClearCachedPartyRunSummaries();
	ClearPendingFriendJoinRetry();
	PendingInviteFriendPlayerId.Reset();
	PendingInviteFriendDisplayName.Reset();
	PendingJoinFriendPlayerId.Reset();
	PendingExpectedJoinLobbyId.Reset();
	PendingJoinInviteId.Reset();
	PendingJoinSourceAppId.Reset();
	PendingFoundLobbyId.Reset();
	PendingDirectJoinConnectString.Reset();
	PendingJoinFriendLookupAttempts = 0;
	LocalFrontendScreen = ET66ScreenType::MainMenu;
	GI->PendingFrontendScreen = ET66ScreenType::MainMenu;
	ClearSteamRichPresence();

	if (AT66PlayerController* PlayerController = GetPrimaryPlayerController())
	{
		PlayerController->SetPause(false);
	}

	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
	return true;
}

bool UT66SessionSubsystem::LeaveFrontendLobby(ET66ScreenType ReturnScreen)
{
	PendingFrontendReturnScreen = ReturnScreen;
	bPendingCreateLobbySession = false;
	bPendingJoinAfterDestroy = false;
	ClearCachedPartyRunSummaries();
	ClearPendingFriendJoinRetry();
	PendingJoinFriendPlayerId.Reset();
	PendingExpectedJoinLobbyId.Reset();
	PendingJoinInviteId.Reset();
	PendingJoinSourceAppId.Reset();
	PendingFoundLobbyId.Reset();
	PendingDirectJoinConnectString.Reset();
	PendingJoinFriendLookupAttempts = 0;
	bLocalReadyState = false;
	LocalFrontendScreen = ReturnScreen;

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

void UT66SessionSubsystem::SetLocalFrontendScreen(ET66ScreenType ScreenType, bool bResetReady)
{
	LocalFrontendScreen = ScreenType;
	if (bResetReady)
	{
		bLocalReadyState = false;
	}

	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		GI->PendingFrontendScreen = ScreenType;
	}

	SyncLocalLobbyProfile();
	BroadcastStateChanged();
}

bool UT66SessionSubsystem::IsPartySessionActive() const
{
	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		return SessionInterface->GetNamedSession(PartySessionName) != nullptr;
	}
	return false;
}

bool UT66SessionSubsystem::IsPartyLobbyContextActive() const
{
	if (IsPartySessionActive())
	{
		return true;
	}

	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Standalone)
	{
		return false;
	}

	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	GetCurrentLobbyProfiles(LobbyProfiles);
	return LobbyProfiles.Num() > 1;
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

bool UT66SessionSubsystem::IsFriendInvitePending(const FString& FriendPlayerId)
{
	PruneInviteFeedbackState();
	return !FriendPlayerId.IsEmpty() && InviteFeedbackExpiryByFriendId.Contains(FriendPlayerId);
}

bool UT66SessionSubsystem::StartJoinByFriendId(const FString& FriendPlayerId, const FString& ExpectedLobbyId, const FString& InviteId, const FString& AppId)
{
	if (FriendPlayerId.IsEmpty())
	{
		SubmitSessionDiagnostic(TEXT("join_lookup_missing_host"), TEXT("error"), TEXT("Party invite is missing the host Steam ID."), InviteId, ExpectedLobbyId);
		BroadcastStateChanged(TEXT("Party invite is missing the host Steam ID."));
		return false;
	}

	IOnlineIdentityPtr IdentityInterface = GetIdentityInterface();
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!IdentityInterface.IsValid() || !SessionInterface.IsValid())
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("has_identity_interface"), IdentityInterface.IsValid() ? TEXT("true") : TEXT("false"));
		DiagnosticFields.Add(TEXT("has_session_interface"), SessionInterface.IsValid() ? TEXT("true") : TEXT("false"));
		SubmitSessionDiagnostic(TEXT("join_lookup_interfaces_unavailable"), TEXT("error"), TEXT("Steam session interfaces are not available."), InviteId, ExpectedLobbyId, FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Steam session interfaces are not available."));
		return false;
	}

	FUniqueNetIdPtr FriendNetId = IdentityInterface->CreateUniquePlayerId(FriendPlayerId);
	if (!FriendNetId.IsValid())
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("host_steam_id"), FriendPlayerId);
		SubmitSessionDiagnostic(TEXT("join_lookup_host_resolve_failed"), TEXT("error"), TEXT("Could not resolve the host Steam ID."), InviteId, ExpectedLobbyId, FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Could not resolve the host Steam ID."));
		return false;
	}

	PendingJoinFriendPlayerId = FriendPlayerId;
	PendingExpectedJoinLobbyId = ExpectedLobbyId;
	PendingJoinInviteId = InviteId;
	PendingJoinSourceAppId = AppId;
	PendingFoundLobbyId.Reset();
	PendingJoinFriendLookupAttempts = 0;
	ClearPendingFriendJoinRetry();
	UE_LOG(LogT66Session, Log, TEXT("StartJoinByFriendId friend=%s expectedLobby=%s"), *PendingJoinFriendPlayerId, *PendingExpectedJoinLobbyId);
	TMap<FString, FString> DiagnosticFields;
	DiagnosticFields.Add(TEXT("host_steam_id"), FriendPlayerId);
	DiagnosticFields.Add(TEXT("source_app_id"), AppId);
	SubmitSessionDiagnostic(TEXT("join_lookup_start"), TEXT("info"), TEXT("Starting host party lookup."), InviteId, ExpectedLobbyId, FString(), DiagnosticFields);
	return AttemptPendingFriendJoinLookup();
}

bool UT66SessionSubsystem::AttemptPendingFriendJoinLookup()
{
	if (PendingJoinFriendPlayerId.IsEmpty())
	{
		return false;
	}

	if (UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		FString ResolvedAppId;
		FString ResolvedLobbyId;
		if (SteamHelper->TryGetFriendCurrentGame(PendingJoinFriendPlayerId, &ResolvedAppId, &ResolvedLobbyId)
			&& !ResolvedLobbyId.IsEmpty()
			&& (PendingExpectedJoinLobbyId.IsEmpty() || PendingExpectedJoinLobbyId == ResolvedLobbyId))
		{
			const FString EffectiveAppId = PendingJoinSourceAppId.IsEmpty() ? ResolvedAppId : PendingJoinSourceAppId;
			TMap<FString, FString> DiagnosticFields;
			DiagnosticFields.Add(TEXT("resolved_lobby_id"), ResolvedLobbyId);
			DiagnosticFields.Add(TEXT("resolved_app_id"), EffectiveAppId);
			DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts + 1));
			SubmitSessionDiagnostic(TEXT("join_lookup_direct_fast_path"), TEXT("info"), TEXT("Resolved host lobby directly from Steam presence."), PendingJoinInviteId, PendingExpectedJoinLobbyId, ResolvedLobbyId, DiagnosticFields);
			if (JoinPartySessionByLobbyId(ResolvedLobbyId, PendingJoinFriendPlayerId, EffectiveAppId, PendingJoinInviteId))
			{
				return true;
			}
		}
	}

	if (bJoinInProgress || bDestroyInProgress)
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("join_in_progress"), bJoinInProgress ? TEXT("true") : TEXT("false"));
		DiagnosticFields.Add(TEXT("destroy_in_progress"), bDestroyInProgress ? TEXT("true") : TEXT("false"));
		SubmitSessionDiagnostic(TEXT("join_lookup_deferred"), TEXT("info"), TEXT("Join lookup deferred until current transition finishes."), FString(), FString(), FString(), DiagnosticFields);
		SchedulePendingFriendJoinRetry();
		return true;
	}

	IOnlineIdentityPtr IdentityInterface = GetIdentityInterface();
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!IdentityInterface.IsValid() || !SessionInterface.IsValid())
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("has_identity_interface"), IdentityInterface.IsValid() ? TEXT("true") : TEXT("false"));
		DiagnosticFields.Add(TEXT("has_session_interface"), SessionInterface.IsValid() ? TEXT("true") : TEXT("false"));
		SubmitSessionDiagnostic(TEXT("join_lookup_interfaces_unavailable"), TEXT("error"), TEXT("Steam session interfaces are not available."), FString(), FString(), FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Steam session interfaces are not available."));
		return false;
	}

	FUniqueNetIdPtr FriendNetId = IdentityInterface->CreateUniquePlayerId(PendingJoinFriendPlayerId);
	if (!FriendNetId.IsValid())
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("host_steam_id"), PendingJoinFriendPlayerId);
		SubmitSessionDiagnostic(TEXT("join_lookup_host_resolve_failed"), TEXT("error"), TEXT("Could not resolve the host Steam ID."), FString(), FString(), FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Could not resolve the host Steam ID."));
		ClearPendingJoinState();
		return false;
	}

	++PendingJoinFriendLookupAttempts;
	UE_LOG(LogT66Session, Log, TEXT("AttemptPendingFriendJoinLookup friend=%s expectedLobby=%s attempt=%d"), *PendingJoinFriendPlayerId, *PendingExpectedJoinLobbyId, PendingJoinFriendLookupAttempts);
	const bool bStarted = SessionInterface->FindFriendSession(0, *FriendNetId);
	if (bStarted)
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
		DiagnosticFields.Add(TEXT("host_steam_id"), PendingJoinFriendPlayerId);
		SubmitSessionDiagnostic(TEXT("join_lookup_attempt_started"), TEXT("info"), TEXT("Looking up host party."), FString(), FString(), FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Looking up host party..."));
		return true;
	}

	if (PendingJoinFriendLookupAttempts >= T66MaxPendingFriendJoinLookupAttempts)
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
		SubmitSessionDiagnostic(TEXT("join_lookup_exhausted"), TEXT("error"), TEXT("Could not find the host party session."), FString(), FString(), FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Could not find the host party session."));
		ClearPendingJoinState();
		return false;
	}

	TMap<FString, FString> DiagnosticFields;
	DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
	SubmitSessionDiagnostic(TEXT("join_lookup_retry_scheduled"), TEXT("warning"), TEXT("Host party is not joinable yet. Retrying."), FString(), FString(), FString(), DiagnosticFields);
	BroadcastStateChanged(TEXT("Host party is not joinable yet. Retrying..."));
	SchedulePendingFriendJoinRetry();
	return true;
}

void UT66SessionSubsystem::SchedulePendingFriendJoinRetry()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("MP-03 Session::SchedulePendingFriendJoinRetry"));

	if (PendingJoinFriendPlayerId.IsEmpty() || PendingJoinFriendRetryTickerHandle.IsValid())
	{
		return;
	}

	const float RetryDelaySeconds = FMath::Max(0.05f, CVarT66PendingFriendJoinRetryDelaySeconds.GetValueOnGameThread());
	PendingJoinFriendRetryTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UT66SessionSubsystem::HandlePendingFriendJoinRetryTicker),
		RetryDelaySeconds);
}

void UT66SessionSubsystem::ClearPendingFriendJoinRetry()
{
	if (PendingJoinFriendRetryTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PendingJoinFriendRetryTickerHandle);
		PendingJoinFriendRetryTickerHandle.Reset();
	}
}

void UT66SessionSubsystem::ClearPendingJoinState()
{
	ClearPendingFriendJoinRetry();
	PendingJoinFriendPlayerId.Reset();
	PendingExpectedJoinLobbyId.Reset();
	PendingJoinInviteId.Reset();
	PendingJoinSourceAppId.Reset();
	PendingFoundLobbyId.Reset();
	PendingDirectJoinConnectString.Reset();
	PendingJoinFriendLookupAttempts = 0;
}

bool UT66SessionSubsystem::HandlePendingFriendJoinRetryTicker(float DeltaTime)
{
	ClearPendingFriendJoinRetry();
	AttemptPendingFriendJoinLookup();
	return false;
}

void UT66SessionSubsystem::PruneInviteFeedbackState()
{
	if (InviteFeedbackExpiryByFriendId.Num() == 0)
	{
		return;
	}

	const double NowSeconds = FPlatformTime::Seconds();
	for (auto It = InviteFeedbackExpiryByFriendId.CreateIterator(); It; ++It)
	{
		if (It.Value() <= NowSeconds)
		{
			It.RemoveCurrent();
		}
	}
}

int32 UT66SessionSubsystem::GetMaxPartyMembers() const
{
	switch (PendingPartySize)
	{
	case ET66PartySize::Quad:
	case ET66PartySize::Trio:
	case ET66PartySize::Duo:
		return 4;
	case ET66PartySize::Solo:
	default:
		return 1;
	}
}

FString UT66SessionSubsystem::GetCurrentPartyLobbyId() const
{
	if (IOnlineSessionPtr SessionInterface = GetSessionInterface())
	{
		if (const FNamedOnlineSession* Session = SessionInterface->GetNamedSession(PartySessionName))
		{
			if (Session->SessionInfo.IsValid())
			{
				return Session->SessionInfo->GetSessionId().ToString();
			}
		}
	}

	return FString();
}

int32 UT66SessionSubsystem::GetCurrentLobbyPlayerCount() const
{
	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	GetCurrentLobbyProfiles(LobbyProfiles);
	return LobbyProfiles.Num();
}

bool UT66SessionSubsystem::IsLocalPlayerPartyHost() const
{
	FT66LobbyPlayerInfo HostLobbyInfo;
	if (!GetHostLobbyProfile(HostLobbyInfo))
	{
		return IsHostingPartySession();
	}

	const FString LocalPlayerId = GetSteamHelper() ? GetSteamHelper()->GetLocalSteamId() : FString();
	if (LocalPlayerId.IsEmpty())
	{
		return IsHostingPartySession();
	}

	return HostLobbyInfo.SteamId == LocalPlayerId;
}

bool UT66SessionSubsystem::SubmitLocalPartyRunSummaryToHost(const FString& RequestKey, const FString& RunSummaryJson)
{
	if (RequestKey.IsEmpty() || RunSummaryJson.IsEmpty() || !IsPartySessionActive())
	{
		return false;
	}

	AT66PlayerController* PlayerController = GetPrimaryPlayerController();
	if (!PlayerController)
	{
		return false;
	}

	if (AT66SessionPlayerState* SessionPlayerState = PlayerController->GetPlayerState<AT66SessionPlayerState>())
	{
		if (IsLocalPlayerPartyHost() || PlayerController->HasAuthority())
		{
			StorePartyRunSummaryForSteamId(RequestKey, SessionPlayerState->GetSteamId(), RunSummaryJson);
			return true;
		}
	}

	PlayerController->PushPartyRunSummaryToServer(RequestKey, RunSummaryJson);
	return true;
}

void UT66SessionSubsystem::GetCurrentLobbyProfiles(TArray<FT66LobbyPlayerInfo>& OutProfiles) const
{
	OutProfiles.Reset();

	UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (GameState)
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			const AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState);
			if (!SessionPlayerState)
			{
				continue;
			}

			const FT66LobbyPlayerInfo& LobbyInfo = SessionPlayerState->GetLobbyInfo();
			if (LobbyInfo.SteamId.IsEmpty() && LobbyInfo.DisplayName.IsEmpty())
			{
				continue;
			}

			OutProfiles.Add(LobbyInfo);
		}
	}

	if (OutProfiles.Num() == 0)
	{
		OutProfiles.Add(BuildLocalLobbyProfile());
	}

	OutProfiles.Sort([](const FT66LobbyPlayerInfo& A, const FT66LobbyPlayerInfo& B)
	{
		if (A.bPartyHost != B.bPartyHost)
		{
			return A.bPartyHost;
		}

		if (A.DisplayName != B.DisplayName)
		{
			return A.DisplayName < B.DisplayName;
		}

		return A.SteamId < B.SteamId;
	});
}

bool UT66SessionSubsystem::GetCachedPartyRunSummaryJson(const FString& RequestKey, const FString& SteamId, FString& OutRunSummaryJson) const
{
	const TMap<FString, FString>* RequestCache = PartyRunSummaryJsonByRequestKey.Find(RequestKey);
	if (!RequestCache)
	{
		return false;
	}

	if (const FString* FoundJson = RequestCache->Find(SteamId))
	{
		OutRunSummaryJson = *FoundJson;
		return true;
	}

	return false;
}

void UT66SessionSubsystem::ClearCachedPartyRunSummaries()
{
	PartyRunSummaryJsonByRequestKey.Reset();
}

void UT66SessionSubsystem::StorePartyRunSummaryForSteamId(const FString& RequestKey, const FString& SteamId, const FString& RunSummaryJson)
{
	if (RequestKey.IsEmpty() || SteamId.IsEmpty() || RunSummaryJson.IsEmpty())
	{
		return;
	}

	PartyRunSummaryJsonByRequestKey.FindOrAdd(RequestKey).FindOrAdd(SteamId) = RunSummaryJson;
}

bool UT66SessionSubsystem::GetHostLobbyProfile(FT66LobbyPlayerInfo& OutLobbyInfo) const
{
	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	GetCurrentLobbyProfiles(LobbyProfiles);
	for (const FT66LobbyPlayerInfo& LobbyInfo : LobbyProfiles)
	{
		if (LobbyInfo.bPartyHost)
		{
			OutLobbyInfo = LobbyInfo;
			return true;
		}
	}

	if (LobbyProfiles.Num() > 0)
	{
		OutLobbyInfo = LobbyProfiles[0];
		return true;
	}

	return false;
}

ET66ScreenType UT66SessionSubsystem::GetDesiredPartyFrontendScreen() const
{
	FT66LobbyPlayerInfo HostLobbyInfo;
	if (GetHostLobbyProfile(HostLobbyInfo) && HostLobbyInfo.FrontendScreen != ET66ScreenType::None)
	{
		return HostLobbyInfo.FrontendScreen;
	}

	return PartyHubScreenType;
}

ET66Difficulty UT66SessionSubsystem::GetSharedLobbyDifficulty() const
{
	FT66LobbyPlayerInfo HostLobbyInfo;
	if (GetHostLobbyProfile(HostLobbyInfo))
	{
		return HostLobbyInfo.LobbyDifficulty;
	}

	if (const UT66GameInstance* GI = GetT66GameInstance())
	{
		return GI->SelectedDifficulty;
	}

	return ET66Difficulty::Easy;
}

bool UT66SessionSubsystem::AreAllPartyMembersReadyForGameplay(FString* OutFailureReason) const
{
	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	GetCurrentLobbyProfiles(LobbyProfiles);
	if (LobbyProfiles.Num() <= 0)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("No lobby players are available.");
		}
		return false;
	}

	for (const FT66LobbyPlayerInfo& LobbyInfo : LobbyProfiles)
	{
		if (LobbyInfo.SelectedHeroID.IsNone())
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s has not selected a hero yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}

		if (!LobbyInfo.bPartyHost && LobbyProfiles.Num() > 1 && !LobbyInfo.bLobbyReady)
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s is not ready yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}
	}

	return true;
}

bool UT66SessionSubsystem::AreAllPartyMembersReadyForMiniGameplay(FString* OutFailureReason) const
{
	TArray<FT66LobbyPlayerInfo> LobbyProfiles;
	GetCurrentLobbyProfiles(LobbyProfiles);
	if (LobbyProfiles.Num() <= 0)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("No mini party members are available.");
		}
		return false;
	}

	for (const FT66LobbyPlayerInfo& LobbyInfo : LobbyProfiles)
	{
		if (!LobbyInfo.bMiniFlowActive)
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s is not in the mini lobby yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}

		if (LobbyInfo.MiniSelectedHeroID.IsNone())
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s has not selected a mini hero yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}

		if (LobbyInfo.MiniSelectedCompanionID.IsNone())
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s has not selected a mini companion yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}

		if (LobbyInfo.MiniSelectedDifficultyID.IsNone())
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s has not selected a mini difficulty yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}

		if (LobbyInfo.MiniSelectedIdolIDs.Num() <= 0)
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s has not equipped any mini idols yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}

		if (!LobbyInfo.bPartyHost && LobbyProfiles.Num() > 1 && !LobbyInfo.bLobbyReady)
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("%s is not ready yet."), *LobbyInfo.DisplayName);
			}
			return false;
		}
	}

	return true;
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
		SubmitSessionDiagnostic(TEXT("create_party_session_interfaces_unavailable"), TEXT("error"), TEXT("Steam session interface is unavailable."));
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
	// The pre-game Steam lobby must stay joinable so invite acceptance can
	// discover and join the host before gameplay travel begins.
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
		SubmitSessionDiagnostic(TEXT("create_party_session_start_failed"), TEXT("error"), TEXT("CreateSession failed to start."));
		BroadcastStateChanged(TEXT("CreateSession failed to start."));
	}
	else
	{
		SubmitSessionDiagnostic(TEXT("create_party_session_started"), TEXT("info"), TEXT("Creating Steam lobby."));
		BroadcastStateChanged(TEXT("Creating Steam lobby..."));
	}

	return bStarted;
}

bool UT66SessionSubsystem::UpdatePartySessionJoinability(bool bAllowJoinInProgress, const TCHAR* Context)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(PartySessionName);
	if (!Session)
	{
		return false;
	}

	if (Session->SessionSettings.bAllowJoinInProgress == bAllowJoinInProgress)
	{
		return true;
	}

	FOnlineSessionSettings UpdatedSessionSettings = Session->SessionSettings;
	UpdatedSessionSettings.bAllowJoinInProgress = bAllowJoinInProgress;

	const bool bStarted = SessionInterface->UpdateSession(PartySessionName, UpdatedSessionSettings, true);
	if (bStarted)
	{
		UE_LOG(
			LogT66Session,
			Log,
			TEXT("Updating party session joinability to %s (context=%s) started=%d"),
			bAllowJoinInProgress ? TEXT("true") : TEXT("false"),
			Context ? Context : TEXT("unspecified"),
			1);
	}
	else
	{
		UE_LOG(
			LogT66Session,
			Warning,
			TEXT("Updating party session joinability to %s (context=%s) started=%d"),
			bAllowJoinInProgress ? TEXT("true") : TEXT("false"),
			Context ? Context : TEXT("unspecified"),
			0);
	}

	if (bStarted)
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("allow_join_in_progress"), bAllowJoinInProgress ? TEXT("true") : TEXT("false"));
		DiagnosticFields.Add(TEXT("context"), Context ? FString(Context) : TEXT("unspecified"));
		SubmitSessionDiagnostic(
			TEXT("party_session_joinability_update_started"),
			TEXT("info"),
			FString::Printf(TEXT("Updating party session joinability to %s."), bAllowJoinInProgress ? TEXT("true") : TEXT("false")),
			FString(),
			GetCurrentPartyLobbyId(),
			FString(),
			DiagnosticFields);
	}

	return bStarted;
}

bool UT66SessionSubsystem::JoinPartySession(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SubmitSessionDiagnostic(TEXT("join_session_interface_unavailable"), TEXT("error"), TEXT("Steam session interface is unavailable."));
		BroadcastStateChanged(TEXT("Steam session interface is unavailable."));
		return false;
	}

	bJoinInProgress = true;
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		GI->PendingFrontendScreen = PartyHubScreenType;
	}

	FString ResolvedLobbyId;
	if (SearchResult.Session.SessionInfo.IsValid())
	{
		ResolvedLobbyId = SearchResult.Session.SessionInfo->GetSessionId().ToString();
	}
	UE_LOG(LogT66Session, Log, TEXT("JoinPartySession owner=%s lobby=%s"), *SearchResult.Session.OwningUserName, *ResolvedLobbyId);
	TMap<FString, FString> JoinStartFields;
	JoinStartFields.Add(TEXT("resolved_lobby_id"), ResolvedLobbyId);
	JoinStartFields.Add(TEXT("owner_name"), SearchResult.Session.OwningUserName);
	SubmitSessionDiagnostic(TEXT("join_session_started"), TEXT("info"), TEXT("Starting JoinSession for party lobby."), FString(), ResolvedLobbyId, FString(), JoinStartFields);

	const bool bStarted = SessionInterface->JoinSession(0, PartySessionName, SearchResult);
	if (!bStarted)
	{
		bJoinInProgress = false;
		SubmitSessionDiagnostic(TEXT("join_session_start_failed"), TEXT("error"), TEXT("JoinSession failed to start."), FString(), ResolvedLobbyId, FString(), JoinStartFields);
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
	ClearCachedPartyRunSummaries();

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

void UT66SessionSubsystem::SubmitSessionDiagnostic(
	const FString& EventName,
	const FString& Severity,
	const FString& Message,
	const FString& InviteId,
	const FString& LobbyId,
	const FString& FoundLobbyId,
	const TMap<FString, FString>& ExtraFields) const
{
	UT66GameInstance* GI = GetT66GameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return;
	}

	FT66MultiplayerDiagnosticContext Diagnostic;
	Diagnostic.EventName = EventName;
	Diagnostic.Severity = Severity;
	Diagnostic.Message = Message;
	Diagnostic.InviteId = InviteId.IsEmpty() ? PendingJoinInviteId : InviteId;
	Diagnostic.HostSteamId = PendingJoinFriendPlayerId;
	Diagnostic.LobbyId = LobbyId.IsEmpty() ? PendingExpectedJoinLobbyId : LobbyId;
	Diagnostic.ExpectedLobbyId = PendingExpectedJoinLobbyId;
	Diagnostic.FoundLobbyId = FoundLobbyId.IsEmpty() ? PendingFoundLobbyId : FoundLobbyId;
	Diagnostic.SourceAppId = PendingJoinSourceAppId;
	Diagnostic.StatusText = LastStatusText;
	Diagnostic.ExtraFields = ExtraFields;
	Backend->SubmitMultiplayerDiagnostic(Diagnostic);
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
		TArray<FT66LobbyPlayerInfo> LobbyProfiles;
		GetCurrentLobbyProfiles(LobbyProfiles);
		const int32 CurrentPartySize = FMath::Max(1, LobbyProfiles.Num());
		Friends->SetRichPresence("steam_player_group_size", TCHAR_TO_UTF8(*FString::FromInt(CurrentPartySize)));
		if (const UT66SteamHelper* SteamHelper = GetSteamHelper())
		{
			const FString JoinToken = T66BuildSteamJoinConnectString(SteamHelper->GetLocalSteamId());
			Friends->SetRichPresence("connect", TCHAR_TO_UTF8(*JoinToken));
		}
	}
	else
	{
		Friends->SetRichPresence("steam_display", "#Status_MainMenu");
		Friends->SetRichPresence("steam_player_group_size", "");
		Friends->SetRichPresence("connect", "");
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
		LobbyInfo.LobbyDifficulty = GI->SelectedDifficulty;
		LobbyInfo.RunSeed = GI->RunSeed;
		LobbyInfo.MainMapLayoutVariant = GI->CurrentMainMapLayoutVariant;
		LobbyInfo.bMiniFlowActive = T66IsMiniFrontendScreen(LocalFrontendScreen);
		if (LobbyInfo.bMiniFlowActive)
		{
			LobbyInfo.MiniSelectedHeroID = GI->MiniSelectedHeroID;
			LobbyInfo.MiniSelectedCompanionID = GI->MiniSelectedCompanionID;
			LobbyInfo.MiniSelectedDifficultyID = GI->MiniSelectedDifficultyID;
			LobbyInfo.MiniSelectedIdolIDs = GI->MiniSelectedIdolIDs;
			LobbyInfo.bMiniLoadFlow = GI->bMiniLoadFlow;
			LobbyInfo.bMiniIntermissionFlow = GI->bMiniIntermissionFlow;
		}

		if (LobbyInfo.bMiniFlowActive
			|| GI->bMiniIntermissionFlow
			|| GI->MiniIntermissionStateRevision > 0
			|| GI->MiniIntermissionRequestRevision > 0)
		{
			LobbyInfo.MiniIntermissionStateRevision = GI->MiniIntermissionStateRevision;
			LobbyInfo.MiniIntermissionStateJson = GI->MiniIntermissionStateJson;
			LobbyInfo.MiniIntermissionRequestRevision = GI->MiniIntermissionRequestRevision;
			LobbyInfo.MiniIntermissionRequestJson = GI->MiniIntermissionRequestJson;
		}
	}

	if (LobbyInfo.DisplayName.IsEmpty())
	{
		LobbyInfo.DisplayName = TEXT("You");
	}

	LobbyInfo.bLobbyReady = bLocalReadyState;
	LobbyInfo.FrontendScreen = LocalFrontendScreen;
	LobbyInfo.bPartyHost = IsHostingPartySession() || bPendingCreateLobbySession;
	return LobbyInfo;
}

void UT66SessionSubsystem::ApplyLoadedRunToGameInstance(const UT66RunSaveGame* LoadedSave, int32 SaveSlotIndex) const
{
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI || !LoadedSave)
	{
		return;
	}

	GI->SelectedHeroID = LoadedSave->HeroID;
	GI->SelectedHeroBodyType = LoadedSave->HeroBodyType;
	GI->SelectedCompanionID = LoadedSave->CompanionID;
	GI->SelectedDifficulty = LoadedSave->Difficulty;
	GI->SelectedPartySize = LoadedSave->PartySize;
	GI->RunSeed = LoadedSave->RunSeed;
	if (LoadedSave->bIsDailyClimbRun && LoadedSave->DailyClimbChallenge.IsValid())
	{
		GI->CachedDailyClimbChallenge = LoadedSave->DailyClimbChallenge;
		GI->ActiveDailyClimbChallenge = LoadedSave->DailyClimbChallenge;
		GI->bIsDailyClimbRunActive = true;
		GI->SelectedRunModifierKind = ET66RunModifierKind::None;
		GI->SelectedRunModifierID = NAME_None;
		GI->SelectedPartySize = ET66PartySize::Solo;
	}
	else
	{
		GI->ClearActiveDailyClimbRun();
	}
	GI->CurrentMainMapLayoutVariant = ET66MainMapLayoutVariant::Tower;
	GI->PendingLoadedTransform = LoadedSave->PlayerTransform;
	GI->bApplyLoadedTransform = true;
	GI->PendingLoadedRunSnapshot = LoadedSave->RunSnapshot;
	GI->bApplyLoadedRunSnapshot = LoadedSave->RunSnapshot.bValid;
	GI->CurrentSaveSlotIndex = SaveSlotIndex;
	GI->bRunIneligibleForLeaderboard = LoadedSave->bRunIneligibleForLeaderboard;
	if (UT66RunIntegritySubsystem* Integrity = GI->GetSubsystem<UT66RunIntegritySubsystem>())
	{
		Integrity->RestoreActiveRunContext(LoadedSave->IntegrityContext);
		Integrity->MarkLoadedSnapshot();
		GI->bRunIneligibleForLeaderboard = GI->bRunIneligibleForLeaderboard || !Integrity->GetCurrentContext().ShouldAllowRankedSubmission();
	}
	GI->CurrentRunOwnerPlayerId = LoadedSave->OwnerPlayerId;
	GI->CurrentRunOwnerDisplayName = LoadedSave->OwnerDisplayName;
	GI->CurrentRunPartyMemberIds = LoadedSave->PartyMemberIds;
	GI->CurrentRunPartyMemberDisplayNames = LoadedSave->PartyMemberDisplayNames;
}

void UT66SessionSubsystem::ApplySavedPartyProfilesToCurrentSession(const UT66RunSaveGame* LoadedSave) const
{
	if (!LoadedSave)
	{
		return;
	}

	UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return;
	}

	TMap<FString, FT66SavedPartyPlayerState> SavedPlayersById;
	for (const FT66SavedPartyPlayerState& SavedPlayer : LoadedSave->SavedPartyPlayers)
	{
		if (!SavedPlayer.PlayerId.IsEmpty())
		{
			SavedPlayersById.Add(SavedPlayer.PlayerId, SavedPlayer);
		}
	}

	if (SavedPlayersById.Num() == 0)
	{
		return;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState);
		if (!SessionPlayerState)
		{
			continue;
		}

		const FT66SavedPartyPlayerState* SavedPlayer = SavedPlayersById.Find(SessionPlayerState->GetSteamId());
		if (!SavedPlayer)
		{
			continue;
		}

		FT66LobbyPlayerInfo LobbyInfo = SessionPlayerState->GetLobbyInfo();
		LobbyInfo.SteamId = SavedPlayer->PlayerId;
		LobbyInfo.DisplayName = SavedPlayer->DisplayName;
		LobbyInfo.SelectedHeroID = SavedPlayer->HeroID;
		LobbyInfo.SelectedHeroBodyType = SavedPlayer->HeroBodyType;
		LobbyInfo.SelectedHeroSkinID = SavedPlayer->HeroSkinID.IsNone() ? FName(TEXT("Default")) : SavedPlayer->HeroSkinID;
		LobbyInfo.SelectedCompanionID = SavedPlayer->CompanionID;
		LobbyInfo.SelectedCompanionBodyType = SavedPlayer->CompanionBodyType;
		LobbyInfo.bPartyHost = SavedPlayer->bIsPartyHost;
		SessionPlayerState->ApplyLobbyInfo(LobbyInfo);
	}
}

void UT66SessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionName != PartySessionName)
	{
		return;
	}

	bPendingCreateLobbySession = false;
	SubmitSessionDiagnostic(
		bWasSuccessful ? TEXT("create_party_session_success") : TEXT("create_party_session_failed"),
		bWasSuccessful ? TEXT("info") : TEXT("error"),
		bWasSuccessful ? TEXT("Steam lobby ready.") : TEXT("Steam lobby creation failed."),
		FString(),
		GetCurrentPartyLobbyId());
	BroadcastStateChanged(bWasSuccessful ? TEXT("Steam lobby ready.") : TEXT("Steam lobby creation failed."));
	SyncLocalLobbyProfile();
	UpdateSteamRichPresence();

	if (bWasSuccessful && !PendingInviteFriendPlayerId.IsEmpty())
	{
		const FString FriendPlayerId = PendingInviteFriendPlayerId;
		const FString FriendDisplayName = PendingInviteFriendDisplayName;
		PendingInviteFriendPlayerId.Reset();
		PendingInviteFriendDisplayName.Reset();
		SendInviteToFriendInternal(FriendPlayerId, FriendDisplayName);
	}
}

void UT66SessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionName != PartySessionName)
	{
		return;
	}

	bDestroyInProgress = false;
	ClearCachedPartyRunSummaries();

	if (bPendingJoinAfterDestroy)
	{
		bPendingJoinAfterDestroy = false;
		JoinPartySession(PendingJoinSearchResult);
		return;
	}

	if (!PendingDirectJoinConnectString.IsEmpty())
	{
		const FString ConnectString = PendingDirectJoinConnectString;
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("connect_string"), ConnectString);
		DiagnosticFields.Add(TEXT("continued_after_destroy"), TEXT("true"));
		SubmitSessionDiagnostic(
			bWasSuccessful ? TEXT("invite_join_destroy_complete") : TEXT("invite_join_destroy_failed_continue"),
			bWasSuccessful ? TEXT("info") : TEXT("warning"),
			bWasSuccessful
				? TEXT("Previous party lobby closed; continuing direct join.")
				: TEXT("DestroySession reported failure; continuing direct join anyway."),
			PendingJoinInviteId,
			PendingExpectedJoinLobbyId,
			FString(),
			DiagnosticFields);
		if (AT66PlayerController* PlayerController = GetPrimaryPlayerController())
		{
			SubmitSessionDiagnostic(TEXT("invite_join_direct_travel_started"), TEXT("info"), TEXT("Travelling directly to the invited Steam host."), PendingJoinInviteId, PendingExpectedJoinLobbyId, FString(), DiagnosticFields);
			BroadcastStateChanged(TEXT("Joining party lobby..."));
			PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
			ClearPendingJoinState();
			return;
		}

		SubmitSessionDiagnostic(TEXT("invite_join_direct_start_failed"), TEXT("error"), TEXT("Could not resolve the local player controller for direct Steam travel."), PendingJoinInviteId, PendingExpectedJoinLobbyId, FString(), DiagnosticFields);
		ClearPendingJoinState();
	}

	if (bPendingTravelToStandaloneFrontendAfterDestroy)
	{
		bPendingTravelToStandaloneFrontendAfterDestroy = false;
		TravelToStandaloneFrontendMap();
		return;
	}

	BroadcastStateChanged(bWasSuccessful ? TEXT("Lobby closed.") : TEXT("Lobby close failed."));
}

void UT66SessionSubsystem::HandleFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& FriendSearchResults)
{
	const FString ExpectedLobbyId = PendingExpectedJoinLobbyId;
	FString FoundLobbyId;
	if (bWasSuccessful && FriendSearchResults.Num() > 0 && FriendSearchResults[0].Session.SessionInfo.IsValid())
	{
		FoundLobbyId = FriendSearchResults[0].Session.SessionInfo->GetSessionId().ToString();
	}

	PendingFoundLobbyId = FoundLobbyId;

	UE_LOG(
		LogT66Session,
		Log,
		TEXT("FindFriendSession complete success=%d results=%d expectedLobby=%s foundLobby=%s friend=%s attempts=%d"),
		bWasSuccessful ? 1 : 0,
		FriendSearchResults.Num(),
		*ExpectedLobbyId,
		*FoundLobbyId,
		*PendingJoinFriendPlayerId,
		PendingJoinFriendLookupAttempts);

	if (!bWasSuccessful || FriendSearchResults.Num() == 0)
	{
		if (!PendingJoinFriendPlayerId.IsEmpty() && PendingJoinFriendLookupAttempts < T66MaxPendingFriendJoinLookupAttempts)
		{
			TMap<FString, FString> DiagnosticFields;
			DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
			SubmitSessionDiagnostic(TEXT("find_friend_session_retry"), TEXT("warning"), TEXT("Host party was not found yet. Retrying."), FString(), FString(), FoundLobbyId, DiagnosticFields);
			BroadcastStateChanged(TEXT("Host party was not found yet. Retrying..."));
			SchedulePendingFriendJoinRetry();
			return;
		}

		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
		SubmitSessionDiagnostic(TEXT("find_friend_session_not_found"), TEXT("error"), TEXT("Host party was not found."), FString(), FString(), FoundLobbyId, DiagnosticFields);
		BroadcastStateChanged(TEXT("Host party was not found."));
		ClearPendingJoinState();
		return;
	}

	ClearPendingFriendJoinRetry();
	const FOnlineSessionSearchResult& SearchResult = FriendSearchResults[0];
	if (!ExpectedLobbyId.IsEmpty() && !FoundLobbyId.IsEmpty() && FoundLobbyId != ExpectedLobbyId)
	{
		if (PendingJoinFriendLookupAttempts < T66MaxPendingFriendJoinLookupAttempts)
		{
			TMap<FString, FString> DiagnosticFields;
			DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
			SubmitSessionDiagnostic(TEXT("find_friend_session_outdated_retry"), TEXT("warning"), TEXT("Found an outdated host party. Retrying."), FString(), FString(), FoundLobbyId, DiagnosticFields);
			BroadcastStateChanged(TEXT("Found an outdated host party. Retrying..."));
			SchedulePendingFriendJoinRetry();
			return;
		}

		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
		SubmitSessionDiagnostic(TEXT("find_friend_session_lobby_mismatch"), TEXT("error"), TEXT("Found a host party, but it did not match the invite lobby."), FString(), FString(), FoundLobbyId, DiagnosticFields);
		BroadcastStateChanged(TEXT("Found a host party, but it did not match the invite lobby."));
		ClearPendingJoinState();
		return;
	}

	if (!PendingJoinFriendPlayerId.IsEmpty()
		&& !FoundLobbyId.IsEmpty()
		&& JoinPartySessionByLobbyId(FoundLobbyId, PendingJoinFriendPlayerId, PendingJoinSourceAppId, PendingJoinInviteId))
	{
		return;
	}

	if (IsPartySessionActive())
	{
		SubmitSessionDiagnostic(TEXT("find_friend_session_destroy_before_join"), TEXT("info"), TEXT("Destroying the current party before joining the target party."), FString(), FString(), FoundLobbyId);
		PendingJoinSearchResult = SearchResult;
		bPendingJoinAfterDestroy = true;
		DestroyPartySession();
		return;
	}

	JoinPartySession(SearchResult);
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
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("join_result"), T66JoinSessionResultToString(Result));
		DiagnosticFields.Add(TEXT("has_session_interface"), SessionInterface.IsValid() ? TEXT("true") : TEXT("false"));
		DiagnosticFields.Add(TEXT("has_player_controller"), PlayerController ? TEXT("true") : TEXT("false"));
		SubmitSessionDiagnostic(TEXT("join_session_missing_connect_target"), TEXT("error"), TEXT("Joined session, but no connect target was available."), FString(), FString(), FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Joined session, but no connect target was available."));
		return;
	}

	FString ConnectString;
	if (Result == EOnJoinSessionCompleteResult::Success && SessionInterface->GetResolvedConnectString(PartySessionName, ConnectString))
	{
		ClearPendingJoinState();
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("join_result"), T66JoinSessionResultToString(Result));
		DiagnosticFields.Add(TEXT("connect_string"), ConnectString);
		SubmitSessionDiagnostic(TEXT("join_session_success"), TEXT("info"), TEXT("Joined party lobby."), FString(), FString(), FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Joined party lobby."));
		PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
		return;
	}

	if (!PendingJoinFriendPlayerId.IsEmpty() && PendingJoinFriendLookupAttempts < T66MaxPendingFriendJoinLookupAttempts)
	{
		TMap<FString, FString> DiagnosticFields;
		DiagnosticFields.Add(TEXT("join_result"), T66JoinSessionResultToString(Result));
		DiagnosticFields.Add(TEXT("attempt"), FString::FromInt(PendingJoinFriendLookupAttempts));
		SubmitSessionDiagnostic(TEXT("join_session_retry"), TEXT("warning"), TEXT("Join target was not ready. Retrying."), FString(), FString(), FString(), DiagnosticFields);
		BroadcastStateChanged(TEXT("Join target was not ready. Retrying..."));
		SchedulePendingFriendJoinRetry();
		return;
	}

	TMap<FString, FString> DiagnosticFields;
	DiagnosticFields.Add(TEXT("join_result"), T66JoinSessionResultToString(Result));
	SubmitSessionDiagnostic(TEXT("join_session_failed"), TEXT("error"), TEXT("JoinSession completed without a valid connection."), FString(), FString(), FString(), DiagnosticFields);
	ClearPendingJoinState();
	BroadcastStateChanged(TEXT("JoinSession completed without a valid connection."));
}

void UT66SessionSubsystem::HandleStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionName == PartySessionName)
	{
		SubmitSessionDiagnostic(
			bWasSuccessful ? TEXT("party_session_start_success") : TEXT("party_session_start_failed"),
			bWasSuccessful ? TEXT("info") : TEXT("warning"),
			bWasSuccessful ? TEXT("Lobby session started.") : TEXT("Lobby session start failed."));
		BroadcastStateChanged(bWasSuccessful ? TEXT("Lobby session started.") : TEXT("Lobby session start failed."));
	}
}

void UT66SessionSubsystem::HandleSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful)
	{
		SubmitSessionDiagnostic(TEXT("steam_oss_invite_invalid"), TEXT("error"), TEXT("Steam invite was accepted, but the session result was invalid."));
		BroadcastStateChanged(TEXT("Steam invite was accepted, but the session result was invalid."));
		return;
	}

	UE_LOG(LogT66Session, Log, TEXT("Steam invite accepted via OSS. ControllerId=%d SessionOwner=%s"), ControllerId, *InviteResult.Session.OwningUserName);
	TMap<FString, FString> DiagnosticFields;
	DiagnosticFields.Add(TEXT("controller_id"), FString::FromInt(ControllerId));
	DiagnosticFields.Add(TEXT("session_owner"), InviteResult.Session.OwningUserName);
	SubmitSessionDiagnostic(TEXT("steam_oss_invite_accepted"), TEXT("info"), TEXT("Steam invite accepted via OSS."), FString(), FString(), FString(), DiagnosticFields);

	FString InviteLobbyId;
	if (InviteResult.Session.SessionInfo.IsValid())
	{
		InviteLobbyId = InviteResult.Session.SessionInfo->GetSessionId().ToString();
	}

	const FString HostSteamId = T66ExtractSearchResultHostSteamId(InviteResult);
	if (!HostSteamId.IsEmpty())
	{
		FString InviteAppId;
		if (UT66SteamHelper* SteamHelper = GetSteamHelper())
		{
			SteamHelper->TryGetFriendCurrentGame(HostSteamId, &InviteAppId, nullptr);
			if (InviteAppId.IsEmpty())
			{
				InviteAppId = SteamHelper->GetActiveSteamAppId();
			}
		}

		if (StartDirectJoinByHostSteamId(HostSteamId, InviteLobbyId, InviteAppId, FString(), TEXT("steam_oss_invite")))
		{
			return;
		}
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

void UT66SessionSubsystem::HandleSteamLobbyJoinRequested(const FString& FriendPlayerId, const FString& LobbyId)
{
	if (FriendPlayerId.IsEmpty() || LobbyId.IsEmpty())
	{
		return;
	}

	const FString LocalPlayerId = GetSteamHelper() ? GetSteamHelper()->GetLocalSteamId() : FString();
	if (!LocalPlayerId.IsEmpty() && FriendPlayerId == LocalPlayerId)
	{
		return;
	}

	FString InviteAppId;
	if (UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		SteamHelper->TryGetFriendCurrentGame(FriendPlayerId, &InviteAppId, nullptr);
		if (InviteAppId.IsEmpty())
		{
			InviteAppId = SteamHelper->GetActiveSteamAppId();
		}
	}

	UE_LOG(LogT66Session, Log, TEXT("Steam lobby join request received. Host=%s Lobby=%s App=%s"), *FriendPlayerId, *LobbyId, *InviteAppId);
	TMap<FString, FString> DiagnosticFields;
	DiagnosticFields.Add(TEXT("host_steam_id"), FriendPlayerId);
	DiagnosticFields.Add(TEXT("lobby_id"), LobbyId);
	if (!InviteAppId.IsEmpty())
	{
		DiagnosticFields.Add(TEXT("invite_app_id"), InviteAppId);
	}
	SubmitSessionDiagnostic(TEXT("steam_lobby_join_requested"), TEXT("info"), TEXT("Steam lobby join request received."), FString(), LobbyId, FString(), DiagnosticFields);

	if (!JoinPartySessionByLobbyId(LobbyId, FriendPlayerId, InviteAppId))
	{
		SubmitSessionDiagnostic(TEXT("steam_lobby_join_request_failed"), TEXT("error"), TEXT("Steam lobby join request could not be resolved into a join."), FString(), LobbyId, FString(), DiagnosticFields);
	}
}

void UT66SessionSubsystem::HandleSteamJoinRequested(const FString& FriendPlayerId)
{
	if (FriendPlayerId.IsEmpty())
	{
		return;
	}

	const FString LocalPlayerId = GetSteamHelper() ? GetSteamHelper()->GetLocalSteamId() : FString();
	if (!LocalPlayerId.IsEmpty() && FriendPlayerId == LocalPlayerId)
	{
		return;
	}

	UE_LOG(LogT66Session, Log, TEXT("Steam rich-presence join request fallback received for friend %s."), *FriendPlayerId);
	TMap<FString, FString> DiagnosticFields;
	DiagnosticFields.Add(TEXT("host_steam_id"), FriendPlayerId);
	SubmitSessionDiagnostic(TEXT("steam_rich_presence_join_requested"), TEXT("info"), TEXT("Steam rich presence join request received via fallback path."), FString(), FString(), FString(), DiagnosticFields);
	JoinFriendPartySessionBySteamId(FriendPlayerId);
}
