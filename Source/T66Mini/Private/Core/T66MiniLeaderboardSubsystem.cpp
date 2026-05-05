// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniLeaderboardSubsystem.h"

#include "Async/Async.h"

THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END

DEFINE_LOG_CATEGORY_STATIC(LogT66MiniLeaderboard, Log, All);

class FT66MiniSteamLeaderboardBridge
{
public:
	explicit FT66MiniSteamLeaderboardBridge(UT66MiniLeaderboardSubsystem& InOwner)
		: Owner(&InOwner)
	{
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
	void OnUploadFind(LeaderboardFindResult_t* Param, const bool bIOFailure)
	{
		const bool bSuccess = Param && !bIOFailure && Param->m_bLeaderboardFound != 0;
		const uint64 Handle = bSuccess ? static_cast<uint64>(Param->m_hSteamLeaderboard) : 0;
		AsyncTask(ENamedThreads::GameThread, [Owner = Owner, bSuccess, Handle]()
		{
			if (UT66MiniLeaderboardSubsystem* OwnerPtr = Owner.Get())
			{
				OwnerPtr->HandleUploadLeaderboardFound(bSuccess, Handle);
			}
		});
	}

	void OnUploadScore(LeaderboardScoreUploaded_t* Param, const bool bIOFailure)
	{
		const bool bSuccess = Param && !bIOFailure && Param->m_bSuccess != 0;
		const bool bScoreChanged = Param && Param->m_bScoreChanged != 0;
		const int32 NewGlobalRank = Param ? Param->m_nGlobalRankNew : 0;
		const int32 PreviousGlobalRank = Param ? Param->m_nGlobalRankPrevious : 0;
		AsyncTask(ENamedThreads::GameThread, [Owner = Owner, bSuccess, bScoreChanged, NewGlobalRank, PreviousGlobalRank]()
		{
			if (UT66MiniLeaderboardSubsystem* OwnerPtr = Owner.Get())
			{
				OwnerPtr->HandleUploadScoreUploaded(bSuccess, bScoreChanged, NewGlobalRank, PreviousGlobalRank);
			}
		});
	}

	TWeakObjectPtr<UT66MiniLeaderboardSubsystem> Owner;
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
	delete SteamBridge;
	SteamBridge = nullptr;

	LeaderboardHandlesByName.Reset();
	bHasActiveUploadRequest = false;
	bHasQueuedUploadRequest = false;

	Super::Deinitialize();
}

bool UT66MiniLeaderboardSubsystem::IsSteamAvailable() const
{
	return SteamUserStats() != nullptr && SteamUser() != nullptr;
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

FString UT66MiniLeaderboardSubsystem::MakeBoardName(const FName DifficultyId)
{
	return FString::Printf(
		TEXT("t66mini_score_alltime_solo_%s"),
		*SanitizeLeaderboardToken(DifficultyId.ToString()));
}

void UT66MiniLeaderboardSubsystem::SubmitScore(const FName DifficultyId, const int32 Score)
{
	if (Score <= 0 || !IsSteamAvailable())
	{
		return;
	}

	FPendingUploadRequest Request;
	Request.DifficultyId = DifficultyId;
	Request.BoardName = MakeBoardName(DifficultyId);
	Request.Score = Score;
	Request.Handle = 0;

	if (bHasActiveUploadRequest)
	{
		QueuedUploadRequest = Request;
		bHasQueuedUploadRequest = true;
		return;
	}

	StartUploadRequest(Request);
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
		TEXT("Mini score upload board=%s score=%d success=%d changed=%d new_rank=%d previous_rank=%d"),
		*ActiveUploadRequest.BoardName,
		ActiveUploadRequest.Score,
		bSuccess ? 1 : 0,
		bScoreChanged ? 1 : 0,
		NewGlobalRank,
		PreviousGlobalRank);

	bHasActiveUploadRequest = false;
	PumpQueuedUpload();
}
