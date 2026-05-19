// Copyright Tribulation 66. All Rights Reserved.

#include "PerformanceSystem/T66PerformanceSubsystem.h"

#include "PerformanceSystem/T66PerformanceSystemSettings.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformProcess.h"
#include "Internationalization/Internationalization.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/CoreDelegates.h"
#include "Misc/DateTime.h"
#include "Misc/EngineVersion.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66PerformanceSystem, Log, All);

namespace
{
constexpr int32 T66PerformanceSchemaVersion = 1;
constexpr int32 T66PerformanceMaxRecentEvents = 100;
constexpr double BytesToMegabytes = 1.0 / (1024.0 * 1024.0);

FString SeverityToString(const ET66PerformanceSeverity Severity)
{
	switch (Severity)
	{
	case ET66PerformanceSeverity::Info:
		return TEXT("Info");
	case ET66PerformanceSeverity::Warning:
		return TEXT("Warning");
	case ET66PerformanceSeverity::Error:
		return TEXT("Error");
	case ET66PerformanceSeverity::Critical:
		return TEXT("Critical");
	default:
		return TEXT("Info");
	}
}

FString ConfidenceToString(const ET66PerformanceConfidence Confidence)
{
	switch (Confidence)
	{
	case ET66PerformanceConfidence::Exact:
		return TEXT("Exact");
	case ET66PerformanceConfidence::Sampled:
		return TEXT("Sampled");
	case ET66PerformanceConfidence::Inferred:
		return TEXT("Inferred");
	case ET66PerformanceConfidence::Unavailable:
	default:
		return TEXT("Unavailable");
	}
}

FString VerbosityToString(const ELogVerbosity::Type Verbosity)
{
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:
		return TEXT("Fatal");
	case ELogVerbosity::Error:
		return TEXT("Error");
	case ELogVerbosity::Warning:
		return TEXT("Warning");
	case ELogVerbosity::Display:
		return TEXT("Display");
	case ELogVerbosity::Log:
		return TEXT("Log");
	case ELogVerbosity::Verbose:
		return TEXT("Verbose");
	case ELogVerbosity::VeryVerbose:
		return TEXT("VeryVerbose");
	default:
		return TEXT("Unknown");
	}
}

FString JsonObjectToString(const TSharedRef<FJsonObject>& JsonObject, const bool bPretty)
{
	FString Output;
	if (bPretty)
	{
		const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Output);
		FJsonSerializer::Serialize(JsonObject, Writer);
	}
	else
	{
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
		FJsonSerializer::Serialize(JsonObject, Writer);
	}
	return Output;
}

bool SaveStringAtomic(const FString& TargetPath, const FString& Contents)
{
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(TargetPath), true);

	const FString TempPath = TargetPath + TEXT(".tmp");
	if (!FFileHelper::SaveStringToFile(Contents, *TempPath))
	{
		return false;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*TargetPath))
	{
		PlatformFile.DeleteFile(*TargetPath);
	}

	return PlatformFile.MoveFile(*TargetPath, *TempPath);
}

void SetNumberOrUnavailable(const TSharedRef<FJsonObject>& Object, const TCHAR* FieldName, const double Value)
{
	if (Value > 0.0)
	{
		Object->SetNumberField(FieldName, Value);
	}
	else
	{
		Object->SetStringField(FieldName, TEXT("Unavailable"));
	}
}
}

void UT66PerformanceSubsystem::FPerformanceLogOutputDevice::Serialize(
	const TCHAR* V,
	ELogVerbosity::Type Verbosity,
	const FName& Category)
{
	Owner.CaptureLogLine(Category.ToString(), Verbosity, V ? FString(V) : FString());
}

void UT66PerformanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Settings = GetDefault<UT66PerformanceSystemSettings>();
	if (!Settings || !Settings->bEnablePerformanceSystem)
	{
		return;
	}

	SessionStartedUtc = FDateTime::UtcNow();
	SessionId = FString::Printf(
		TEXT("%s_%s"),
		*SessionStartedUtc.ToString(TEXT("%Y%m%dT%H%M%SZ")),
		*FGuid::NewGuid().ToString(EGuidFormats::Short));

	PerformanceRootDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("PerformanceSystem"));
	SessionsRootDir = FPaths::Combine(PerformanceRootDir, TEXT("Sessions"));
	SessionDir = FPaths::Combine(SessionsRootDir, SessionId);
	EventsJsonlPath = FPaths::Combine(SessionDir, TEXT("events.jsonl"));
	SnapshotCurrentPath = FPaths::Combine(PerformanceRootDir, TEXT("snapshot.current.json"));
	SnapshotPreviousPath = FPaths::Combine(PerformanceRootDir, TEXT("snapshot.previous.json"));

	IFileManager::Get().MakeDirectory(*SessionDir, true);

	FrameTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UT66PerformanceSubsystem::TickPerformanceSystem));

	PreGarbageCollectHandle = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddUObject(
		this, &UT66PerformanceSubsystem::HandlePreGarbageCollect);
	PostGarbageCollectHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(
		this, &UT66PerformanceSubsystem::HandlePostGarbageCollect);
	SystemErrorHandle = FCoreDelegates::OnHandleSystemError.AddUObject(
		this, &UT66PerformanceSubsystem::HandleSystemError);

	if (GLog && Settings->MaxCapturedLogLines > 0)
	{
		LogOutputDevice = MakeUnique<FPerformanceLogOutputDevice>(*this);
		GLog->AddOutputDevice(LogOutputDevice.Get());
	}

	bInitialized = true;

	EmitPerformanceEvent(
		TEXT("PerformanceSystem"),
		TEXT("SessionStarted"),
		ET66PerformanceSeverity::Info,
		ET66PerformanceConfidence::Exact,
		TEXT("PerformanceSystem session initialized."),
		{
			{ TEXT("ProtonStatus"), GetProtonStatusString(), ET66PerformanceConfidence::Inferred, TEXT("Environment"), 0.0 },
			{ TEXT("HardwareFingerprintIncluded"), ShouldIncludeHardwareFingerprint() ? TEXT("true") : TEXT("false"), ET66PerformanceConfidence::Exact, TEXT("Settings"), 0.0 }
		});

	WritePeriodicSnapshot(true);
	EnforceRetentionBudget();
}

void UT66PerformanceSubsystem::Deinitialize()
{
	if (bInitialized)
	{
		WriteFinalReport(TEXT("SubsystemDeinitialize"));
	}

	if (FrameTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FrameTickerHandle);
		FrameTickerHandle.Reset();
	}

	if (PreGarbageCollectHandle.IsValid())
	{
		FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(PreGarbageCollectHandle);
		PreGarbageCollectHandle.Reset();
	}

	if (PostGarbageCollectHandle.IsValid())
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGarbageCollectHandle);
		PostGarbageCollectHandle.Reset();
	}

	if (SystemErrorHandle.IsValid())
	{
		FCoreDelegates::OnHandleSystemError.Remove(SystemErrorHandle);
		SystemErrorHandle.Reset();
	}

	if (GLog && LogOutputDevice.IsValid())
	{
		GLog->RemoveOutputDevice(LogOutputDevice.Get());
		LogOutputDevice.Reset();
	}

	bInitialized = false;
	Super::Deinitialize();
}

void UT66PerformanceSubsystem::RecordMeasuredOperation(
	const FString& OperationName,
	const double DurationMs,
	const FString& Source)
{
	if (!bInitialized || !Settings || DurationMs < Settings->ProjectOperationWarningMs)
	{
		return;
	}

	EmitPerformanceEvent(
		TEXT("ProjectOperationStallDetector"),
		TEXT("ProjectOperationStall"),
		ET66PerformanceSeverity::Warning,
		ET66PerformanceConfidence::Sampled,
		FString::Printf(TEXT("%s took %.2f ms."), *OperationName, DurationMs),
		{
			{ TEXT("OperationName"), OperationName, ET66PerformanceConfidence::Exact, Source, 0.0 },
			{ TEXT("DurationMs"), FString::Printf(TEXT("%.3f"), DurationMs), ET66PerformanceConfidence::Sampled, Source, 0.0 }
		});
}

bool UT66PerformanceSubsystem::TickPerformanceSystem(const float DeltaSeconds)
{
	if (!Settings || !Settings->bEnablePerformanceSystem)
	{
		return true;
	}

	const double TickStartSeconds = FPlatformTime::Seconds();
	const double NowSeconds = TickStartSeconds;

	FrameSamples.Add({ NowSeconds, static_cast<double>(DeltaSeconds) * 1000.0 });
	PruneRollingSamples(NowSeconds);

	RunDetector(TEXT("FramePacingDetector"), Settings->DetectorBudgetUs, 1.0, [this, DeltaSeconds]()
	{
		CheckFrameDetectors(DeltaSeconds);
	});

	RunDetector(TEXT("MemoryGrowthDetector"), Settings->DetectorBudgetUs, 1.0, [this]()
	{
		CheckMemoryDetector();
	});

	RunDetector(TEXT("BasicHangDetector"), Settings->DetectorBudgetUs, 0.0, [this, DeltaSeconds]()
	{
		CheckBasicHangDetector(DeltaSeconds);
	});

	WritePeriodicSnapshot(false);

	const double FrameworkCostUs = (FPlatformTime::Seconds() - TickStartSeconds) * 1000000.0;
	if (FrameworkCostUs > Settings->FrameworkFrameBudgetUs
		&& NowSeconds - LastFrameworkBudgetEventSeconds > Settings->FrameDetectorCooldownSeconds)
	{
		LastFrameworkBudgetEventSeconds = NowSeconds;
		EmitPerformanceEvent(
			TEXT("PerformanceSystemOverhead"),
			TEXT("FrameworkBudgetExceeded"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Exact,
			FString::Printf(TEXT("PerformanceSystem frame cost was %.2f us."), FrameworkCostUs),
			{
				{ TEXT("FrameworkCostUs"), FString::Printf(TEXT("%.3f"), FrameworkCostUs), ET66PerformanceConfidence::Exact, TEXT("FPlatformTime"), 0.0 },
				{ TEXT("BudgetUs"), FString::Printf(TEXT("%.3f"), Settings->FrameworkFrameBudgetUs), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 }
			});
	}

	return true;
}

void UT66PerformanceSubsystem::RunDetector(
	const TCHAR* DetectorName,
	const double BudgetUs,
	const double CadenceSeconds,
	TFunctionRef<void()> Work)
{
	const double NowSeconds = FPlatformTime::Seconds();
	FDetectorRuntime& Runtime = DetectorRuntime.FindOrAdd(DetectorName);
	Runtime.Name = DetectorName;
	Runtime.BudgetUs = BudgetUs;
	Runtime.CadenceSeconds = CadenceSeconds;

	if (Runtime.bDisabled)
	{
		return;
	}

	if (CadenceSeconds > 0.0 && NowSeconds - Runtime.LastRunSeconds < Runtime.CadenceSeconds)
	{
		return;
	}

	Runtime.LastRunSeconds = NowSeconds;

	const double StartSeconds = FPlatformTime::Seconds();
	Work();
	Runtime.LastCostUs = (FPlatformTime::Seconds() - StartSeconds) * 1000000.0;
	Runtime.PeakCostUs = FMath::Max(Runtime.PeakCostUs, Runtime.LastCostUs);

	if (Runtime.LastCostUs <= Runtime.BudgetUs)
	{
		Runtime.ConsecutiveBudgetOverruns = 0;
		return;
	}

	++Runtime.ConsecutiveBudgetOverruns;
	if (Runtime.ConsecutiveBudgetOverruns == 3)
	{
		Runtime.CadenceSeconds = FMath::Max(Runtime.CadenceSeconds * 2.0, 1.0);
		EmitPerformanceEvent(
			TEXT("PerformanceSystemOverhead"),
			TEXT("DetectorCadenceDegraded"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Exact,
			FString::Printf(TEXT("%s exceeded budget repeatedly; cadence degraded to %.2f seconds."), DetectorName, Runtime.CadenceSeconds),
			{
				{ TEXT("DetectorName"), DetectorName, ET66PerformanceConfidence::Exact, TEXT("DetectorRuntime"), 0.0 },
				{ TEXT("LastCostUs"), FString::Printf(TEXT("%.3f"), Runtime.LastCostUs), ET66PerformanceConfidence::Exact, TEXT("FPlatformTime"), 0.0 },
				{ TEXT("BudgetUs"), FString::Printf(TEXT("%.3f"), Runtime.BudgetUs), ET66PerformanceConfidence::Exact, TEXT("DetectorRuntime"), 0.0 }
			});
	}
	else if (Runtime.ConsecutiveBudgetOverruns >= 6)
	{
		Runtime.bDisabled = true;
		EmitPerformanceEvent(
			TEXT("PerformanceSystemOverhead"),
			TEXT("DetectorDisabled"),
			ET66PerformanceSeverity::Error,
			ET66PerformanceConfidence::Exact,
			FString::Printf(TEXT("%s disabled after repeated budget overruns."), DetectorName),
			{
				{ TEXT("DetectorName"), DetectorName, ET66PerformanceConfidence::Exact, TEXT("DetectorRuntime"), 0.0 },
				{ TEXT("PeakCostUs"), FString::Printf(TEXT("%.3f"), Runtime.PeakCostUs), ET66PerformanceConfidence::Exact, TEXT("FPlatformTime"), 0.0 }
			});
	}
}

void UT66PerformanceSubsystem::CaptureLogLine(
	const FString& Category,
	const ELogVerbosity::Type Verbosity,
	const FString& Message)
{
	if (!Settings || Settings->MaxCapturedLogLines <= 0)
	{
		return;
	}

	const FString Line = FString::Printf(
		TEXT("%s [%s] %s"),
		*Category,
		*VerbosityToString(Verbosity),
		*SanitizeForReport(Message));

	FScopeLock Lock(&LogLinesCriticalSection);
	RecentLogLines.Add(Line);
	while (RecentLogLines.Num() > Settings->MaxCapturedLogLines)
	{
		RecentLogLines.RemoveAt(0, 1, EAllowShrinking::No);
	}
}

void UT66PerformanceSubsystem::HandlePreGarbageCollect()
{
	PreGarbageCollectSeconds = FPlatformTime::Seconds();
}

void UT66PerformanceSubsystem::HandlePostGarbageCollect()
{
	if (!bInitialized || !Settings || PreGarbageCollectSeconds <= 0.0)
	{
		return;
	}

	const double DurationMs = (FPlatformTime::Seconds() - PreGarbageCollectSeconds) * 1000.0;
	if (DurationMs >= Settings->GCPauseWarningMs)
	{
		EmitPerformanceEvent(
			TEXT("GCPauseDetector"),
			TEXT("GCPauseSpike"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Exact,
			FString::Printf(TEXT("Garbage collection pause took %.2f ms."), DurationMs),
			{
				{ TEXT("DurationMs"), FString::Printf(TEXT("%.3f"), DurationMs), ET66PerformanceConfidence::Exact, TEXT("FCoreUObjectDelegates"), 0.0 },
				{ TEXT("ThresholdMs"), FString::Printf(TEXT("%.3f"), Settings->GCPauseWarningMs), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 }
			});
	}
}

void UT66PerformanceSubsystem::HandleSystemError()
{
	if (!bInitialized)
	{
		return;
	}

	const FString CrashMarkerPath = FPaths::Combine(SessionDir, TEXT("crash_marker.json"));
	const TSharedRef<FJsonObject> Marker = MakeShared<FJsonObject>();
	Marker->SetNumberField(TEXT("SchemaVersion"), T66PerformanceSchemaVersion);
	Marker->SetStringField(TEXT("SessionId"), SessionId);
	Marker->SetStringField(TEXT("WallClockUtc"), FDateTime::UtcNow().ToIso8601());
	Marker->SetStringField(TEXT("Reason"), TEXT("FCoreDelegates::OnHandleSystemError"));
	Marker->SetStringField(TEXT("Caveat"), TEXT("Best-effort crash marker; snapshot.current.json is the primary forensic artifact."));
	SaveStringAtomic(CrashMarkerPath, JsonObjectToString(Marker, true));
}

void UT66PerformanceSubsystem::CheckFrameDetectors(const float DeltaSeconds)
{
	const double NowSeconds = FPlatformTime::Seconds();
	const double FrameMs = static_cast<double>(DeltaSeconds) * 1000.0;

	if (FrameMs >= Settings->HitchThresholdMs && NowSeconds - LastHitchEventSeconds >= Settings->FrameDetectorCooldownSeconds)
	{
		LastHitchEventSeconds = NowSeconds;
		EmitPerformanceEvent(
			TEXT("FramePacingDetector"),
			TEXT("SingleFrameHitch"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Exact,
			FString::Printf(TEXT("Frame took %.2f ms."), FrameMs),
			{
				{ TEXT("FrameTimeMs"), FString::Printf(TEXT("%.3f"), FrameMs), ET66PerformanceConfidence::Exact, TEXT("FTSTicker DeltaSeconds"), 0.0 },
				{ TEXT("ThresholdMs"), FString::Printf(TEXT("%.3f"), Settings->HitchThresholdMs), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 }
			});
	}

	const FFrameSummary Summary = CalculateFrameSummary(Settings->SustainedLowFpsWindowSeconds);
	if (Summary.SampleCount <= 1)
	{
		return;
	}

	if (Summary.AverageFps > 0.0
		&& Summary.AverageFps < Settings->SustainedLowFpsThreshold
		&& NowSeconds - LastLowFpsEventSeconds >= Settings->FrameDetectorCooldownSeconds)
	{
		LastLowFpsEventSeconds = NowSeconds;
		EmitPerformanceEvent(
			TEXT("FramePacingDetector"),
			TEXT("SustainedLowFps"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Sampled,
			FString::Printf(TEXT("Rolling FPS was %.2f over %.1f seconds."), Summary.AverageFps, Settings->SustainedLowFpsWindowSeconds),
			{
				{ TEXT("AverageFps"), FString::Printf(TEXT("%.3f"), Summary.AverageFps), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds },
				{ TEXT("ThresholdFps"), FString::Printf(TEXT("%.3f"), Settings->SustainedLowFpsThreshold), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 },
				{ TEXT("OnePercentLowFps"), FString::Printf(TEXT("%.3f"), Summary.OnePercentLowFps), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds },
				{ TEXT("PointOnePercentLowFps"), FString::Printf(TEXT("%.3f"), Summary.PointOnePercentLowFps), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds }
			});
	}

	if (Summary.StdDevMs >= Settings->StutterStdDevThresholdMs
		&& NowSeconds - LastStutterEventSeconds >= Settings->FrameDetectorCooldownSeconds)
	{
		LastStutterEventSeconds = NowSeconds;
		EmitPerformanceEvent(
			TEXT("FramePacingDetector"),
			TEXT("FrameVarianceStutter"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Sampled,
			FString::Printf(TEXT("Frame-time standard deviation was %.2f ms over %.1f seconds."), Summary.StdDevMs, Settings->SustainedLowFpsWindowSeconds),
			{
				{ TEXT("StdDevMs"), FString::Printf(TEXT("%.3f"), Summary.StdDevMs), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds },
				{ TEXT("ThresholdMs"), FString::Printf(TEXT("%.3f"), Settings->StutterStdDevThresholdMs), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 }
			});
	}
}

void UT66PerformanceSubsystem::CheckMemoryDetector()
{
	const double NowSeconds = FPlatformTime::Seconds();
	if (NowSeconds - LastMemorySampleSeconds < 1.0)
	{
		return;
	}

	LastMemorySampleSeconds = NowSeconds;
	MemorySamples.Add(ReadMemorySample(NowSeconds));
	PruneRollingSamples(NowSeconds);

	if (MemorySamples.Num() < 2)
	{
		return;
	}

	const FMemorySample& Oldest = MemorySamples[0];
	const FMemorySample& Newest = MemorySamples.Last();
	const double WindowMinutes = (Newest.TimeSeconds - Oldest.TimeSeconds) / 60.0;
	if (WindowMinutes <= 0.0)
	{
		return;
	}

	const double GrowthMb = static_cast<double>(Newest.UsedPhysicalBytes) * BytesToMegabytes
		- static_cast<double>(Oldest.UsedPhysicalBytes) * BytesToMegabytes;
	const double GrowthMbPerMinute = GrowthMb / WindowMinutes;

	if (GrowthMbPerMinute >= Settings->MemoryGrowthWarningMbPerMinute
		&& NowSeconds - LastMemoryGrowthEventSeconds >= Settings->FrameDetectorCooldownSeconds)
	{
		LastMemoryGrowthEventSeconds = NowSeconds;
		EmitPerformanceEvent(
			TEXT("MemoryGrowthDetector"),
			TEXT("PhysicalMemoryGrowth"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Sampled,
			FString::Printf(TEXT("Physical memory grew %.2f MB/min over %.1f seconds."), GrowthMbPerMinute, Newest.TimeSeconds - Oldest.TimeSeconds),
			{
				{ TEXT("GrowthMbPerMinute"), FString::Printf(TEXT("%.3f"), GrowthMbPerMinute), ET66PerformanceConfidence::Sampled, TEXT("FPlatformMemory::GetStats"), Newest.TimeSeconds - Oldest.TimeSeconds },
				{ TEXT("UsedPhysicalMb"), FString::Printf(TEXT("%.3f"), static_cast<double>(Newest.UsedPhysicalBytes) * BytesToMegabytes), ET66PerformanceConfidence::Exact, TEXT("FPlatformMemory::GetStats"), 0.0 },
				{ TEXT("ThresholdMbPerMinute"), FString::Printf(TEXT("%.3f"), Settings->MemoryGrowthWarningMbPerMinute), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 }
			});
	}
}

void UT66PerformanceSubsystem::CheckBasicHangDetector(const float DeltaSeconds)
{
	const double NowSeconds = FPlatformTime::Seconds();
	if (DeltaSeconds < Settings->BasicHangFrameDeltaSeconds
		|| NowSeconds - LastBasicHangEventSeconds < Settings->FrameDetectorCooldownSeconds)
	{
		return;
	}

	LastBasicHangEventSeconds = NowSeconds;
	EmitPerformanceEvent(
		TEXT("BasicHangDetector"),
		TEXT("LargeFrameDelta"),
		ET66PerformanceSeverity::Error,
		ET66PerformanceConfidence::Inferred,
		FString::Printf(TEXT("Frame delta was %.2f seconds; this is a best-effort in-engine hang signal."), static_cast<double>(DeltaSeconds)),
		{
			{ TEXT("DeltaSeconds"), FString::Printf(TEXT("%.3f"), static_cast<double>(DeltaSeconds)), ET66PerformanceConfidence::Exact, TEXT("FTSTicker DeltaSeconds"), 0.0 },
			{ TEXT("Caveat"), TEXT("A fully stuck game thread may not record itself; external hang detection is future work."), ET66PerformanceConfidence::Exact, TEXT("PerformanceSystemDesign"), 0.0 }
		});
}

void UT66PerformanceSubsystem::WritePeriodicSnapshot(const bool bForce)
{
	if (!Settings || !bInitialized)
	{
		return;
	}

	const double NowSeconds = FPlatformTime::Seconds();
	if (!bForce && NowSeconds - LastSnapshotSeconds < Settings->SnapshotCadenceSeconds)
	{
		return;
	}

	LastSnapshotSeconds = NowSeconds;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*SnapshotCurrentPath))
	{
		PlatformFile.DeleteFile(*SnapshotPreviousPath);
		PlatformFile.MoveFile(*SnapshotPreviousPath, *SnapshotCurrentPath);
	}

	const TSharedRef<FJsonObject> Snapshot = CreateSnapshotJson(TEXT("Running"));
	SaveStringAtomic(SnapshotCurrentPath, JsonObjectToString(Snapshot, true));
}

void UT66PerformanceSubsystem::WriteFinalReport(const FString& ExitReason)
{
	if (!Settings)
	{
		return;
	}

	const TSharedRef<FJsonObject> Report = CreateSnapshotJson(ExitReason);
	const FString JsonPath = FPaths::Combine(SessionDir, TEXT("session_summary.json"));
	SaveStringAtomic(JsonPath, JsonObjectToString(Report, true));

	FString Markdown;
	Markdown += TEXT("# T66 PerformanceSystem Session\n\n");
	Markdown += FString::Printf(TEXT("- SchemaVersion: %d\n"), T66PerformanceSchemaVersion);
	Markdown += FString::Printf(TEXT("- SessionId: %s\n"), *SessionId);
	Markdown += FString::Printf(TEXT("- StartedUtc: %s\n"), *SessionStartedUtc.ToIso8601());
	Markdown += FString::Printf(TEXT("- EndedUtc: %s\n"), *FDateTime::UtcNow().ToIso8601());
	Markdown += FString::Printf(TEXT("- ExitReason: %s\n"), *ExitReason);
	Markdown += FString::Printf(TEXT("- BuildConfig: %s\n"), *GetBuildConfigString());
	Markdown += FString::Printf(TEXT("- World: %s\n"), *GetWorldNameForReports());
	Markdown += FString::Printf(TEXT("- Map: %s\n\n"), *GetMapNameForReports());

	Markdown += TEXT("## Event Counts\n\n");
	if (EventCountsByName.Num() == 0)
	{
		Markdown += TEXT("- None\n");
	}
	else
	{
		for (const TPair<FString, int32>& Pair : EventCountsByName)
		{
			Markdown += FString::Printf(TEXT("- %s: %d\n"), *Pair.Key, Pair.Value);
		}
	}

	const FFrameSummary FrameSummary = CalculateFrameSummary(Settings->FrameWindowSeconds);
	Markdown += TEXT("\n## Frame Summary\n\n");
	Markdown += FString::Printf(TEXT("- Samples: %d\n"), FrameSummary.SampleCount);
	Markdown += FString::Printf(TEXT("- Average FPS: %.2f\n"), FrameSummary.AverageFps);
	Markdown += FString::Printf(TEXT("- Average frame: %.2f ms\n"), FrameSummary.AverageFrameMs);
	Markdown += FString::Printf(TEXT("- Std dev: %.2f ms\n"), FrameSummary.StdDevMs);
	Markdown += FString::Printf(TEXT("- 1%% low FPS: %.2f\n"), FrameSummary.OnePercentLowFps);
	Markdown += FString::Printf(TEXT("- 0.1%% low FPS: %.2f\n"), FrameSummary.PointOnePercentLowFps);

	Markdown += TEXT("\n## Recent Events\n\n");
	if (RecentEventSummaries.Num() == 0)
	{
		Markdown += TEXT("- None\n");
	}
	else
	{
		for (const FString& Event : RecentEventSummaries)
		{
			Markdown += FString::Printf(TEXT("- %s\n"), *Event);
		}
	}

	const FString MarkdownPath = FPaths::Combine(SessionDir, TEXT("session_summary.md"));
	SaveStringAtomic(MarkdownPath, Markdown);
	EnforceRetentionBudget();
}

void UT66PerformanceSubsystem::EmitPerformanceEvent(
	const FString& DetectorName,
	const FString& EventName,
	const ET66PerformanceSeverity Severity,
	const ET66PerformanceConfidence Confidence,
	const FString& Summary,
	const TArray<FT66PerformanceAttribution>& Attributions)
{
	if (!bInitialized)
	{
		return;
	}

	++EventCounter;
	EventCountsByName.FindOrAdd(EventName)++;
	RecentEventSummaries.Add(FString::Printf(TEXT("%llu %s/%s: %s"), EventCounter, *DetectorName, *EventName, *Summary));
	while (RecentEventSummaries.Num() > T66PerformanceMaxRecentEvents)
	{
		RecentEventSummaries.RemoveAt(0, 1, EAllowShrinking::No);
	}

	const TSharedRef<FJsonObject> Event = CreateBaseEventJson(DetectorName, EventName, Severity, Confidence, Summary, Attributions);
	const FString JsonLine = JsonObjectToString(Event, false) + LINE_TERMINATOR;
	FFileHelper::SaveStringToFile(
		JsonLine,
		*EventsJsonlPath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(),
		FILEWRITE_Append);
}

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateBaseEventJson(
	const FString& DetectorName,
	const FString& EventName,
	const ET66PerformanceSeverity Severity,
	const ET66PerformanceConfidence Confidence,
	const FString& Summary,
	const TArray<FT66PerformanceAttribution>& Attributions) const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("SchemaVersion"), T66PerformanceSchemaVersion);
	Root->SetNumberField(TEXT("EventId"), static_cast<double>(EventCounter));
	Root->SetStringField(TEXT("SessionId"), SessionId);
	Root->SetStringField(TEXT("WallClockUtc"), FDateTime::UtcNow().ToIso8601());
	Root->SetNumberField(TEXT("GameTimeSeconds"), GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0);
	Root->SetStringField(TEXT("DetectorName"), DetectorName);
	Root->SetStringField(TEXT("EventName"), EventName);
	Root->SetStringField(TEXT("Severity"), SeverityToString(Severity));
	Root->SetStringField(TEXT("Confidence"), ConfidenceToString(Confidence));
	Root->SetStringField(TEXT("Summary"), SanitizeForReport(Summary));
	Root->SetStringField(TEXT("WorldName"), GetWorldNameForReports());
	Root->SetStringField(TEXT("MapName"), GetMapNameForReports());
	Root->SetObjectField(TEXT("Build"), CreateBuildJson());
	Root->SetObjectField(TEXT("Frame"), CreateFrameSummaryJson(CalculateFrameSummary(Settings ? Settings->SustainedLowFpsWindowSeconds : 10.0)));
	Root->SetObjectField(TEXT("Memory"), CreateMemorySummaryJson());

	TArray<TSharedPtr<FJsonValue>> AttributionValues;
	for (const FT66PerformanceAttribution& Attribution : Attributions)
	{
		const TSharedRef<FJsonObject> JsonAttribution = MakeShared<FJsonObject>();
		JsonAttribution->SetStringField(TEXT("Name"), Attribution.Name);
		JsonAttribution->SetStringField(TEXT("Value"), SanitizeForReport(Attribution.Value));
		JsonAttribution->SetStringField(TEXT("Confidence"), ConfidenceToString(Attribution.Confidence));
		if (!Attribution.Source.IsEmpty())
		{
			JsonAttribution->SetStringField(TEXT("Source"), Attribution.Source);
		}
		if (Attribution.SampleWindowSeconds > 0.0)
		{
			JsonAttribution->SetNumberField(TEXT("SampleWindowSeconds"), Attribution.SampleWindowSeconds);
		}
		AttributionValues.Add(MakeShared<FJsonValueObject>(JsonAttribution));
	}
	Root->SetArrayField(TEXT("Attributions"), AttributionValues);

	return Root;
}

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateSnapshotJson(const FString& ExitReason) const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("SchemaVersion"), T66PerformanceSchemaVersion);
	Root->SetStringField(TEXT("SessionId"), SessionId);
	Root->SetStringField(TEXT("StartedUtc"), SessionStartedUtc.ToIso8601());
	Root->SetStringField(TEXT("EndedUtc"), FDateTime::UtcNow().ToIso8601());
	Root->SetStringField(TEXT("ExitReason"), ExitReason);
	Root->SetObjectField(TEXT("Build"), CreateBuildJson());
	Root->SetStringField(TEXT("WorldName"), GetWorldNameForReports());
	Root->SetStringField(TEXT("MapName"), GetMapNameForReports());
	Root->SetObjectField(TEXT("FrameSummary"), CreateFrameSummaryJson(CalculateFrameSummary(Settings ? Settings->FrameWindowSeconds : 60.0)));
	Root->SetObjectField(TEXT("MemorySummary"), CreateMemorySummaryJson());
	Root->SetArrayField(TEXT("RecentLogs"), CreateRecentLogsJson());
	Root->SetArrayField(TEXT("RecentEvents"), CreateRecentEventsJson());
	Root->SetArrayField(TEXT("DetectorRuntime"), CreateDetectorRuntimeJson());

	const TSharedRef<FJsonObject> EventCounts = MakeShared<FJsonObject>();
	for (const TPair<FString, int32>& Pair : EventCountsByName)
	{
		EventCounts->SetNumberField(Pair.Key, Pair.Value);
	}
	Root->SetObjectField(TEXT("EventCounts"), EventCounts);

	return Root;
}

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateBuildJson() const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("BuildConfig"), GetBuildConfigString());
	Root->SetStringField(TEXT("EngineVersion"), FEngineVersion::Current().ToString());
	Root->SetStringField(TEXT("BuildVersion"), FApp::GetBuildVersion());
	Root->SetStringField(TEXT("ProjectName"), FApp::GetProjectName());
	Root->SetStringField(TEXT("ProtonStatus"), GetProtonStatusString());

	FString ProjectVersion;
	if (GConfig)
	{
		GConfig->GetString(
			TEXT("/Script/EngineSettings.GeneralProjectSettings"),
			TEXT("ProjectVersion"),
			ProjectVersion,
			GGameIni);
	}
	Root->SetStringField(TEXT("ProjectVersion"), ProjectVersion.IsEmpty() ? TEXT("Unavailable") : ProjectVersion);
	Root->SetStringField(TEXT("CookTimestamp"), TEXT("Unavailable"));

	const bool bIncludeHardwareFingerprint = ShouldIncludeHardwareFingerprint();
	Root->SetBoolField(TEXT("HardwareFingerprintIncluded"), bIncludeHardwareFingerprint);
	if (bIncludeHardwareFingerprint)
	{
		const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
		const TSharedRef<FJsonObject> Hardware = MakeShared<FJsonObject>();
		Hardware->SetStringField(TEXT("CpuBrand"), SanitizeForReport(FPlatformMisc::GetCPUBrand()));
		Hardware->SetStringField(TEXT("GpuBrand"), SanitizeForReport(FPlatformMisc::GetPrimaryGPUBrand()));
		Hardware->SetNumberField(TEXT("TotalPhysicalMemoryMb"), static_cast<double>(MemoryStats.TotalPhysical) * BytesToMegabytes);
		Root->SetObjectField(TEXT("HardwareFingerprint"), Hardware);
	}

	return Root;
}

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateFrameSummaryJson(const FFrameSummary& Summary) const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("SampleCount"), Summary.SampleCount);
	Root->SetNumberField(TEXT("LastFrameMs"), Summary.LastFrameMs);
	Root->SetNumberField(TEXT("AverageFrameMs"), Summary.AverageFrameMs);
	SetNumberOrUnavailable(Root, TEXT("AverageFps"), Summary.AverageFps);
	Root->SetNumberField(TEXT("StdDevMs"), Summary.StdDevMs);
	Root->SetNumberField(TEXT("P99FrameMs"), Summary.P99FrameMs);
	Root->SetNumberField(TEXT("P999FrameMs"), Summary.P999FrameMs);
	SetNumberOrUnavailable(Root, TEXT("OnePercentLowFps"), Summary.OnePercentLowFps);
	SetNumberOrUnavailable(Root, TEXT("PointOnePercentLowFps"), Summary.PointOnePercentLowFps);
	return Root;
}

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateMemorySummaryJson() const
{
	const FMemorySample Sample = MemorySamples.Num() > 0 ? MemorySamples.Last() : ReadMemorySample(FPlatformTime::Seconds());
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("UsedPhysicalMb"), static_cast<double>(Sample.UsedPhysicalBytes) * BytesToMegabytes);
	Root->SetNumberField(TEXT("PeakUsedPhysicalMb"), static_cast<double>(Sample.PeakUsedPhysicalBytes) * BytesToMegabytes);
	Root->SetNumberField(TEXT("TotalPhysicalMb"), static_cast<double>(Sample.TotalPhysicalBytes) * BytesToMegabytes);
	Root->SetStringField(TEXT("VramPressure"), TEXT("Unavailable"));
	Root->SetStringField(TEXT("VramPressureSource"), TEXT("No Shipping-safe adapter bound in first pass."));
	return Root;
}

TArray<TSharedPtr<FJsonValue>> UT66PerformanceSubsystem::CreateRecentLogsJson() const
{
	TArray<TSharedPtr<FJsonValue>> Values;
	FScopeLock Lock(&LogLinesCriticalSection);
	for (const FString& Line : RecentLogLines)
	{
		Values.Add(MakeShared<FJsonValueString>(Line));
	}
	return Values;
}

TArray<TSharedPtr<FJsonValue>> UT66PerformanceSubsystem::CreateRecentEventsJson() const
{
	TArray<TSharedPtr<FJsonValue>> Values;
	for (const FString& EventSummary : RecentEventSummaries)
	{
		Values.Add(MakeShared<FJsonValueString>(EventSummary));
	}
	return Values;
}

TArray<TSharedPtr<FJsonValue>> UT66PerformanceSubsystem::CreateDetectorRuntimeJson() const
{
	TArray<TSharedPtr<FJsonValue>> Values;
	for (const TPair<FString, FDetectorRuntime>& Pair : DetectorRuntime)
	{
		const FDetectorRuntime& Runtime = Pair.Value;
		const TSharedRef<FJsonObject> RuntimeJson = MakeShared<FJsonObject>();
		RuntimeJson->SetStringField(TEXT("Name"), Runtime.Name);
		RuntimeJson->SetNumberField(TEXT("CadenceSeconds"), Runtime.CadenceSeconds);
		RuntimeJson->SetNumberField(TEXT("BudgetUs"), Runtime.BudgetUs);
		RuntimeJson->SetNumberField(TEXT("LastCostUs"), Runtime.LastCostUs);
		RuntimeJson->SetNumberField(TEXT("PeakCostUs"), Runtime.PeakCostUs);
		RuntimeJson->SetNumberField(TEXT("ConsecutiveBudgetOverruns"), Runtime.ConsecutiveBudgetOverruns);
		RuntimeJson->SetBoolField(TEXT("Disabled"), Runtime.bDisabled);
		Values.Add(MakeShared<FJsonValueObject>(RuntimeJson));
	}
	return Values;
}

UT66PerformanceSubsystem::FFrameSummary UT66PerformanceSubsystem::CalculateFrameSummary(const double WindowSeconds) const
{
	FFrameSummary Summary;
	const double NowSeconds = FPlatformTime::Seconds();
	TArray<double> FrameTimesMs;
	double TotalFrameMs = 0.0;
	double LastFrameMs = 0.0;

	for (const FFrameSample& Sample : FrameSamples)
	{
		if (NowSeconds - Sample.TimeSeconds <= WindowSeconds)
		{
			FrameTimesMs.Add(Sample.FrameTimeMs);
			TotalFrameMs += Sample.FrameTimeMs;
			LastFrameMs = Sample.FrameTimeMs;
		}
	}

	Summary.SampleCount = FrameTimesMs.Num();
	Summary.LastFrameMs = LastFrameMs;
	if (FrameTimesMs.Num() == 0)
	{
		return Summary;
	}

	Summary.AverageFrameMs = TotalFrameMs / static_cast<double>(FrameTimesMs.Num());
	Summary.AverageFps = Summary.AverageFrameMs > 0.0 ? 1000.0 / Summary.AverageFrameMs : 0.0;

	double Variance = 0.0;
	for (const double FrameTimeMs : FrameTimesMs)
	{
		const double Delta = FrameTimeMs - Summary.AverageFrameMs;
		Variance += Delta * Delta;
	}
	Summary.StdDevMs = FMath::Sqrt(Variance / static_cast<double>(FrameTimesMs.Num()));

	FrameTimesMs.Sort([](const double A, const double B)
	{
		return A > B;
	});

	const int32 P99Index = FMath::Clamp(FMath::FloorToInt(static_cast<double>(FrameTimesMs.Num() - 1) * 0.01), 0, FrameTimesMs.Num() - 1);
	const int32 P999Index = FMath::Clamp(FMath::FloorToInt(static_cast<double>(FrameTimesMs.Num() - 1) * 0.001), 0, FrameTimesMs.Num() - 1);
	Summary.P99FrameMs = FrameTimesMs[P99Index];
	Summary.P999FrameMs = FrameTimesMs[P999Index];
	Summary.OnePercentLowFps = Summary.P99FrameMs > 0.0 ? 1000.0 / Summary.P99FrameMs : 0.0;
	Summary.PointOnePercentLowFps = Summary.P999FrameMs > 0.0 ? 1000.0 / Summary.P999FrameMs : 0.0;

	return Summary;
}

UT66PerformanceSubsystem::FMemorySample UT66PerformanceSubsystem::ReadMemorySample(const double NowSeconds) const
{
	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	FMemorySample Sample;
	Sample.TimeSeconds = NowSeconds;
	Sample.UsedPhysicalBytes = MemoryStats.UsedPhysical;
	Sample.PeakUsedPhysicalBytes = MemoryStats.PeakUsedPhysical;
	Sample.TotalPhysicalBytes = MemoryStats.TotalPhysical;
	return Sample;
}

void UT66PerformanceSubsystem::PruneRollingSamples(const double NowSeconds)
{
	const double FrameWindow = Settings ? FMath::Max(Settings->FrameWindowSeconds, Settings->SustainedLowFpsWindowSeconds) : 60.0;
	for (int32 Index = FrameSamples.Num() - 1; Index >= 0; --Index)
	{
		if (NowSeconds - FrameSamples[Index].TimeSeconds > FrameWindow)
		{
			FrameSamples.RemoveAt(Index, 1, EAllowShrinking::No);
		}
	}

	const double MemoryWindow = Settings ? Settings->MemorySlopeWindowSeconds : 300.0;
	for (int32 Index = MemorySamples.Num() - 1; Index >= 0; --Index)
	{
		if (NowSeconds - MemorySamples[Index].TimeSeconds > MemoryWindow)
		{
			MemorySamples.RemoveAt(Index, 1, EAllowShrinking::No);
		}
	}
}

void UT66PerformanceSubsystem::EnforceRetentionBudget() const
{
	if (!Settings)
	{
		return;
	}

#if UE_BUILD_SHIPPING
	const int64 BudgetBytes = static_cast<int64>(Settings->ShippingDirectoryBudgetMb) * 1024LL * 1024LL;
#else
	const int64 BudgetBytes = static_cast<int64>(Settings->DevelopmentDirectoryBudgetMb) * 1024LL * 1024LL;
#endif

	if (BudgetBytes <= 0)
	{
		return;
	}

	TArray<FString> SessionNames;
	IFileManager::Get().FindFiles(SessionNames, *FPaths::Combine(SessionsRootDir, TEXT("*")), false, true);

	struct FSessionDirectoryInfo
	{
		FString Path;
		FDateTime Timestamp;
		int64 SizeBytes = 0;
	};

	TArray<FSessionDirectoryInfo> Directories;
	int64 TotalBytes = 0;
	for (const FString& SessionName : SessionNames)
	{
		const FString DirectoryPath = FPaths::Combine(SessionsRootDir, SessionName);
		const int64 SizeBytes = GetDirectorySizeBytes(DirectoryPath);
		TotalBytes += SizeBytes;
		Directories.Add({ DirectoryPath, IFileManager::Get().GetTimeStamp(*DirectoryPath), SizeBytes });
	}

	Directories.Sort([](const FSessionDirectoryInfo& A, const FSessionDirectoryInfo& B)
	{
		return A.Timestamp < B.Timestamp;
	});

	for (const FSessionDirectoryInfo& Directory : Directories)
	{
		if (TotalBytes <= BudgetBytes)
		{
			break;
		}

		if (Directory.Path == SessionDir)
		{
			continue;
		}

		if (IFileManager::Get().DeleteDirectory(*Directory.Path, false, true))
		{
			TotalBytes -= Directory.SizeBytes;
		}
	}
}

int64 UT66PerformanceSubsystem::GetDirectorySizeBytes(const FString& Directory) const
{
	class FDirectorySizeVisitor final : public IPlatformFile::FDirectoryVisitor
	{
	public:
		virtual bool Visit(const TCHAR* FilenameOrDirectory, const bool bIsDirectory) override
		{
			if (!bIsDirectory)
			{
				TotalSizeBytes += FPlatformFileManager::Get().GetPlatformFile().FileSize(FilenameOrDirectory);
			}
			return true;
		}

		int64 TotalSizeBytes = 0;
	};

	FDirectorySizeVisitor Visitor;
	FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*Directory, Visitor);
	return Visitor.TotalSizeBytes;
}

bool UT66PerformanceSubsystem::ShouldIncludeHardwareFingerprint() const
{
	if (!Settings)
	{
		return false;
	}

#if UE_BUILD_SHIPPING
	return Settings->bIncludeHardwareFingerprintInShipping;
#else
	return Settings->bIncludeHardwareFingerprintInDevelopment;
#endif
}

FString UT66PerformanceSubsystem::GetBuildConfigString() const
{
#if UE_BUILD_SHIPPING
	return TEXT("Shipping");
#elif UE_BUILD_TEST
	return TEXT("Test");
#elif UE_BUILD_DEBUG
	return TEXT("Debug");
#elif UE_BUILD_DEVELOPMENT
	return TEXT("Development");
#else
	return TEXT("Unknown");
#endif
}

FString UT66PerformanceSubsystem::GetProtonStatusString() const
{
	const FString SteamDeck = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamDeck"));
	const FString SteamOs = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamOS"));
	const FString WinePrefix = FPlatformMisc::GetEnvironmentVariable(TEXT("WINEPREFIX"));
	const FString ProtonLog = FPlatformMisc::GetEnvironmentVariable(TEXT("PROTON_LOG"));
	const FString SteamCompatPath = FPlatformMisc::GetEnvironmentVariable(TEXT("STEAM_COMPAT_CLIENT_INSTALL_PATH"));

	if (!SteamDeck.IsEmpty() || !SteamOs.IsEmpty())
	{
		return TEXT("Detected");
	}

	if (!WinePrefix.IsEmpty() || !ProtonLog.IsEmpty() || !SteamCompatPath.IsEmpty())
	{
		return TEXT("Likely");
	}

	return TEXT("NotDetected");
}

FString UT66PerformanceSubsystem::GetWorldNameForReports() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetName() : TEXT("Unavailable");
}

FString UT66PerformanceSubsystem::GetMapNameForReports() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetMapName() : TEXT("Unavailable");
}

FString UT66PerformanceSubsystem::SanitizeForReport(FString Value) const
{
	const FString UserDir = FPlatformProcess::UserDir();
	if (!UserDir.IsEmpty())
	{
		Value.ReplaceInline(*UserDir, TEXT("%USERPROFILE%"), ESearchCase::IgnoreCase);
	}

	const FString UserSettingsDir = FPlatformProcess::UserSettingsDir();
	if (!UserSettingsDir.IsEmpty())
	{
		Value.ReplaceInline(*UserSettingsDir, TEXT("%USERPROFILE%"), ESearchCase::IgnoreCase);
	}

	const FString UserProfile = FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"));
	if (!UserProfile.IsEmpty())
	{
		Value.ReplaceInline(*UserProfile, TEXT("%USERPROFILE%"), ESearchCase::IgnoreCase);
	}

	return Value;
}
