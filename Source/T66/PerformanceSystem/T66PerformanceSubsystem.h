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

	struct FBoardSaturationFrameSample
	{
		double SessionTimeSeconds = 0.0;
		double GameTimeSeconds = 0.0;
		double FrameTimeMs = 0.0;
		int32 LiveRegularEnemies = -1;
		int32 LiveRichEnemies = -1;
		int32 LiveLightweightMobs = -1;
		int32 LiveLightweightMeleeMobs = -1;
		int32 LiveLightweightRushMobs = -1;
		int32 LiveLightweightFlyingMobs = -1;
		int32 LiveLightweightRangedMobs = -1;
		int32 PendingSpawns = -1;
		int32 ActiveEnemyProjectiles = -1;
		int32 LightweightPoolReuseAcquires = -1;
		int32 LightweightPoolReleases = -1;
		int32 LightweightPoolInactive = -1;
		int32 LightweightPoolInactivePeak = -1;
		double BoardSaturationSampleAgeSeconds = 0.0;
		bool bBoardSaturationValid = false;
		FString WorldName;
		FString MapName;
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

	struct FFrameworkSubstepTimingStats
	{
		int32 SampleCount = 0;
		double FrameSampleAppendPeakUs = 0.0;
		double BoardSampleCapturePeakUs = 0.0;
		double PruneSamplesPeakUs = 0.0;
		double SingleFrameHitchPeakUs = 0.0;
		double FramePacingDetectorPeakUs = 0.0;
		double MemoryGrowthDetectorPeakUs = 0.0;
		double BasicHangDetectorPeakUs = 0.0;
		double PeriodicSnapshotPeakUs = 0.0;
		double EventJsonBuildPeakUs = 0.0;
		double EventJsonAppendPeakUs = 0.0;
		double FrameworkTotalPeakUs = 0.0;
		double InstrumentationProbePeakUs = 0.0;
		double LastFrameSampleAppendUs = 0.0;
		double LastBoardSampleCaptureUs = 0.0;
		double LastPruneSamplesUs = 0.0;
		double LastSingleFrameHitchUs = 0.0;
		double LastFramePacingDetectorUs = 0.0;
		double LastMemoryGrowthDetectorUs = 0.0;
		double LastBasicHangDetectorUs = 0.0;
		double LastPeriodicSnapshotUs = 0.0;
		double LastFrameworkTotalUs = 0.0;
		double LastInstrumentationProbeUs = 0.0;
	};

	struct FPerformanceWriteQueueStats
	{
		uint64 AttemptedWrites = 0;
		uint64 QueuedWrites = 0;
		uint64 CompletedWrites = 0;
		uint64 FailedWrites = 0;
		uint64 FallbackWrites = 0;
		uint64 QueueFullFallbackWrites = 0;
		uint64 ClosingFallbackWrites = 0;
		uint64 WorkerUnavailableFallbackWrites = 0;
		uint64 AbandonedWrites = 0;
		uint64 AppendWrites = 0;
		uint64 AtomicReplaceWrites = 0;
		uint64 MaxQueueDepth = 0;
		double LastWorkerWriteUs = 0.0;
		double WorkerWritePeakUs = 0.0;
		double LastFallbackWriteUs = 0.0;
		double FallbackWritePeakUs = 0.0;
		double LastFlushWaitMs = 0.0;
		double ShutdownFlushWaitMs = 0.0;
		int32 Capacity = 0;
		int32 CurrentQueueDepth = 0;
		bool bWorkerRunning = false;
		bool bClosing = false;
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

	class FPerformanceWriteWorker;
	void StartPerformanceWriteWorker();
	void BeginPerformanceWriteShutdown();
	void StopPerformanceWriteWorker();
	bool FlushPerformanceWrites(double TimeoutSeconds, const TCHAR* Reason);
	bool EnsurePerformanceProducerGameThread(const TCHAR* FunctionName) const;
	void QueuePerformanceAppend(const FString& TargetPath, const FString& Payload, const TCHAR* StreamName);
	void QueuePerformanceAtomicReplace(const FString& TargetPath, const FString& Payload, const TCHAR* StreamName);
	void RunWriteQueueOrderingSelfTest();

	void CaptureLogLine(const FString& Category, ELogVerbosity::Type Verbosity, const FString& Message);
	void HandlePreGarbageCollect();
	void HandlePostGarbageCollect();
	void HandleSystemError();

	void CheckFrameDetectors(float DeltaSeconds);
	void CheckSingleFrameHitch(float DeltaSeconds);
	void CheckMemoryDetector();
	void CheckBasicHangDetector(float DeltaSeconds);
	void CaptureBoardSaturationFrameSample(double NowSeconds, double FrameMs);
	void WritePeriodicSnapshot(bool bForce);
	void WriteFinalReport(const FString& ExitReason);
	void WriteBoardSaturationFrameSamples() const;

	void EmitPerformanceEvent(
		const FString& DetectorName,
		const FString& EventName,
		ET66PerformanceSeverity Severity,
		ET66PerformanceConfidence Confidence,
		const FString& Summary,
		const TArray<FT66PerformanceAttribution>& Attributions);
	void AppendBoardSaturationAttributions(TArray<FT66PerformanceAttribution>& Attributions) const;

	TSharedRef<FJsonObject> CreateBaseEventJson(
		const FString& DetectorName,
		const FString& EventName,
		ET66PerformanceSeverity Severity,
		ET66PerformanceConfidence Confidence,
		const FString& Summary,
		const TArray<FT66PerformanceAttribution>& Attributions) const;

	TSharedRef<FJsonObject> CreateSnapshotJson(const FString& ExitReason, bool bFullFrameSummary = true, int32 RecentLogLimit = INDEX_NONE) const;
	TSharedRef<FJsonObject> CreateRunningSnapshotJson() const;
	TSharedRef<FJsonObject> CreateBuildJson() const;
	TSharedRef<FJsonObject> CreateFrameSummaryJson(const FFrameSummary& Summary) const;
	TSharedRef<FJsonObject> CreateMemorySummaryJson() const;
	TSharedRef<FJsonObject> CreateFrameworkSubstepAttributionJson() const;
	TSharedRef<FJsonObject> CreateWriteQueueStatsJson() const;
	TArray<TSharedPtr<FJsonValue>> CreateRecentLogsJson(int32 MaxLines = INDEX_NONE) const;
	TArray<TSharedPtr<FJsonValue>> CreateRecentEventsJson() const;
	TArray<TSharedPtr<FJsonValue>> CreateDetectorRuntimeJson() const;

	FFrameSummary CalculateFrameSummary(double WindowSeconds, bool bIncludePercentiles = true) const;
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
	FString BoardSaturationSamplesJsonlPath;
	FString SnapshotCurrentPath;
	FString SnapshotPreviousPath;
	uint64 EventCounter = 0;

	FTSTicker::FDelegateHandle FrameTickerHandle;
	FDelegateHandle PreGarbageCollectHandle;
	FDelegateHandle PostGarbageCollectHandle;
	FDelegateHandle SystemErrorHandle;
	TUniquePtr<FPerformanceLogOutputDevice> LogOutputDevice;
	FPerformanceWriteWorker* WriteWorker = nullptr;

	TArray<FFrameSample> FrameSamples;
	TArray<FMemorySample> MemorySamples;
	TArray<FBoardSaturationFrameSample> BoardSaturationFrameSamples;
	TMap<FString, FDetectorRuntime> DetectorRuntime;
	TMap<FString, int32> EventCountsByName;
	TArray<FString> RecentEventSummaries;
	mutable FCriticalSection LogLinesCriticalSection;
	TArray<FString> RecentLogLines;

	double LastSnapshotSeconds = -DBL_MAX;
	double LastMemorySampleSeconds = -DBL_MAX;
	double LastLowFpsEventSeconds = -DBL_MAX;
	double LastStutterEventSeconds = -DBL_MAX;
	double LastMemoryGrowthEventSeconds = -DBL_MAX;
	double LastBasicHangEventSeconds = -DBL_MAX;
	double LastFrameworkBudgetEventSeconds = -DBL_MAX;
	double PreGarbageCollectSeconds = 0.0;
	FFrameworkSubstepTimingStats FrameworkSubstepTimingStats;
};
