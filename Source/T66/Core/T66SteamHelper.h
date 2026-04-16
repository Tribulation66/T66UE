// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66SteamHelper.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSteamTicketReady,
	bool, bSuccess,
	const FString&, TicketHex);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnT66SteamJoinRequested, const FString& /*FriendSteamId*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnT66SteamLobbyJoinRequested, const FString& /*FriendSteamId*/, const FString& /*LobbySteamId*/);

USTRUCT(BlueprintType)
struct T66_API FT66SteamFriendInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	FString SteamId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	FString PresenceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	bool bOnline = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	bool bPlayingThisGame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	FString CurrentGameAppId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steam")
	FString CurrentLobbyId;
};

/**
 * Manages Steam identity for the T66 backend.
 *
 * On Initialize, requests a Steam Web API ticket via the
 * Steamworks API. Once ready, it hex-encodes the ticket
 * and passes it to UT66BackendSubsystem::SetSteamTicketHex.
 *
 * Also caches the local player's SteamID and Steam friends list
 * for the leaderboard Friends tab.
 */
UCLASS()
class T66_API UT66SteamHelper : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** True if Steam API is initialized and a ticket has been obtained. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	bool IsSteamReady() const { return bSteamReady; }

	/** Local player SteamID as string (e.g. "76561198012345678"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	FString GetLocalSteamId() const { return LocalSteamIdStr; }

	/** The hex-encoded session ticket (empty until ready). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	FString GetTicketHex() const { return TicketHex; }

	/** Get the Steam display name of the local player. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	FString GetLocalDisplayName() const { return LocalDisplayName; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	FString GetActiveSteamAppId() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	int32 GetActiveSteamBuildId() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	FString GetCurrentSteamBetaName() const;

	/** Get friend SteamIDs (for leaderboard friends tab). */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	TArray<FString> GetFriendSteamIds() const { return FriendSteamIds; }

	UFUNCTION(BlueprintCallable, Category = "Steam")
	TArray<FT66SteamFriendInfo> GetFriendInfos() const { return FriendInfos; }

	/** Refresh local identity, avatar, and Steam friends presence from the live Steam client. */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	void RefreshSteamPresence();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	UTexture2D* GetLocalAvatarTexture() const { return LocalAvatarTexture; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	UTexture2D* GetAvatarTextureForSteamId(const FString& SteamId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	int32 GetAvatarRevision() const { return AvatarRevision; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Steam")
	bool IsFriendPlayingActiveApp(const FString& SteamId) const;

	bool TryGetFriendCurrentGame(const FString& SteamId, FString* OutAppId = nullptr, FString* OutLobbyId = nullptr) const;

	/** Request a new session ticket (call again if the old one expired). */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	void RequestNewTicket();

	UPROPERTY(BlueprintAssignable, Category = "Steam")
	FOnSteamTicketReady OnSteamTicketReady;

	FOnT66SteamJoinRequested& OnSteamJoinRequested() { return SteamJoinRequested; }
	FOnT66SteamLobbyJoinRequested& OnSteamLobbyJoinRequested() { return SteamLobbyJoinRequested; }
	void HandleSteamJoinRequested(const FString& FriendSteamId);
	void HandleSteamLobbyJoinRequested(const FString& FriendSteamId, const FString& LobbySteamId);
	void HandleWebApiTicketReady(uint32 InTicketHandle, bool bSuccess, const uint8* TicketBytes, int32 TicketByteCount);

private:
	bool bSteamReady = false;
	FString LocalSteamIdStr;
	FString LocalDisplayName;
	FString TicketHex;
	TArray<FString> FriendSteamIds;
	TArray<FT66SteamFriendInfo> FriendInfos;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> LocalAvatarTexture;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UTexture2D>> FriendAvatarTextures;

	int32 AvatarRevision = 0;

	// Steamworks ticket handle (needed to cancel)
	uint32 TicketHandle = 0;
	class FT66SteamJoinRequestBridge* SteamJoinRequestBridge = nullptr;
	FOnT66SteamJoinRequested SteamJoinRequested;
	FOnT66SteamLobbyJoinRequested SteamLobbyJoinRequested;

	void ObtainTicket();
	void CollectFriendsList();
	void RefreshLocalSteamIdentity();
	void CacheAvatarForSteamId(const FString& SteamId, int32 ImageHandle);
	UTexture2D* CreateTextureFromSteamImage(int32 ImageHandle) const;
};
