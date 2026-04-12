// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Delegates/Delegate.h"
#include "Data/T66DataTypes.h"
#include "UI/T66UITypes.h"
#include "T66PartySubsystem.generated.h"

class UT66RunSaveGame;

USTRUCT(BlueprintType)
struct T66_API FT66PartyFriendEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString PlayerId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString PresenceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	bool bOnline = false;
};

USTRUCT(BlueprintType)
struct T66_API FT66PartyMemberEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString PlayerId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	bool bIsLocal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	bool bOnline = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	bool bReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Party")
	bool bIsPartyHost = false;
};

DECLARE_MULTICAST_DELEGATE(FOnT66PartyStateChanged);

UCLASS()
class T66_API UT66PartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	const TArray<FT66PartyFriendEntry>& GetFriends() const { return Friends; }
	const TArray<FT66PartyMemberEntry>& GetPartyMembers() const { return PartyMembers; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Party")
	int32 GetPartyMemberCount() const { return PartyMembers.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Party")
	bool HasRemotePartyMembers() const { return PartyMembers.Num() > 1; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Party")
	FString GetLocalPlayerId() const { return LocalPlayerId; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Party")
	FString GetLocalDisplayName() const { return LocalDisplayName; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Party")
	bool IsFriendInParty(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Party")
	bool InviteFriend(const FString& PlayerId, const FString& DisplayName = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Party")
	bool RemovePartyMember(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Party")
	void ResetToLocalParty();

	UFUNCTION(BlueprintCallable, Category = "Party")
	void ApplyCurrentPartyToGameInstanceRunContext();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Party")
	ET66PartySize GetCurrentPartySizeEnum() const;

	TArray<FString> GetCurrentPartyMemberIds() const;
	TArray<FString> GetCurrentPartyMemberDisplayNames() const;
	bool DoesSaveMatchCurrentParty(const UT66RunSaveGame* SaveGame) const;

	FOnT66PartyStateChanged& OnPartyStateChanged() { return PartyStateChanged; }

private:
	bool HandlePartyTicker(float DeltaTime);
	void RefreshPartyState(bool bBroadcastChanges);
	void RebuildFriendList();
	void RebuildPartyMembers();
	int32 GetMaxPartyMembers() const;
	void SyncSelectedPartySize();
	const FT66PartyFriendEntry* FindFriend(const FString& PlayerId) const;
	int32 FindPartyMemberIndex(const FString& PlayerId) const;

	TArray<FT66PartyFriendEntry> Friends;
	TArray<FT66PartyMemberEntry> PartyMembers;
	FString LocalPlayerId;
	FString LocalDisplayName;
	int32 LastObservedAvatarRevision = INDEX_NONE;
	ET66ScreenType LastObservedDesiredFrontendScreen = ET66ScreenType::None;
	ET66Difficulty LastObservedSharedDifficulty = ET66Difficulty::Easy;
	FOnT66PartyStateChanged PartyStateChanged;
	FTSTicker::FDelegateHandle PartyTickerHandle;
};
