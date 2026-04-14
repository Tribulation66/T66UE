// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66MiniRuntimeSubsystem.generated.h"

UCLASS()
class T66MINI_API UT66MiniRuntimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool LaunchMiniBattle(UObject* WorldContextObject, FString* OutFailureReason = nullptr) const;

	FName GetMiniBattleLevelName() const;
	FString GetMiniBattleTravelOptions() const;
};
