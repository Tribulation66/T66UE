// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LagTrackerSubsystem.h"
#include "Algo/Sort.h"
#include "Containers/Ticker.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66LagTracker, Log, All);

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

static TAutoConsoleVariable<float> CVarLagTrackerRecordThresholdMs(
	TEXT("T66.LagTracker.RecordThresholdMs"),
	0.1f,
	TEXT("Operations exceeding this many ms are recorded into the in-memory perf session summary"),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarLagTrackerFrameHitchThresholdMs(
	TEXT("T66.LagTracker.FrameHitchThresholdMs"),
	25.0f,
	TEXT("Frames exceeding this many ms are logged as hitches"),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarLagTrackerRecentWindowMs(
	TEXT("T66.LagTracker.RecentWindowMs"),
	250.0f,
	TEXT("How far back to look for recent operations when a hitch occurs"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarLagTrackerSummaryTopN(
	TEXT("T66.LagTracker.SummaryTopN"),
	10,
	TEXT("How many top lag causes to print in summaries"),
	ECVF_Default);

namespace
{
	UT66LagTrackerSubsystem* GetLagTrackerSubsystem(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UT66LagTrackerSubsystem>();
		}

		return nullptr;
	}
}

static FAutoConsoleCommandWithWorldAndArgs T66LagTrackerDumpCommand(
	TEXT("T66.LagTracker.DumpSummary"),
	TEXT("Dump the in-memory lag tracker summary. Pass 'reset' to clear after dumping."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (UT66LagTrackerSubsystem* Lag = GetLagTrackerSubsystem(World))
		{
			const bool bResetAfterDump = Args.ContainsByPredicate([](const FString& Arg)
			{
				return Arg.Equals(TEXT("reset"), ESearchCase::IgnoreCase);
			});
			Lag->DumpSummary(bResetAfterDump);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs T66PerfDumpCommand(
	TEXT("T66.Perf.Dump"),
	TEXT("Alias for T66.LagTracker.DumpSummary"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (UT66LagTrackerSubsystem* Lag = GetLagTrackerSubsystem(World))
		{
			const bool bResetAfterDump = Args.ContainsByPredicate([](const FString& Arg)
			{
				return Arg.Equals(TEXT("reset"), ESearchCase::IgnoreCase);
			});
			Lag->DumpSummary(bResetAfterDump);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs T66LagTrackerResetCommand(
	TEXT("T66.LagTracker.Reset"),
	TEXT("Reset the in-memory lag tracker session"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		if (UT66LagTrackerSubsystem* Lag = GetLagTrackerSubsystem(World))
		{
			Lag->ResetSession();
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs T66PerfResetCommand(
	TEXT("T66.Perf.Reset"),
	TEXT("Alias for T66.LagTracker.Reset"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
	{
		if (UT66LagTrackerSubsystem* Lag = GetLagTrackerSubsystem(World))
		{
			Lag->ResetSession();
		}
	}));

void UT66LagTrackerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetSession();
	FrameTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UT66LagTrackerSubsystem::TickFrame));
}

void UT66LagTrackerSubsystem::Deinitialize()
{
	if (FrameTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FrameTickerHandle);
		FrameTickerHandle.Reset();
	}

	if (CauseStats.Num() > 0 || HitchCount > 0)
	{
		DumpSummary(false);
	}

	RecentOperations.Reset();
	CauseStats.Reset();

	Super::Deinitialize();
}

void UT66LagTrackerSubsystem::RecordOperation(const FString& Cause, float DurationMs)
{
	if (!IsEnabled() || Cause.IsEmpty())
	{
		return;
	}

	const float RecordThresholdMs = GetRecordThresholdMs();
	if (DurationMs < RecordThresholdMs)
	{
		return;
	}

	const double NowSeconds = FPlatformTime::Seconds();
	PruneRecentOperations(NowSeconds);

	FLagCauseStats& Stats = CauseStats.FindOrAdd(Cause);
	++Stats.Count;
	Stats.TotalMs += DurationMs;
	Stats.MaxMs = FMath::Max(Stats.MaxMs, DurationMs);
	Stats.LastSeenSeconds = NowSeconds;

	FRecentLagOperation& Recent = RecentOperations.AddDefaulted_GetRef();
	Recent.Cause = Cause;
	Recent.DurationMs = DurationMs;
	Recent.TimestampSeconds = NowSeconds;
	++TotalRecordedOperations;

	static constexpr int32 MaxRecentOperations = 512;
	if (RecentOperations.Num() > MaxRecentOperations)
	{
		RecentOperations.RemoveAt(0, RecentOperations.Num() - MaxRecentOperations, EAllowShrinking::No);
	}
}

void UT66LagTrackerSubsystem::ReportSlowOperation(const FString& Cause, float DurationMs)
{
	if (!IsEnabled() || DurationMs < GetThresholdMs()) return;
	++TotalLoggedSlowOperations;
	UE_LOG(LogT66LagTracker, Verbose, TEXT("[LAG] %s: %.2fms"), *Cause, DurationMs);
}

void UT66LagTrackerSubsystem::ResetSession()
{
	SessionStartSeconds = FPlatformTime::Seconds();
	LastFrameTimestampSeconds = SessionStartSeconds;
	TotalRecordedOperations = 0;
	TotalLoggedSlowOperations = 0;
	HitchCount = 0;
	CauseStats.Reset();
	RecentOperations.Reset();
}

void UT66LagTrackerSubsystem::DumpSummary(bool bResetAfterDump)
{
	const double NowSeconds = FPlatformTime::Seconds();
	const double SessionDurationSeconds = FMath::Max(0.0, NowSeconds - SessionStartSeconds);
	PruneRecentOperations(NowSeconds);

	UE_LOG(
		LogT66LagTracker,
		Verbose,
		TEXT("[PERF SUMMARY] Session=%.2fs RecordedOps=%d LoggedSlowOps=%d Hitches=%d UniqueCauses=%d"),
		SessionDurationSeconds,
		TotalRecordedOperations,
		TotalLoggedSlowOperations,
		HitchCount,
		CauseStats.Num());

	if (CauseStats.Num() == 0)
	{
		if (bResetAfterDump)
		{
			ResetSession();
		}
		return;
	}

	struct FSummaryRow
	{
		FString Cause;
		int32 Count = 0;
		double TotalMs = 0.0;
		float MaxMs = 0.f;
		double LastSeenSeconds = 0.0;
	};

	TArray<FSummaryRow> Rows;
	Rows.Reserve(CauseStats.Num());
	for (const TPair<FString, FLagCauseStats>& Pair : CauseStats)
	{
		FSummaryRow& Row = Rows.AddDefaulted_GetRef();
		Row.Cause = Pair.Key;
		Row.Count = Pair.Value.Count;
		Row.TotalMs = Pair.Value.TotalMs;
		Row.MaxMs = Pair.Value.MaxMs;
		Row.LastSeenSeconds = Pair.Value.LastSeenSeconds;
	}

	Algo::Sort(Rows, [](const FSummaryRow& A, const FSummaryRow& B)
	{
		if (A.TotalMs != B.TotalMs)
		{
			return A.TotalMs > B.TotalMs;
		}
		return A.MaxMs > B.MaxMs;
	});

	const int32 TopN = FMath::Clamp(CVarLagTrackerSummaryTopN.GetValueOnGameThread(), 1, 50);
	for (int32 Index = 0; Index < Rows.Num() && Index < TopN; ++Index)
	{
		const FSummaryRow& Row = Rows[Index];
		const double AvgMs = Row.Count > 0 ? (Row.TotalMs / static_cast<double>(Row.Count)) : 0.0;
		const double AgeSeconds = FMath::Max(0.0, NowSeconds - Row.LastSeenSeconds);
		UE_LOG(
			LogT66LagTracker,
			Verbose,
			TEXT("[PERF SUMMARY] #%d Cause=%s Count=%d Total=%.2fms Avg=%.3fms Max=%.2fms LastSeen=%.2fsAgo"),
			Index + 1,
			*Row.Cause,
			Row.Count,
			Row.TotalMs,
			AvgMs,
			Row.MaxMs,
			AgeSeconds);
	}

	if (bResetAfterDump)
	{
		ResetSession();
	}
}

bool UT66LagTrackerSubsystem::IsEnabled() const
{
	return CVarLagTrackerEnabled.GetValueOnGameThread() != 0;
}

float UT66LagTrackerSubsystem::GetThresholdMs() const
{
	return FMath::Max(0.1f, CVarLagTrackerThresholdMs.GetValueOnGameThread());
}

float UT66LagTrackerSubsystem::GetRecordThresholdMs() const
{
	return FMath::Max(0.01f, CVarLagTrackerRecordThresholdMs.GetValueOnGameThread());
}

float UT66LagTrackerSubsystem::GetFrameHitchThresholdMs() const
{
	return FMath::Max(1.0f, CVarLagTrackerFrameHitchThresholdMs.GetValueOnGameThread());
}

float UT66LagTrackerSubsystem::GetRecentWindowMs() const
{
	return FMath::Max(16.0f, CVarLagTrackerRecentWindowMs.GetValueOnGameThread());
}

bool UT66LagTrackerSubsystem::TickFrame(float DeltaSeconds)
{
	if (!IsEnabled())
	{
		LastFrameTimestampSeconds = FPlatformTime::Seconds();
		return true;
	}

	const double NowSeconds = FPlatformTime::Seconds();
	if (LastFrameTimestampSeconds > 0.0)
	{
		const double FrameMs = (NowSeconds - LastFrameTimestampSeconds) * 1000.0;
		if (FrameMs >= GetFrameHitchThresholdMs())
		{
			LogFrameHitch(FrameMs, NowSeconds);
		}
	}

	LastFrameTimestampSeconds = NowSeconds;
	return true;
}

void UT66LagTrackerSubsystem::PruneRecentOperations(double NowSeconds)
{
	const double WindowSeconds = static_cast<double>(GetRecentWindowMs()) / 1000.0;
	int32 RemoveCount = 0;
	while (RemoveCount < RecentOperations.Num()
		&& (NowSeconds - RecentOperations[RemoveCount].TimestampSeconds) > WindowSeconds)
	{
		++RemoveCount;
	}

	if (RemoveCount > 0)
	{
		RecentOperations.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
}

void UT66LagTrackerSubsystem::LogFrameHitch(double FrameMs, double NowSeconds)
{
	++HitchCount;
	PruneRecentOperations(NowSeconds);

	struct FRecentCauseSummary
	{
		int32 Count = 0;
		double TotalMs = 0.0;
		float MaxMs = 0.f;
	};

	TMap<FString, FRecentCauseSummary> RecentByCause;
	for (const FRecentLagOperation& Op : RecentOperations)
	{
		FRecentCauseSummary& Summary = RecentByCause.FindOrAdd(Op.Cause);
		++Summary.Count;
		Summary.TotalMs += Op.DurationMs;
		Summary.MaxMs = FMath::Max(Summary.MaxMs, Op.DurationMs);
	}

	FString RecentSummaryText = TEXT("RecentOps=<none>");
	if (RecentByCause.Num() > 0)
	{
		struct FRecentRow
		{
			FString Cause;
			int32 Count = 0;
			double TotalMs = 0.0;
			float MaxMs = 0.f;
		};

		TArray<FRecentRow> Rows;
		Rows.Reserve(RecentByCause.Num());
		for (const TPair<FString, FRecentCauseSummary>& Pair : RecentByCause)
		{
			FRecentRow& Row = Rows.AddDefaulted_GetRef();
			Row.Cause = Pair.Key;
			Row.Count = Pair.Value.Count;
			Row.TotalMs = Pair.Value.TotalMs;
			Row.MaxMs = Pair.Value.MaxMs;
		}

		Algo::Sort(Rows, [](const FRecentRow& A, const FRecentRow& B)
		{
			if (A.TotalMs != B.TotalMs)
			{
				return A.TotalMs > B.TotalMs;
			}
			return A.MaxMs > B.MaxMs;
		});

		const int32 TopN = FMath::Min(3, Rows.Num());
		TArray<FString> Parts;
		Parts.Reserve(TopN);
		for (int32 Index = 0; Index < TopN; ++Index)
		{
			const FRecentRow& Row = Rows[Index];
			Parts.Add(FString::Printf(
				TEXT("%s=%.2fms (%dx, max %.2fms)"),
				*Row.Cause,
				Row.TotalMs,
				Row.Count,
				Row.MaxMs));
		}

		RecentSummaryText = FString::Printf(TEXT("RecentOps=%s"), *FString::Join(Parts, TEXT(", ")));
	}

	UE_LOG(
		LogT66LagTracker,
		Verbose,
		TEXT("[HITCH] Frame=%.2fms %s"),
		FrameMs,
		*RecentSummaryText);
}
