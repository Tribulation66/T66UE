// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LagTrackerSubsystem.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarLagTrackerEnabled(
	TEXT("T66.LagTracker.Enabled"),
	1,
	TEXT("1 = log slow operations under [LAG], 0 = disabled"),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarLagTrackerThresholdMs(
	TEXT("T66.LagTracker.ThresholdMs"),
	5.0f,
	TEXT("Operations exceeding this many ms are logged"),
	ECVF_Default);

void UT66LagTrackerSubsystem::ReportSlowOperation(const FString& Cause, float DurationMs)
{
	if (!IsEnabled() || DurationMs < GetThresholdMs()) return;
	UE_LOG(LogTemp, Warning, TEXT("[LAG] %s: %.2fms"), *Cause, DurationMs);
}

bool UT66LagTrackerSubsystem::IsEnabled() const
{
	return CVarLagTrackerEnabled.GetValueOnGameThread() != 0;
}

float UT66LagTrackerSubsystem::GetThresholdMs() const
{
	return FMath::Max(0.1f, CVarLagTrackerThresholdMs.GetValueOnGameThread());
}
