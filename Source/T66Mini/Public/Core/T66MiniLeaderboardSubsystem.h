// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniLeaderboardSubsystem.generated.h"

UENUM()
enum class ET66MiniLeaderboardFilter : uint8
{
	Global,
	Friends
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini|Leaderboard")
	int32 Rank = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini|Leaderboard")
	FString SteamId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini|Leaderboard")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini|Leaderboard")
	int32 Score = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mini|Leaderboard")
	bool bIsLocalPlayer = false;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnT66MiniLeaderboardUpdated, const FString& /*CacheKey*/);

UCLASS()
class T66MINI_API UT66MiniLeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static FString MakeCacheKey(FName DifficultyId, ET66PartySize PartySize, ET66MiniLeaderboardFilter Filter);

	bool IsSteamAvailable() const;
	void RequestLeaderboard(FName DifficultyId, ET66PartySize PartySize, ET66MiniLeaderboardFilter Filter);
	void SubmitScore(FName DifficultyId, ET66PartySize PartySize, int32 Score);

	bool HasCachedLeaderboard(FName DifficultyId, ET66PartySize PartySize, ET66MiniLeaderboardFilter Filter) const;
	TArray<FT66MiniLeaderboardEntry> GetCachedLeaderboard(FName DifficultyId, ET66PartySize PartySize, ET66MiniLeaderboardFilter Filter) const;
	bool IsLeaderboardLoading(FName DifficultyId, ET66PartySize PartySize, ET66MiniLeaderboardFilter Filter) const;
	FString GetLeaderboardStatusMessage(FName DifficultyId, ET66PartySize PartySize, ET66MiniLeaderboardFilter Filter) const;

	FOnT66MiniLeaderboardUpdated& OnLeaderboardUpdated() { return LeaderboardUpdated; }

private:
	friend class FT66MiniSteamLeaderboardBridge;

	struct FCachedMiniLeaderboard
	{
		TArray<FT66MiniLeaderboardEntry> Entries;
		bool bLoading = false;
		FString StatusMessage;
	};

	struct FPendingFetchRequest
	{
		FName DifficultyId = NAME_None;
		ET66PartySize PartySize = ET66PartySize::Solo;
		ET66MiniLeaderboardFilter Filter = ET66MiniLeaderboardFilter::Global;
		FString CacheKey;
		FString BoardName;
		uint64 Handle = 0;
	};

	struct FPendingUploadRequest
	{
		FName DifficultyId = NAME_None;
		ET66PartySize PartySize = ET66PartySize::Solo;
		FString BoardName;
		int32 Score = 0;
		uint64 Handle = 0;
	};

	static FString PartySizeToken(ET66PartySize PartySize);
	static FString SanitizeLeaderboardToken(const FString& Source);
	static FString MakeBoardName(FName DifficultyId, ET66PartySize PartySize);

	FCachedMiniLeaderboard* FindCacheMutable(const FString& CacheKey);
	const FCachedMiniLeaderboard* FindCache(const FString& CacheKey) const;
	void SetCacheLoading(const FString& CacheKey, const FString& StatusMessage);
	void SetCacheFailed(const FString& CacheKey, const FString& StatusMessage);
	void CompleteActiveFetchWithFailure(const FString& StatusMessage);
	void StartFetchRequest(const FPendingFetchRequest& Request);
	void StartUploadRequest(const FPendingUploadRequest& Request);
	void PumpQueuedFetch();
	void PumpQueuedUpload();
	void StartDownloadForActiveFetch();
	void StartUploadForActiveRequest();
	void InvalidateBoardCaches(const FString& BoardName);

	void HandleFetchLeaderboardFound(bool bSuccess, uint64 Handle);
	void HandleFetchLeaderboardDownloaded(bool bSuccess, uint64 Handle, uint64 DownloadedEntriesHandle, int32 EntryCount);
	void HandleUploadLeaderboardFound(bool bSuccess, uint64 Handle);
	void HandleUploadScoreUploaded(bool bSuccess, bool bScoreChanged, int32 NewGlobalRank, int32 PreviousGlobalRank);

	FString ResolveSteamPersonaName(uint64 SteamIdNumeric, const FString& FallbackSteamId) const;
	static FString SteamIdFromUint64(uint64 SteamIdNumeric);

	TMap<FString, FCachedMiniLeaderboard> LeaderboardCache;
	TMap<FString, uint64> LeaderboardHandlesByName;
	FOnT66MiniLeaderboardUpdated LeaderboardUpdated;

	FPendingFetchRequest ActiveFetchRequest;
	bool bHasActiveFetchRequest = false;
	FPendingFetchRequest QueuedFetchRequest;
	bool bHasQueuedFetchRequest = false;

	FPendingUploadRequest ActiveUploadRequest;
	bool bHasActiveUploadRequest = false;
	FPendingUploadRequest QueuedUploadRequest;
	bool bHasQueuedUploadRequest = false;

	FT66MiniSteamLeaderboardBridge* SteamBridge = nullptr;
};
