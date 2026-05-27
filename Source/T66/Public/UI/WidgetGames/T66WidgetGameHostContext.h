// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "UI/WidgetGames/T66WidgetGameTypes.h"
#include "UObject/WeakObjectPtr.h"

class APlayerController;
struct FT66WidgetGameResult;

struct T66_API FT66WidgetGameHostContext
{
	TWeakObjectPtr<const UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> OwningPlayer;
	TWeakObjectPtr<UObject> TimelineProvider;

	TFunction<void(const FText&)> StatusTextCallback;
	TFunction<void(const FT66WidgetGameResult&)> ResultCallback;
	TFunction<void(ET66WidgetGameExitReason)> ReturnNavigationCallback;
	TFunction<bool(FName)> AvailabilityQueryCallback;
	TFunction<void(FName)> AudioEventCallback;
	TFunction<int32()> WagerCallback;
	TFunction<void(int32)> PayoutCallback;

	bool IsGameAvailable(const FName GameID) const
	{
		return AvailabilityQueryCallback ? AvailabilityQueryCallback(GameID) : true;
	}

	void EmitAudioEvent(const FName EventID) const
	{
		if (AudioEventCallback)
		{
			AudioEventCallback(EventID);
		}
	}

	void ReportStatus(const FText& StatusText) const
	{
		if (StatusTextCallback)
		{
			StatusTextCallback(StatusText);
		}
	}

	void ReportResult(const FT66WidgetGameResult& Result) const
	{
		if (ResultCallback)
		{
			ResultCallback(Result);
		}
	}

	void RequestExit(const ET66WidgetGameExitReason ExitReason) const
	{
		if (ReturnNavigationCallback)
		{
			ReturnNavigationCallback(ExitReason);
		}
	}

	int32 GetWager() const
	{
		return WagerCallback ? WagerCallback() : 0;
	}

	void SubmitPayout(const int32 Payout) const
	{
		if (PayoutCallback)
		{
			PayoutCallback(Payout);
		}
	}
};
