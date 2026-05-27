// Copyright Tribulation 66. All Rights Reserved.

#include "PerformanceSystem/T66PerformanceSubsystem.h"

#include "PerformanceSystem/T66PerformanceSystemSettings.h"

#include "Core/T66LagTrackerSubsystem.h"
#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/T66UniqueDebuffProjectile.h"
#include "Gameplay/Traps/T66TrapArrowProjectile.h"
#include "Containers/Queue.h"
#include "Dom/JsonObject.h"
#include "HAL/Event.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "HAL/ThreadSafeCounter.h"
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
constexpr int32 T66PerformanceSchemaVersion = 8;
constexpr int32 T66PerformanceMaxRecentEvents = 100;
constexpr int32 T66PerformanceWriteQueueCapacity = 4096;
constexpr double T66PerformanceWriteQueueShutdownTimeoutSeconds = 10.0;
constexpr double BytesToMegabytes = 1.0 / (1024.0 * 1024.0);

static TAutoConsoleVariable<int32> CVarT66PerfSubstepAttributionEnabled(
	TEXT("T66.Performance.Diagnostics.SubstepAttribution"),
	0,
	TEXT("Non-shipping diagnostic: when 1, records PerformanceSystem TickPerformanceSystem substep timing for B.10.1 overhead attribution. Default 0."),
#if UE_BUILD_SHIPPING
	ECVF_ReadOnly
#else
	ECVF_Default
#endif
);

bool IsPerfSubstepAttributionEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarT66PerfSubstepAttributionEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

double MeasureSubstepUs(TFunctionRef<void()> Work)
{
	const double StartSeconds = FPlatformTime::Seconds();
	Work();
	return (FPlatformTime::Seconds() - StartSeconds) * 1000000.0;
}

int32 GetPerformanceWriteQueueCapacityForRun()
{
#if !UE_BUILD_SHIPPING
	int32 TestCapacity = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66PerfWriteQueueTestCapacity="), TestCapacity) && TestCapacity > 0)
	{
		return FMath::Clamp(TestCapacity, 1, T66PerformanceWriteQueueCapacity);
	}
#endif
	return T66PerformanceWriteQueueCapacity;
}

double GetPerformanceWriteQueueTestDelayMs()
{
#if !UE_BUILD_SHIPPING
	double TestDelayMs = 0.0;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66PerfWriteQueueTestDelayMs="), TestDelayMs))
	{
		return FMath::Clamp(TestDelayMs, 0.0, 1000.0);
	}
#endif
	return 0.0;
}

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

class UT66PerformanceSubsystem::FPerformanceWriteWorker final : public FRunnable
{
public:
	enum class EWriteOperation : uint8
	{
		Append,
		AtomicReplace
	};

	FPerformanceWriteWorker(const int32 InCapacity, const double InTestDelayMs)
		: Capacity(FMath::Max(1, InCapacity))
		, TestDelayMs(FMath::Max(0.0, InTestDelayMs))
	{
		Stats.Capacity = Capacity;
		WorkEvent = FPlatformProcess::GetSynchEventFromPool(false);
		FlushEvent = FPlatformProcess::GetSynchEventFromPool(true);
		if (FlushEvent)
		{
			FlushEvent->Trigger();
		}
		Thread = FRunnableThread::Create(this, TEXT("T66PerfWriteWorker"));
		if (Thread)
		{
			bWorkerRunning = true;
			UpdateSnapshotState();
		}
	}

	virtual ~FPerformanceWriteWorker() override
	{
		StopAndJoin(T66PerformanceWriteQueueShutdownTimeoutSeconds);
		if (WorkEvent)
		{
			FPlatformProcess::ReturnSynchEventToPool(WorkEvent);
			WorkEvent = nullptr;
		}
		if (FlushEvent)
		{
			FPlatformProcess::ReturnSynchEventToPool(FlushEvent);
			FlushEvent = nullptr;
		}
	}

	virtual uint32 Run() override
	{
		while (true)
		{
			FQueuedWriteCommand Command;
			bool bDidWork = false;
			while (Queue.Dequeue(Command))
			{
				ActiveWrites.Increment();
				CurrentQueueDepth.Decrement();
				bDidWork = true;
				if (TestDelayMs > 0.0)
				{
					FPlatformProcess::Sleep(TestDelayMs / 1000.0);
				}
				WriteCommand(Command, false, true);
			}

			if (IsIdle())
			{
				if (FlushEvent)
				{
					FlushEvent->Trigger();
				}
				UpdateSnapshotState();
			}

			if (bStopRequested && IsIdle())
			{
				break;
			}

			if (!bDidWork && WorkEvent)
			{
				WorkEvent->Wait(10);
			}
		}

		bWorkerRunning = false;
		UpdateSnapshotState();
		if (FlushEvent)
		{
			FlushEvent->Trigger();
		}
		return 0;
	}

	virtual void Stop() override
	{
		bStopRequested = true;
		if (WorkEvent)
		{
			WorkEvent->Trigger();
		}
	}

	void BeginClosing()
	{
		bClosing = true;
		UpdateSnapshotState();
	}

	void SubmitAppend(const FString& TargetPath, const FString& Payload, const TCHAR* StreamName)
	{
		SubmitWrite(EWriteOperation::Append, TargetPath, Payload, StreamName);
	}

	void SubmitAtomicReplace(const FString& TargetPath, const FString& Payload, const TCHAR* StreamName)
	{
		SubmitWrite(EWriteOperation::AtomicReplace, TargetPath, Payload, StreamName);
	}

	bool FlushAndWait(const double TimeoutSeconds, const TCHAR* Reason)
	{
		(void)Reason;
		const double StartSeconds = FPlatformTime::Seconds();
		if (IsIdle())
		{
			RecordFlushWait(StartSeconds);
			return true;
		}

		const bool bBounded = TimeoutSeconds >= 0.0;
		while (!IsIdle())
		{
			if (!bWorkerRunning && CurrentQueueDepth.GetValue() > 0)
			{
				RecordFlushWait(StartSeconds);
				return false;
			}

			uint32 WaitMilliseconds = 10;
			if (bBounded)
			{
				const double ElapsedSeconds = FPlatformTime::Seconds() - StartSeconds;
				if (ElapsedSeconds >= TimeoutSeconds)
				{
					RecordFlushWait(StartSeconds);
					return false;
				}
				WaitMilliseconds = FMath::Max(1u, static_cast<uint32>(FMath::Min(10.0, (TimeoutSeconds - ElapsedSeconds) * 1000.0)));
			}

			if (FlushEvent)
			{
				FlushEvent->Wait(WaitMilliseconds);
			}
			else
			{
				FPlatformProcess::Sleep(static_cast<float>(WaitMilliseconds) / 1000.0f);
			}
		}

		RecordFlushWait(StartSeconds);
		return true;
	}

	bool StopAndJoin(const double TimeoutSeconds)
	{
		if (!Thread)
		{
			return true;
		}

		BeginClosing();
		const double StartSeconds = FPlatformTime::Seconds();
		const bool bFlushed = FlushAndWait(TimeoutSeconds, TEXT("Shutdown"));
		{
			FScopeLock Lock(&StatsCriticalSection);
			Stats.ShutdownFlushWaitMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
			if (!bFlushed)
			{
				Stats.AbandonedWrites += static_cast<uint64>(FMath::Max(0, CurrentQueueDepth.GetValue()));
			}
		}

		if (!bFlushed)
		{
			Stop();
			Thread->Kill(false);
			delete Thread;
			Thread = nullptr;
			bWorkerRunning = false;
			UpdateSnapshotState();
			return false;
		}

		Stop();
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
		bWorkerRunning = false;
		UpdateSnapshotState();
		return true;
	}

	FPerformanceWriteQueueStats GetStats() const
	{
		FScopeLock Lock(&StatsCriticalSection);
		FPerformanceWriteQueueStats Copy = Stats;
		Copy.CurrentQueueDepth = CurrentQueueDepth.GetValue();
		Copy.bWorkerRunning = bWorkerRunning;
		Copy.bClosing = bClosing;
		return Copy;
	}

private:
	struct FQueuedWriteCommand
	{
		EWriteOperation Operation = EWriteOperation::Append;
		FString TargetPath;
		FString Payload;
		FString StreamName;
		uint64 Sequence = 0;
	};

	void SubmitWrite(const EWriteOperation Operation, const FString& TargetPath, const FString& Payload, const TCHAR* StreamName)
	{
		FQueuedWriteCommand Command;
		Command.Operation = Operation;
		Command.TargetPath = TargetPath;
		Command.Payload = Payload;
		Command.StreamName = StreamName ? FString(StreamName) : FString(TEXT("Unknown"));
		Command.Sequence = ++NextSequence;

		RecordAttempt(Operation);

		if (!Thread || !bWorkerRunning)
		{
			RecordFallback(EFallbackKind::WorkerUnavailable);
			WriteCommand(Command, true);
			return;
		}

		if (bClosing)
		{
			RecordFallback(EFallbackKind::Closing);
			FlushAndWait(-1.0, TEXT("ClosingFallback"));
			WriteCommand(Command, true);
			return;
		}

		if (CurrentQueueDepth.GetValue() >= Capacity)
		{
			RecordFallback(EFallbackKind::QueueFull);
			FlushAndWait(-1.0, TEXT("QueueFullFallback"));
			WriteCommand(Command, true);
			return;
		}

		if (FlushEvent)
		{
			FlushEvent->Reset();
		}
		Queue.Enqueue(MoveTemp(Command));
		const int32 NewDepth = CurrentQueueDepth.Increment();
		{
			FScopeLock Lock(&StatsCriticalSection);
			++Stats.QueuedWrites;
			Stats.MaxQueueDepth = FMath::Max<uint64>(Stats.MaxQueueDepth, static_cast<uint64>(FMath::Max(0, NewDepth)));
			Stats.CurrentQueueDepth = NewDepth;
		}
		if (WorkEvent)
		{
			WorkEvent->Trigger();
		}
	}

	enum class EFallbackKind : uint8
	{
		QueueFull,
		Closing,
		WorkerUnavailable
	};

	void RecordAttempt(const EWriteOperation Operation)
	{
		FScopeLock Lock(&StatsCriticalSection);
		++Stats.AttemptedWrites;
		if (Operation == EWriteOperation::Append)
		{
			++Stats.AppendWrites;
		}
		else
		{
			++Stats.AtomicReplaceWrites;
		}
	}

	void RecordFallback(const EFallbackKind Kind)
	{
		FScopeLock Lock(&StatsCriticalSection);
		++Stats.FallbackWrites;
		switch (Kind)
		{
		case EFallbackKind::QueueFull:
			++Stats.QueueFullFallbackWrites;
			break;
		case EFallbackKind::Closing:
			++Stats.ClosingFallbackWrites;
			break;
		case EFallbackKind::WorkerUnavailable:
			++Stats.WorkerUnavailableFallbackWrites;
			break;
		}
	}

	bool WriteCommand(const FQueuedWriteCommand& Command, const bool bFallback, const bool bActiveWriteAlreadyCounted = false)
	{
		if (!bActiveWriteAlreadyCounted)
		{
			ActiveWrites.Increment();
		}
		const double StartSeconds = FPlatformTime::Seconds();
		bool bSuccess = false;
		{
			FScopeLock WriteLock(&FileWriteCriticalSection);
			switch (Command.Operation)
			{
			case EWriteOperation::Append:
				IFileManager::Get().MakeDirectory(*FPaths::GetPath(Command.TargetPath), true);
				bSuccess = FFileHelper::SaveStringToFile(
					Command.Payload,
					*Command.TargetPath,
					FFileHelper::EEncodingOptions::AutoDetect,
					&IFileManager::Get(),
					FILEWRITE_Append);
				break;
			case EWriteOperation::AtomicReplace:
				bSuccess = SaveStringAtomic(Command.TargetPath, Command.Payload);
				break;
			}
		}

		const double CostUs = (FPlatformTime::Seconds() - StartSeconds) * 1000000.0;
		ActiveWrites.Decrement();
		{
			FScopeLock Lock(&StatsCriticalSection);
			if (bSuccess)
			{
				++Stats.CompletedWrites;
			}
			else
			{
				++Stats.FailedWrites;
			}

			if (bFallback)
			{
				Stats.LastFallbackWriteUs = CostUs;
				Stats.FallbackWritePeakUs = FMath::Max(Stats.FallbackWritePeakUs, CostUs);
			}
			else
			{
				Stats.LastWorkerWriteUs = CostUs;
				Stats.WorkerWritePeakUs = FMath::Max(Stats.WorkerWritePeakUs, CostUs);
			}
			Stats.CurrentQueueDepth = CurrentQueueDepth.GetValue();
		}

		if (IsIdle() && FlushEvent)
		{
			FlushEvent->Trigger();
		}
		return bSuccess;
	}

	bool IsIdle() const
	{
		return CurrentQueueDepth.GetValue() <= 0 && ActiveWrites.GetValue() <= 0;
	}

	void RecordFlushWait(const double StartSeconds)
	{
		FScopeLock Lock(&StatsCriticalSection);
		Stats.LastFlushWaitMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
		Stats.CurrentQueueDepth = CurrentQueueDepth.GetValue();
	}

	void UpdateSnapshotState()
	{
		FScopeLock Lock(&StatsCriticalSection);
		Stats.CurrentQueueDepth = CurrentQueueDepth.GetValue();
		Stats.bWorkerRunning = bWorkerRunning;
		Stats.bClosing = bClosing;
	}

	const int32 Capacity = T66PerformanceWriteQueueCapacity;
	const double TestDelayMs = 0.0;
	TQueue<FQueuedWriteCommand, EQueueMode::Spsc> Queue;
	FThreadSafeCounter CurrentQueueDepth;
	FThreadSafeCounter ActiveWrites;
	FThreadSafeBool bStopRequested = false;
	FThreadSafeBool bWorkerRunning = false;
	FThreadSafeBool bClosing = false;
	FEvent* WorkEvent = nullptr;
	FEvent* FlushEvent = nullptr;
	FRunnableThread* Thread = nullptr;
	uint64 NextSequence = 0;
	mutable FCriticalSection StatsCriticalSection;
	FCriticalSection FileWriteCriticalSection;
	FPerformanceWriteQueueStats Stats;
};

void UT66PerformanceSubsystem::FPerformanceLogOutputDevice::Serialize(
	const TCHAR* V,
	ELogVerbosity::Type Verbosity,
	const FName& Category)
{
	Owner.CaptureLogLine(Category.ToString(), Verbosity, V ? FString(V) : FString());
}

void UT66PerformanceSubsystem::StartPerformanceWriteWorker()
{
	if (WriteWorker)
	{
		return;
	}

	WriteWorker = new FPerformanceWriteWorker(GetPerformanceWriteQueueCapacityForRun(), GetPerformanceWriteQueueTestDelayMs());
}

void UT66PerformanceSubsystem::BeginPerformanceWriteShutdown()
{
	if (WriteWorker)
	{
		WriteWorker->BeginClosing();
	}
}

void UT66PerformanceSubsystem::StopPerformanceWriteWorker()
{
	if (WriteWorker)
	{
		WriteWorker->StopAndJoin(T66PerformanceWriteQueueShutdownTimeoutSeconds);
		delete WriteWorker;
		WriteWorker = nullptr;
	}
}

bool UT66PerformanceSubsystem::FlushPerformanceWrites(const double TimeoutSeconds, const TCHAR* Reason)
{
	return !WriteWorker || WriteWorker->FlushAndWait(TimeoutSeconds, Reason);
}

bool UT66PerformanceSubsystem::EnsurePerformanceProducerGameThread(const TCHAR* FunctionName) const
{
#if !UE_BUILD_SHIPPING
	if (!IsInGameThread())
	{
		ensureMsgf(false, TEXT("UT66PerformanceSubsystem::%s must run on the game thread for the B.10.1B write queue producer contract."), FunctionName ? FunctionName : TEXT("Unknown"));
		return false;
	}
#endif
	return true;
}

void UT66PerformanceSubsystem::QueuePerformanceAppend(const FString& TargetPath, const FString& Payload, const TCHAR* StreamName)
{
	if (WriteWorker)
	{
		WriteWorker->SubmitAppend(TargetPath, Payload, StreamName);
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(TargetPath), true);
	FFileHelper::SaveStringToFile(
		Payload,
		*TargetPath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(),
		FILEWRITE_Append);
}

void UT66PerformanceSubsystem::QueuePerformanceAtomicReplace(const FString& TargetPath, const FString& Payload, const TCHAR* StreamName)
{
	if (WriteWorker)
	{
		WriteWorker->SubmitAtomicReplace(TargetPath, Payload, StreamName);
		return;
	}

	SaveStringAtomic(TargetPath, Payload);
}

void UT66PerformanceSubsystem::RunWriteQueueOrderingSelfTest()
{
#if !UE_BUILD_SHIPPING
	if (!FParse::Param(FCommandLine::Get(), TEXT("T66PerfWriteQueueOrderingSelfTest")))
	{
		return;
	}

	for (int32 Index = 0; Index < 8; ++Index)
	{
		EmitPerformanceEvent(
			TEXT("PerformanceSystem"),
			TEXT("WriteQueueOrderingSelfTest"),
			ET66PerformanceSeverity::Info,
			ET66PerformanceConfidence::Exact,
			FString::Printf(TEXT("Write queue ordering self-test event %d."), Index),
			{
				{ TEXT("SelfTestIndex"), FString::FromInt(Index), ET66PerformanceConfidence::Exact, TEXT("T66PerfWriteQueueOrderingSelfTest"), 0.0 }
			});
	}
	FlushPerformanceWrites(T66PerformanceWriteQueueShutdownTimeoutSeconds, TEXT("WriteQueueOrderingSelfTest"));
#endif
}

void UT66PerformanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Settings = GetDefault<UT66PerformanceSystemSettings>();
	if (!Settings || !Settings->bEnablePerformanceSystem)
	{
		return;
	}

#if !UE_BUILD_SHIPPING
	int32 SubstepAttributionOverride = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66PerfSubstepAttribution="), SubstepAttributionOverride))
	{
		if (IConsoleVariable* SubstepAttributionCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Performance.Diagnostics.SubstepAttribution")))
		{
			SubstepAttributionCVar->Set(SubstepAttributionOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
		}
	}
#endif

	SessionStartedUtc = FDateTime::UtcNow();
	SessionId = FString::Printf(
		TEXT("%s_%s"),
		*SessionStartedUtc.ToString(TEXT("%Y%m%dT%H%M%SZ")),
		*FGuid::NewGuid().ToString(EGuidFormats::Short));

	PerformanceRootDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("PerformanceSystem"));
	SessionsRootDir = FPaths::Combine(PerformanceRootDir, TEXT("Sessions"));
	SessionDir = FPaths::Combine(SessionsRootDir, SessionId);
	EventsJsonlPath = FPaths::Combine(SessionDir, TEXT("events.jsonl"));
	BoardSaturationSamplesJsonlPath = FPaths::Combine(SessionDir, TEXT("board_saturation_samples.jsonl"));
	SnapshotCurrentPath = FPaths::Combine(PerformanceRootDir, TEXT("snapshot.current.json"));
	SnapshotPreviousPath = FPaths::Combine(PerformanceRootDir, TEXT("snapshot.previous.json"));

	IFileManager::Get().MakeDirectory(*SessionDir, true);
	StartPerformanceWriteWorker();

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
	RunWriteQueueOrderingSelfTest();
	EnforceRetentionBudget();
}

void UT66PerformanceSubsystem::Deinitialize()
{
	if (bInitialized)
	{
		BeginPerformanceWriteShutdown();
		FlushPerformanceWrites(T66PerformanceWriteQueueShutdownTimeoutSeconds, TEXT("PreFinalReport"));
		WriteFinalReport(TEXT("SubsystemDeinitialize"));
		StopPerformanceWriteWorker();
	}
	else
	{
		StopPerformanceWriteWorker();
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
	if (!EnsurePerformanceProducerGameThread(TEXT("RecordMeasuredOperation")))
	{
		return;
	}

	if (!bInitialized || !Settings || DurationMs < Settings->ProjectOperationWarningMs)
	{
		return;
	}

	TArray<FT66PerformanceAttribution> Attributions = {
		{ TEXT("OperationName"), OperationName, ET66PerformanceConfidence::Exact, Source, 0.0 },
		{ TEXT("DurationMs"), FString::Printf(TEXT("%.3f"), DurationMs), ET66PerformanceConfidence::Sampled, Source, 0.0 }
	};
	AppendBoardSaturationAttributions(Attributions);

	EmitPerformanceEvent(
		TEXT("ProjectOperationStallDetector"),
		TEXT("ProjectOperationStall"),
		ET66PerformanceSeverity::Warning,
		ET66PerformanceConfidence::Sampled,
		FString::Printf(TEXT("%s took %.2f ms."), *OperationName, DurationMs),
		Attributions);
}

bool UT66PerformanceSubsystem::TickPerformanceSystem(const float DeltaSeconds)
{
	if (!Settings || !Settings->bEnablePerformanceSystem)
	{
		return true;
	}

	const double TickStartSeconds = FPlatformTime::Seconds();
	const double NowSeconds = TickStartSeconds;
	const bool bSubstepAttributionEnabled = IsPerfSubstepAttributionEnabled();

	double FrameSampleAppendUs = 0.0;
	double BoardSampleCaptureUs = 0.0;
	double PruneSamplesUs = 0.0;
	double SingleFrameHitchUs = 0.0;
	double FramePacingDetectorUs = 0.0;
	double MemoryGrowthDetectorUs = 0.0;
	double BasicHangDetectorUs = 0.0;
	double PeriodicSnapshotUs = 0.0;

	if (bSubstepAttributionEnabled)
	{
		FrameSampleAppendUs = MeasureSubstepUs([this, NowSeconds, DeltaSeconds]()
		{
			FrameSamples.Add({ NowSeconds, static_cast<double>(DeltaSeconds) * 1000.0 });
		});
		BoardSampleCaptureUs = MeasureSubstepUs([this, NowSeconds, DeltaSeconds]()
		{
			CaptureBoardSaturationFrameSample(NowSeconds, static_cast<double>(DeltaSeconds) * 1000.0);
		});
		PruneSamplesUs = MeasureSubstepUs([this, NowSeconds]()
		{
			PruneRollingSamples(NowSeconds);
		});
		SingleFrameHitchUs = MeasureSubstepUs([this, DeltaSeconds]()
		{
			CheckSingleFrameHitch(DeltaSeconds);
		});
		FramePacingDetectorUs = MeasureSubstepUs([this, DeltaSeconds]()
		{
			RunDetector(TEXT("FramePacingDetector"), Settings->DetectorBudgetUs, 1.0, [this, DeltaSeconds]()
			{
				CheckFrameDetectors(DeltaSeconds);
			});
		});
		MemoryGrowthDetectorUs = MeasureSubstepUs([this]()
		{
			RunDetector(TEXT("MemoryGrowthDetector"), Settings->DetectorBudgetUs, 1.0, [this]()
			{
				CheckMemoryDetector();
			});
		});
		BasicHangDetectorUs = MeasureSubstepUs([this, DeltaSeconds]()
		{
			RunDetector(TEXT("BasicHangDetector"), Settings->DetectorBudgetUs, 0.0, [this, DeltaSeconds]()
			{
				CheckBasicHangDetector(DeltaSeconds);
			});
		});
		PeriodicSnapshotUs = MeasureSubstepUs([this]()
		{
			WritePeriodicSnapshot(false);
		});
	}
	else
	{
		FrameSamples.Add({ NowSeconds, static_cast<double>(DeltaSeconds) * 1000.0 });
		CaptureBoardSaturationFrameSample(NowSeconds, static_cast<double>(DeltaSeconds) * 1000.0);
		PruneRollingSamples(NowSeconds);
		CheckSingleFrameHitch(DeltaSeconds);

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
	}

	const double FrameworkCostUs = (FPlatformTime::Seconds() - TickStartSeconds) * 1000000.0;
	double InstrumentationProbeCostUs = 0.0;
	if (bSubstepAttributionEnabled)
	{
		const double MeasuredSubstepUs =
			FrameSampleAppendUs
			+ BoardSampleCaptureUs
			+ PruneSamplesUs
			+ SingleFrameHitchUs
			+ FramePacingDetectorUs
			+ MemoryGrowthDetectorUs
			+ BasicHangDetectorUs
			+ PeriodicSnapshotUs;
		InstrumentationProbeCostUs = FMath::Max(0.0, FrameworkCostUs - MeasuredSubstepUs);
		++FrameworkSubstepTimingStats.SampleCount;
		FrameworkSubstepTimingStats.LastFrameSampleAppendUs = FrameSampleAppendUs;
		FrameworkSubstepTimingStats.LastBoardSampleCaptureUs = BoardSampleCaptureUs;
		FrameworkSubstepTimingStats.LastPruneSamplesUs = PruneSamplesUs;
		FrameworkSubstepTimingStats.LastSingleFrameHitchUs = SingleFrameHitchUs;
		FrameworkSubstepTimingStats.LastFramePacingDetectorUs = FramePacingDetectorUs;
		FrameworkSubstepTimingStats.LastMemoryGrowthDetectorUs = MemoryGrowthDetectorUs;
		FrameworkSubstepTimingStats.LastBasicHangDetectorUs = BasicHangDetectorUs;
		FrameworkSubstepTimingStats.LastPeriodicSnapshotUs = PeriodicSnapshotUs;
		FrameworkSubstepTimingStats.LastFrameworkTotalUs = FrameworkCostUs;
		FrameworkSubstepTimingStats.LastInstrumentationProbeUs = InstrumentationProbeCostUs;
		FrameworkSubstepTimingStats.FrameSampleAppendPeakUs = FMath::Max(FrameworkSubstepTimingStats.FrameSampleAppendPeakUs, FrameSampleAppendUs);
		FrameworkSubstepTimingStats.BoardSampleCapturePeakUs = FMath::Max(FrameworkSubstepTimingStats.BoardSampleCapturePeakUs, BoardSampleCaptureUs);
		FrameworkSubstepTimingStats.PruneSamplesPeakUs = FMath::Max(FrameworkSubstepTimingStats.PruneSamplesPeakUs, PruneSamplesUs);
		FrameworkSubstepTimingStats.SingleFrameHitchPeakUs = FMath::Max(FrameworkSubstepTimingStats.SingleFrameHitchPeakUs, SingleFrameHitchUs);
		FrameworkSubstepTimingStats.FramePacingDetectorPeakUs = FMath::Max(FrameworkSubstepTimingStats.FramePacingDetectorPeakUs, FramePacingDetectorUs);
		FrameworkSubstepTimingStats.MemoryGrowthDetectorPeakUs = FMath::Max(FrameworkSubstepTimingStats.MemoryGrowthDetectorPeakUs, MemoryGrowthDetectorUs);
		FrameworkSubstepTimingStats.BasicHangDetectorPeakUs = FMath::Max(FrameworkSubstepTimingStats.BasicHangDetectorPeakUs, BasicHangDetectorUs);
		FrameworkSubstepTimingStats.PeriodicSnapshotPeakUs = FMath::Max(FrameworkSubstepTimingStats.PeriodicSnapshotPeakUs, PeriodicSnapshotUs);
		FrameworkSubstepTimingStats.FrameworkTotalPeakUs = FMath::Max(FrameworkSubstepTimingStats.FrameworkTotalPeakUs, FrameworkCostUs);
		FrameworkSubstepTimingStats.InstrumentationProbePeakUs = FMath::Max(FrameworkSubstepTimingStats.InstrumentationProbePeakUs, InstrumentationProbeCostUs);
	}
	if (FrameworkCostUs > Settings->FrameworkFrameBudgetUs
		&& NowSeconds - LastFrameworkBudgetEventSeconds > Settings->FrameDetectorCooldownSeconds)
	{
		LastFrameworkBudgetEventSeconds = NowSeconds;
		TArray<FT66PerformanceAttribution> Attributions = {
			{ TEXT("FrameworkCostUs"), FString::Printf(TEXT("%.3f"), FrameworkCostUs), ET66PerformanceConfidence::Exact, TEXT("FPlatformTime"), 0.0 },
			{ TEXT("BudgetUs"), FString::Printf(TEXT("%.3f"), Settings->FrameworkFrameBudgetUs), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 }
		};
		if (bSubstepAttributionEnabled)
		{
			Attributions.Append({
				{ TEXT("FrameSampleAppendUs"), FString::Printf(TEXT("%.3f"), FrameSampleAppendUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("BoardSampleCaptureUs"), FString::Printf(TEXT("%.3f"), BoardSampleCaptureUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("PruneSamplesUs"), FString::Printf(TEXT("%.3f"), PruneSamplesUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("SingleFrameHitchCheckUs"), FString::Printf(TEXT("%.3f"), SingleFrameHitchUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("FramePacingDetectorUs"), FString::Printf(TEXT("%.3f"), FramePacingDetectorUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("MemoryGrowthDetectorUs"), FString::Printf(TEXT("%.3f"), MemoryGrowthDetectorUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("BasicHangDetectorUs"), FString::Printf(TEXT("%.3f"), BasicHangDetectorUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("PeriodicSnapshotUs"), FString::Printf(TEXT("%.3f"), PeriodicSnapshotUs), ET66PerformanceConfidence::Sampled, TEXT("TickPerformanceSystem"), 0.0 },
				{ TEXT("InstrumentationProbeCostUs"), FString::Printf(TEXT("%.3f"), InstrumentationProbeCostUs), ET66PerformanceConfidence::Inferred, TEXT("TickPerformanceSystem"), 0.0 }
			});
		}
		EmitPerformanceEvent(
			TEXT("PerformanceSystemOverhead"),
			TEXT("FrameworkBudgetExceeded"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Exact,
			FString::Printf(TEXT("PerformanceSystem frame cost was %.2f us."), FrameworkCostUs),
			Attributions);
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
		TArray<FT66PerformanceAttribution> LowFpsAttributions = {
			{ TEXT("AverageFps"), FString::Printf(TEXT("%.3f"), Summary.AverageFps), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds },
			{ TEXT("ThresholdFps"), FString::Printf(TEXT("%.3f"), Settings->SustainedLowFpsThreshold), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 },
			{ TEXT("OnePercentLowFps"), FString::Printf(TEXT("%.3f"), Summary.OnePercentLowFps), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds },
			{ TEXT("PointOnePercentLowFps"), FString::Printf(TEXT("%.3f"), Summary.PointOnePercentLowFps), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds }
		};
		AppendBoardSaturationAttributions(LowFpsAttributions);
		EmitPerformanceEvent(
			TEXT("FramePacingDetector"),
			TEXT("SustainedLowFps"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Sampled,
			FString::Printf(TEXT("Rolling FPS was %.2f over %.1f seconds."), Summary.AverageFps, Settings->SustainedLowFpsWindowSeconds),
			LowFpsAttributions);
	}

	if (Summary.StdDevMs >= Settings->StutterStdDevThresholdMs
		&& NowSeconds - LastStutterEventSeconds >= Settings->FrameDetectorCooldownSeconds)
	{
		LastStutterEventSeconds = NowSeconds;
		TArray<FT66PerformanceAttribution> StutterAttributions = {
			{ TEXT("StdDevMs"), FString::Printf(TEXT("%.3f"), Summary.StdDevMs), ET66PerformanceConfidence::Sampled, TEXT("FrameWindow"), Settings->SustainedLowFpsWindowSeconds },
			{ TEXT("ThresholdMs"), FString::Printf(TEXT("%.3f"), Settings->StutterStdDevThresholdMs), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 }
		};
		AppendBoardSaturationAttributions(StutterAttributions);
		EmitPerformanceEvent(
			TEXT("FramePacingDetector"),
			TEXT("FrameVarianceStutter"),
			ET66PerformanceSeverity::Warning,
			ET66PerformanceConfidence::Sampled,
			FString::Printf(TEXT("Frame-time standard deviation was %.2f ms over %.1f seconds."), Summary.StdDevMs, Settings->SustainedLowFpsWindowSeconds),
			StutterAttributions);
	}
}

void UT66PerformanceSubsystem::CheckSingleFrameHitch(const float DeltaSeconds)
{
	const double FrameMs = static_cast<double>(DeltaSeconds) * 1000.0;
	if (!Settings || FrameMs < Settings->HitchThresholdMs)
	{
		return;
	}

	TArray<FT66PerformanceAttribution> HitchAttributions;
	HitchAttributions.Reserve(6);
	HitchAttributions.Add({ TEXT("FrameTimeMs"), FString::Printf(TEXT("%.3f"), FrameMs), ET66PerformanceConfidence::Exact, TEXT("FTSTicker DeltaSeconds"), 0.0 });
	HitchAttributions.Add({ TEXT("ThresholdMs"), FString::Printf(TEXT("%.3f"), Settings->HitchThresholdMs), ET66PerformanceConfidence::Exact, TEXT("UT66PerformanceSystemSettings"), 0.0 });
	AppendBoardSaturationAttributions(HitchAttributions);
	EmitPerformanceEvent(
		TEXT("FramePacingDetector"),
		TEXT("SingleFrameHitch"),
		ET66PerformanceSeverity::Warning,
		ET66PerformanceConfidence::Exact,
		FString::Printf(TEXT("Frame took %.2f ms."), FrameMs),
		HitchAttributions);
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

void UT66PerformanceSubsystem::CaptureBoardSaturationFrameSample(const double NowSeconds, const double FrameMs)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FBoardSaturationFrameSample Sample;
	Sample.SessionTimeSeconds = (FDateTime::UtcNow() - SessionStartedUtc).GetTotalSeconds();
	Sample.GameTimeSeconds = World->GetTimeSeconds();
	Sample.FrameTimeMs = FrameMs;
	Sample.WorldName = SanitizeForReport(World->GetName());
	Sample.MapName = SanitizeForReport(World->GetMapName());

	UGameInstance* GameInstance = GetGameInstance();
	const UT66LagTrackerSubsystem* LagTracker = GameInstance ? GameInstance->GetSubsystem<UT66LagTrackerSubsystem>() : nullptr;
	FT66LagTrackerBoardSaturationSample BoardSample;
	if (LagTracker && LagTracker->GetLatestBoardSaturationSample(BoardSample))
	{
		Sample.LiveRegularEnemies = BoardSample.LiveRegularEnemies;
		Sample.LiveRichEnemies = BoardSample.LiveRichEnemies;
		Sample.LiveLightweightMobs = BoardSample.LiveLightweightMobs;
		Sample.LiveLightweightMeleeMobs = BoardSample.LiveLightweightMeleeMobs;
		Sample.LiveLightweightRushMobs = BoardSample.LiveLightweightRushMobs;
		Sample.LiveLightweightFlyingMobs = BoardSample.LiveLightweightFlyingMobs;
		Sample.LiveLightweightRangedMobs = BoardSample.LiveLightweightRangedMobs;
		Sample.PendingSpawns = BoardSample.PendingSpawns;
		Sample.ActiveEnemyProjectiles = BoardSample.ActiveEnemyProjectiles;
		Sample.LightweightPoolReuseAcquires = BoardSample.LightweightPoolReuseAcquires;
		Sample.LightweightPoolReleases = BoardSample.LightweightPoolReleases;
		Sample.LightweightPoolInactive = BoardSample.LightweightPoolInactive;
		Sample.LightweightPoolInactivePeak = BoardSample.LightweightPoolInactivePeak;
		Sample.BoardSaturationSampleAgeSeconds = FMath::Max(0.0, NowSeconds - BoardSample.TimestampSeconds);
		Sample.bBoardSaturationValid = true;
	}
	Sample.ActiveEnemyProjectiles =
		AT66EnemyProjectileBase::GetActiveEnemyProjectileCount()
		+ AT66UniqueDebuffProjectile::GetActiveEnemyProjectileCount()
		+ AT66TrapArrowProjectile::GetActiveTrapProjectileCount();

	BoardSaturationFrameSamples.Add(MoveTemp(Sample));
}

void UT66PerformanceSubsystem::WriteBoardSaturationFrameSamples() const
{
	if (BoardSaturationSamplesJsonlPath.IsEmpty() || BoardSaturationFrameSamples.Num() <= 0)
	{
		return;
	}

	FString Jsonl;
	Jsonl.Reserve(BoardSaturationFrameSamples.Num() * 560);
	for (const FBoardSaturationFrameSample& Sample : BoardSaturationFrameSamples)
	{
		const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("SchemaVersion"), T66PerformanceSchemaVersion);
		Object->SetNumberField(TEXT("SessionTimeSeconds"), Sample.SessionTimeSeconds);
		Object->SetNumberField(TEXT("GameTimeSeconds"), Sample.GameTimeSeconds);
		Object->SetNumberField(TEXT("FrameTimeMs"), Sample.FrameTimeMs);
		Object->SetBoolField(TEXT("BoardSaturationValid"), Sample.bBoardSaturationValid);
		Object->SetNumberField(TEXT("LiveRegularEnemies"), Sample.LiveRegularEnemies);
		Object->SetNumberField(TEXT("LiveRichEnemies"), Sample.LiveRichEnemies);
		Object->SetNumberField(TEXT("LiveLightweightMobs"), Sample.LiveLightweightMobs);
		Object->SetNumberField(TEXT("LiveLightweightMeleeMobs"), Sample.LiveLightweightMeleeMobs);
		Object->SetNumberField(TEXT("LiveLightweightRushMobs"), Sample.LiveLightweightRushMobs);
		Object->SetNumberField(TEXT("LiveLightweightFlyingMobs"), Sample.LiveLightweightFlyingMobs);
		Object->SetNumberField(TEXT("LiveLightweightRangedMobs"), Sample.LiveLightweightRangedMobs);
		Object->SetNumberField(TEXT("PendingSpawns"), Sample.PendingSpawns);
		Object->SetNumberField(TEXT("ActiveEnemyProjectiles"), Sample.ActiveEnemyProjectiles);
		Object->SetNumberField(TEXT("LightweightPoolReuseAcquires"), Sample.LightweightPoolReuseAcquires);
		Object->SetNumberField(TEXT("LightweightPoolReleases"), Sample.LightweightPoolReleases);
		Object->SetNumberField(TEXT("LightweightPoolInactive"), Sample.LightweightPoolInactive);
		Object->SetNumberField(TEXT("LightweightPoolInactivePeak"), Sample.LightweightPoolInactivePeak);
		Object->SetNumberField(TEXT("BoardSaturationSampleAgeSeconds"), Sample.BoardSaturationSampleAgeSeconds);
		Object->SetStringField(TEXT("WorldName"), Sample.WorldName);
		Object->SetStringField(TEXT("MapName"), Sample.MapName);
		Jsonl += JsonObjectToString(Object, false);
		Jsonl += LINE_TERMINATOR;
	}

	SaveStringAtomic(BoardSaturationSamplesJsonlPath, Jsonl);
}

void UT66PerformanceSubsystem::WritePeriodicSnapshot(const bool bForce)
{
	if (!EnsurePerformanceProducerGameThread(TEXT("WritePeriodicSnapshot")))
	{
		return;
	}

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

	if (bForce)
	{
		FlushPerformanceWrites(T66PerformanceWriteQueueShutdownTimeoutSeconds, TEXT("ForcedSnapshot"));
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*SnapshotCurrentPath))
		{
			PlatformFile.DeleteFile(*SnapshotPreviousPath);
			PlatformFile.MoveFile(*SnapshotPreviousPath, *SnapshotCurrentPath);
		}

		const TSharedRef<FJsonObject> Snapshot = CreateSnapshotJson(TEXT("Running"), false, 50);
		SaveStringAtomic(SnapshotCurrentPath, JsonObjectToString(Snapshot, false));
		return;
	}

	const TSharedRef<FJsonObject> Snapshot = CreateRunningSnapshotJson();
	QueuePerformanceAtomicReplace(SnapshotCurrentPath, JsonObjectToString(Snapshot, false), TEXT("SnapshotCurrent"));
}

void UT66PerformanceSubsystem::WriteFinalReport(const FString& ExitReason)
{
	if (!Settings)
	{
		return;
	}

	FlushPerformanceWrites(T66PerformanceWriteQueueShutdownTimeoutSeconds, TEXT("FinalReport"));
	WriteBoardSaturationFrameSamples();

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

	const FPerformanceWriteQueueStats WriteStats = WriteWorker ? WriteWorker->GetStats() : FPerformanceWriteQueueStats{};
	Markdown += TEXT("\n## Performance Write Queue\n\n");
	Markdown += FString::Printf(TEXT("- Capacity: %d\n"), WriteStats.Capacity);
	Markdown += FString::Printf(TEXT("- Attempted writes: %llu\n"), WriteStats.AttemptedWrites);
	Markdown += FString::Printf(TEXT("- Queued writes: %llu\n"), WriteStats.QueuedWrites);
	Markdown += FString::Printf(TEXT("- Completed writes: %llu\n"), WriteStats.CompletedWrites);
	Markdown += FString::Printf(TEXT("- Failed writes: %llu\n"), WriteStats.FailedWrites);
	Markdown += FString::Printf(TEXT("- Fallback writes: %llu\n"), WriteStats.FallbackWrites);
	Markdown += FString::Printf(TEXT("- Queue-full fallback writes: %llu\n"), WriteStats.QueueFullFallbackWrites);
	Markdown += FString::Printf(TEXT("- Abandoned writes: %llu\n"), WriteStats.AbandonedWrites);
	Markdown += FString::Printf(TEXT("- Max queue depth: %llu\n"), WriteStats.MaxQueueDepth);
	Markdown += FString::Printf(TEXT("- Worker write peak: %.3f us\n"), WriteStats.WorkerWritePeakUs);
	Markdown += FString::Printf(TEXT("- Fallback write peak: %.3f us\n"), WriteStats.FallbackWritePeakUs);
	Markdown += FString::Printf(TEXT("- Shutdown flush wait: %.3f ms\n"), WriteStats.ShutdownFlushWaitMs);
	Markdown += FString::Printf(
		TEXT("- Accounting balanced: %s\n"),
		WriteStats.AttemptedWrites == WriteStats.CompletedWrites + WriteStats.FailedWrites + WriteStats.AbandonedWrites ? TEXT("true") : TEXT("false"));

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

void UT66PerformanceSubsystem::AppendBoardSaturationAttributions(TArray<FT66PerformanceAttribution>& Attributions) const
{
	UGameInstance* GameInstance = GetGameInstance();
	const UT66LagTrackerSubsystem* LagTracker = GameInstance ? GameInstance->GetSubsystem<UT66LagTrackerSubsystem>() : nullptr;

	FT66LagTrackerBoardSaturationSample Sample;
	if (!LagTracker || !LagTracker->GetLatestBoardSaturationSample(Sample))
	{
		Attributions.Add({ TEXT("LiveRegularEnemies"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LiveRichEnemies"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LiveLightweightMobs"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LiveLightweightMeleeMobs"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LiveLightweightRushMobs"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LiveLightweightFlyingMobs"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LiveLightweightRangedMobs"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("PendingSpawns"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("ActiveEnemyProjectiles"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LightweightPoolReuseAcquires"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LightweightPoolReleases"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LightweightPoolInactive"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		Attributions.Add({ TEXT("LightweightPoolInactivePeak"), TEXT("Unavailable"), ET66PerformanceConfidence::Unavailable, TEXT("UT66LagTrackerSubsystem"), 0.0 });
		return;
	}

	const double SampleAgeSeconds = FMath::Max(0.0, FPlatformTime::Seconds() - Sample.TimestampSeconds);
	Attributions.Add({ TEXT("LiveRegularEnemies"), FString::FromInt(Sample.LiveRegularEnemies), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LiveRichEnemies"), FString::FromInt(Sample.LiveRichEnemies), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LiveLightweightMobs"), FString::FromInt(Sample.LiveLightweightMobs), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LiveLightweightMeleeMobs"), FString::FromInt(Sample.LiveLightweightMeleeMobs), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LiveLightweightRushMobs"), FString::FromInt(Sample.LiveLightweightRushMobs), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LiveLightweightFlyingMobs"), FString::FromInt(Sample.LiveLightweightFlyingMobs), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LiveLightweightRangedMobs"), FString::FromInt(Sample.LiveLightweightRangedMobs), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("PendingSpawns"), FString::FromInt(Sample.PendingSpawns), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("ActiveEnemyProjectiles"), FString::FromInt(Sample.ActiveEnemyProjectiles), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LightweightPoolReuseAcquires"), FString::FromInt(Sample.LightweightPoolReuseAcquires), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LightweightPoolReleases"), FString::FromInt(Sample.LightweightPoolReleases), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LightweightPoolInactive"), FString::FromInt(Sample.LightweightPoolInactive), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("LightweightPoolInactivePeak"), FString::FromInt(Sample.LightweightPoolInactivePeak), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
	Attributions.Add({ TEXT("BoardSaturationSampleAgeSeconds"), FString::Printf(TEXT("%.3f"), SampleAgeSeconds), ET66PerformanceConfidence::Sampled, TEXT("UT66LagTrackerSubsystem"), 1.0 });
}

void UT66PerformanceSubsystem::EmitPerformanceEvent(
	const FString& DetectorName,
	const FString& EventName,
	const ET66PerformanceSeverity Severity,
	const ET66PerformanceConfidence Confidence,
	const FString& Summary,
	const TArray<FT66PerformanceAttribution>& Attributions)
{
	if (!EnsurePerformanceProducerGameThread(TEXT("EmitPerformanceEvent")))
	{
		return;
	}

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

	if (IsPerfSubstepAttributionEnabled())
	{
		TSharedPtr<FJsonObject> Event;
		const double EventJsonBuildUs = MeasureSubstepUs([this, &Event, &DetectorName, &EventName, Severity, Confidence, &Summary, &Attributions]()
		{
			Event = CreateBaseEventJson(DetectorName, EventName, Severity, Confidence, Summary, Attributions);
		});
		FString JsonLine;
		const double EventJsonSerializeUs = MeasureSubstepUs([&Event, &JsonLine]()
		{
			if (Event.IsValid())
			{
				JsonLine = JsonObjectToString(Event.ToSharedRef(), false) + LINE_TERMINATOR;
			}
		});
		const double EventJsonAppendUs = MeasureSubstepUs([this, &JsonLine]()
		{
			QueuePerformanceAppend(EventsJsonlPath, JsonLine, TEXT("EventsJsonl"));
		});
		FrameworkSubstepTimingStats.EventJsonBuildPeakUs = FMath::Max(FrameworkSubstepTimingStats.EventJsonBuildPeakUs, EventJsonBuildUs + EventJsonSerializeUs);
		FrameworkSubstepTimingStats.EventJsonAppendPeakUs = FMath::Max(FrameworkSubstepTimingStats.EventJsonAppendPeakUs, EventJsonAppendUs);
		return;
	}

	const TSharedRef<FJsonObject> Event = CreateBaseEventJson(DetectorName, EventName, Severity, Confidence, Summary, Attributions);
	const FString JsonLine = JsonObjectToString(Event, false) + LINE_TERMINATOR;
	QueuePerformanceAppend(EventsJsonlPath, JsonLine, TEXT("EventsJsonl"));
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

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateSnapshotJson(const FString& ExitReason, const bool bFullFrameSummary, const int32 RecentLogLimit) const
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
	Root->SetObjectField(TEXT("FrameSummary"), CreateFrameSummaryJson(CalculateFrameSummary(Settings ? Settings->FrameWindowSeconds : 60.0, bFullFrameSummary)));
	Root->SetObjectField(TEXT("MemorySummary"), CreateMemorySummaryJson());
	Root->SetObjectField(TEXT("FrameworkSubstepAttribution"), CreateFrameworkSubstepAttributionJson());
	Root->SetObjectField(TEXT("PerformanceWriteQueue"), CreateWriteQueueStatsJson());
	Root->SetArrayField(TEXT("RecentLogs"), CreateRecentLogsJson(RecentLogLimit));
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

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateRunningSnapshotJson() const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("SchemaVersion"), T66PerformanceSchemaVersion);
	Root->SetStringField(TEXT("SessionId"), SessionId);
	Root->SetStringField(TEXT("StartedUtc"), SessionStartedUtc.ToIso8601());
	Root->SetStringField(TEXT("UpdatedUtc"), FDateTime::UtcNow().ToIso8601());
	Root->SetStringField(TEXT("SnapshotKind"), TEXT("Running"));
	Root->SetStringField(TEXT("WorldName"), GetWorldNameForReports());
	Root->SetStringField(TEXT("MapName"), GetMapNameForReports());
	Root->SetObjectField(TEXT("FrameSummary"), CreateFrameSummaryJson(CalculateFrameSummary(Settings ? Settings->FrameWindowSeconds : 60.0, false)));
	Root->SetObjectField(TEXT("MemorySummary"), CreateMemorySummaryJson());
	Root->SetObjectField(TEXT("FrameworkSubstepAttribution"), CreateFrameworkSubstepAttributionJson());
	Root->SetObjectField(TEXT("PerformanceWriteQueue"), CreateWriteQueueStatsJson());

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

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateFrameworkSubstepAttributionJson() const
{
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetBoolField(TEXT("Enabled"), IsPerfSubstepAttributionEnabled());
	Root->SetNumberField(TEXT("SampleCount"), FrameworkSubstepTimingStats.SampleCount);
	Root->SetNumberField(TEXT("FrameSampleAppendPeakUs"), FrameworkSubstepTimingStats.FrameSampleAppendPeakUs);
	Root->SetNumberField(TEXT("BoardSampleCapturePeakUs"), FrameworkSubstepTimingStats.BoardSampleCapturePeakUs);
	Root->SetNumberField(TEXT("PruneSamplesPeakUs"), FrameworkSubstepTimingStats.PruneSamplesPeakUs);
	Root->SetNumberField(TEXT("SingleFrameHitchPeakUs"), FrameworkSubstepTimingStats.SingleFrameHitchPeakUs);
	Root->SetNumberField(TEXT("FramePacingDetectorPeakUs"), FrameworkSubstepTimingStats.FramePacingDetectorPeakUs);
	Root->SetNumberField(TEXT("MemoryGrowthDetectorPeakUs"), FrameworkSubstepTimingStats.MemoryGrowthDetectorPeakUs);
	Root->SetNumberField(TEXT("BasicHangDetectorPeakUs"), FrameworkSubstepTimingStats.BasicHangDetectorPeakUs);
	Root->SetNumberField(TEXT("PeriodicSnapshotPeakUs"), FrameworkSubstepTimingStats.PeriodicSnapshotPeakUs);
	Root->SetNumberField(TEXT("EventJsonBuildPeakUs"), FrameworkSubstepTimingStats.EventJsonBuildPeakUs);
	Root->SetNumberField(TEXT("EventJsonAppendPeakUs"), FrameworkSubstepTimingStats.EventJsonAppendPeakUs);
	Root->SetNumberField(TEXT("FrameworkTotalPeakUs"), FrameworkSubstepTimingStats.FrameworkTotalPeakUs);
	Root->SetNumberField(TEXT("InstrumentationProbePeakUs"), FrameworkSubstepTimingStats.InstrumentationProbePeakUs);
	Root->SetNumberField(TEXT("LastFrameSampleAppendUs"), FrameworkSubstepTimingStats.LastFrameSampleAppendUs);
	Root->SetNumberField(TEXT("LastBoardSampleCaptureUs"), FrameworkSubstepTimingStats.LastBoardSampleCaptureUs);
	Root->SetNumberField(TEXT("LastPruneSamplesUs"), FrameworkSubstepTimingStats.LastPruneSamplesUs);
	Root->SetNumberField(TEXT("LastSingleFrameHitchUs"), FrameworkSubstepTimingStats.LastSingleFrameHitchUs);
	Root->SetNumberField(TEXT("LastFramePacingDetectorUs"), FrameworkSubstepTimingStats.LastFramePacingDetectorUs);
	Root->SetNumberField(TEXT("LastMemoryGrowthDetectorUs"), FrameworkSubstepTimingStats.LastMemoryGrowthDetectorUs);
	Root->SetNumberField(TEXT("LastBasicHangDetectorUs"), FrameworkSubstepTimingStats.LastBasicHangDetectorUs);
	Root->SetNumberField(TEXT("LastPeriodicSnapshotUs"), FrameworkSubstepTimingStats.LastPeriodicSnapshotUs);
	Root->SetNumberField(TEXT("LastFrameworkTotalUs"), FrameworkSubstepTimingStats.LastFrameworkTotalUs);
	Root->SetNumberField(TEXT("LastInstrumentationProbeUs"), FrameworkSubstepTimingStats.LastInstrumentationProbeUs);
	return Root;
}

TSharedRef<FJsonObject> UT66PerformanceSubsystem::CreateWriteQueueStatsJson() const
{
	const FPerformanceWriteQueueStats Stats = WriteWorker ? WriteWorker->GetStats() : FPerformanceWriteQueueStats{};
	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("Capacity"), Stats.Capacity);
	Root->SetNumberField(TEXT("AttemptedWrites"), static_cast<double>(Stats.AttemptedWrites));
	Root->SetNumberField(TEXT("QueuedWrites"), static_cast<double>(Stats.QueuedWrites));
	Root->SetNumberField(TEXT("CompletedWrites"), static_cast<double>(Stats.CompletedWrites));
	Root->SetNumberField(TEXT("FailedWrites"), static_cast<double>(Stats.FailedWrites));
	Root->SetNumberField(TEXT("FallbackWrites"), static_cast<double>(Stats.FallbackWrites));
	Root->SetNumberField(TEXT("QueueFullFallbackWrites"), static_cast<double>(Stats.QueueFullFallbackWrites));
	Root->SetNumberField(TEXT("ClosingFallbackWrites"), static_cast<double>(Stats.ClosingFallbackWrites));
	Root->SetNumberField(TEXT("WorkerUnavailableFallbackWrites"), static_cast<double>(Stats.WorkerUnavailableFallbackWrites));
	Root->SetNumberField(TEXT("AbandonedWrites"), static_cast<double>(Stats.AbandonedWrites));
	Root->SetNumberField(TEXT("AppendWrites"), static_cast<double>(Stats.AppendWrites));
	Root->SetNumberField(TEXT("AtomicReplaceWrites"), static_cast<double>(Stats.AtomicReplaceWrites));
	Root->SetNumberField(TEXT("MaxQueueDepth"), static_cast<double>(Stats.MaxQueueDepth));
	Root->SetNumberField(TEXT("CurrentQueueDepth"), Stats.CurrentQueueDepth);
	Root->SetNumberField(TEXT("LastWorkerWriteUs"), Stats.LastWorkerWriteUs);
	Root->SetNumberField(TEXT("WorkerWritePeakUs"), Stats.WorkerWritePeakUs);
	Root->SetNumberField(TEXT("LastFallbackWriteUs"), Stats.LastFallbackWriteUs);
	Root->SetNumberField(TEXT("FallbackWritePeakUs"), Stats.FallbackWritePeakUs);
	Root->SetNumberField(TEXT("LastFlushWaitMs"), Stats.LastFlushWaitMs);
	Root->SetNumberField(TEXT("ShutdownFlushWaitMs"), Stats.ShutdownFlushWaitMs);
	Root->SetBoolField(TEXT("WorkerRunning"), Stats.bWorkerRunning);
	Root->SetBoolField(TEXT("Closing"), Stats.bClosing);
	Root->SetBoolField(
		TEXT("AccountingBalanced"),
		Stats.AttemptedWrites == Stats.CompletedWrites + Stats.FailedWrites + Stats.AbandonedWrites);
	return Root;
}

TArray<TSharedPtr<FJsonValue>> UT66PerformanceSubsystem::CreateRecentLogsJson(const int32 MaxLines) const
{
	TArray<TSharedPtr<FJsonValue>> Values;
	FScopeLock Lock(&LogLinesCriticalSection);
	const int32 StartIndex = (MaxLines > 0 && RecentLogLines.Num() > MaxLines)
		? RecentLogLines.Num() - MaxLines
		: 0;
	for (int32 Index = StartIndex; Index < RecentLogLines.Num(); ++Index)
	{
		Values.Add(MakeShared<FJsonValueString>(RecentLogLines[Index]));
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

UT66PerformanceSubsystem::FFrameSummary UT66PerformanceSubsystem::CalculateFrameSummary(const double WindowSeconds, const bool bIncludePercentiles) const
{
	FFrameSummary Summary;
	const double NowSeconds = FPlatformTime::Seconds();
	TArray<double> FrameTimesMs;
	if (bIncludePercentiles)
	{
		FrameTimesMs.Reserve(FrameSamples.Num());
	}
	double TotalFrameMs = 0.0;
	double LastFrameMs = 0.0;
	int32 SampleCount = 0;

	for (const FFrameSample& Sample : FrameSamples)
	{
		if (NowSeconds - Sample.TimeSeconds <= WindowSeconds)
		{
			if (bIncludePercentiles)
			{
				FrameTimesMs.Add(Sample.FrameTimeMs);
			}
			++SampleCount;
			TotalFrameMs += Sample.FrameTimeMs;
			LastFrameMs = Sample.FrameTimeMs;
		}
	}

	Summary.SampleCount = SampleCount;
	Summary.LastFrameMs = LastFrameMs;
	if (SampleCount == 0)
	{
		return Summary;
	}

	Summary.AverageFrameMs = TotalFrameMs / static_cast<double>(SampleCount);
	Summary.AverageFps = Summary.AverageFrameMs > 0.0 ? 1000.0 / Summary.AverageFrameMs : 0.0;

	double Variance = 0.0;
	for (const FFrameSample& Sample : FrameSamples)
	{
		if (NowSeconds - Sample.TimeSeconds > WindowSeconds)
		{
			continue;
		}
		const double FrameTimeMs = Sample.FrameTimeMs;
		const double Delta = FrameTimeMs - Summary.AverageFrameMs;
		Variance += Delta * Delta;
	}
	Summary.StdDevMs = FMath::Sqrt(Variance / static_cast<double>(SampleCount));

	if (!bIncludePercentiles)
	{
		return Summary;
	}

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
