// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66InteractionPromptSubsystem.generated.h"

class AActor;

UENUM()
enum class ET66InteractionPromptAction : uint8
{
	None,
	PilotTractor,
	ExitTractor,
	UseFountain,
	OpenChest,
	SpinWheel,
	OpenCrate,
	EnterCasino,
	GetQuickRevive,
	UseTeleporter,
	RaiseDifficulty,
	ClaimGold,
	ClaimLoot,
};

UCLASS()
class T66_API UT66InteractionPromptSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ET66InteractionPromptAction GetPromptActionForActor(const AActor* Actor) const;

	FText BuildPromptText(ET66InteractionPromptAction Action) const;
	FText BuildPromptTextWithSeconds(ET66InteractionPromptAction Action, int32 RemainingSeconds) const;

private:
	FText BuildPromptTextInternal(ET66InteractionPromptAction Action, int32 RemainingSeconds, bool bIncludeSeconds) const;
};
