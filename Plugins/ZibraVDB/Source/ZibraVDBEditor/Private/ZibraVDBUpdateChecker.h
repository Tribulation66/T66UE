// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "HttpFwd.h"

#include "ZibraVDBUpdateChecker.generated.h"

USTRUCT()
struct FUpdateCheckResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString version;
};

class FZibraVDBUpdateChecker
{
public:
	static void CheckForUpdates(bool IsAutomatic);

private:
	static const FString UpdateCheckURL;

	static void GetUpdateCheckResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, bool IsAutomatic);
	static TOptional<TArray<int32>> ParseVersion(const FString& VersionString);
	static bool IsLatest(const TArray<int32>& CurrentVersion, const TArray<int32>& NewVersion);
};
