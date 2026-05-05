// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniLeaderboardSubsystem.generated.h"

UCLASS()
class T66MINI_API UT66MiniLeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsSteamAvailable() const;
	void SubmitScore(FName DifficultyId, int32 Score);

private:
	friend class FT66MiniSteamLeaderboardBridge;

	struct FPendingUploadRequest
	{
		FName DifficultyId = NAME_None;
		FString BoardName;
		int32 Score = 0;
		uint64 Handle = 0;
	};

	static FString SanitizeLeaderboardToken(const FString& Source);
	static FString MakeBoardName(FName DifficultyId);

	void StartUploadRequest(const FPendingUploadRequest& Request);
	void PumpQueuedUpload();
	void StartUploadForActiveRequest();

	void HandleUploadLeaderboardFound(bool bSuccess, uint64 Handle);
	void HandleUploadScoreUploaded(bool bSuccess, bool bScoreChanged, int32 NewGlobalRank, int32 PreviousGlobalRank);

	TMap<FString, uint64> LeaderboardHandlesByName;

	FPendingUploadRequest ActiveUploadRequest;
	bool bHasActiveUploadRequest = false;
	FPendingUploadRequest QueuedUploadRequest;
	bool bHasQueuedUploadRequest = false;

	FT66MiniSteamLeaderboardBridge* SteamBridge = nullptr;
};
