// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66WidgetGameTypes.generated.h"

UENUM(BlueprintType)
enum class ET66WidgetGameCategory : uint8
{
	Casino,
};

UENUM(BlueprintType)
enum class ET66WidgetGamePlayModel : uint8
{
	TurnCasino,
	PhysicalCasino,
};

UENUM(BlueprintType)
enum class ET66WidgetGameLaunchKind : uint8
{
	CasinoChildWidget,
};

UENUM(BlueprintType)
enum class ET66WidgetGameDemoGateKind : uint8
{
	None,
	CasinoAllowList,
};

UENUM(BlueprintType)
enum class ET66WidgetGameExitReason : uint8
{
	Completed,
	PlayerCancelled,
	TimedOut,
	Error,
	ForcedExit,
};

USTRUCT(BlueprintType)
struct T66_API FT66WidgetGameCapabilityFlags
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Games")
	bool bUsesTimeline = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Games")
	bool bUsesAudioMarkers = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Games")
	bool bUsesCustomPaint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Games")
	bool bUsesWager = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Games")
	bool bUsesPersistentRun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Games")
	bool bUsesScoreResult = false;
};
