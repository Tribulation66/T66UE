// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Core/T66CommunityContentTypes.h"
#include "T66CommunityContentSubsystem.generated.h"

class UT66RunStateSubsystem;
class UT66CommunityContentSaveGame;

DECLARE_MULTICAST_DELEGATE(FOnT66CommunityContentChanged);

UCLASS()
class T66_API UT66CommunityContentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const TArray<FT66CommunityContentEntry>& GetOfficialEntries(ET66CommunityContentKind Kind) const;
	const TArray<FT66CommunityContentEntry>& GetCommunityEntries(ET66CommunityContentKind Kind) const;
	const TArray<FT66CommunityContentEntry>& GetDraftEntries(ET66CommunityContentKind Kind) const;
	TArray<FT66CommunityContentEntry> GetCommunityBrowserEntries(ET66CommunityContentKind Kind) const;

	FT66CommunityContentEntry CreateDraftTemplate(ET66CommunityContentKind Kind) const;
	bool SaveDraft(const FT66CommunityContentEntry& Draft);
	bool DeleteDraft(FName LocalId);
	bool FindEntryById(FName LocalId, FT66CommunityContentEntry& OutEntry) const;
	bool ActivateEntry(FName LocalId);
	bool GetActiveEntry(FT66CommunityContentEntry& OutEntry) const;
	void ClearActiveEntry();

	void RefreshCommunityCatalog(bool bForce = false);
	void RefreshMySubmissionStates(bool bForce = false);
	bool SubmitDraftForApproval(FName LocalId);

	TArray<FString> BuildRuleSummaryLines(const FT66CommunityContentEntry& Entry) const;
	FString BuildRewardSummary(const FT66CommunityContentEntry& Entry) const;
	FString BuildSelectionSummary(const FT66CommunityContentEntry& Entry) const;
	bool EvaluateChallengeCompletion(const FT66CommunityContentEntry& Entry, const UT66RunStateSubsystem* RunState, FString& OutFailureReason) const;
	int32 GetApprovedRewardForActiveChallenge(const UT66RunStateSubsystem* RunState, FString* OutRewardLabel = nullptr, FString* OutFailureReason = nullptr) const;
	ET66PassiveType GetActivePassiveOverride() const;
	ET66UltimateType GetActiveUltimateOverride() const;

	bool IsCatalogRefreshInFlight() const { return bCatalogRefreshInFlight; }
	bool IsSubmitInFlight() const { return bSubmitInFlight; }
	const FString& GetLastStatusMessage() const { return LastStatusMessage; }

	FOnT66CommunityContentChanged& OnContentChanged() { return ContentChanged; }

private:
	void SeedOfficialContent();
	void LoadOrCreateSave();
	void PersistSave();

	TArray<FT66CommunityContentEntry>& GetMutableBucket(ET66CommunityContentOrigin Origin, ET66CommunityContentKind Kind);
	const TArray<FT66CommunityContentEntry>& GetBucket(ET66CommunityContentOrigin Origin, ET66CommunityContentKind Kind) const;
	bool ResolveActiveEntry(FT66CommunityContentEntry& OutEntry) const;
	FString GetBackendBaseUrl() const;
	FString GetSteamTicketHex() const;
	void BroadcastContentChanged();

	void OnCatalogResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnMySubmissionsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void OnSubmitResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FName DraftId);
	static FString KindToApiString(ET66CommunityContentKind Kind);
	static bool TryParseApiEntry(const TSharedPtr<class FJsonObject>& Json, FT66CommunityContentEntry& OutEntry);

	UPROPERTY(Transient)
	TObjectPtr<UT66CommunityContentSaveGame> SaveData;

	TArray<FT66CommunityContentEntry> OfficialChallenges;
	TArray<FT66CommunityContentEntry> OfficialMods;
	TArray<FT66CommunityContentEntry> CommunityChallenges;
	TArray<FT66CommunityContentEntry> CommunityMods;
	TArray<FT66CommunityContentEntry> DraftChallenges;
	TArray<FT66CommunityContentEntry> DraftMods;

	FName ActiveEntryId = NAME_None;
	bool bCatalogRefreshInFlight = false;
	bool bMySubmissionsRefreshInFlight = false;
	bool bSubmitInFlight = false;
	FString LastStatusMessage;
	FOnT66CommunityContentChanged ContentChanged;
};
