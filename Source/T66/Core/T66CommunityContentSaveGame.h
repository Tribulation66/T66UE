// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Core/T66CommunityContentTypes.h"
#include "T66CommunityContentSaveGame.generated.h"

UCLASS(BlueprintType)
class T66_API UT66CommunityContentSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Community")
	int32 SaveVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Community")
	TArray<FT66CommunityContentEntry> DraftEntries;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Community")
	TArray<FT66CommunityContentEntry> CachedCommunityEntries;
};
