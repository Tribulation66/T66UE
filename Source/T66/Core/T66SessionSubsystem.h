// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/T66UITypes.h"
#include "T66SessionSubsystem.generated.h"

class AT66PlayerController;

DECLARE_MULTICAST_DELEGATE(FOnT66SessionStateChanged);

UCLASS()
class T66_API UT66SessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool EnsurePartySessionReady(ET66PartySize DesiredPartySize = ET66PartySize::Duo, ET66ScreenType PartyHubScreen = ET66ScreenType::MainMenu);
	bool PrepareToHostFrontendLobby(ET66PartySize DesiredPartySize);
	void HandlePartyHubScreenActivated();
	void HandleLobbyScreenActivated();
	bool SendInviteToFriend(const FString& FriendPlayerId);
	bool StartLoadedGameplayTravel(const class UT66RunSaveGame* LoadedSave, int32 SaveSlotIndex);
	bool StartGameplayTravel();
	bool LeaveFrontendLobby(ET66ScreenType ReturnScreen = ET66ScreenType::MainMenu);
	bool SyncLocalLobbyProfile();
	void SetLocalLobbyReady(bool bReady);

	bool IsPartySessionActive() const;
	bool IsHostingPartySession() const;
	bool IsJoiningPartySession() const { return bJoinInProgress; }
	bool CanSendInvites() const;
	int32 GetMaxPartyMembers() const;
	const FString& GetLastStatusText() const { return LastStatusText; }

	FOnT66SessionStateChanged& OnSessionStateChanged() { return SessionStateChanged; }

private:
	static const FName PartySessionName;

	IOnlineSessionPtr GetSessionInterface() const;
	IOnlineIdentityPtr GetIdentityInterface() const;
	AT66PlayerController* GetPrimaryPlayerController() const;
	class UT66GameInstance* GetT66GameInstance() const;
	class UT66SteamHelper* GetSteamHelper() const;

	bool CreatePartySession();
	bool JoinPartySession(const FOnlineSessionSearchResult& SearchResult);
	void DestroyPartySession();
	void BroadcastStateChanged(const TCHAR* NewStatus = nullptr);
	void TravelToStandaloneFrontendMap();
	void UpdateSteamRichPresence();
	void ClearSteamRichPresence();
	struct FT66LobbyPlayerInfo BuildLocalLobbyProfile() const;
	bool SendInviteToFriendInternal(const FString& FriendPlayerId);
	void ApplyLoadedRunToGameInstance(const class UT66RunSaveGame* LoadedSave, int32 SaveSlotIndex) const;
	void ApplySavedPartyProfilesToCurrentSession(const class UT66RunSaveGame* LoadedSave) const;

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle StartSessionCompleteHandle;
	FDelegateHandle SessionInviteAcceptedHandle;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnSessionUserInviteAcceptedDelegate SessionInviteAcceptedDelegate;

	ET66PartySize PendingPartySize = ET66PartySize::Duo;
	bool bPendingCreateLobbySession = false;
	bool bJoinInProgress = false;
	bool bDestroyInProgress = false;
	bool bPendingJoinAfterDestroy = false;
	bool bPendingTravelToStandaloneFrontendAfterDestroy = false;
	bool bLocalReadyState = false;
	FOnlineSessionSearchResult PendingJoinSearchResult;
	ET66ScreenType PendingFrontendReturnScreen = ET66ScreenType::MainMenu;
	ET66ScreenType PartyHubScreenType = ET66ScreenType::MainMenu;
	FString PendingInviteFriendPlayerId;
	FString LastStatusText;
	FOnT66SessionStateChanged SessionStateChanged;
};
