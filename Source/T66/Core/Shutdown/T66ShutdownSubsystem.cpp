// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Shutdown/T66ShutdownSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Shutdown, Log, All);

#if !UE_BUILD_SHIPPING
namespace
{
	static void T66_RequestShutdownQuit(const TArray<FString>& Args, UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogT66Shutdown, Warning, TEXT("[Shutdown] T66.Shutdown.RequestQuit failed: no world."));
			return;
		}

		UGameInstance* GameInstance = World->GetGameInstance();
		UT66ShutdownSubsystem* Shutdown = GameInstance ? GameInstance->GetSubsystem<UT66ShutdownSubsystem>() : nullptr;
		if (!Shutdown)
		{
			UE_LOG(LogT66Shutdown, Warning, TEXT("[Shutdown] T66.Shutdown.RequestQuit failed: no shutdown subsystem."));
			return;
		}

		int32 ExitCode = 0;
		if (Args.Num() > 0)
		{
			LexTryParseString(ExitCode, *Args[0]);
		}

		Shutdown->RequestQuitGame(ET66ShutdownReason::UserQuit, ExitCode);
	}

	static FAutoConsoleCommandWithWorldAndArgs T66ShutdownRequestQuitCommand(
		TEXT("T66.Shutdown.RequestQuit"),
		TEXT("Development automation: route quit through UT66ShutdownSubsystem::RequestQuitGame. Optional first arg is exit code."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_RequestShutdownQuit));
}
#endif

const TCHAR* UT66ShutdownSubsystem::LexToString(const ET66ShutdownReason Reason)
{
	switch (Reason)
	{
	case ET66ShutdownReason::UserQuit:
		return TEXT("UserQuit");
	case ET66ShutdownReason::EnginePreExit:
		return TEXT("EnginePreExit");
	case ET66ShutdownReason::TestHarness:
		return TEXT("TestHarness");
	case ET66ShutdownReason::Unknown:
	default:
		return TEXT("Unknown");
	}
}

const TCHAR* UT66ShutdownSubsystem::LexToString(const ET66ShutdownPhase Phase)
{
	switch (Phase)
	{
	case ET66ShutdownPhase::InputLock:
		return TEXT("InputLock");
	case ET66ShutdownPhase::NativeExternal:
		return TEXT("NativeExternal");
	case ET66ShutdownPhase::NetworkPlatform:
		return TEXT("NetworkPlatform");
	case ET66ShutdownPhase::DurableState:
		return TEXT("DurableState");
	case ET66ShutdownPhase::AsyncWork:
		return TEXT("AsyncWork");
	case ET66ShutdownPhase::RuntimeTick:
		return TEXT("RuntimeTick");
	case ET66ShutdownPhase::MediaAudio:
		return TEXT("MediaAudio");
	case ET66ShutdownPhase::GameplayWorld:
		return TEXT("GameplayWorld");
	case ET66ShutdownPhase::FinalReport:
		return TEXT("FinalReport");
	default:
		return TEXT("UnknownPhase");
	}
}

int32 UT66ShutdownSubsystem::GetPhaseSortKey(const ET66ShutdownPhase Phase)
{
	return static_cast<int32>(Phase);
}

void UT66ShutdownSubsystem::Deinitialize()
{
	Participants.Reset();
	Super::Deinitialize();
}

FT66ShutdownParticipantHandle UT66ShutdownSubsystem::RegisterParticipant(
	UObject* Owner,
	const FName ParticipantName,
	const ET66ShutdownPhase Phase,
	const int32 Priority,
	const double TimeoutSeconds,
	const bool bRequired,
	FT66ShutdownParticipantDelegate Delegate)
{
	FT66ShutdownParticipantHandle InvalidHandle;

	if (!Owner || ParticipantName.IsNone() || !Delegate.IsBound())
	{
		UE_LOG(LogT66Shutdown, Warning, TEXT("[Shutdown] Refused invalid participant registration Name=%s Owner=%s Bound=%d"),
			*ParticipantName.ToString(),
			*GetNameSafe(Owner),
			Delegate.IsBound() ? 1 : 0);
		return InvalidHandle;
	}

	if (bShutdownInProgress || bShutdownCompleted)
	{
		UE_LOG(LogT66Shutdown, Warning, TEXT("[Shutdown] Refused late participant registration Name=%s Owner=%s"),
			*ParticipantName.ToString(),
			*GetNameSafe(Owner));
		return InvalidHandle;
	}

	FParticipant Participant;
	Participant.Handle.Id = NextParticipantId++;
	Participant.Owner = Owner;
	Participant.Name = ParticipantName;
	Participant.Phase = Phase;
	Participant.Priority = Priority;
	Participant.TimeoutSeconds = FMath::Max(0.0, TimeoutSeconds);
	Participant.bRequired = bRequired;
	Participant.Delegate = MoveTemp(Delegate);

	Participants.Add(MoveTemp(Participant));

	UE_LOG(LogT66Shutdown, Verbose, TEXT("[Shutdown] Registered Name=%s Phase=%s Priority=%d Required=%d Timeout=%.2fs Owner=%s"),
		*ParticipantName.ToString(),
		LexToString(Phase),
		Priority,
		bRequired ? 1 : 0,
		TimeoutSeconds,
		*GetNameSafe(Owner));

	FT66ShutdownParticipantHandle Handle;
	Handle.Id = NextParticipantId - 1;
	return Handle;
}

void UT66ShutdownSubsystem::UnregisterParticipant(const FT66ShutdownParticipantHandle Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	Participants.RemoveAllSwap([Handle](const FParticipant& Participant)
	{
		return Participant.Handle.Id == Handle.Id;
	});
}

void UT66ShutdownSubsystem::RequestQuitGame(const ET66ShutdownReason Reason, const int32 ExitCode)
{
	RunShutdown(Reason, true, ExitCode, TEXT("T66ShutdownSystem"));
}

bool UT66ShutdownSubsystem::RunShutdown(
	const ET66ShutdownReason Reason,
	const bool bRequestEngineExit,
	const int32 ExitCode,
	const TCHAR* ExitTag)
{
	if (bShutdownInProgress)
	{
		UE_LOG(LogT66Shutdown, Warning, TEXT("[Shutdown] Request ignored while shutdown is already in progress. Reason=%s"),
			LexToString(Reason));
		return false;
	}

	if (bShutdownCompleted)
	{
		UE_LOG(LogT66Shutdown, Log, TEXT("[Shutdown] Request received after shutdown already completed. Reason=%s RequestExit=%d"),
			LexToString(Reason),
			bRequestEngineExit ? 1 : 0);
		if (bRequestEngineExit && !bEngineExitRequested)
		{
			bEngineExitRequested = true;
			FPlatformMisc::RequestExitWithStatus(false, ExitCode, ExitTag ? ExitTag : TEXT("T66ShutdownSystem"));
		}
		return true;
	}

	bShutdownInProgress = true;
	LastShutdownStartedSeconds = FPlatformTime::Seconds();

	TArray<FParticipant> OrderedParticipants = Participants;
	OrderedParticipants.Sort([](const FParticipant& Left, const FParticipant& Right)
	{
		const int32 LeftPhase = GetPhaseSortKey(Left.Phase);
		const int32 RightPhase = GetPhaseSortKey(Right.Phase);
		if (LeftPhase != RightPhase)
		{
			return LeftPhase < RightPhase;
		}
		if (Left.Priority != Right.Priority)
		{
			return Left.Priority < Right.Priority;
		}
		return Left.Handle.Id < Right.Handle.Id;
	});

	UE_LOG(LogT66Shutdown, Log, TEXT("[Shutdown] Begin Reason=%s Participants=%d RequestExit=%d"),
		LexToString(Reason),
		OrderedParticipants.Num(),
		bRequestEngineExit ? 1 : 0);

	bool bAllRequiredSucceeded = true;
	int32 RanCount = 0;
	int32 SkippedCount = 0;
	int32 FailedCount = 0;

	for (FParticipant& Participant : OrderedParticipants)
	{
		if (!Participant.Owner.IsValid())
		{
			++SkippedCount;
			UE_LOG(LogT66Shutdown, Verbose, TEXT("[Shutdown] Skip invalid owner Name=%s Phase=%s"),
				*Participant.Name.ToString(),
				LexToString(Participant.Phase));
			continue;
		}

		FT66ShutdownContext Context;
		Context.Reason = Reason;
		Context.Phase = Participant.Phase;
		Context.ParticipantName = Participant.Name;
		Context.ShutdownStartedSeconds = LastShutdownStartedSeconds;
		Context.bWillRequestEngineExit = bRequestEngineExit;

		const double ParticipantStart = FPlatformTime::Seconds();
		UE_LOG(LogT66Shutdown, Log, TEXT("[Shutdown] ParticipantStart Phase=%s Name=%s Required=%d Timeout=%.2fs"),
			LexToString(Participant.Phase),
			*Participant.Name.ToString(),
			Participant.bRequired ? 1 : 0,
			Participant.TimeoutSeconds);

		bool bParticipantOk = false;
		if (Participant.Delegate.IsBound())
		{
			bParticipantOk = Participant.Delegate.Execute(Context);
		}

		const double ElapsedSeconds = FPlatformTime::Seconds() - ParticipantStart;
		const bool bSlow = Participant.TimeoutSeconds > 0.0 && ElapsedSeconds > Participant.TimeoutSeconds;

		if (!bParticipantOk)
		{
			++FailedCount;
			if (Participant.bRequired)
			{
				bAllRequiredSucceeded = false;
				UE_LOG(LogT66Shutdown, Error, TEXT("[Shutdown] ParticipantFailed Phase=%s Name=%s Elapsed=%.3fs Required=1"),
					LexToString(Participant.Phase),
					*Participant.Name.ToString(),
					ElapsedSeconds);
			}
			else
			{
				UE_LOG(LogT66Shutdown, Warning, TEXT("[Shutdown] ParticipantFailed Phase=%s Name=%s Elapsed=%.3fs Required=0"),
					LexToString(Participant.Phase),
					*Participant.Name.ToString(),
					ElapsedSeconds);
			}
		}
		else if (bSlow)
		{
			UE_LOG(LogT66Shutdown, Warning, TEXT("[Shutdown] ParticipantSlow Phase=%s Name=%s Elapsed=%.3fs Budget=%.3fs"),
				LexToString(Participant.Phase),
				*Participant.Name.ToString(),
				ElapsedSeconds,
				Participant.TimeoutSeconds);
		}
		else
		{
			UE_LOG(LogT66Shutdown, Log, TEXT("[Shutdown] ParticipantDone Phase=%s Name=%s Elapsed=%.3fs"),
				LexToString(Participant.Phase),
				*Participant.Name.ToString(),
				ElapsedSeconds);
		}

		++RanCount;
	}

	const double TotalElapsedSeconds = FPlatformTime::Seconds() - LastShutdownStartedSeconds;
	bShutdownCompleted = true;
	bShutdownInProgress = false;

	UE_LOG(LogT66Shutdown, Log, TEXT("[Shutdown] Complete Reason=%s Ran=%d Skipped=%d Failed=%d RequiredOk=%d Elapsed=%.3fs"),
		LexToString(Reason),
		RanCount,
		SkippedCount,
		FailedCount,
		bAllRequiredSucceeded ? 1 : 0,
		TotalElapsedSeconds);

	if (bRequestEngineExit && !bEngineExitRequested)
	{
		bEngineExitRequested = true;
		UE_LOG(LogT66Shutdown, Log, TEXT("[Shutdown] RequestExit ExitCode=%d Tag=%s"),
			ExitCode,
			ExitTag ? ExitTag : TEXT("T66ShutdownSystem"));
		FPlatformMisc::RequestExitWithStatus(false, ExitCode, ExitTag ? ExitTag : TEXT("T66ShutdownSystem"));
	}

	return bAllRequiredSucceeded;
}
