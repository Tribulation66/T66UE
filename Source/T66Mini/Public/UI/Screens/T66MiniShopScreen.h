// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Save/T66MiniRunSaveGame.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniShopScreen.generated.h"

class STextBlock;

USTRUCT()
struct T66MINI_API FT66MiniShopSyncPayload
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Revision = 0;

	UPROPERTY()
	FString StatusText;

	UPROPERTY()
	FString HostSteamId;

	UPROPERTY()
	FString HostDisplayName;

	UPROPERTY()
	FName DifficultyID = NAME_None;

	UPROPERTY()
	int32 WaveIndex = 1;

	UPROPERTY()
	float TotalRunSeconds = 0.f;

	UPROPERTY()
	TArray<FName> CurrentShopOfferIDs;

	UPROPERTY()
	TArray<FName> LockedShopOfferIDs;

	UPROPERTY()
	int32 ShopRerollCount = 0;

	UPROPERTY()
	int32 CircusDebt = 0;

	UPROPERTY()
	float CircusAnger01 = 0.f;

	UPROPERTY()
	TArray<FName> CircusBuybackItemIDs;

	UPROPERTY()
	int32 CircusVendorRerollCount = 0;

	UPROPERTY()
	TArray<FString> OnlinePartyMemberIds;

	UPROPERTY()
	TArray<FString> OnlinePartyMemberDisplayNames;

	UPROPERTY()
	TArray<FT66MiniPartyPlayerSnapshot> PartyPlayerSnapshots;
};

USTRUCT()
struct T66MINI_API FT66MiniShopRequestPayload
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Revision = 0;

	UPROPERTY()
	FName Action = NAME_None;

	UPROPERTY()
	FName ItemID = NAME_None;

	UPROPERTY()
	FName GameID = NAME_None;

	UPROPERTY()
	int32 Amount = 0;
};

UCLASS(Blueprintable)
class T66MINI_API UT66MiniShopScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniShopScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EMiniCircusTab : uint8
	{
		Vendor = 0,
		Gambling,
		Alchemy
	};

	FReply HandleBuyItemClicked(FName ItemID);
	FReply HandleStealItemClicked(FName ItemID);
	FReply HandleSellItemClicked(FName ItemID);
	FReply HandleBuybackClicked(FName ItemID);
	FReply HandleLockClicked(FName ItemID);
	FReply HandleRerollClicked();
	FReply HandleBorrowGoldClicked(int32 Amount);
	FReply HandlePayDebtClicked(int32 Amount);
	FReply HandleCircusGameClicked(FName GameID, FString GameTitle);
	FReply HandleAlchemyTransmuteClicked();
	FReply HandleAlchemyDissolveClicked();
	FReply HandleContinueClicked();
	FReply HandleVendorTabClicked();
	FReply HandleGamblingTabClicked();
	FReply HandleAlchemyTabClicked();
	bool HandleShopSyncTicker(float DeltaTime);
	void HandlePartyStateChanged();
	void HandleSessionStateChanged();
	void SyncToSharedPartyScreen();
	FString BuildShopUiStateKey() const;
	void RequestShopRebuildIfStateChanged();
	bool IsOnlineMiniParty(bool* bOutIsHost = nullptr) const;
	bool ResolveLocalPartyIdentity(FString& OutPlayerId, FString& OutDisplayName) const;
	void ResetMiniIntermissionTransport() const;
	bool BuildAuthoritativeSyncPayload(FT66MiniShopSyncPayload& OutPayload) const;
	bool PublishAuthoritativeState(const FText& StatusText, bool bIncrementRevision);
	bool ApplyAuthoritativeStateFromLobby(bool bForceRebuild);
	bool TryQueueRemoteRequest(const FT66MiniShopRequestPayload& Request, const FText& PendingStatus);
	bool TryProcessRemoteRequests(bool bForceRebuild);
	bool ExecuteAuthoritativeRequest(const FString& SourcePlayerId, const FString& SourceDisplayName, const FT66MiniShopRequestPayload& Request, bool& bOutNavigateToIdolSelect);
	void SetStatus(const FText& InText);

	TSharedPtr<STextBlock> StatusTextBlock;
	FText CurrentStatusText;
	EMiniCircusTab ActiveTab = EMiniCircusTab::Vendor;
	int32 LastAppliedStateRevision = 0;
	FString LastShopUiStateKey;
	TMap<FString, int32> ProcessedRequestRevisionByPlayerId;
	FDelegateHandle PartyStateChangedHandle;
	FDelegateHandle SessionStateChangedHandle;
	FTSTicker::FDelegateHandle ShopSyncTickerHandle;
};
