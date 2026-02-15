// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "T66LagTrackerSubsystem.generated.h"

/**
 * Tracks slow operations and logs them under [LAG] with the cause.
 * Use FLagScopedScope in hot paths to identify performance culprits.
 *
 * Console: T66.LagTracker.Enabled 1, T66.LagTracker.ThresholdMs 5.0
 */
UCLASS()
class T66_API UT66LagTrackerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Report a slow operation. Logs [LAG] Cause: X.XXms when duration exceeds threshold. */
	UFUNCTION(BlueprintCallable, Category = "T66|Lag")
	void ReportSlowOperation(const FString& Cause, float DurationMs);

	/** Whether lag tracking is enabled (respects CVar). */
	bool IsEnabled() const;

	/** Threshold in ms above which an operation is logged. */
	float GetThresholdMs() const;
};

/**
 * RAII scope: measures elapsed time and reports to LagTracker when exceeded.
 * Constructor and destructor are inline so every translation unit that uses the scope compiles and links.
 * Usage: FLagScopedScope Scope(GetWorld(), TEXT("RefreshMapData")); then do work; scope logs on exit if slow.
 */
struct FLagScopedScope
{
	FLagScopedScope(UWorld* World, const TCHAR* Cause, float CustomThresholdMs = -1.f)
		: StartSeconds(FPlatformTime::Seconds())
		, ThresholdMs(CustomThresholdMs)
	{
		CauseStr = Cause;
		if (World && World->GetGameInstance())
		{
			Subsystem = World->GetGameInstance()->GetSubsystem<UT66LagTrackerSubsystem>();
		}
	}

	~FLagScopedScope()
	{
		UT66LagTrackerSubsystem* Lag = Subsystem.Get();
		if (!Lag || !Lag->IsEnabled()) return;

		const float DurationMs = static_cast<float>((FPlatformTime::Seconds() - StartSeconds) * 1000.0);
		const float Thresh = (ThresholdMs > 0.f) ? ThresholdMs : Lag->GetThresholdMs();
		if (DurationMs >= Thresh)
		{
			Lag->ReportSlowOperation(CauseStr, DurationMs);
		}
	}

	FLagScopedScope(const FLagScopedScope&) = delete;
	FLagScopedScope& operator=(const FLagScopedScope&) = delete;

private:
	TWeakObjectPtr<UT66LagTrackerSubsystem> Subsystem;
	FString CauseStr;
	double StartSeconds;
	float ThresholdMs;
};
