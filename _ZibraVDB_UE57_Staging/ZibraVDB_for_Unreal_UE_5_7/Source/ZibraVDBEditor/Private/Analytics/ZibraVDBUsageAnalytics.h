// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"

class FZibraVDBUsageAnalytics
{
public:
	static void Initialize();
	static void Shutdown();

private:
	static bool OnTimer(float);
	static bool OnDelayedStartup(float);
	static void SendUsageEvent(bool isStartup = false);

	constexpr static float TimerInterval = 1800.0f; // Seconds
	static FTSTicker::FDelegateHandle DelayedStartupHandle;
	static FTSTicker::FDelegateHandle TimerHandle;
};
