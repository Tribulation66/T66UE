// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniLeaderboardSubsystem.h"

#include "Engine/GameInstance.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY_STATIC(LogT66MiniLeaderboard, Log, All);

namespace
{
	constexpr int32 MiniLeaderboardVisibleEntryCount = 10;

	FString T66MiniDefaultStatusMessage()
	{
		return TEXT("All-time score is based on materials banked.");
	}
}

class FT66MiniSteamLeaderboardBridge
{
public:
	explicit FT66MiniSteamLeaderboardBridge(UT66MiniLeaderboardSubsystem& InOwner)
		: Owner(InOwner)
	{
	}

	void StartFetchFind(const SteamAPICall_t ApiCall)
	{
		FetchFindCallResult.Set(ApiCall, this, &FT66MiniSteamLeaderboardBridge::OnFetchFind);
	}

	void StartFetchDownload(const SteamAPICall_t ApiCall)
	{
		FetchDownloadCallResult.Set(ApiCall, this, &FT66MiniSteamLeaderboardBridge::OnFetchDownload);
	}

	void StartUploadFind(const SteamAPICall_t ApiCall)
	{
		UploadFindCallResult.Set(ApiCall, this, &FT66MiniSteamLeaderboardBridge::OnUploadFind);
	}

	void StartUploadScore(const SteamAPICall_t ApiCall)
	{
		UploadScoreCallResult.Set(ApiCall, this, &FT66MiniSteamLeaderboardBridge::OnUploadScore);
	}

private:
	void OnFetchFind(LeaderboardFindResult_t* Param, const bool bIOFailure)
	{
		const bool bSuccess = Param && !bIOFailure && Param->m_bLeaderboardFound != 0;
		Owner.HandleFetchLeaderboardFound(bSuccess, bSuccess ? static_cast<uint64>(Param->m_hSteamLeaderboard) : 0);
	}

	void OnFetchDownload(LeaderboardScoresDownloaded_t* Param, const bool bIOFailure)
	{
		const bool bSuccess = Param && !bIOFailure;
		Owner.HandleFetchLeaderboardDownloaded(
			bSuccess,
			bSuccess ? static_cast<uint64>(Param->m_hSteamLeaderboard) : 0,
			bSuccess ? static_cast<uint64>(Param->m_hSteamLeaderboardEntries) : 0,
			bSuccess ? Param->m_cEntryCount : 0);
	}

	void OnUploadFind(LeaderboardFindResult_t* Param, const bool bIOFailure)
	{
		const bool bSuccess = Param && !bIOFailure && Param->m_bLeaderboardFound != 0;
		Owner.HandleUploadLeaderboardFound(bSuccess, bSuccess ? static_cast<uint64>(Param->m_hSteamLeaderboard) : 0);
	}

	void OnUploadScore(LeaderboardScoreUploaded_t* Param, const bool bIOFailure)
	{
		Owner.HandleUploadScoreUploaded(
			Param && !bIOFailure && Param->m_bSuccess != 0,
			Param && Param->m_bScoreChanged != 0,
			Param ? Param->m_nGlobalRankNew : 0,
			Param ? Param->m_nGlobalRankPrevious : 0);
	}

	UT66MiniLeaderboardSubsystem& Owner;
	CCallResult<FT66MiniSteamLeaderboardBridge, LeaderboardFindResult_t> FetchFindCallResult;
	CCallResult<FT66MiniSteamLeaderboardBridge, LeaderboardScoresDownloaded_t> FetchDownloadCallResult;
	CCallResult<FT66MiniSteamLeaderboardBridge, LeaderboardFindResult_t> UploadFindCallResult;
	CCallResult<FT66MiniSteamLeaderboardBridge, LeaderboardScoreUploaded_t> UploadScoreCallResult;
};

void UT66MiniLeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!SteamBridge)
	{
		SteamBridge = new FT66MiniSteamLeaderboardBridge(*this);
	}
}

void UT66MiniLeaderboardSubsystem::Deinitialize()
{
	if (SteamBridge)
	{
		delete SteamBridge;
		SteamBridge = nullptr;
	}

	LeaderboardCache.Reset();
	LeaderboardHandlesByName.Reset();
	bHasActiveFetchRequest = false;
	bHasQueuedFetchRequest = false;
	bHasActiveUploadRequest = false;
	bHasQueuedUploadRequest = false;
	LeaderboardUpdated.Clear();

	Super::Deinitialize();
}

FString UT66MiniLeaderboardSubsystem::PartySizeToken(const ET66PartySize PartySize)
{
	switch (PartySize)
	{
	case ET66PartySize::Duo:
		return TEXT("duo");
	case ET66PartySize::Trio:
		return TEXT("trio");
	case ET66PartySize::Quad:
		return TEXT("quad");
	case ET66PartySize::Solo:
	default:
		return TEXT("solo");
	}
}

FString UT66MiniLeaderboardSubsystem::SanitizeLeaderboardToken(const FString& Source)
{
	FString Lower = Source.ToLower();
	FString Sanitized;
	Sanitized.Reserve(Lower.Len());

	bool bLastWasUnderscore = false;
	for (const TCHAR Character : Lower)
	{
		if (FChar::IsAlnum(Character))
		{
			Sanitized.AppendChar(Character);
			bLastWasUnderscore = false;
		}
		else if (!bLastWasUnderscore)
		{
			Sanitized.AppendChar(TEXT('_'));
			bLastWasUnderscore = true;
		}
	}

	while (Sanitized.StartsWith(TEXT("_")))
	{
		Sanitized.RightChopInline(1);
	}

	while (Sanitized.EndsWith(TEXT("_")))
	{
		Sanitized.LeftChopInline(1);
	}
	return Sanitized.IsEmpty() ? TEXT("unknown") : Sanitized;
}

FString UT66MiniLeaderboardSubsystem::MakeBoardName(const FName DifficultyId, const ET66PartySize PartySize)
{
	return FString::Printf(
		TEXT("t66mini_score_alltime_%s_%s"),
		*PartySizeToken(PartySize),
		*SanitizeLeaderboardToken(DifficultyId.ToString()));
}

FString UT66MiniLeaderboardSubsystem::MakeCacheKey(const FName DifficultyId, const ET66PartySize PartySize, const ET66MiniLeaderboardFilter Filter)
{
	return FString::Printf(
		TEXT("%s_%s"),
		*MakeBoardName(DifficultyId, PartySize),
		Filter == ET66MiniLeaderboardFilter::Friends ? TEXT("friends") : TEXT("global"));
}

bool UT66MiniLeaderboardSubsystem::IsSteamAvailable() const
{
	return SteamUserStats() != nullptr && SteamUser() != nullptr;
}

UT66MiniLeaderboardSubsystem::FCachedMiniLeaderboard* UT66MiniLeaderboardSubsystem::FindCacheMutable(const FString& CacheKey)
{
	return LeaderboardCache.Find(CacheKey);
}

const UT66MiniLeaderboardSubsystem::FCachedMiniLeaderboard* UT66MiniLeaderboardSubsystem::FindCache(const FString& CacheKey) const
{
	return LeaderboardCache.Find(CacheKey);
}

void UT66MiniLeaderboardSubsystem::SetCacheLoading(const FString& CacheKey, const FString& StatusMessage)
{
	FCachedMiniLeaderboard& Cache = LeaderboardCache.FindOrAdd(CacheKey);
	Cache.bLoading = true;
	Cache.StatusMessage = StatusMessage;
}

void UT66MiniLeaderboardSubsystem::SetCacheFailed(const FString& CacheKey, const FString& StatusMessage)
{
	FCachedMiniLeaderboard& Cache = LeaderboardCache.FindOrAdd(CacheKey);
	Cache.Entries.Reset();
	Cache.bLoading = false;
	Cache.StatusMessage = StatusMessage;
	LeaderboardUpdated.Broadcast(CacheKey);
}

void UT66MiniLeaderboardSubsystem::CompleteActiveFetchWithFailure(const FString& StatusMessage)
{
	if (!bHasActiveFetchRequest)
	{
		return;
	}

	SetCacheFailed(ActiveFetchRequest.CacheKey, StatusMessage);
	bHasActiveFetchRequest = false;
	PumpQueuedFetch();
}

void UT66MiniLeaderboardSubsystem::StartFetchRequest(const FPendingFetchRequest& Request)
{
	bHasActiveFetchRequest = true;
	ActiveFetchRequest = Request;
	SetCacheLoading(
		Request.CacheKey,
		Request.Filter == ET66MiniLeaderboardFilter::Friends
			? TEXT("Loading friend scores...")
			: TEXT("Loading global scores..."));

	if (const uint64* ExistingHandle = LeaderboardHandlesByName.Find(Request.BoardName))
	{
		ActiveFetchRequest.Handle = *ExistingHandle;
		StartDownloadForActiveFetch();
		return;
	}

	ISteamUserStats* Stats = SteamUserStats();
	if (!SteamBridge || !Stats)
	{
		CompleteActiveFetchWithFailure(TEXT("Steam leaderboards are unavailable in this session."));
		return;
	}

	const SteamAPICall_t ApiCall = Stats->FindOrCreateLeaderboard(
		TCHAR_TO_UTF8(*Request.BoardName),
		k_ELeaderboardSortMethodDescending,
		k_ELeaderboardDisplayTypeNumeric);
	SteamBridge->StartFetchFind(ApiCall);
}

void UT66MiniLeaderboardSubsystem::StartUploadRequest(const FPendingUploadRequest& Request)
{
	bHasActiveUploadRequest = true;
	ActiveUploadRequest = Request;

	if (const uint64* ExistingHandle = LeaderboardHandlesByName.Find(Request.BoardName))
	{
		ActiveUploadRequest.Handle = *ExistingHandle;
		StartUploadForActiveRequest();
		return;
	}

	ISteamUserStats* Stats = SteamUserStats();
	if (!SteamBridge || !Stats)
	{
		bHasActiveUploadRequest = false;
		PumpQueuedUpload();
		return;
	}

	const SteamAPICall_t ApiCall = Stats->FindOrCreateLeaderboard(
		TCHAR_TO_UTF8(*Request.BoardName),
		k_ELeaderboardSortMethodDescending,
		k_ELeaderboardDisplayTypeNumeric);
	SteamBridge->StartUploadFind(ApiCall);
}

void UT66MiniLeaderboardSubsystem::PumpQueuedFetch()
{
	if (!bHasQueuedFetchRequest)
	{
		return;
	}

	const FPendingFetchRequest QueuedRequest = QueuedFetchRequest;
	bHasQueuedFetchRequest = false;
	StartFetchRequest(QueuedRequest);
}

void UT66MiniLeaderboardSubsystem::PumpQueuedUpload()
{
	if (!bHasQueuedUploadRequest)
	{
		return;
	}

	const FPendingUploadRequest QueuedRequest = QueuedUploadRequest;
	bHasQueuedUploadRequest = false;
	StartUploadRequest(QueuedRequest);
}

void UT66MiniLeaderboardSubsystem::StartDownloadForActiveFetch()
{
	if (!bHasActiveFetchRequest)
	{
		return;
	}

	ISteamUserStats* Stats = SteamUserStats();
	if (!SteamBridge || !Stats || ActiveFetchRequest.Handle == 0)
	{
		CompleteActiveFetchWithFailure(TEXT("Failed to open the Steam leaderboard."));
		return;
	}

	const ELeaderboardDataRequest DataRequest =
		ActiveFetchRequest.Filter == ET66MiniLeaderboardFilter::Friends
			? k_ELeaderboardDataRequestFriends
			: k_ELeaderboardDataRequestGlobal;

	const SteamAPICall_t ApiCall = Stats->DownloadLeaderboardEntries(
		static_cast<SteamLeaderboard_t>(ActiveFetchRequest.Handle),
		DataRequest,
		1,
		MiniLeaderboardVisibleEntryCount);
	SteamBridge->StartFetchDownload(ApiCall);
}

void UT66MiniLeaderboardSubsystem::StartUploadForActiveRequest()
{
	if (!bHasActiveUploadRequest)
	{
		return;
	}

	ISteamUserStats* Stats = SteamUserStats();
	if (!SteamBridge || !Stats || ActiveUploadRequest.Handle == 0)
	{
		bHasActiveUploadRequest = false;
		PumpQueuedUpload();
		return;
	}

	const SteamAPICall_t ApiCall = Stats->UploadLeaderboardScore(
		static_cast<SteamLeaderboard_t>(ActiveUploadRequest.Handle),
		k_ELeaderboardUploadScoreMethodKeepBest,
		ActiveUploadRequest.Score,
		nullptr,
		0);
	SteamBridge->StartUploadScore(ApiCall);
}

void UT66MiniLeaderboardSubsystem::InvalidateBoardCaches(const FString& BoardName)
{
	LeaderboardCache.Remove(FString::Printf(TEXT("%s_global"), *BoardName));
	LeaderboardCache.Remove(FString::Printf(TEXT("%s_friends"), *BoardName));
}

void UT66MiniLeaderboardSubsystem::RequestLeaderboard(const FName DifficultyId, const ET66PartySize PartySize, const ET66MiniLeaderboardFilter Filter)
{
	const FString CacheKey = MakeCacheKey(DifficultyId, PartySize, Filter);
	if (!IsSteamAvailable())
	{
		SetCacheFailed(CacheKey, TEXT("Launch the mini through Steam to use leaderboards."));
		return;
	}

	FPendingFetchRequest Request;
	Request.DifficultyId = DifficultyId;
	Request.PartySize = PartySize;
	Request.Filter = Filter;
	Request.CacheKey = CacheKey;
	Request.BoardName = MakeBoardName(DifficultyId, PartySize);
	Request.Handle = 0;

	if (bHasActiveFetchRequest)
	{
		if (ActiveFetchRequest.CacheKey == Request.CacheKey)
		{
			return;
		}

		QueuedFetchRequest = Request;
		bHasQueuedFetchRequest = true;
		SetCacheLoading(
			CacheKey,
			Filter == ET66MiniLeaderboardFilter::Friends
				? TEXT("Queued friend leaderboard refresh...")
				: TEXT("Queued global leaderboard refresh..."));
		return;
	}

	StartFetchRequest(Request);
}

void UT66MiniLeaderboardSubsystem::SubmitScore(const FName DifficultyId, const ET66PartySize PartySize, const int32 Score)
{
	if (Score <= 0 || !IsSteamAvailable())
	{
		return;
	}

	FPendingUploadRequest Request;
	Request.DifficultyId = DifficultyId;
	Request.PartySize = PartySize;
	Request.BoardName = MakeBoardName(DifficultyId, PartySize);
	Request.Score = Score;
	Request.Handle = 0;

	InvalidateBoardCaches(Request.BoardName);

	if (bHasActiveUploadRequest)
	{
		QueuedUploadRequest = Request;
		bHasQueuedUploadRequest = true;
		return;
	}

	StartUploadRequest(Request);
}

bool UT66MiniLeaderboardSubsystem::HasCachedLeaderboard(const FName DifficultyId, const ET66PartySize PartySize, const ET66MiniLeaderboardFilter Filter) const
{
	return FindCache(MakeCacheKey(DifficultyId, PartySize, Filter)) != nullptr;
}

TArray<FT66MiniLeaderboardEntry> UT66MiniLeaderboardSubsystem::GetCachedLeaderboard(
	const FName DifficultyId,
	const ET66PartySize PartySize,
	const ET66MiniLeaderboardFilter Filter) const
{
	if (const FCachedMiniLeaderboard* Cache = FindCache(MakeCacheKey(DifficultyId, PartySize, Filter)))
	{
		return Cache->Entries;
	}

	return {};
}

bool UT66MiniLeaderboardSubsystem::IsLeaderboardLoading(
	const FName DifficultyId,
	const ET66PartySize PartySize,
	const ET66MiniLeaderboardFilter Filter) const
{
	if (const FCachedMiniLeaderboard* Cache = FindCache(MakeCacheKey(DifficultyId, PartySize, Filter)))
	{
		return Cache->bLoading;
	}

	return false;
}

FString UT66MiniLeaderboardSubsystem::GetLeaderboardStatusMessage(
	const FName DifficultyId,
	const ET66PartySize PartySize,
	const ET66MiniLeaderboardFilter Filter) const
{
	if (!IsSteamAvailable())
	{
		return TEXT("Launch the mini through Steam to use leaderboards.");
	}

	if (const FCachedMiniLeaderboard* Cache = FindCache(MakeCacheKey(DifficultyId, PartySize, Filter)))
	{
		return Cache->StatusMessage.IsEmpty() ? T66MiniDefaultStatusMessage() : Cache->StatusMessage;
	}

	return T66MiniDefaultStatusMessage();
}

void UT66MiniLeaderboardSubsystem::HandleFetchLeaderboardFound(const bool bSuccess, const uint64 Handle)
{
	if (!bHasActiveFetchRequest)
	{
		return;
	}

	if (!bSuccess || Handle == 0)
	{
		CompleteActiveFetchWithFailure(TEXT("Failed to find the Steam leaderboard."));
		return;
	}

	ActiveFetchRequest.Handle = Handle;
	LeaderboardHandlesByName.Add(ActiveFetchRequest.BoardName, Handle);
	StartDownloadForActiveFetch();
}

void UT66MiniLeaderboardSubsystem::HandleFetchLeaderboardDownloaded(
	const bool bSuccess,
	const uint64 Handle,
	const uint64 DownloadedEntriesHandle,
	const int32 EntryCount)
{
	if (!bHasActiveFetchRequest)
	{
		return;
	}

	FCachedMiniLeaderboard& Cache = LeaderboardCache.FindOrAdd(ActiveFetchRequest.CacheKey);
	Cache.Entries.Reset();
	Cache.bLoading = false;

	if (!bSuccess || Handle == 0 || DownloadedEntriesHandle == 0)
	{
		Cache.StatusMessage = TEXT("Failed to download Steam leaderboard scores.");
		LeaderboardUpdated.Broadcast(ActiveFetchRequest.CacheKey);
		bHasActiveFetchRequest = false;
		PumpQueuedFetch();
		return;
	}

	ISteamUserStats* Stats = SteamUserStats();
	ISteamUser* User = SteamUser();
	if (!Stats || !User)
	{
		Cache.StatusMessage = TEXT("Steam leaderboards are unavailable in this session.");
		LeaderboardUpdated.Broadcast(ActiveFetchRequest.CacheKey);
		bHasActiveFetchRequest = false;
		PumpQueuedFetch();
		return;
	}

	const uint64 LocalSteamId = User->GetSteamID().ConvertToUint64();
	for (int32 EntryIndex = 0; EntryIndex < EntryCount; ++EntryIndex)
	{
		LeaderboardEntry_t SteamEntry;
		if (!Stats->GetDownloadedLeaderboardEntry(
			static_cast<SteamLeaderboardEntries_t>(DownloadedEntriesHandle),
			EntryIndex,
			&SteamEntry,
			nullptr,
			0))
		{
			continue;
		}

		FT66MiniLeaderboardEntry& Entry = Cache.Entries.AddDefaulted_GetRef();
		Entry.Rank = SteamEntry.m_nGlobalRank;
		Entry.SteamId = SteamIdFromUint64(SteamEntry.m_steamIDUser.ConvertToUint64());
		Entry.DisplayName = ResolveSteamPersonaName(SteamEntry.m_steamIDUser.ConvertToUint64(), Entry.SteamId);
		Entry.Score = SteamEntry.m_nScore;
		Entry.bIsLocalPlayer = SteamEntry.m_steamIDUser.ConvertToUint64() == LocalSteamId;
	}

	if (Cache.Entries.Num() == 0)
	{
		Cache.StatusMessage =
			ActiveFetchRequest.Filter == ET66MiniLeaderboardFilter::Friends
				? TEXT("No friend scores yet.")
				: TEXT("No scores have been posted yet.");
	}
	else
	{
		Cache.StatusMessage = T66MiniDefaultStatusMessage();
	}

	UE_LOG(
		LogT66MiniLeaderboard,
		Log,
		TEXT("Mini leaderboard refreshed board=%s filter=%s entries=%d"),
		*ActiveFetchRequest.BoardName,
		ActiveFetchRequest.Filter == ET66MiniLeaderboardFilter::Friends ? TEXT("friends") : TEXT("global"),
		Cache.Entries.Num());

	LeaderboardUpdated.Broadcast(ActiveFetchRequest.CacheKey);
	bHasActiveFetchRequest = false;
	PumpQueuedFetch();
}

void UT66MiniLeaderboardSubsystem::HandleUploadLeaderboardFound(const bool bSuccess, const uint64 Handle)
{
	if (!bHasActiveUploadRequest)
	{
		return;
	}

	if (!bSuccess || Handle == 0)
	{
		bHasActiveUploadRequest = false;
		PumpQueuedUpload();
		return;
	}

	ActiveUploadRequest.Handle = Handle;
	LeaderboardHandlesByName.Add(ActiveUploadRequest.BoardName, Handle);
	StartUploadForActiveRequest();
}

void UT66MiniLeaderboardSubsystem::HandleUploadScoreUploaded(
	const bool bSuccess,
	const bool bScoreChanged,
	const int32 NewGlobalRank,
	const int32 PreviousGlobalRank)
{
	if (!bHasActiveUploadRequest)
	{
		return;
	}

	UE_LOG(
		LogT66MiniLeaderboard,
		Log,
		TEXT("Mini leaderboard upload board=%s score=%d success=%d changed=%d new_rank=%d previous_rank=%d"),
		*ActiveUploadRequest.BoardName,
		ActiveUploadRequest.Score,
		bSuccess ? 1 : 0,
		bScoreChanged ? 1 : 0,
		NewGlobalRank,
		PreviousGlobalRank);

	bHasActiveUploadRequest = false;
	PumpQueuedUpload();
}

FString UT66MiniLeaderboardSubsystem::ResolveSteamPersonaName(const uint64 SteamIdNumeric, const FString& FallbackSteamId) const
{
	ISteamFriends* Friends = SteamFriends();
	if (!Friends)
	{
		return FallbackSteamId.IsEmpty() ? TEXT("Unknown") : FString::Printf(TEXT("Steam %s"), *FallbackSteamId.Right(4));
	}

	CSteamID SteamId(SteamIdNumeric);
	const char* PersonaNameUtf8 = Friends->GetFriendPersonaName(SteamId);
	FString PersonaName = PersonaNameUtf8 ? FString(UTF8_TO_TCHAR(PersonaNameUtf8)).TrimStartAndEnd() : FString();
	if (PersonaName.IsEmpty() || PersonaName.Equals(TEXT("[unknown]"), ESearchCase::IgnoreCase))
	{
		Friends->RequestUserInformation(SteamId, false);
	}

	if (!PersonaName.IsEmpty() && !PersonaName.Equals(TEXT("[unknown]"), ESearchCase::IgnoreCase))
	{
		return PersonaName;
	}

	return FallbackSteamId.IsEmpty() ? TEXT("Unknown") : FString::Printf(TEXT("Steam %s"), *FallbackSteamId.Right(4));
}

FString UT66MiniLeaderboardSubsystem::SteamIdFromUint64(const uint64 SteamIdNumeric)
{
	return FString::Printf(TEXT("%llu"), SteamIdNumeric);
}
