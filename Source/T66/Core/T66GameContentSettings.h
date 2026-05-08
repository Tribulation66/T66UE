// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "T66GameContentSettings.generated.h"

USTRUCT()
struct T66_API FT66LooseRuntimeContentRoot
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, Category = "Runtime Content")
	FString RelativePath;

	UPROPERTY(Config, EditAnywhere, Category = "Runtime Content")
	FString Owner;

	UPROPERTY(Config, EditAnywhere, Category = "Runtime Content")
	FString Classification;

	UPROPERTY(Config, EditAnywhere, Category = "Runtime Content")
	FString Rationale;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "T66 Game Content"))
class T66_API UT66GameContentSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	const TArray<FT66LooseRuntimeContentRoot>& GetLooseRuntimeContentRoots() const
	{
		return LooseRuntimeContentRoots;
	}

private:
	UPROPERTY(Config, EditAnywhere, Category = "Runtime Content", meta = (TitleProperty = "RelativePath"))
	TArray<FT66LooseRuntimeContentRoot> LooseRuntimeContentRoots;
};
