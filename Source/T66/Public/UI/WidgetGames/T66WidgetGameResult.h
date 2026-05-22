// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI/WidgetGames/T66WidgetGameTypes.h"
#include "T66WidgetGameResult.generated.h"

struct T66_API FT66WidgetGameResult
{
	FName GameID = NAME_None;
	ET66WidgetGameExitReason ExitReason = ET66WidgetGameExitReason::Completed;
	int32 FinalScore = 0;
	int32 Payout = 0;
	bool bHasFinalScore = false;
	bool bHasPayout = false;
	bool bSuccessful = false;
	FName ResultID = NAME_None;
};

UINTERFACE()
class T66_API UT66WidgetGameResultReporter : public UInterface
{
	GENERATED_BODY()
};

class T66_API IT66WidgetGameResultReporter
{
	GENERATED_BODY()

public:
	virtual void SubmitResult(const FT66WidgetGameResult& Result) {}
};
