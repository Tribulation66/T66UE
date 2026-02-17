// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66SteamHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSteamTicketReady,
	bool, bSuccess,
	const FString&, TicketHex);

/**
 * Manages Steam identity for the T66 backend.
 *
 * On Initialize, requests a Steam session ticket via the
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

	/** Get friend SteamIDs (for leaderboard friends tab). */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	TArray<FString> GetFriendSteamIds() const { return FriendSteamIds; }

	/** Request a new session ticket (call again if the old one expired). */
	UFUNCTION(BlueprintCallable, Category = "Steam")
	void RequestNewTicket();

	UPROPERTY(BlueprintAssignable, Category = "Steam")
	FOnSteamTicketReady OnSteamTicketReady;

private:
	bool bSteamReady = false;
	FString LocalSteamIdStr;
	FString LocalDisplayName;
	FString TicketHex;
	TArray<FString> FriendSteamIds;

	// Steamworks ticket handle (needed to cancel)
	uint32 TicketHandle = 0;

	void ObtainTicket();
	void CollectFriendsList();
};
