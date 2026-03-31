// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBUsageAnalytics.h"

#include "Dom/JsonObject.h"
#include "JsonObjectWrapper.h"
#include "ZibraVDBAnalyticsManager.h"
#include "ZibraVDBSDKIntegration.h"

FTSTicker::FDelegateHandle FZibraVDBUsageAnalytics::DelayedStartupHandle;
FTSTicker::FDelegateHandle FZibraVDBUsageAnalytics::TimerHandle;

void FZibraVDBUsageAnalytics::Initialize()
{
	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		return;
	}

	DelayedStartupHandle =
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&FZibraVDBUsageAnalytics::OnDelayedStartup), 0.0f);
	TimerHandle =
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&FZibraVDBUsageAnalytics::OnTimer), TimerInterval);
}

void FZibraVDBUsageAnalytics::Shutdown()
{
	FTSTicker::GetCoreTicker().RemoveTicker(DelayedStartupHandle);
	FTSTicker::GetCoreTicker().RemoveTicker(TimerHandle);
}

bool FZibraVDBUsageAnalytics::OnTimer(float)
{
	SendUsageEvent();

	// Repeat timer
	return true;
}

bool FZibraVDBUsageAnalytics::OnDelayedStartup(float)
{
	SendUsageEvent(true);

	// Do not repeat timer
	return false;
}

void FZibraVDBUsageAnalytics::SendUsageEvent(bool isStartup /*= false*/)
{
	FAnalyticsEvent Event;
	Event.EventType = "Using";
	Event.Data.JsonObject->SetBoolField("Startup", isStartup);

	FZibraVDBAnalyticsManager::SubmitEvent(Event);
}
