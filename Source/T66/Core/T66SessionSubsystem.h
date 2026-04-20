// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
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
	bool SendInviteToFriend(const FString& FriendPlayerId, const FString& FriendDisplayName = FString());
	bool JoinFriendPartySessionBySteamId(const FString& FriendPlayerId, const FString& InviteId = FString(), const FString& AppId = FString());
	bool JoinPartySessionByLobbyId(const FString& LobbyId, const FString& HostSteamId = FString(), const FString& AppId = FString(), const FString& InviteId = FString());
	bool StartLoadedGameplayTravel(const class UT66RunSaveGame* LoadedSave, int32 SaveSlotIndex);
	bool StartGameplayTravel();
	bool PreparePartySessionForWorldTravel(const TCHAR* Context = nullptr);
	bool SaveCurrentRunAndReturnToFrontend();
	bool LeaveFrontendLobby(ET66ScreenType ReturnScreen = ET66ScreenType::MainMenu);
	bool SyncLocalLobbyProfile();
	void SetLocalLobbyReady(bool bReady);
	void SetLocalFrontendScreen(ET66ScreenType ScreenType, bool bResetReady = false);

	bool IsPartySessionActive() const;
	bool IsPartyLobbyContextActive() const;
	bool IsHostingPartySession() const;
	bool IsJoiningPartySession() const { return bJoinInProgress; }
	bool CanSendInvites() const;
	int32 GetMaxPartyMembers() const;
	FString GetCurrentPartyLobbyId() const;
	int32 GetCurrentLobbyPlayerCount() const;
	bool IsLocalLobbyReady() const { return bLocalReadyState; }
	bool IsLocalPlayerPartyHost() const;
	bool IsGameplaySaveAndReturnInProgress() const { return bGameplaySaveAndReturnInProgress; }
	bool IsFriendInvitePending(const FString& FriendPlayerId);
	bool GetHostLobbyProfile(struct FT66LobbyPlayerInfo& OutLobbyInfo) const;
	void GetCurrentLobbyProfiles(TArray<FT66LobbyPlayerInfo>& OutProfiles) const;
	bool SubmitLocalPartyRunSummaryToHost(const FString& RequestKey, const FString& RunSummaryJson);
	bool GetCachedPartyRunSummaryJson(const FString& RequestKey, const FString& SteamId, FString& OutRunSummaryJson) const;
	int32 GetCachedPartyRunSummaryCount() const { return PartyRunSummaryJsonByRequestKey.Num(); }
	void ClearCachedPartyRunSummaries();
	void StorePartyRunSummaryForSteamId(const FString& RequestKey, const FString& SteamId, const FString& RunSummaryJson);
	ET66ScreenType GetDesiredPartyFrontendScreen() const;
	ET66Difficulty GetSharedLobbyDifficulty() const;
	bool AreAllPartyMembersReadyForGameplay(FString* OutFailureReason = nullptr) const;
	bool AreAllPartyMembersReadyForMiniGameplay(FString* OutFailureReason = nullptr) const;
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
	bool UpdatePartySessionJoinability(bool bAllowJoinInProgress, const TCHAR* Context = nullptr);
	void DestroyPartySession();
	void BroadcastStateChanged(const TCHAR* NewStatus = nullptr);
	void TravelToStandaloneFrontendMap();
	void UpdateSteamRichPresence();
	void ClearSteamRichPresence();
	struct FT66LobbyPlayerInfo BuildLocalLobbyProfile() const;
	bool SendInviteToFriendInternal(const FString& FriendPlayerId, const FString& FriendDisplayName);
	void PrimePendingJoinContext(const FString& HostSteamId, const FString& LobbyId, const FString& InviteId, const FString& AppId);
	bool StartDirectJoinByHostSteamId(
		const FString& HostSteamId,
		const FString& LobbyId = FString(),
		const FString& AppId = FString(),
		const FString& InviteId = FString(),
		const TCHAR* JoinReason = nullptr);
	bool StartJoinByFriendId(const FString& FriendPlayerId, const FString& ExpectedLobbyId = FString(), const FString& InviteId = FString(), const FString& AppId = FString());
	bool AttemptPendingFriendJoinLookup();
	void SchedulePendingFriendJoinRetry();
	void ClearPendingFriendJoinRetry();
	void ClearPendingJoinState();
	void PruneInviteFeedbackState();
	class UT66RunSaveGame* BuildCurrentRunSaveSnapshot(UObject* Outer) const;
	void ApplyLoadedRunToGameInstance(const class UT66RunSaveGame* LoadedSave, int32 SaveSlotIndex) const;
	void ApplySavedPartyProfilesToCurrentSession(const class UT66RunSaveGame* LoadedSave) const;
	void SubmitSessionDiagnostic(
		const FString& EventName,
		const FString& Severity,
		const FString& Message = FString(),
		const FString& InviteId = FString(),
		const FString& LobbyId = FString(),
		const FString& FoundLobbyId = FString(),
		const TMap<FString, FString>& ExtraFields = TMap<FString, FString>()) const;

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& FriendSearchResults);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void HandleSteamLobbyJoinRequested(const FString& FriendPlayerId, const FString& LobbyId);
	void HandleSteamJoinRequested(const FString& FriendPlayerId);
	bool HandlePendingFriendJoinRetryTicker(float DeltaTime);

	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;
	FDelegateHandle FindFriendSessionCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle StartSessionCompleteHandle;
	FDelegateHandle SessionInviteAcceptedHandle;
	FDelegateHandle SteamLobbyJoinRequestedHandle;
	FDelegateHandle SteamJoinRequestedHandle;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnFindFriendSessionCompleteDelegate FindFriendSessionCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnSessionUserInviteAcceptedDelegate SessionInviteAcceptedDelegate;

	ET66PartySize PendingPartySize = ET66PartySize::Duo;
	bool bPendingCreateLobbySession = false;
	bool bJoinInProgress = false;
	bool bDestroyInProgress = false;
	bool bPendingJoinAfterDestroy = false;
	bool bPendingTravelToStandaloneFrontendAfterDestroy = false;
	bool bGameplaySaveAndReturnInProgress = false;
	bool bLocalReadyState = false;
	ET66ScreenType LocalFrontendScreen = ET66ScreenType::MainMenu;
	FOnlineSessionSearchResult PendingJoinSearchResult;
	ET66ScreenType PendingFrontendReturnScreen = ET66ScreenType::MainMenu;
	ET66ScreenType PartyHubScreenType = ET66ScreenType::MainMenu;
	FString PendingInviteFriendPlayerId;
	FString PendingInviteFriendDisplayName;
	FString PendingJoinFriendPlayerId;
	FString PendingExpectedJoinLobbyId;
	FString PendingJoinInviteId;
	FString PendingJoinSourceAppId;
	FString PendingFoundLobbyId;
	FString PendingDirectJoinConnectString;
	int32 PendingJoinFriendLookupAttempts = 0;
	FTSTicker::FDelegateHandle PendingJoinFriendRetryTickerHandle;
	TMap<FString, double> InviteFeedbackExpiryByFriendId;
	TMap<FString, TMap<FString, FString>> PartyRunSummaryJsonByRequestKey;
	FString LastStatusText;
	FOnT66SessionStateChanged SessionStateChanged;
};
