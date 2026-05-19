// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "HAL/CriticalSection.h"
#include "Misc/OutputDevice.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceSystem/T66PerformanceSystemTypes.h"
#include "T66PerformanceSubsystem.generated.h"

class FJsonObject;

UCLASS()
class T66_API UT66PerformanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RecordMeasuredOperation(const FString& OperationName, double DurationMs, const FString& Source);

private:
	struct FFrameSample
	{
		double TimeSeconds = 0.0;
		double FrameTimeMs = 0.0;
	};

	struct FMemorySample
	{
		double TimeSeconds = 0.0;
		uint64 UsedPhysicalBytes = 0;
		uint64 PeakUsedPhysicalBytes = 0;
		uint64 TotalPhysicalBytes = 0;
	};

	struct FFrameSummary
	{
		int32 SampleCount = 0;
		double LastFrameMs = 0.0;
		double AverageFrameMs = 0.0;
		double AverageFps = 0.0;
		double StdDevMs = 0.0;
		double P99FrameMs = 0.0;
		double P999FrameMs = 0.0;
		double OnePercentLowFps = 0.0;
		double PointOnePercentLowFps = 0.0;
	};

	struct FDetectorRuntime
	{
		FString Name;
		double CadenceSeconds = 0.0;
		double BudgetUs = 0.0;
		double LastRunSeconds = -1.0e30;
		double LastCostUs = 0.0;
		double PeakCostUs = 0.0;
		int32 ConsecutiveBudgetOverruns = 0;
		bool bDisabled = false;
	};

	class FPerformanceLogOutputDevice final : public FOutputDevice
	{
	public:
		explicit FPerformanceLogOutputDevice(UT66PerformanceSubsystem& InOwner)
			: Owner(InOwner)
		{
		}

		virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

	private:
		UT66PerformanceSubsystem& Owner;
	};

	bool TickPerformanceSystem(float DeltaSeconds);
	void RunDetector(const TCHAR* DetectorName, double BudgetUs, double CadenceSeconds, TFunctionRef<void()> Work);

	void CaptureLogLine(const FString& Category, ELogVerbosity::Type Verbosity, const FString& Message);
	void HandlePreGarbageCollect();
	void HandlePostGarbageCollect();
	void HandleSystemError();

	void CheckFrameDetectors(float DeltaSeconds);
	void CheckMemoryDetector();
	void CheckBasicHangDetector(float DeltaSeconds);
	void WritePeriodicSnapshot(bool bForce);
	void WriteFinalReport(const FString& ExitReason);

	void EmitPerformanceEvent(
		const FString& DetectorName,
		const FString& EventName,
		ET66PerformanceSeverity Severity,
		ET66PerformanceConfidence Confidence,
		const FString& Summary,
		const TArray<FT66PerformanceAttribution>& Attributions);

	TSharedRef<FJsonObject> CreateBaseEventJson(
		const FString& DetectorName,
		const FString& EventName,
		ET66PerformanceSeverity Severity,
		ET66PerformanceConfidence Confidence,
		const FString& Summary,
		const TArray<FT66PerformanceAttribution>& Attributions) const;

	TSharedRef<FJsonObject> CreateSnapshotJson(const FString& ExitReason) const;
	TSharedRef<FJsonObject> CreateBuildJson() const;
	TSharedRef<FJsonObject> CreateFrameSummaryJson(const FFrameSummary& Summary) const;
	TSharedRef<FJsonObject> CreateMemorySummaryJson() const;
	TArray<TSharedPtr<FJsonValue>> CreateRecentLogsJson() const;
	TArray<TSharedPtr<FJsonValue>> CreateRecentEventsJson() const;
	TArray<TSharedPtr<FJsonValue>> CreateDetectorRuntimeJson() const;

	FFrameSummary CalculateFrameSummary(double WindowSeconds) const;
	FMemorySample ReadMemorySample(double NowSeconds) const;
	void PruneRollingSamples(double NowSeconds);
	void EnforceRetentionBudget() const;
	int64 GetDirectorySizeBytes(const FString& Directory) const;
	bool ShouldIncludeHardwareFingerprint() const;
	FString GetBuildConfigString() const;
	FString GetProtonStatusString() const;
	FString GetWorldNameForReports() const;
	FString GetMapNameForReports() const;
	FString SanitizeForReport(FString Value) const;

	const class UT66PerformanceSystemSettings* Settings = nullptr;
	bool bInitialized = false;
	FString SessionId;
	FDateTime SessionStartedUtc;
	FString PerformanceRootDir;
	FString SessionsRootDir;
	FString SessionDir;
	FString EventsJsonlPath;
	FString SnapshotCurrentPath;
	FString SnapshotPreviousPath;
	uint64 EventCounter = 0;

	FTSTicker::FDelegateHandle FrameTickerHandle;
	FDelegateHandle PreGarbageCollectHandle;
	FDelegateHandle PostGarbageCollectHandle;
	FDelegateHandle SystemErrorHandle;
	TUniquePtr<FPerformanceLogOutputDevice> LogOutputDevice;

	TArray<FFrameSample> FrameSamples;
	TArray<FMemorySample> MemorySamples;
	TMap<FString, FDetectorRuntime> DetectorRuntime;
	TMap<FString, int32> EventCountsByName;
	TArray<FString> RecentEventSummaries;
	mutable FCriticalSection LogLinesCriticalSection;
	TArray<FString> RecentLogLines;

	double LastSnapshotSeconds = -DBL_MAX;
	double LastMemorySampleSeconds = -DBL_MAX;
	double LastHitchEventSeconds = -DBL_MAX;
	double LastLowFpsEventSeconds = -DBL_MAX;
	double LastStutterEventSeconds = -DBL_MAX;
	double LastMemoryGrowthEventSeconds = -DBL_MAX;
	double LastBasicHangEventSeconds = -DBL_MAX;
	double LastFrameworkBudgetEventSeconds = -DBL_MAX;
	double PreGarbageCollectSeconds = 0.0;
};
